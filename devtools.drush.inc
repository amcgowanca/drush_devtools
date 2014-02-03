<?php
/**
 * @file
 * Drush DevTools commands.
 */

/**
 * Implements hook_drush_command().
 */
function devtools_drush_command() {
  $commands = array();
  $commands['devtools-patch'] = array(
    'description' => 'Applies a single patch file.',
    'arguments' => array(
      'patchfile' => 'The patch file to patch the project with.',
    ),
    'required arguments' => 1,
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
 * Patches a single project with the specified patch file.
 */
function drush_devtools_patch($patchfile) {

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
