//
// Grid.h - tbSpread, the Toolbox spreadsheet grid, as a report-mode
// ListView (Spec #2 design 4).
//
// RegisterCustomControls() registers the window class "tbSpread", so dialog
// templates that contain a tbSpread control (1011, 1015, 1017, 1029, 1043,
// 1061, 1160) create one unchanged; Grid::Create makes one in code, for the
// owned list windows the original created with `new TSpread`. The template
// text ("tbSpread" or "") is ignored. The control holds a ListView child that
// fills it (LVS_REPORT, full-row select, grid lines) and is a
// WS_EX_CONTROLPARENT, so the dialog manager tabs into the list.
//
// fx::Grid is a non-owning handle on such a control, with the subset of the
// Toolbox SS* calls the GL screens make. Rows and columns are 1-based, as in
// SS*; kAll stands for SS_ALLCOLS / SS_ALLROWS.
//
//   SSSetMaxCols + SSSetData(c, SS_HEADER) + SSSetColWidth   SetColumns
//   SSSetColWidth / SSGetColWidthInPixels                    SetColumnWidth / ColumnWidth
//   SSSetMaxRows / SSClearData                               SetMaxRows / Clear
//   SSSetData / SSGetData                                    SetData / GetData
//   SSSetColor                                               SetColor
//   SSSetBool(SSB_REDRAW)                                    SetRedraw
//   SSGetActiveCell / SSSetActiveCell                        ActiveRow, ActiveCol / SetActiveCell
//   SSSetTypeButton (two-state pictures)                     SetImageList + SetCellImage
//   SSSetTypeFloat on an unprotected column                  SetColumnEditable
//
// The control tells its parent with WM_COMMAND, MAKEWPARAM(control ID, code),
// lParam = the control (codes in CustomControls.h, numbered as the SSM_*
// messages they replace):
//
//   GRN_PICK         double-click or Enter on a row (SSM_DBLCLK). ActiveRow()
//                    is the row and ActiveCol() the column that was clicked.
//                    Not sent for a double-click on an editable cell, which
//                    starts editing instead.
//   GRN_SELCHANGE    another row became the active row (SSM_CLICK)
//   GRN_CELLCHANGED  an edited cell was committed with a new value
//                    (SSM_DATACHANGE); LastChangedRow() / LastChangedCol()
//   GRN_KEYDOWN      a key went down in the list (SSM_KEYDOWN); LastKey()
//   GRN_RCLICK       right-click on the list (SSM_RBUTTON); the row under the
//                    pointer, if any, is made active first
//
// Editing (Budget Data): SetColumnEditable(col, mask) makes that column's
// cells editable. Enter, F2 or a double-click places a tbFloat with that mask
// over the active row's cell. Enter, Tab or leaving the cell commits; Esc
// cancels; Up and Down commit and edit the cell above or below.
//
// Differences from tbSpread: the first column is always left-aligned (a
// ListView limitation); the active row shows as the selected row; there are
// no row headers, formulas, block selection or user column resizing.
//
#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#include <string>
#include <vector>

namespace win32fx {

enum class GridAlign { Left, Right, Center };

struct GridColumn {
    std::wstring header;
    int width = 0;   // pixels; Grid::CharWidth() converts tbSpread's character widths
    GridAlign align = GridAlign::Left;
};

class Grid {
public:
    static constexpr int kAll = -1;   // SS_ALLCOLS / SS_ALLROWS

    Grid() = default;
    explicit Grid(HWND control) noexcept : hwnd_(control) {}

    // A tbSpread child of `parent` with control ID `id`, in parent client
    // coordinates. RegisterCustomControls must have run. Null on failure.
    static HWND Create(HWND parent, int id, int x, int y, int cx, int cy,
                       DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP);

    HWND hwnd() const noexcept { return hwnd_; }
    HWND list() const noexcept;   // the ListView inside
    bool IsGrid() const noexcept; // hwnd() is a tbSpread control

    void SetColumns(const std::vector<GridColumn>& columns);
    int Columns() const;
    void SetColumnWidth(int col, int pixels);
    int ColumnWidth(int col) const;
    int CharWidth() const;            // width of "0" in the list's font
    // The column that takes the width the others leave whenever the control
    // is resized (TAccountList::WMSize, TAccessMenus::SetupWindow). 0: none.
    void SetStretchColumn(int col);

    void SetMaxRows(int rows);        // adds blank rows or removes rows from the end
    int Rows() const;
    void Clear();                     // removes every row, colour and image

    // Writing past the last row adds rows.
    void SetData(int col, int row, const std::wstring& text);
    std::wstring GetData(int col, int row) const;   // "" outside the grid

    // col and/or row may be kAll. (kAll, kAll) sets the default and clears
    // every row and cell colour.
    void SetColor(int col, int row, COLORREF back, COLORREF fore);

    // The caller keeps the image list (LVS_SHAREIMAGELISTS).
    void SetImageList(HIMAGELIST images);
    void SetCellImage(int col, int row, int image);   // -1: none

    void SetRedraw(bool on);

    int ActiveRow() const;            // 0: none
    int ActiveCol() const;            // 1 until a cell is clicked
    void SetActiveCell(int col, int row);   // selects and scrolls to the row; row 0 clears

    void SetColumnEditable(int col, const std::wstring& floatMask, DWORD tbStyle = 0);
    bool BeginEdit(int col, int row);
    void EndEdit(bool commit);
    bool IsEditing() const;

    int LastKey() const;              // virtual key of the last GRN_KEYDOWN
    int LastChangedRow() const;
    int LastChangedCol() const;

private:
    HWND hwnd_ = nullptr;
};

} // namespace win32fx
