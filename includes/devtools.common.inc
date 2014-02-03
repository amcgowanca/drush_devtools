<?php
/**
 * @file
 */

/**
 * Logs an error using drush_set_error.
 *
 * @param string $error_code
 *   The error code (e.g. DEVTOOLS_PATCH_FILE_ERROR).
 * @param string $message
 *   The error message.
 */
function devtools_error($error_code, $message) {
  drush_set_error("$error_code: $message");
}

/**
 * Wrapper for drush_download_file().
 *
 * @param string $download_url
 *   The url of the file to download.
 * @param int $cache_duration
 *   The time in seconds to to cache the downloaded file.
 *
 * @return string
 *   The location of the download file if successful, otherwise FALSE.
 */
function devtools_download_file($download_url, $cache_duration = DRUSH_CACHE_LIFETIME_DEFAULT) {
  if (drush_get_option('no-cache', FALSE)) {
    $cache_duration = 0;
  }

  $temp_path = devtools_tempdir();
  $file = basename(current(explode('?', $download_url, 2)));
  return drush_download_file($download_url, $temp_path . '/' . $file, $cache_duration);
}

/**
 * Find and possibly create a temporary directory.
 *
 * @param boolean $set
 *   Must be TRUE to create a directory.
 * @param string $directory
 *   Pass in a directory to use. This is required if using concurrent operations.
 *
 * @return string
 *   The temporary directory.
 */
function devtools_tempdir($set = TRUE, $directory = NULL) {
  static $temp_directory;
  if (isset($directory) && !isset($temp_directory)) {
    $temp_directory = $directory;
  }

  if (!isset($temp_directory) && $set) {
    $temp_directory = drush_find_tmp();
    if (strrpos($temp_directory, '/') == strlen($temp_directory) - 1) {
      $temp_directory .= 'devtools_tmp_' . time() . '_' . uniqid();
    }
    else {
      $temp_directory .= '/devtools_tmp_' . time() . '_' . uniqid();
    }
    drush_register_file_for_deletion($temp_directory);
    if (file_exists($temp_directory)) {
      return devtools_tempdir(TRUE);
    }
    drush_mkdir($temp_directory);
  }
  return $temp_directory;
}
