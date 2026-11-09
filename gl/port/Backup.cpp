//
// Backup.cpp - see Backup.h.
//
// Translation notes (spec D5, D6):
//   - GLBackup asked string 1073 ("...Place a floppy disk in drive A:"),
//     closed every table and ran savegl.pif. The port's Save As dialog is
//     the question; backupTo copies the open database, so nothing is closed.
//   - GLRestore asked string 1074, which is the menu's help line ("Restore
//     the current databases to the GL directory."), then ran restore.pif.
//     The port asks a question that names the file and says what happens to
//     the current one.
//   - Reindex (dialog 1065, gl_index.cpp) rebuilt each .mdx file; there are
//     none after migration.
//
#include "Backup.h"

#include "CompanyFiles.h"
#include "Globals.h"

#include "win32fx.h"

#include <commdlg.h>

#include <exception>
#include <filesystem>
#include <string>

#pragma comment(lib, "comdlg32.lib")

namespace fs = std::filesystem;

namespace {

constexpr wchar_t kFilter[] = L"Company database (*.db)\0*.db\0All files (*.*)\0*.*\0";

fs::path companyPath() { return fs::path(CurDir); }

int message(HWND owner, const std::wstring& text, UINT flags)
{
    return MessageBoxW(owner, text.c_str(), szApp, flags);
}

} // namespace

void RunBackup(HWND frame)
{
    const fs::path current = companyPath();
    SYSTEMTIME now{};
    GetLocalTime(&now);
    std::wstring file = BackupFileName(current, now.wYear, now.wMonth, now.wDay).wstring();
    file.resize(MAX_PATH);
    const std::wstring dir = current.parent_path().wstring();

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof ofn;
    ofn.hwndOwner = frame;
    ofn.lpstrFilter = kFilter;
    ofn.lpstrFile = file.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = dir.c_str();
    ofn.lpstrTitle = L"Backup Databases";
    ofn.lpstrDefExt = L"db";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
    if (!GetSaveFileNameW(&ofn)) return;
    file.resize(wcslen(file.c_str()));

    std::error_code ec;
    if (fs::equivalent(fs::path(file), current, ec)) {
        message(frame, L"Choose a different file: that is the company database itself.", MB_ICONEXCLAMATION);
        return;
    }
    try {
        HCURSOR old = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
        cb.backupTo(std::wstring_view(file));
        SetCursor(old);
    } catch (const std::exception& e) {
        message(frame, L"The backup failed:\n\n" + fx::Utf8ToWide(e.what()), MB_ICONSTOP);
        return;
    }
    message(frame, L"The company database was backed up to\n\n" + file, MB_ICONINFORMATION);
}

bool RunRestore(HWND frame)
{
    const fs::path current = companyPath();
    std::wstring file(MAX_PATH, L'\0');
    const std::wstring dir = current.parent_path().wstring();

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof ofn;
    ofn.hwndOwner = frame;
    ofn.lpstrFilter = kFilter;
    ofn.lpstrFile = file.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = dir.c_str();
    ofn.lpstrTitle = L"Restore Databases";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
    if (!GetOpenFileNameW(&ofn)) return false;
    file.resize(wcslen(file.c_str()));

    std::string why;
    if (!CheckRestoreFile(current, fs::path(file), &why)) {
        message(frame, L"This file can't be restored: " + fx::Utf8ToWide(why) + L".\n\n" + file, MB_ICONEXCLAMATION);
        return false;
    }
    const std::wstring bak = BackupOfCurrent(current).filename().wstring();
    if (message(frame,
                L"Replace the company database with\n\n" + file +
                    L"\n\nEverything entered since that backup will be lost. The current database is kept as " +
                    bak + L".",
                MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
        return false;

    HCURSOR old = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    const RestoreResult r = RestoreDatabase(cb, current, fs::path(file));
    SetCursor(old);
    if (r.restored) {
        message(frame, L"The company database was restored. The previous one is " + bak + L".", MB_ICONINFORMATION);
        return true;
    }
    std::wstring text = L"The restore failed: " + fx::Utf8ToWide(r.error) + L".";
    if (r.reopened)
        text += L"\n\nThe company database is unchanged.";
    else
        text += L"\n\nThe company database could not be reopened. Close the program and check " +
                current.wstring() + L" and " + bak + L".";
    message(frame, text, MB_ICONSTOP);
    return false;
}

void RunReindex(HWND frame)
{
    message(frame,
            L"The company database doesn't need reindexing: it is an SQLite file, which keeps its own indexes up to "
            L"date. (The original rebuilt the dBase .mdx files, which the migration replaced.)",
            MB_ICONINFORMATION);
}
