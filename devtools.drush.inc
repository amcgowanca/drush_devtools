<?php
/**
 * This file is part of Drush DevTools.
 *
 * (c) Aaron McGowan (www.amcgowan.ca) <me@amcgowan.ca>
 *
 * This source file is subject to the GPL version 2 license that is
 * bundled with this source code in the file LICENSE.md.
 *
 * Drupal is a registered trademark of Dries Buytaert (www.buytaert.com).
 */

include_once 'includes/devtools.common.inc';
include_once 'includes/devtools.filesystem.inc';
include_once 'includes/devtools.patch.inc';
include_once 'includes/devtools.utilities.inc';

/**
 * Denotes the current major.minor version.
 */
define('DEVTOOLS_VERSION', '0.1');

/**
 * Implements hook_drush_command().
 */
function devtools_drush_command() {
  $commands = array();
  $commands['devtools-version'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => 'Displays the current version of DevTools.',
  );
  $commands['devtools-flush'] = array(
    'description' => 'Flushes all system data and rebuilds.',
    'aliases' => array('flush'),
  );
  $commands['devtools-download-file'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => 'Download a single file from a specified URL to a directory.',
    'arguments' => array(
      'file' => 'The URL of the file to download.',
      'directory' => 'The destination directory in which to download the file too.',
    ),
    'required arguments' => 1,
    'aliases' => array('dl-file', 'dlf'),
  );
  $commands['devtools-patch'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => 'Applies a single patch file.',
    'arguments' => array(
      'patchfile' => 'The patch file to patch the project with.',
      'project' => 'The name of the project to patch.',
    ),
    'required arguments' => 1,
    'options' => array(
      'no-patch-txt' => 'Do not write a PATCHES.txt file in the directory of the patched project.',
    ),
    'aliases' => array('patch'),
  );
  $commands['devtools-patch-revert'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => 'Reverts a one or more patch files for a specified Drupal project.',
    'arguments' => array(
      'project' => 'The name of the project to revert patch files on.',
      'patchfile' => 'The patch file to revert on a single project.',
    ),
    'aliases' => array('patch-revert', 'rpatch'),
  );
  return $commands;
}

/**
 * Implements hook_drush_help().
 */
function devtools_drush_help($section) {
  switch ($section) {
    case 'meta:devtools:title':
      return dt('Developer Tools');
    case 'meta:devtools:summary':
      return dt('A collection of common developer oriented tools.');
  }
}

/**
 * Displays the current version of DevTools.
 */
function drush_devtools_version() {
  drush_print(dt('drush devtools version @version.', array(
    '@version' => DEVTOOLS_VERSION,
  )));
}

/**
 * Performs a complete flush.
 */
function drush_devtools_flush() {
  drupal_static_reset('system_rebuild_module_data');
  drupal_static_reset('system_rebuild_theme_data');
  system_rebuild_module_data();
  system_rebuild_theme_data();
  drupal_flush_all_caches();
  drush_print(dt('The system project (modules, themes) data have been rebuilt and all caches flushed.'));
}

/**
 * Downloads a single file from a remote URL to a specified directory.
 *
 * @param string $url
 *   The URL of the file to download.
 * @param string $directory
 *   The directory in which the file should be downloaded too.
 */
function drush_devtools_download_file($url, $directory = '.') {
  // Ensure that the specified `$url` is valid prior to continuing.
  if (devtools_is_valid_url($url)) {
    // If the URL is valid, proceed with download and store the file
    // within the temporary directory prior to moving.
    $file = devtools_download_file($url);
  }
  else {
    return drush_error(dt('Specified file URL is not valid.'), 'error');
  }

  // Check and validate the specified `$directory`, ensure it is a directory
  // and is writable prior to attempting to move. Should any errors be encountered,
  // return with a drush_log() warning the user of the error.
  $destination = '';
  if (!empty($directory) && '.' !== $directory) {
    if (file_exists($directory)) {
      if (is_dir($directory)) {
        if (is_writable($directory)) {
          $destination = realpath($directory);
        }
        else {
          return drush_log(dt('Destination directory, "@directory", exists but is not writable.', array('@directory' => $directory)), 'error');
        }
      }
      else {
        return drush_log(dt('Destination directory, "@directory", exists but does not appear to be a directory.', array('@directory' => $directory)), 'error');
      }
    }
    else {
      if (!drush_confirm(dt('The specified directory, "@directory", does not exist. Do you want to create it?', array('@directory' => $directory)))) {
        return;
      }
      if (!drush_mkdir($directory)) {
        return drush_log(dt('Destination directory, "@directory", could not be created.', array('@directory' => $directory)));
      }
      $destination = realpath($directory);
    }
  }
  else {
    // Should no directory be specified or if '.' is given, use the
    // current working directory for the download location.
    $destination = drush_cwd();
  }

  $filename = basename($file);
  $destination = $destination . '/' . $filename;

  // Attempt to copy the downloaded file from the temporary file location
  // to the destination directory, log an error should one occur.
  if (!drush_copy_dir($file, $destination)) {
    return drush_log(dt('The file failed to download and write to @directory as the file already exists.', array('@directory' => $destination)), 'error');
  }

  drush_log(dt('Successfully downloaded file @filename.', array('@filename' => $filename)), 'ok');
}

/**
 * Patches a single project with the specified patch file.
 *
 * @param string $patchfile
 *   The file path or URL to a valid patch file to use.
 * @param string $project
 *   The project to patch. If NULL, current working directory is patched.
 */
