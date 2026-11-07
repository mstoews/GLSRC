//
// Login.cpp - port of gl/src/gl_acces.cpp. See Login.h.
//
// Translation notes (plan per-port checklist):
//   - TAccess/TAccessl come from the hand-written gl/header/db_acces.h,
//     unchanged. Its constructors `new` five/two Field4s and never free them;
//     AccessTable/AccessLevels below delete them, so the header stays as it is.
//   - TGLAccess::SetupWindow rebuilt the "u" tag of ACCESS.PSW on every
//     login (Index4::create). The migrated database already has it; the port
//     creates it only if it is missing.
//   - Ok() read the user name with WM_GETTEXT(sizeof(char*)), i.e. its
//     first 3 characters, and relied on seek()'s partial match. The port
//     reads the whole name.
//
#include "Login.h"

#include "Globals.h"
#include "Ini.h"
#include "MainDialog.h"

#include "cb4sqlite.h"

#include <cstring>
#include <string>

#include "legacy_headers_begin.h"
#include "db_acces.h"
#include "legacy_headers_end.h"

#include "LegacyIds.h"   // last: see LegacyIds.h

// gl_acces.cpp's own IDs (not in gl.h).
#define ID_UPDATEWINDOW 1845
#define ID_CLEAR        1846
#define ID_RESET        1094
#define ID_RECNO        1173

BOOL bMenuSetup = TRUE;
BOOL bUpdate = FALSE;
char szAccessLevel[11] = "";
char szUser[11] = "";

namespace {

class AccessTable : public TAccess {
public:
    AccessTable() : TAccess(&cb) {}
    ~AccessTable() override
    {
        delete fUsername;
        delete fDescript;
        delete fDate;
        delete fPassword;
        delete fLevel;
    }
};

class AccessLevels : public TAccessl {
public:
    AccessLevels() : TAccessl(&cb) {}
    ~AccessLevels() override
    {
        delete fLevel;
        delete fMenuCode;
    }
};

// strcpy into a fixed global buffer, truncated to fit.
void copyField(char* dst, size_t size, const char* src)
{
    const std::string s = src ? src : "";
    dst[s.copy(dst, size - 1)] = '\0';
}

// ---- TGLAccess ------------------------------------------------------------

class Login : public MainDialog {
protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    void Cancel() override;

private:
    void Ok();

