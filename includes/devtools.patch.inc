<?php
/**
 * @file
 */

/**
 * @return array
 */
function devtools_patch_levels() {
  return array('-p1', '-p0');
}

/**
 * @param $patchfile
 * @param $filepath
 * @return bool
 */
function devtools_patch($patchfile, $filepath) {
  foreach (devtools_patch_levels() as $level) {
    $checked = drush_shell_exec('cd %s && GIT_DIR=. git apply --check %s %s --verbose', $filepath, $level, $patchfile);
    if ($checked) {
      drush_shell_exec('cd %s && GIT_DIR=. git apply %s %s --verbose', $filepath, $level, $patchfile);
      return TRUE;
    }
  }

  foreach (devtools_patch_levels() as $level) {
    if (drush_shell_exec('patch %s --no-backup-if-mismatch -d %s < %s', $level, $filepath, $patchfile)) {
      return TRUE;
    }
  }

  return FALSE;
}
