//
// AccountList.h - port of TAccountList (gl/header/gl_acctl.h, bodies in
// gl/src/gl_btche.cpp) and the TListWindow grid it holds (include/tlist_w.h,
// gl/src/gl_acctl.cpp): the "General Ledger Accounts List" window beside the
// Chart of Accounts.
//
// It is a captioned, sizable window owned by the dialog (an OWL TWindow with
// a dialog parent and no WS_CHILD), not an MDI child, so it never changes the
// frame menu. It lists account / dept / description of the rows that pass
// GL.INI [GLACCTS] QUERY (default ".NOT. DELETED()"), in "accounts" tag
// order. Double-click or Enter on a row seeks the dialog's glaccts to it,
// copies account, dept and description into the dialog, sends the dialog
// WM_COMMAND ID_ACCTLIST, and hides the list. Right-click opens List Window
// Parameters (dialog 1035) to edit the query.
//
// The grid is fx::Grid, the port's tbSpread (Spec #2 S2-M0). Typing on it
// selects the first account starting with the typed characters (the
// ListView's own type-ahead, which replaces TListWindow::GetKey).
//
#pragma once

#include "win32fx.h"

#include "cb4sqlite.h"

#include <memory>
#include <string>

class Tglaccts;

class AccountList : public fx::Window {
public:
    AccountList() = default;
    ~AccountList() override;

    // Creates the window, shown beside `owner` without taking the focus.
    // `gl` is the dialog's table, which a selection positions.
    bool Create(HWND owner, Tglaccts& gl);

    // TListWindow::Redraw: reload the rows.
    void Redraw();

    // TListWindow::Seek(ID): highlight the first row whose account starts
    // with `account` (trimmed), or the first row; show the window and focus
    // the grid. True if a row matched.
    bool Seek(const std::string& account);

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnDestroy() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;   // the grid's GRN_* notifications
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    void SelectRow(int row);   // TAccountList::HandleListBox; row is 1-based
    void SetQuery();           // TListWindow::SetParamenters
    std::string RowText(int row, int column) const;   // 1-based, as SSGetData
    void SavePosition() const;

    HWND owner_ = nullptr;
    fx::Grid grid_;
    Tglaccts* gl_ = nullptr;
    std::unique_ptr<Data4> rows_;   // the list's own handle on glaccts, as TListWindow::SetFields opened
    int fields_[3] = {1, 2, 3};     // GL.INI [GLACCTS] FIELD1..3
};
