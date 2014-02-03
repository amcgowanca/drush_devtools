<?php
/**
 * @file
 */

/**
 * Returns an array of valid patch levels.
 *
 * @return array
 *   Array of patch levels (e.g. -p1).
 */
function devtools_patch_levels() {
  return array('-p1', '-p0');
}

/**
 * Applies the specified $patchfile to the $filepath.
 *
 * @param string $patchfile
 *   The local path to the patch file to apply.
 * @param string $filepath
 *   The file path to the directory to apply the patch.
 * @param boolean $log
 *   A boolean indicating whether the shell output should be logged.
 *
 * @return bool
 *   Returns TRUE if successful, otherwise FALSE.
 */
function devtools_patch($patchfile, $filepath, $log = TRUE) {
  $patched = FALSE;
  foreach (devtools_patch_levels() as $level) {
    $checked = drush_shell_exec('cd %s && GIT_DIR=. git apply --check %s %s --verbose', $filepath, $level, $patchfile);
    if ($checked) {
      drush_shell_exec('cd %s && GIT_DIR=. git apply %s %s --verbose', $filepath, $level, $patchfile);
      $patched = TRUE;
      break;
    }
  }

  foreach (devtools_patch_levels() as $level) {
    if (drush_shell_exec('patch %s --no-backup-if-mismatch -d %s < %s', $level, $filepath, $patchfile)) {
      $patched = TRUE;
      break;
    }
  }

  if ($log && $output = drush_shell_exec_output()) {
    drush_log(implode("\n", $output));
  }

  return $patched;
}

/**
 * Helper method for retrieving the patch file.
 *
 * @param string $patchfile
 *   The file or URL of the patch file.
 *
 * @return string
 *   The file path to the patch file on the local system.
 */
function _devtools_patch_get_patchfile($patchfile) {
  if (devtools_is_valid_url($patchfile, TRUE)) {
    $patchfile = devtools_download_file($patchfile);
  }
  else {
    if(!file_exists($patchfile)) {
      devtools_error('DEVTOOLS_PATCH_FILE_ERROR', dt('Specified patch file "@patchfile" missing.', array(
        '@patchfile' => $patchfile,
      )));
    }
  }
  return $patchfile;
}
