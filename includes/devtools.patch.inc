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
 * @param boolean $revert
 *   A boolean indicating wheth the patch should be reverted or not.
 * @param boolean $log
 *   A boolean indicating whether the shell output should be logged.
 *
 * @return bool
 *   Returns TRUE if successful, otherwise FALSE.
 */
function devtools_patch($patchfile, $filepath, $revert = FALSE, $log = TRUE) {
  $patched = FALSE;
  foreach (devtools_patch_levels() as $level) {
    $command = 'cd %s && GIT_DIR=. git apply --check ' . ($revert ? '-R' : '') . ' %s %s --verbose';
    $checked = drush_shell_exec($command, $filepath, $level, $patchfile);
    if ($checked) {
      $command = 'cd %s && GIT_DIR=. git apply ' . ($revert ? '-R' : '') . ' %s %s --verbose';
      drush_shell_exec($command, $filepath, $level, $patchfile);
      $patched = TRUE;
      break;
    }
  }

  foreach (devtools_patch_levels() as $level) {
    $command = 'patch ' . ($revert ? '-R' : '') . ' %s --no-backup-if-mismatch -d %s < %s';
    if (drush_shell_exec($command, $level, $filepath, $patchfile)) {
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
 * A wrapper for devtools_patch with revert flag TRUE.
 *
 * @see devtools_patch
 */
function devtools_patch_revert($patchfile, $filepath, $log = TRUE) {
  return devtools_patch($patchfile, $filepath, TRUE, $log);
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

/**
 * Reads and parses all patch file URIs from a PATCHES.txt file.
 *
 * @param string $patches_txt
 *   The file path of the PATCHES.txt file.
 *
 * @return array
 *   Returns an array of patch file URIs, FALSE if file does not exist.
 */
function _devtools_patch_parse_patches_text($patches_txt) {
  if (!file_exists($patches_txt)) {
    return FALSE;
  }

  $contents = file_get_contents($patches_txt);
  $contents = explode("\n", $contents);

  $patches = array();
  foreach ($contents as $line) {
    if (0 === strpos($line, '- ')) {
      $line = ltrim($line, '- ');
      $patches[] = $line;
    }
  }
  return $patches;
}
