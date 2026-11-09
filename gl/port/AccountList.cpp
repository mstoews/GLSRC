//
// AccountList.cpp - see AccountList.h. The list window itself, and its
// translation notes, are in TableList.cpp.
//
#include "AccountList.h"

#include "Accounts.h"
#include "Globals.h"

#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

bool AccountList::Create(HWND owner, Tglaccts& gl)
{
    gl_ = &gl;
    Spec spec;
    spec.section = L"GLACCTS";   // TAccountList::GetClassName (the table's name) and the GL.INI section
    spec.table = "glaccts";
    spec.tag = "accounts";
    spec.caption = fx::LoadResString(hInst, 6084);   // "General Ledger Accounts List"
    // TAccountList::SetWndDetail: 6068 "Acct", 6054 "Dept", 6044 "Description".
    spec.headers = {fx::LoadResString(hInst, 6068), fx::LoadResString(hInst, 6054), fx::LoadResString(hInst, 6044)};
    return TableList::Create(owner, std::move(spec), [this](int row) { SelectRow(row); });
}

void AccountList::SelectRow(int row)
{
    if (!gl_ || !SeekAccount(*gl_, RowText(row, 1), RowText(row, 2))) return;
    const HWND dlg = owner();
    gl_->descript->SetField(dlg, ID_JDESCRIPT);
    gl_->accounts->SetField(dlg, ID_ACCOUNTS);
    gl_->dept->SetField(dlg, ID_DEPART);
    SendMessageW(dlg, WM_COMMAND, ID_ACCTLIST, 0);   // TGLAcctsAdd::Update
}
