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

/**
 * Returns an array of file paths to valid projects with the name $name.
 *
 * @param string $name
 *   The name of the project.
 * @return array
 *   Returns an array of file paths if $name exists; otherwise NULL.
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

function devtools_trim_path($path, $ltrim = FALSE, $os = NULL) {
  if (drush_is_windows($os)) {
    return $ltrim ? ltrim(rtrim($path, '/\\'), '/\\') : rtrim($path, '/\\');
  }
  return $ltrim ? ltrim(rtrim($path, '/'), '/') : rtrim($path, '/');
}
