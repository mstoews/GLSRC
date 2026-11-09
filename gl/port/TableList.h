//
// TableList.h - port of TListWindow (include/tlist_w.h, gl/src/gl_acctl.cpp)
// and the owned TWindow that held it: the list window the GL maintenance
// screens show beside their dialog.
//
//   Chart of Accounts   GLACCTS   (AccountList.h)
//   Department Codes    GLDEPT    (MasterData.cpp)
//   Medical Reps        GL_MEDRP
//   Doctor Codes        GL_DRCOD
//   Product Codes       GL_ITEMM
//
// The window is captioned and sizable, owned by the dialog (no WS_CHILD), so
// it never changes the frame menu. It shows three columns: GL.INI <section>
// FIELD1..3 (field numbers; default 1, 2, 3, or fewer if the table has fewer
// fields) of the rows that pass <section> QUERY (default ".NOT. DELETED()"),
// in `tag` order. Its position is <section> LEFT/TOP/RIGHT/BOTTOM, saved when
// the window is destroyed; without one it opens beside the dialog.
//
//   double-click / Enter   calls onPick with the row (TListWindow's owner's
//                          HandleListBox); hides the window if hideOnPick
//   typing                 the ListView's type-ahead on the first column
//                          (TListWindow::GetKey)
//   Esc                    resets the selection to the first row
//   right-click            List Window Parameters (dialog 1035): the QUERY
//   close box              hides the window
//
// The grid is fx::Grid, the port's tbSpread (Spec #2 S2-M0).
//
#pragma once

#include "win32fx.h"

#include "cb4sqlite.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

class TableList : public fx::Window {
public:
    struct Spec {
        const wchar_t* section = nullptr;   // GL.INI section and window class (the original's GetClassName)
        const char* table = nullptr;        // cb4sqlite table
        const char* tag = nullptr;          // row order
        std::wstring caption;
        std::vector<std::wstring> headers;  // one per column shown (at most 3)
        int stretchColumn = 3;              // the column that takes the remaining width
    };

    TableList() = default;
    ~TableList() override;

    // Creates the window, shown beside `owner` without taking the focus.
    bool Create(HWND owner, Spec spec, std::function<void(int row)> onPick, bool hideOnPick = true);

    // TListWindow::Redraw: reload the rows.
    void Redraw();

    // TListWindow::Seek(ID): select the first row whose first column starts
    // with `text` (trimmed), or the first row; show the window and focus
    // the grid. True if a row matched.
    bool Seek(const std::string& text);

    // Column text of a row as UTF-8 (1-based, as SSGetData).
    std::string RowText(int row, int column) const;

    HWND owner() const noexcept { return owner_; }

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnDestroy() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;   // the grid's GRN_* notifications
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    void SetQuery();   // TListWindow::SetParamenters
    void SavePosition() const;
    int IniInt(const wchar_t* key) const;
    std::string CurrentQuery() const;

    HWND owner_ = nullptr;
    Spec spec_;
    std::function<void(int)> onPick_;
    bool hideOnPick_ = true;
    fx::Grid grid_;
    std::unique_ptr<Data4> rows_;   // the list's own handle on the table, as TListWindow::SetFields opened
    std::vector<int> fields_;       // FIELD1..3
};
