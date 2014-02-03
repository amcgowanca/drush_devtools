<?php
/**
 * @file
 */

function devtools_find_project_dirs_by_filescan($name) {
  static $projects = NULL;
  if (NULL === $projects) {
    $drupal_root = drush_locate_root(drush_cwd());
    if (!$drupal_root) {
      devtools_error('DEVTOOLS_LOCATE_ROOT', 'Unable to locate Drupal root from current working directory.');
      return NULL;
    }

    $projects = array();
    $listings = drush_scan_directory($drupal_root, "/$project\.module$/");
    foreach ($listings as $listing) {
      if (!isset($projects[$listing->name])) {
        $projects[$listing->name] = array();
      }
      $projects[$listing->name][] = dirname($listing->filename);
    }
  }
  return isset($projects[$name]) ? $projects[$name] : NULL;
}
