//
// CompanyProfile.cpp - port of gl/src/gl_profi.cpp. See CompanyProfile.h.
//
// Translation notes:
//   - The original kept gl_prof, gl_pds and gl_setat as module globals,
//     opened by one dialog and used by the next. Here gl_prof/gl_pds belong
//     to their dialog, and SetAccounts lends its gl_setat to CashAccounts
//     (which, as before, leaves its "cash" tag selected; SetAccounts::Ok
//     selects "gl_accts" again before writing).
//   - Dates: the original formatted CCYYMMDD with Date4::format and string
//     4024 ("MM/DD/YY") and parsed it back. The tbDate controls take and give
//     YYYYMMDD (MainDialog::GetDate); the two static fields on 1021 get
//     FormatYmd().
//   - Account lookups seek account (padded to its width) + dept. The original
//     concatenated the control texts, which only matched while the account
//     control still held the padded text SetField had put there.
//
#include "CompanyProfile.h"

#include "Company.h"
#include "Globals.h"
#include "Ini.h"
#include "MainDialog.h"
#include "Tables.h"

#include "cb4sqlite.h"

#include <cstring>
#include <string>

#include "LegacyIds.h"   // last: see LegacyIds.h

// gl_profi.cpp's own IDs (not in gl.h).
#define ID_CLEAR       2001
#define ID_DIRECTORY   1017
#define ID_WITHHOLDING 4026
#define ID_SEARCHCASH  7010   // TAPCashAccts::Search; no control has this ID in gl_new.rc's 1034
#define ID_CASHACCTS   2236   // TGLSetAccounts::Cash
#define ID_SETPERIODS  4000   // TGLProfile::SetPeriodDates
#define ID_SETACCOUNTS 1125   // TGLProfile::SetAccounts

namespace {

// "19940401" -> "04/01/94" (string 4024, MM/DD/YY); blank or malformed -> "".
std::wstring FormatYmd(const char* ymd)
{
    const std::string s = StrTrim(ymd ? ymd : "");
    if (s.size() != 8 || s.find_first_not_of("0123456789") != std::string::npos) return {};
    return fx::Utf8ToWide(s.substr(4, 2) + "/" + s.substr(6, 2) + "/" + s.substr(2, 2));
}

// The seek key for account + dept: the account padded to its field width.
std::string AccountKey(const std::string& account, const std::string& dept, unsigned accountWidth)
{
    std::string key = StrTrim(account);
    if (key.size() < accountWidth) key.append(accountWidth - key.size(), ' ');
    return key + StrTrim(dept);
}

// String 8121, "Unable to locate the account number: %s  %s".
std::wstring AccountNotFound(const std::wstring& fmt, const std::string& account, const std::string& dept)
{
    std::wstring text = fmt;
    for (const std::string* arg : {&account, &dept}) {
        const size_t at = text.find(L"%s");
        if (at == std::wstring::npos) break;
        text.replace(at, 2, fx::Utf8ToWide(StrTrim(*arg)));
    }
    return fx::Trim(text);
}

// TGLSetAccounts::Account / TAPCashAccts::Account: is account+dept in glaccts?
bool AccountExists(const std::string& account, const std::string& dept, const std::wstring& fmt, HWND owner)
{
    Data4 gl(&cb, "glaccts");
    Field4 fAccount(gl, "account");
    bool found = false;
    if (gl.isValid() && fAccount.isValid()) {
        gl.SetIndex("accounts");
        found = gl.seek(AccountKey(account, dept, fAccount.len()).c_str()) == r4success;
    }
    gl.close();
    if (!found) MessageBoxW(owner, AccountNotFound(fmt, account, dept).c_str(), szApp, MB_ICONEXCLAMATION);
    return found;
}

// ---- TAPCashAccts ---------------------------------------------------------

class CashAccounts : public MainDialog {
public:
    explicit CashAccounts(GL_SETAT& setat) : gl_setat(setat) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;
    void Cancel() override { End(IDCANCEL); }   // ShutDownWindow only

private:
    void Ok();
    void Modify();
    void Delete();
    void GoNext();
    void Update();   // CM_SETDATA: record -> controls
    void Clear();