    AccessTable access_;
};

bool Login::OnInitDialog()
{
    MainDialog::OnInitDialog();
    SetBitmap(ID_ADD);
    if (!access_.isValid()) {
        Message(L"The company database has no ACCESS.PSW table, so no one can log in.\n\n"
                L"Migrate the access tables with dbf2sqlite, or set GL.INI [ACCESS] PSW=F.",
                MB_ICONSTOP);
        End(IDCANCEL);
        return true;
    }
    // TGLAccess::SetupWindow: Index4::create(Dbf, NULL, accessTag) with tag "u" on username.
    if (access_.SetIndex("u") != r4success) {
        cb4sqlite::createTag(cb, "access.psw", "u", "username");
        access_.SetIndex("u");
    }
    for (access_.top(); !access_.eof(); access_.skip(1)) {
        const std::wstring name = fx::Utf8ToWide(StrTrim(access_.fUsername->str()));
        SendDlgItemMessageW(hwnd(), ID_USER, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
    }
    return true;
}

bool Login::OnCommand(WORD id, WORD code, HWND ctl)
{
    switch (id) {
    case IDOK: Ok(); return true;
    case ID_ADD: return true;   // TGLAccess::Add: empty
    default: return MainDialog::OnCommand(id, code, ctl);
    }
}

void Login::Ok()
{
    const std::string user = StrTrim(GetField(ID_USER));
    const std::string password = GetField(ID_PASSWORD);
    bool accept = false;
    if (!user.empty() && access_.seek(user.c_str()) == r4success) {
        if (password == StrTrim(access_.fPassword->str())) {
            accept = true;
            copyField(szAccessLevel, sizeof szAccessLevel, access_.fLevel->str());
            copyField(szUser, sizeof szUser, access_.fUsername->str());
            AccessLevels accessl;
            if (accessl.isValid()) {
                if (accessl.seek(szAccessLevel) != r4success) accessl.top();
                copyField(szMsg, sizeof szMsg, accessl.fMenuCode->str());
            } else {
                szMsg[0] = '\0';   // no accessl table: nothing is grayed
            }
        } else {
            Message(GetString(1032), MB_ICONEXCLAMATION);   // "The current password that was entered is incorrect."
        }
    } else {
        Message(GetString(1033), MB_ICONEXCLAMATION);       // "The user name can not be found."
    }
    if (!accept) {
        // The original sent ID_CLEAR, which TGLAccess didn't handle; clear the password at least.
        SetFieldW(ID_PASSWORD, L"");
        SetFocus(GetDlgItem(hwnd(), ID_PASSWORD));
        return;
    }
    access_.close();
    bMenuSetup = FALSE;   // the next WM_INITMENUPOPUP applies szMsg
    End(IDOK);
}

void Login::Cancel()
{
    // TGLAccess::Cancel: no closeAll here; the caller closes the program.
    access_.close();
    End(IDCANCEL);
}

// ---- TGLAdministration ----------------------------------------------------

class UserAdministration : public MainDialog {
protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;

private:
    void Add();
    void Modify();
    void Delete();
    void Next();
    void Previous();
    void Top();
    void Bottom();
    void UpdateWindow();    // ID_UPDATEWINDOW: record -> controls
    void UpdateRecord();    // TGLAdministration::Update(): controls -> record
    void Clear();
    void Reset();
    void Menus();

    AccessTable access_;
    AccessLevels accessl_;
};

bool UserAdministration::OnInitDialog()
{
    MainDialog::OnInitDialog();
    SetBitmap(ID_MODIFY);
    SetBitmap(ID_SEARCH);
    SetBitmap(ID_ADD);
    SetBitmap(ID_VIEW);
    SetBitmap(ID_RESET);
    if (!access_.isValid()) {
        Message(L"The company database has no ACCESS.PSW table.", MB_ICONSTOP);
        End(IDCANCEL);
        return true;
    }
    access_.SetIndex("u");
    access_.top();
    // As the original: str() is the padded field, never "", so nothing is
    // deleted here; the pack() still removes records deleted earlier.
    while (!access_.eof()) {
        if (std::strcmp(access_.fUsername->str(), "") == 0) access_.deleteRec();
        access_.skip();
    }
    access_.pack();
    access_.top();
    UpdateWindow();
    return true;
}

bool UserAdministration::OnCommand(WORD id, WORD code, HWND ctl)
{
    switch (id) {
    case ID_ADD:          Add(); return true;
    case ID_MODIFY:       Modify(); return true;
    case ID_DELETE:       Delete(); return true;
    case ID_NEXT:         Next(); return true;
    case ID_BOTTOM:       Bottom(); return true;
    case ID_PREVIOUS:     Previous(); return true;
    case ID_TOP:          Top(); return true;
    case ID_UPDATEWINDOW: UpdateWindow(); return true;
    case ID_CLEAR:        Clear(); return true;
    case ID_RESET:        Reset(); return true;
    case ID_VIEW:         Menus(); return true;
    default:              return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL: closeAll, close
    }
}

void UserAdministration::UpdateWindow()
{
    SetFieldW(ID_RECNO, std::to_wstring(access_.recNo()));
    SetField(ID_USER, *access_.fUsername);
    SetField(ID_DESCRIPT, *access_.fDescript);
    SetField(ID_PASSWORD, *access_.fPassword);
    SetField(ID_ACCESSLEVEL, *access_.fLevel);
    // The original formatted the CCYYMMDD field with string 4024 (MM/DD/YY);
    // tbDate takes YYYYMMDD and shows it in its own format.
    SetField(ID_DATE, StrTrim(access_.fDate->str()));
}

void UserAdministration::Top()
{
    access_.top();
    UpdateWindow();
}

void UserAdministration::Bottom()
{
    access_.bottom();
    UpdateWindow();
}

void UserAdministration::Clear()
{
    SetField(ID_USER, "");
    SetField(ID_DESCRIPT, "");
    SetField(ID_DATE, "");
    SetField(ID_PASSWORD, "");
    SetField(ID_ACCESSLEVEL, "");
}

void UserAdministration::Next()
{
    if (!access_.eof()) {
        access_.skip();
        if (access_.eof()) access_.bottom();
        UpdateWindow();
    }
}

void UserAdministration::Previous()
{
    const std::string user = GetField(ID_USER);
    if (!user.empty() && access_.seek(user.c_str()) == r4success && !access_.bof()) {
        access_.skip(-1L);
        UpdateWindow();
    }
}

void UserAdministration::Delete()
{
    if (access_.seek(GetField(ID_USER).c_str()) == r4success)
        if (Message(GetString(1036), MB_ICONQUESTION | MB_YESNO) == IDYES) {   // "...delete this user?"
            access_.deleteRec();
            access_.pack();
        }
    UpdateWindow();
}

void UserAdministration::UpdateRecord()
{
    access_.fUsername->assign(GetField(ID_USER).c_str());
    access_.fDescript->assign(GetField(ID_DESCRIPT).c_str());
    access_.fDate->assign(GetDate(ID_DATE).c_str());
    access_.fPassword->assign(GetField(ID_PASSWORD).c_str());
    access_.fLevel->assign(GetField(ID_ACCESSLEVEL).c_str());
}

void UserAdministration::Add()
{
    const std::string user = StrTrim(GetField(ID_USER));
    if (user.empty()) {
        Message(L"User ID must not be blank.", MB_OK);
        return;
    }
    if (access_.seek(GetField(ID_USER).c_str()) != r4success) {
        access_.appendStart();
        UpdateRecord();
        access_.append();
    } else {
        Modify();
    }
    Clear();
}

void UserAdministration::Modify()
{
    if (access_.seek(GetField(ID_USER).c_str()) == r4success) {
        if (Message(GetString(1034), MB_YESNO | MB_ICONQUESTION) == IDYES) {   // "...modify this user?"
            UpdateRecord();
            access_.flush();
        }
    } else {
        // The original's dangling `else` showed this when the user answered
        // No; its text is for a user that isn't there.
        Message(GetString(1035), MB_ICONEXCLAMATION | MB_OK);   // "...can not be found in the user database."
    }
}

void UserAdministration::Reset()
{
    if (Message(GetString(8152), MB_ICONQUESTION | MB_YESNO) == IDYES) {
        // As the original: the level's menu code is set to the current
        // string (written when the table is next flushed or closed), and the
        // frame menu is re-grayed from it on the next WM_INITMENUPOPUP.
        if (accessl_.isValid() && accessl_.seek(GetField(ID_ACCESSLEVEL).c_str()) == r4success) {
            accessl_.fMenuCode->assign(szMsg);
            copyField(szMsg, sizeof szMsg, accessl_.fMenuCode->str());
        }
        bMenuSetup = FALSE;
    }
}

void UserAdministration::Menus()
{
    if (StrTrim(GetField(ID_USER)).empty()) {
        Message(GetString(6095), MB_ICONINFORMATION);   // "Enter a user code before designing a menu choose."
        return;
    }
    // TAccessMenus (dialog 1160) edits the string in a tbSpread grid, then
    // this saved it to accessl. Without the grid there is nothing to edit,
    // so nothing is saved either.
    Message(L"Menu access editing needs the spreadsheet grid control (tbSpread), which is not part "
            L"of Spec #1.\n\nThe access levels in the accessl table are unchanged.",
            MB_ICONINFORMATION);
}

} // namespace

bool LoginRequired()
{
    return fx::Trim(GetIniString(L"GL.INI", L"ACCESS", L"PSW")) == L"T";
}

INT_PTR RunLogin(HWND frame)
{
    Login dlg;
    return dlg.DoModal(1158, frame);
}

void RunUserAdministration(HWND frame)
{
    UserAdministration dlg;
    dlg.DoModal(1159, frame);
}

void ApplyMenuAccess(HMENU menu)
{
    if (bMenuSetup || !menu) return;
    Data4 accessm(cb, "accessm");
    if (accessm.isValid()) {
        accessm.SetIndex("menu");
        accessm.top();
        Field4 MenuID(accessm, 4);
        for (const char* t = szMsg; *t != '\0' && !accessm.eof(); ++t, accessm.skip()) {
            const UINT iMenuID = static_cast<UINT>(static_cast<long>(Str4ptr(MenuID.str())));
            EnableMenuItem(menu, iMenuID, MF_BYCOMMAND | (*t == '1' ? MF_GRAYED : MF_ENABLED));
        }
        accessm.close();
    }
    bMenuSetup = TRUE;
}
