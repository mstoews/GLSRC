//
// VoucherBrowser.cpp - see VoucherBrowser.h.
//
#include "VoucherBrowser.h"

#include "Globals.h"
#include "VoucherEntry.h"
#include "VoucherLists.h"   // FormatAmount
#include "Vouchers.h"
#include "reports/RunReport.h"
#include "reports/VoucherReports.h"

#include "report/Format.h"

#include <string>
#include <vector>

namespace {

constexpr wchar_t kClass[] = L"GLVoucherBrowser";
constexpr wchar_t kTitle[] = L"General Ledger Vouchers";
constexpr int kGridId = 100;

constexpr COLORREF kPostedText = RGB(96, 96, 96);
constexpr COLORREF kWhite = RGB(255, 255, 255);

class VoucherBrowser : public fx::MdiChild {
public:
    bool selfOwned = false;   // set once the window exists: OnFinalMessage deletes

    void Reload();

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;
    void OnFinalMessage() override
    {
        if (selfOwned) delete this;
    }

private:
    void Open(int row);

    fx::Grid grid_;
    std::vector<VoucherHeader> rows_;   // row - 1 -> voucher
};

HWND gOpen = nullptr;   // the one voucher list

bool VoucherBrowser::OnCreate(LPCREATESTRUCTW)
{
    grid_ = fx::Grid(fx::Grid::Create(hwnd(), kGridId, 0, 0, 0, 0));
    if (!grid_.IsGrid()) return false;
    const int ch = grid_.CharWidth();
    grid_.SetColumns({{L"Voucher", 9 * ch},
                      {L"Status", 16 * ch},
                      {L"Date", 11 * ch},
                      {L"Period", 7 * ch, fx::GridAlign::Right},
                      {L"Description", 30 * ch},
                      {L"Lines", 6 * ch, fx::GridAlign::Right},
                      {L"Debit", 14 * ch, fx::GridAlign::Right},
                      {L"Credit", 14 * ch, fx::GridAlign::Right}});
    grid_.SetStretchColumn(5);
    Reload();
    return false;
}

void VoucherBrowser::Reload()
{
    const int keep = grid_.ActiveRow();
    const long keepNumber = keep >= 1 && keep <= static_cast<int>(rows_.size()) ? rows_[keep - 1].number : 0;
    {
        VoucherHeaders h(cb);   // closed again at the end of the block (see the header)
        rows_ = ListVouchers(h, {voucher_mode::kOpen, voucher_mode::kAdjustment, voucher_mode::kPosted,
                                 voucher_mode::kPostedAdjustment, voucher_mode::kClosedYear});
    }
    grid_.SetRedraw(false);
    grid_.Clear();
    grid_.SetMaxRows(static_cast<int>(rows_.size()));
    int select = static_cast<int>(rows_.size());   // the newest, unless the kept one is still there
    for (int i = 0; i < static_cast<int>(rows_.size()); ++i) {
        const VoucherHeader& v = rows_[i];
        const int row = i + 1;
        grid_.SetData(1, row, std::to_wstring(v.number));
        grid_.SetData(2, row, fx::Utf8ToWide(VoucherStatus(v.mode)));
        grid_.SetData(3, row, fx::Utf8ToWide(rpt::FormatDate(v.date)));
        grid_.SetData(4, row, v.period ? std::to_wstring(v.period) : std::wstring());
        grid_.SetData(5, row, fx::Utf8ToWide(v.description));
        grid_.SetData(6, row, std::to_wstring(v.lines));
        grid_.SetData(7, row, FormatAmount(v.debit, false));
        grid_.SetData(8, row, FormatAmount(v.credit, false));
        if (IsPostedMode(v.mode)) grid_.SetColor(fx::Grid::kAll, row, kWhite, kPostedText);
        if (v.number == keepNumber) select = row;
    }
    grid_.SetRedraw(true);
    if (select) grid_.SetActiveCell(1, select);
    SetWindowTextW(hwnd(), (std::wstring(kTitle) + L" (" + std::to_wstring(rows_.size()) + L")").c_str());
}

void VoucherBrowser::Open(int row)
{
    if (row < 1 || row > static_cast<int>(rows_.size()) || !frame()) return;
    const VoucherHeader v = rows_[row - 1];
    HWND owner = frame()->hwnd();
    if (v.mode == voucher_mode::kOpen) {
        RunVoucherEntry(owner, v.number);
    } else if (v.mode == voucher_mode::kAdjustment) {
        RunPriorPeriodEntry(owner, v.number);
    } else {   // posted: read only
        const long n = v.number;
        RunReport(*frame(), {}, [n](const ReportContext& ctx) { return VoucherSlip(cb, ctx, n); });
        return;   // the preview is active now; nothing changed
    }
    if (IsWindow(hwnd())) {
        Reload();
        SetFocus(grid_.list());
    }
}

bool VoucherBrowser::OnSize(UINT type, int cx, int cy)
{
    if (type != SIZE_MINIMIZED && grid_.hwnd()) MoveWindow(grid_.hwnd(), 0, 0, cx, cy, TRUE);
    return false;   // DefMDIChildProc too (maximize / minimize)
}

bool VoucherBrowser::OnCommand(WORD id, WORD code, HWND)
{
    if (id != kGridId) return false;
    if (code == GRN_PICK) {
        Open(grid_.ActiveRow());
        return true;
    }
    if (code == GRN_KEYDOWN && grid_.LastKey() == VK_F5) {
        Reload();
        return true;
    }
    return false;
}

bool VoucherBrowser::OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result)
{
    MdiChild::OnMessage(msg, wp, lp, result);   // reports WM_MDIACTIVATE to the frame
    switch (msg) {
    case WM_SETFOCUS:
        if (grid_.list()) SetFocus(grid_.list());
        return false;
    case WM_DESTROY:
        if (gOpen == hwnd()) gOpen = nullptr;
        return false;
    default:
        return false;
    }
}

} // namespace

void OpenVoucherBrowser(fx::MdiFrame& frame)
{
    if (gOpen && IsWindow(gOpen)) {
        SendMessageW(frame.mdiClient(), WM_MDIACTIVATE, reinterpret_cast<WPARAM>(gOpen), 0);
        if (IsIconic(gOpen)) SendMessageW(frame.mdiClient(), WM_MDIRESTORE, reinterpret_cast<WPARAM>(gOpen), 0);
        if (auto* w = dynamic_cast<VoucherBrowser*>(fx::Window::FromHandle(gOpen))) w->Reload();
        return;
    }
    auto* w = new VoucherBrowser;
    HWND h = w->CreateChild(frame, kClass, kTitle);
    if (!h) {
        delete w;
        MessageBoxW(frame.hwnd(), L"The voucher list could not be opened.", szApp, MB_OK | MB_ICONSTOP);
        return;
    }
    w->selfOwned = true;
    gOpen = h;
    SetFocus(h);
}