    GL_SETAT& gl_setat;
};

bool CashAccounts::OnInitDialog()
{
    MainDialog::OnInitDialog();
    gl_setat.SetIndex("cash");
    SetBitmap(ID_CASHACCTS);
    SetWindowTextW(hwnd(), GetString(6023).c_str());   // "General Ledger Bank Accounts"
    // Redraw() and AcctList->Show(): the tbSpread list (dialog 1011), not ported.
    return true;
}

bool CashAccounts::OnCommand(WORD id, WORD code, HWND ctl)
{
    switch (id) {
    case IDOK:          Ok(); return true;
    case ID_DELETE:     Delete(); return true;
    case ID_MODIFY:     Modify(); return true;
    case CM_SETDATA:    Update(); return true;
    case ID_CLEAR:      Clear(); return true;
    case ID_SEARCHCASH: return true;   // Search(): fills the tbSpread list
    default:            return MainDialog::OnCommand(id, code, ctl);
    }
}

bool CashAccounts::OnMessage(UINT msg, WPARAM, LPARAM, LRESULT&)
{
    if (msg == WM_LBUTTONDBLCLK) {   // TAPCashAccts::GoNext = [WM_FIRST + WM_LBUTTONDBLCLK]
        GoNext();
        return true;
    }
    return false;
}

void CashAccounts::Clear()
{
    SetField(ID_ACCOUNTS, "");
    SetField(ID_DEPART, "");
    SetField(ID_DESCRIPT, "");
}

void CashAccounts::GoNext()
{
    const std::string key = AccountKey(GetField(ID_ACCOUNTS), GetField(ID_DEPART), gl_setat.account.len());
    if (gl_setat.seek(key.c_str()) == r4success)
        gl_setat.skip();
    else
        gl_setat.top();
    Update();
}

void CashAccounts::Update()
{
    // The original first re-seeked to the row selected in the list, if shown.
    SetField(ID_ACCOUNTS, gl_setat.account);
    SetField(ID_DEPART, gl_setat.dept);
    SetField(ID_DESCRIPT, gl_setat.name);
}

void CashAccounts::Ok()
{
    const std::string account = GetField(ID_ACCOUNTS);
    const std::string dept = GetField(ID_DEPART);
    if (!AccountExists(account, dept, GetString(8121), hwnd())) return;
    if (gl_setat.seek(AccountKey(account, dept, gl_setat.account.len()).c_str()) != r4success) {
        gl_setat.appendStart();
        gl_setat.name.assign(GetField(ID_DESCRIPT).c_str());
        gl_setat.account.assign(account.c_str());
        gl_setat.dept.assign(dept.c_str());
        gl_setat.type.assign("CA");
        gl_setat.append();
        Clear();
    } else if (Message(GetString(6022), MB_ICONQUESTION | MB_YESNO) == IDYES) {   // "Already added. ...modify?"
        Modify();
        Clear();
    }
}

void CashAccounts::Modify()
{
    gl_setat.name.assign(GetField(ID_DESCRIPT).c_str());
    gl_setat.account.assign(GetField(ID_ACCOUNTS).c_str());
    gl_setat.dept.assign(GetField(ID_DEPART).c_str());
    gl_setat.type.assign("CA");
    gl_setat.flush();
}

void CashAccounts::Delete()
{
    if (Message(GetString(6024), MB_ICONQUESTION | MB_YESNO) == IDYES) {   // "...delete the record?"
        gl_setat.deleteRec();
        gl_setat.pack();
    }
    Update();
}

// ---- TGLSetAccounts -------------------------------------------------------

// Controls: 1001-1008 account, 2001-2008 name (read-only use), 3001-3008 dept.
constexpr int kFirstAccount = 1001;
constexpr int kAccountRows = 8;

class SetAccounts : public MainDialog {
public:
    SetAccounts() : gl_setat(cb) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    void Cancel() override
    {
        gl_setat.close();
        End(IDCANCEL);
    }

private:
    void Ok();

