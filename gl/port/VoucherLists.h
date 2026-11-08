//
// VoucherLists.h - the two windows Voucher Entry (VoucherEntry.cpp) owns
// besides its account list and voucher list (TableList):
//
//   LineList  TGLDistList (gl/header/ap_class.h, gl/src/gl_btche.cpp): the
//             "Distribution List" of the voucher's lines, with a total row.
//             Its position is GL.INI [GL_DISTLIST] LEFT/TOP/RIGHT/BOTTOM.
//   ToolBox   TGLToolBox (gl/header/gl_toolb.h): a small floating window of
//             picture buttons that send the dialog the same commands as its
//             own buttons.
//
#pragma once

#include "win32fx.h"

#include "Vouchers.h"

#include <functional>
#include <vector>

class LineList : public fx::Window {
public:
    LineList() = default;
    ~LineList() override;

    // Creates the window beside `owner` (below it without a saved position)
    // and shows it without taking the focus. `onPick` gets the line number
    // of a double-clicked / Enter'd row, or 0 for the total row or a blank
    // one (the original then reloaded the voucher).
    bool Create(HWND owner, std::function<void(int line)> onPick);

    // TGLBatchEntry::CurrentBatch's rows: one per line, then the total row,
    // "Total" or "Voucher Out Of Balance" (white on red), with the totals.
    void Show(const std::vector<VoucherLine>& lines, double debit, double credit);
    void Clear();

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnDestroy() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    void SavePosition() const;

    HWND owner_ = nullptr;
    fx::Grid grid_;
    std::function<void(int)> onPick_;
    std::vector<int> lineOfRow_;   // row - 1 -> line number
};

class ToolBox : public fx::Window {
public:
    ToolBox() = default;
    ~ToolBox() override;

    // Creates the hidden tool box near the top right of `owner`. Its buttons
    // send `owner` WM_COMMAND with their IDs.
    bool Create(HWND owner);

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    HWND owner_ = nullptr;
};
