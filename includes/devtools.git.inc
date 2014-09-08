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
 * Recursively removes Git directories and files within the specified directory.
 *
 * @param string $directory
 *   The directory to recursively clean.
 *
 * @return bool
 *   Returns TRUE if cleaning was successful, otherwise FALSE.
 */
function devtools_git_clean($directory) {
  if (is_dir($directory) && $handle = opendir($directory)) {
    while (FALSE !== ($file = readdir($handle))) {
      if (in_array($file, array('.', '..'))) {
        continue;
      }

      if (is_dir($directory . '/' . $file)) {
        if (!devtools_directory_is_git_directory($file)) {
          if (!devtools_git_clean($directory . '/' . $file)) {
            drush_log(dt('Cannot perform recursive Git cleanup on: @file.', array('@file' => $directory . '/' . $file)), 'warning');
          }
        }
        else {
          if (drush_delete_dir($directory . '/' . $file, TRUE)) {
            if (drush_get_option('verbose', FALSE)) {
              drush_log(dt('Deleted directory: @file', array('@file' => $directory . '/' . $file)), 'ok');
            }
          }
          else {
            drush_log(dt('Cannot perform cleanup on potential Git directory: @file.', array('@file' => $directory . '/' . $file)), 'warning');
          }
        }
      }
      elseif (devtools_file_is_git_file($file)) {
        @chmod($directory . '/' . $file, 0777);
        unlink($directory . '/' . $file);
        if (drush_get_option('verbose', FALSE)) {
          drush_log(dt('Deleted file: @file', array('@file' => $directory . '/' . $file)), 'ok');
        }
      }
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * Tests if the specified directory is a Git directory.
 *
 * @param string $directory
 *   The directory to test if it is a Git directory.
 *
 * @return bool
 *   Returns TRUE if $directory is a Git directory, otherwise FALSE.
 */
function devtools_directory_is_git_directory($directory) {
  return in_array($directory, array('.git'));
}

/**
 * Tests if the specified file is a Git specific file (e.g. `.gitignore`).
 *
 * @param string $file
 *   The file to test if it is a Git file.
 *
 * @return bool
 *   Returns TRUE if the $file is a Git file, otherwise FALSE.
 */
function devtools_file_is_git_file($file) {
  return in_array($file, array('.git', '.gitignore', '.gitmodules'));
}