    GL_SETAT gl_setat;
};

bool SetAccounts::OnInitDialog()
{
    MainDialog::OnInitDialog();
    if (!gl_setat.isValid()) {
        Message(L"The company database has no GL_SETAT table.", MB_ICONSTOP);
        End(IDCANCEL);
        return true;
    }
    gl_setat.SetIndex("gl_accts");
    int i = kFirstAccount;
    for (gl_setat.top(); !gl_setat.eof() && i < kFirstAccount + kAccountRows; gl_setat.skip(), ++i) {
        SetField(i + 1000, gl_setat.name);      // 2000's
        SetField(i, gl_setat.account);          // 1000's
        SetField(i + 2000, gl_setat.dept);      // 3000's
    }
    return true;
}

bool SetAccounts::OnCommand(WORD id, WORD code, HWND ctl)
{
    switch (id) {
    case IDOK:
        Ok();
        return true;
    case ID_CASHACCTS: {   // TGLSetAccounts::Cash
        CashAccounts dlg(gl_setat);
        dlg.DoModal(1034, hwnd());
        return true;
    }
    default:
        return MainDialog::OnCommand(id, code, ctl);
    }
}

void SetAccounts::Ok()
{
    const std::wstring fmt = GetString(8121);
    for (int i = kFirstAccount; i < kFirstAccount + kAccountRows; ++i)
        if (!AccountExists(GetField(i), GetField(i + 2000), fmt, hwnd())) return;

    gl_setat.SetIndex("GL_ACCTS");
    int i = kFirstAccount;
    for (gl_setat.top(); !gl_setat.eof() && i < kFirstAccount + kAccountRows; gl_setat.skip(), ++i) {
        gl_setat.account.assign(GetField(i).c_str());
        gl_setat.dept.assign(GetField(i + 2000).c_str());
        gl_setat.flush();
    }
    gl_setat.unlock();
    gl_setat.close();
    End(IDOK);
}

// ---- TGLSetPeriodDates ----------------------------------------------------

// Controls: 4001-4012 start, 5001-5012 end, 6001-6012 description, 6013-6024 "Closed".
constexpr int kFirstPeriod = 4001;
constexpr int kPeriods = 12;

class SetPeriodDates : public MainDialog {
public:
    SetPeriodDates() : gl_pds(cb) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    void Cancel() override { End(IDCANCEL); }   // ShutDownWindow only

private:
    void Ok();
    void ResetDepreciation();

    GL_PDS gl_pds;
};

bool SetPeriodDates::OnInitDialog()
{
    MainDialog::OnInitDialog();
    if (!gl_pds.isValid()) {
        Message(L"The company database has no GL_PDS table.", MB_ICONSTOP);
        End(IDCANCEL);
        return true;
    }
    gl_pds.SetIndex("period");   // the original used TGLProfile's gl_pds, already on this tag
    int i = kFirstPeriod;
    for (gl_pds.top(); !gl_pds.eof() && i < kFirstPeriod + kPeriods; gl_pds.skip(), ++i) {
        SetField(i, StrTrim(gl_pds.periods.str()));
        SetField(i + 1000, StrTrim(gl_pds.periode.str()));
        SetField(i + 2000, gl_pds.pd_desc);
        if (std::strcmp(gl_pds.status.str(), "1") == 0) SetCheck(i + 2012, true);
    }
    return true;
}

bool SetPeriodDates::OnCommand(WORD id, WORD code, HWND ctl)
{
    switch (id) {
    case IDOK:  Ok(); return true;
    case ID_FA: ResetDepreciation(); return true;
    default:    return MainDialog::OnCommand(id, code, ctl);
    }
}

void SetPeriodDates::Ok()
{
    int i = kFirstPeriod;
    for (gl_pds.top(); !gl_pds.eof() && i < kFirstPeriod + kPeriods; gl_pds.skip(), ++i) {
        gl_pds.periods.assign(GetDate(i).c_str());
        gl_pds.periode.assign(GetDate(i + 1000).c_str());
        gl_pds.pd_desc.assign(GetField(i + 2000).c_str());
        gl_pds.status.assign(IsChecked(i + 2012) ? "1" : "0");
        gl_pds.flush();
    }
    gl_pds.unlock();
    End(IDOK);
}

void SetPeriodDates::ResetDepreciation()
{
    Data4 glfa(cb, "gl_fa");
    Field4 mode(glfa, "mode");
    if (Message(GetString(6025), MB_ICONINFORMATION | MB_YESNO) == IDYES && glfa.isValid() && mode.isValid()) {
        for (glfa.top(); !glfa.eof(); glfa.skip()) {   // "Reset the depreciation journal flag."
            mode.assign(" ");
            glfa.flush();
        }
    }
    glfa.close();
}

// ---- TGLProfile -----------------------------------------------------------

class CompanyProfile : public MainDialog {
public:
    CompanyProfile() : gl_prof(cb), gl_pds(cb) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;

private:
    void Ok();
    void GetWithHolding();
    void SetWithHolding();

