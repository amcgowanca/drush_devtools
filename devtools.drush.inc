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
 * Implements hook_drush_command().
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
 */
function drush_devtools_patch($patchfile, $project = NULL) {
  $original_patchfile = $patchfile;
  if (devtools_is_valid_url($patchfile, TRUE)) {
    $patchfile = devtools_download_file($patchfile);
  }
  else {
    if(!file_exists($patchfile)) {
      devtools_error('DEVTOOLS_PATCH_FILE_ERROR', dt('Specified patch file "@patchfile" missing.', array(
        '@patchfile' => $patchfile,
      )));
      return;
    }
  }

  if (!empty($project)) {
    $projects = devtools_find_project_dirs_by_filescan($project);
    if (empty($projects)) {
      drush_set_error(dt('No project found with name "@project".', array(
        '@project' => $project,
      )));
      return;
    }
  }
  else {
    $projects[] = drush_cwd();
  }

  foreach ($projects as $project_dir) {
    $rel_project_dir = str_replace(drush_locate_root() . '/', '', $project_dir);
    if (!drush_confirm(dt('Apply patch to this directory: @project_dir?', array('@project_dir' => $rel_project_dir)))) {
      continue;
    }

    $patched = devtools_patch($patchfile, $project_dir);
    if ($output = drush_shell_exec_output()) {
      drush_log(implode("\n", $output));
    }

    if ($patched) {
      drush_log(dt('Successfully applied patch @patchfile to @project.', array(
        '@project' => $rel_project_dir,
        '@patchfile' => $original_patchfile,
      )), 'ok');
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
