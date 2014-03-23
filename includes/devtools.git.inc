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
 * @param $directory
 * @return bool
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
          if (!drush_delete_dir($directory . '/' . $file, TRUE)) {
            drush_log(dt('Cannot perform cleanup on potential Git directory: @file.', array('@file' => $directory . '/' . $file)), 'warning');
          }
        }
      }
      elseif (devtools_file_is_git_file($file)) {
        @chmod($directory . '/' . $file, 0777);
        unlink($directory . '/' . $file);
      }
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * @param $directory
 * @return bool
 */
function devtools_directory_is_git_directory($directory) {
  return in_array($directory, array('.git'));
}

/**
 * @param $file
 * @return bool
 */
function devtools_file_is_git_file($file) {
  return in_array($file, array('.gitignore', '.gitmodules'));
}