function drush_devtools_patch($patchfile, $project = NULL) {
  $original_patchfile = $patchfile;
  $patchfile = _devtools_patch_get_patchfile($patchfile);

  if (!empty($project) && '.' !== $project) {
    // Attempt to retrieve projects within the filesystem. Note: this is
    // not the same as querying the Drupal {system} table as this will provide
    // all instances of a project (e.g. views) instead of only that which Drupal
    // is currently making use of.
    $projects = devtools_find_project_dirs_by_filescan($project);
    // If no projects are found, we will just exit and prompt an error.
    if (empty($projects)) {
      drush_set_error(dt('No project found with name "@project".', array(
        '@project' => $project,
      )));
      return;
    }
  }
  else {
    // If $project is empty; let's assume that the user wants to
    // apply the specified $patchfile to the current working directory.
    $projects = array(drush_cwd());
  }

  foreach ($projects as $project_dir) {
    $rel_project_dir = str_replace(drush_locate_root() . '/', '', $project_dir);
    // Ask the user whether he or she wants to apply the patch to this directory
    // prior to continuing. If $project == null, this will prompt for current working
    // directory instead of a project.
    if (!drush_confirm(dt('Apply patch to this directory: @project_dir?', array('@project_dir' => $rel_project_dir)))) {
      continue;
    }

    // Attempt to patch the project directory and retrieve the output
    if (devtools_patch($patchfile, $project_dir)) {
      drush_log(dt('Successfully applied patch @patchfile to @project.', array(
        '@project' => $rel_project_dir,
        '@patchfile' => $original_patchfile,
      )), 'ok');

      // If a PATCHES.txt file is to be written, by default this is the case
      // unless the `no-patch-txt` option is specified - create the information of patches.
      if (!drush_get_option('no-patch-txt')) {
        $patches_txt = "The following patches have been applied to this project:\n\n";
        $patches_txt .= '- ' . $original_patchfile . "\n";
        if (file_exists($project_dir . '/PATCHES.txt')) {
          $existing_patches = _devtools_patch_parse_patches_text($project_dir . '/PATCHES.txt');
          if (!empty($existing_patches)) {
            foreach (array_reverse($existing_patches) as $existing_patch) {
              $patches_txt = '- ' . $existing_patch . "\n" . $patches_txt;
            }
          }
        }
        file_put_contents($project_dir . '/PATCHES.txt', $patches_txt);
        drush_log('Generated PATCHES.txt file for path ' . $rel_project_dir, 'ok');
      }
    }
    else {
      drush_log(dt('Unable to apply patch @patchfile to @project.', array(
        '@project' => $rel_project_dir,
        '@patchfile' => $original_patchfile,
      )), 'error');
    }
  }
}

/**
 * Reverts a one or more patches for a specified project.
 *
 * If no patchfile is specified and the current working directory of the project
 * in which the patch is to be reverted, we will attempt to find drush make's
 * 'PATCHES.txt' file and revert all patches upon confirmation.
 *
 * @param string $project
 *   The name of the project to revert the patch on.
 * @param string $patchfile
 *   The patch file to revert.
 */
function drush_devtools_patch_revert($project = NULL, $patchfile = NULL) {
  // Attempt to retrieve projects within the filesystem based on the specified
  // project name. Similarly to drush_devtools_patch, querying {system} table
  // will only provide the project that is currently active by Drupal.
  if (!empty($project) && '.' !== $project) {
    $projects = devtools_find_project_dirs_by_filescan($project);
    if (empty($projects)) {
      drush_set_error(dt('No project found with name "@project".', array(
        '@project' => $project,
      )));
    }
  }
  else {
    // If no project is specified, use the current working directory.
    $projects = array(drush_cwd());
  }

  $patchfiles = array();
  $patches_txt = TRUE;

  // If the patchfile is specified, retrieve it and append it as the patch
  // file to revert on the project directory.
  if (!empty($patchfile)) {
    $patchfiles[] = _devtools_patch_get_patchfile($patchfile);
    $patches_txt = FALSE;
  }

  foreach ($projects as $project_dir) {
    $rel_project_dir = str_replace(drush_locate_root() . '/', '', $project_dir);
    // If `$patches_txt` boolean flag is TRUE, therefore no patchfile specified
    // attempt to parse the PATCHES.txt file to retrieve patch files that shall be
    // reverted from the project directory.
    if ($patches_txt && file_exists($project_dir . '/PATCHES.txt')) {
      $patchfiles = _devtools_patch_parse_patches_text($project_dir . '/PATCHES.txt');
    }

    if (!empty($patchfiles)) {
      foreach ($patchfiles as $patch) {
        // Confirm with the user that he or she wants to revert the current patch on the
        // current project. If no, continue to the next patch; should one exist.
        if (!drush_confirm(dt('Are you sure you want to revert patch, "@patchfile", on this directory: @project_dir?', array('@project_dir' => $rel_project_dir, '@patchfile' => $patch)))) {
          continue;
        }

        // Retrieve the patch file and attempt to revert it.
        $patchfile = _devtools_patch_get_patchfile($patch);
        if (devtools_patch_revert($patchfile, $project_dir)) {
          drush_log(dt('Successfully reverted patch @patchfile to @project.', array(
            '@patchfile' => $patchfile,
            '@project' => $rel_project_dir,
          )), 'ok');
        }
        else {
          drush_log(dt('Unable to revert patch @patchfile on @project.', array(
            '@patchfile' => $patchfile,
            '@project' => $rel_project_dir,
          )), 'error');
        }
      }
    }
  }
}