    GL_PROF gl_prof;
    GL_PDS gl_pds;
};

bool CompanyProfile::OnInitDialog()
{
    MainDialog::OnInitDialog();
    if (!gl_prof.isValid()) {
        Message(L"The company database has no GL_PROF table.", MB_ICONSTOP);
        End(IDCANCEL);
        return true;
    }

    // The opening and closing dates of the fiscal year.
    if (gl_pds.isValid()) {
        gl_pds.SetIndex("period");
        gl_pds.top();
        SetFieldW(ID_PERIODS, FormatYmd(gl_pds.periods.str()));
        gl_pds.bottom();
        SetFieldW(ID_PERIODE, FormatYmd(gl_pds.periode.str()));
    }

    gl_prof.top();
    SetField(ID_VENDORNO, gl_prof.company);
    SetField(ID_ADDRESS1, gl_prof.address1);
    SetField(ID_ADDRESS2, gl_prof.address2);
    SetField(ID_ADDRESS3, gl_prof.address3);
    SetField(ID_POST, gl_prof.postal);
    SetField(ID_TEL, gl_prof.telephone);
    SetField(ID_FAX, gl_prof.fax);
    SetField(ID_CONTACT, gl_prof.contact);
    SetFieldW(ID_DIRECTORY, CurDir);   // the company database (was the .dbf directory)
    GetWithHolding();
    SetBitmap(ID_SETPERIODS);
    SetBitmap(ID_SETACCOUNTS);
    return true;
}

bool CompanyProfile::OnCommand(WORD id, WORD code, HWND ctl)
{
    switch (id) {
    case IDOK:
        Ok();
        return true;
    case ID_SETPERIODS: {
        SetPeriodDates dlg;
        dlg.DoModal(1028, hwnd());
        return true;
    }
    case ID_SETACCOUNTS: {
        SetAccounts dlg;
        dlg.DoModal(1033, hwnd());
        return true;
    }
    case ID_WITHHOLDING:   // a BS_CHECKBOX, not auto: TGLProfile::WithHolding toggles it
        SetCheck(ID_WITHHOLDING, !IsChecked(ID_WITHHOLDING));
        return true;
    default:
        return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL: closeAll, close
    }
}

void CompanyProfile::GetWithHolding()
{
    const std::wstring tax = GetIniString(L"GL.INI", GetString(8150).c_str(), L"TAX", L"T");   // [WITHHOLDING]
    SetCheck(ID_WITHHOLDING, tax == L"T");
}

void CompanyProfile::SetWithHolding()
{
    WriteIniString(L"GL.INI", GetString(8150).c_str(), L"TAX", IsChecked(ID_WITHHOLDING) ? L"T" : L"F");
}

void CompanyProfile::Ok()
{
    gl_prof.top();
    const bool adding = gl_prof.eof();   // no profile record yet
    if (adding) gl_prof.appendStart();
    gl_prof.company.assign(GetField(ID_VENDORNO).c_str());
    gl_prof.address1.assign(GetField(ID_ADDRESS1).c_str());
    gl_prof.address2.assign(GetField(ID_ADDRESS2).c_str());
    gl_prof.address3.assign(GetField(ID_ADDRESS3).c_str());
    gl_prof.postal.assign(GetField(ID_POST).c_str());
    gl_prof.telephone.assign(GetField(ID_TEL).c_str());
    gl_prof.fax.assign(GetField(ID_FAX).c_str());
    gl_prof.contact.assign(GetField(ID_CONTACT).c_str());
    UpdateUserTime(gl_prof, szUser);
    if (adding)
        gl_prof.append();
    else
        gl_prof.flush();
    gl_prof.unlock();
    SetWithHolding();

    cb.closeAll();
    cb.init();
    End(IDOK);
}

} // namespace

INT_PTR RunCompanyProfile(HWND frame)
{
    CompanyProfile dlg;
    return dlg.DoModal(1021, frame);
}
