//
// AccountList.h - port of TAccountList (gl/header/gl_acctl.h, bodies in
// gl/src/gl_btche.cpp): the "General Ledger Accounts List" window beside the
// Chart of Accounts, a TableList (TListWindow) on glaccts.
//
// It lists account / dept / description of the rows that pass GL.INI
// [GLACCTS] QUERY (default ".NOT. DELETED()"), in "accounts" tag order.
// Double-click or Enter on a row seeks the dialog's glaccts to it, copies
// account, dept and description into the dialog, sends the dialog WM_COMMAND
// ID_ACCTLIST, and hides the list. The rest (typing, Esc, right-click query,
// saved position) is TableList's.
//
#pragma once

#include "TableList.h"

#include <string>

class Tglaccts;

class AccountList : public TableList {
public:
    // Creates the window beside `owner`, shown without taking the focus if
    // `show`. `gl` is the dialog's table, which a selection positions.
    bool Create(HWND owner, Tglaccts& gl, bool show = true);

private:
    void SelectRow(int row);   // TAccountList::HandleListBox; row is 1-based

    Tglaccts* gl_ = nullptr;
};
