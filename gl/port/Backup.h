//
// Backup.h - Maintenance > Backup Databases (ID_BACKUP, 1072), Restore
// Databases (ID_RESTORE, 1074) and Reindex Databases (ID_REINDEX, 3047).
// Spec #2 design D5 and D6, plan S2-M6.
//
//   RunBackup    a Save As dialog, then Code4::backupTo
//   RunRestore   an Open dialog, a confirmation, then RestoreDatabase
//                (CompanyFiles.h); true if the company file was replaced,
//                so the frame refreshes the module bar
//   RunReindex   a message: SQLite keeps its own indexes
//
// The original ran the DOS programs savegl.pif and restore.pif, and Reindex
// rebuilt the .mdx files (gl/src/gl.cpp, gl_index.cpp).
//
#pragma once

#include <Windows.h>

void RunBackup(HWND frame);
bool RunRestore(HWND frame);
void RunReindex(HWND frame);
