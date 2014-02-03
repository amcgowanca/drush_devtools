<?php
/**
 * @file
 * Drush DevTools commands.
 */

include_once 'includes/devtools.common.inc';
include_once 'includes/devtools.filesystem.inc';
include_once 'includes/devtools.patch.inc';
include_once 'includes/devtools.utilities.inc';

/**
 * Implements hook_drush_command().g
 */
function devtools_drush_command() {
  $commands = array();
  $commands['devtools-flush'] = array(
    'description' => 'Flushes all system data and rebuilds.',
    'aliases' => array('flush'),
  );
  $commands['devtools-patch'] = array(
    'description' => 'Applies a single patch file.',
    'arguments' => array(
      'patchfile' => 'The patch file to patch the project with.',
      'project' => 'The project to patch.',
    ),
    'required arguments' => 1,
    'options' => array(
      'no-patch-txt' => 'Do not write a PATCHES.txt file in the directory of the patched project.',
    ),
    'aliases' => array('patch'),
  );
  $commands['devtools-patch-revert'] = array(
    'description' => 'Reverts a one or more patch files for a specified Drupal project.',
    'arguments' => array(
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

  if (!empty($project)) {
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
    $projects[] = drush_cwd();
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
      // unless the `no-patch-txt` option is specified - create or append the information
      // of patches.
      if (!drush_get_option('no-patch-txt')) {
        $patches_txt = '- ' . $original_patchfile . "\n";
        $flags = 0;
        if (!file_exists($project_dir . '/PATCHES.txt')) {
          $patches_txt = 'The following patches have been applied to this project:\n' . $patches_txt;
        }
        else {
          $flags = FILE_APPEND;
        }
        file_put_contents($project_dir . '/PATCHES.txt', $patches_txt, $flags);
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
 */
function drush_devtools_patch_revert($patchfile = NULL) {

}
