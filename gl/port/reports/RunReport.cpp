//
// RunReport.cpp - see RunReport.h.
//
#include "RunReport.h"

#include "AccountReports.h"
#include "ApReports.h"
#include "AssetReports.h"
#include "../Ap.h"       // EnsureApTables
#include "../Assets.h"   // EnsureAssetTables
#include "MasterReports.h"
#include "ReportOptions.h"
#include "SampleReport.h"
#include "Statements.h"
#include "TrialBalances.h"
#include "VoucherReports.h"

#include "../Globals.h"

#include "report/win32/Preview.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

class WaitCursor {
public:
    WaitCursor() : old_(SetCursor(LoadCursorW(nullptr, IDC_WAIT))) {}
    ~WaitCursor() { SetCursor(old_); }
    WaitCursor(const WaitCursor&) = delete;
    WaitCursor& operator=(const WaitCursor&) = delete;

private:
    HCURSOR old_;
};

std::string today()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[16] = {};
    std::snprintf(buf, sizeof buf, "%04u%02u%02u", t.wYear, t.wMonth, t.wDay);
    return buf;
}

// Steps 1-3 of RunReport.h; nullopt when cancelled or failed (the failure shown).
std::optional<rpt::Report> build(HWND owner, const ReportOptionsFn& options, const ReportBuilder& builder)
{
    if (options && !options(owner)) return std::nullopt;
    try {
        WaitCursor wait;
        const ReportContext ctx = CurrentReportContext();
        rpt::Report r = builder(ctx);
        if (r.company.empty()) r.company = ctx.company;
        if (r.printedOn.empty()) r.printedOn = ctx.printedOn;
        return r;
    } catch (const std::exception& e) {
        MessageBoxW(owner, (L"The report could not be prepared:\n\n" + fx::Utf8ToWide(e.what())).c_str(), szApp,
                    MB_OK | MB_ICONSTOP);
        return std::nullopt;
    }
}

// The sample report's options and builder, sharing one SampleOptions so
// Options... starts from the last choice.
std::pair<ReportOptionsFn, ReportBuilder> sampleReport(SampleOptions start = {}, bool withDialog = true)
{
    auto opt = std::make_shared<SampleOptions>(start);
    ReportOptionsFn options;
    if (withDialog) options = [opt](HWND owner) { return ShowSampleOptions(owner, *opt); };
    ReportBuilder builder = [opt](const ReportContext& ctx) { return BuildSampleReport(cb, ctx, *opt); };
    return {options, builder};
}

// The options each report keeps for the session (spec #3 5.6).
TrialBalanceOptions& trialBalanceOptions()
{
    static TrialBalanceOptions opt;
    return opt;
}

DistTrialBalanceOptions& distTrialBalanceOptions()
{
    static DistTrialBalanceOptions opt;
    return opt;
}

// One set per statement (0 IS, 1 BS, 2 CF).
StatementOptions& statementOptions(int which)
{
    static StatementOptions opt[3];
    return opt[which];
}

using StatementBuilder = rpt::Report (*)(cb4sqlite::Code4&, const ReportContext&, const StatementOptions&);

void runStatement(fx::MdiFrame& frame, int which, const wchar_t* title, StatementBuilder builder)
{
    RunReport(
        frame,
        [which, title](HWND owner) {
            return ShowStatementOptions(owner, CurrentReportContext(), title, which == 0, statementOptions(which));
        },
        [which, builder](const ReportContext& ctx) { return builder(cb, ctx, statementOptions(which)); });
}

// A frame for /stress that isn't the main window (no WM_QUIT when destroyed).
class StressFrame : public fx::MdiFrame {
protected:
    bool OnDestroy() override { return true; }
};

int guiObjects()
{
    HANDLE self = GetCurrentProcess();
    return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
}

// One preview: open, page through at each zoom (painting each page), close.
bool stressOnce(fx::MdiFrame& frame, const ReportBuilder& builder)
{
    HWND p = rpt::win32::OpenPreview(frame, builder(CurrentReportContext()));
    if (!p) return false;
    HWND view = FindWindowExW(p, nullptr, L"rptPageView", nullptr);
    for (int zoom = 0; zoom < 3; ++zoom) {
        for (int i = 0; i < rpt::win32::PreviewPageCount(p); ++i) {
            if (view) {
                InvalidateRect(view, nullptr, FALSE);
                UpdateWindow(view);
                SendMessageW(view, WM_KEYDOWN, VK_NEXT, 0);
            }
        }
        if (view) SendMessageW(view, WM_KEYDOWN, VK_HOME, 0);
        rpt::win32::CyclePreviewZoom(p);
    }
    SendMessageW(frame.mdiClient(), WM_MDIDESTROY, reinterpret_cast<WPARAM>(p), 0);
    return !IsWindow(p);
}

} // namespace

void RunReport(fx::MdiFrame& frame, ReportOptionsFn options, ReportBuilder builder)
{
    std::optional<rpt::Report> r = build(frame.hwnd(), options, builder);
    if (!r) return;
    rpt::win32::Rebuild rebuild;
    if (options)
        rebuild = [options, builder](HWND owner) { return build(owner, options, builder); };
    if (!rpt::win32::OpenPreview(frame, std::move(*r), std::move(rebuild)))
        MessageBoxW(frame.hwnd(), L"The report preview could not be opened.", szApp, MB_OK | MB_ICONSTOP);
}

ReportContext CurrentReportContext()
{
    return LoadReportContext(cb, today());
}

void RunSampleReport(fx::MdiFrame& frame)
{
    auto [options, builder] = sampleReport();
    RunReport(frame, std::move(options), std::move(builder));
}

void RunTrialBalance(fx::MdiFrame& frame)
{
    RunReport(
        frame, [](HWND owner) { return ShowTrialBalanceOptions(owner, CurrentReportContext(), trialBalanceOptions()); },
        [](const ReportContext& ctx) { return TrialBalance(cb, ctx, trialBalanceOptions()); });
}

void RunDistributionTrialBalance(fx::MdiFrame& frame)
{
    RunReport(
        frame,
        [](HWND owner) { return ShowDistTrialBalanceOptions(owner, CurrentReportContext(), distTrialBalanceOptions()); },
        [](const ReportContext& ctx) { return DistributionTrialBalance(cb, ctx, distTrialBalanceOptions()); });
}

void RunIncomeStatement(fx::MdiFrame& frame) { runStatement(frame, 0, L"Income Statement", &IncomeStatement); }
void RunBalanceSheet(fx::MdiFrame& frame) { runStatement(frame, 1, L"Balance Sheet", &BalanceSheet); }
void RunCashFlowStatement(fx::MdiFrame& frame) { runStatement(frame, 2, L"Cash Flow Statement", &CashFlowStatement); }

void RunTrialBalanceByDept(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return TrialBalanceByDept(cb, ctx); });
}

void RunChartOfAccountsReport(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return ChartOfAccountsReport(cb, ctx); });
}

void RunBudgetReport(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return BudgetReport(cb, ctx); });
}

void RunVoucherSlips(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return VoucherSlips(cb, ctx); });
}

void RunVoucherDetail(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return VoucherDetail(cb, ctx); });
}

void RunVoucherSummary(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return VoucherSummary(cb, ctx); });
}

void RunPrepostingDistribution(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return PrepostingDistribution(cb, ctx); });
}

void RunPrePostingReport(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return PrePostingReport(cb, ctx); });
}

void RunMedicalRepsListing(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return MedicalRepsListing(cb, ctx); });
}

void RunProductCodesListing(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return ProductCodesListing(cb, ctx); });
}

void RunDoctorCodesListing(fx::MdiFrame& frame)
{
    RunReport(frame, {}, [](const ReportContext& ctx) { return DoctorCodesListing(cb, ctx); });
}

void RunFixedAssetListing(fx::MdiFrame& frame)
{
    EnsureAssetTables(cb);
    RunReport(frame, {}, [](const ReportContext& ctx) { return FixedAssetListing(cb, ctx); });
}

void RunFixedAssetTypes(fx::MdiFrame& frame)
{
    EnsureAssetTables(cb);
    RunReport(frame, {}, [](const ReportContext& ctx) { return FixedAssetTypesListing(cb, ctx); });
}

void RunApCheques(fx::MdiFrame& frame)
{
    EnsureApTables(cb);
    RunReport(frame, {}, [](const ReportContext& ctx) { return ApCheques(cb, ctx); });
}

int StressReports(int times)
{
    StressFrame frame;
    if (!frame.CreateFrame(hInst, L"GLReportStress", L"Report stress", nullptr, -1)) return -1;
    SetWindowPos(frame.hwnd(), nullptr, 0, 0, 900, 700, SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(frame.hwnd(), SW_SHOWNOACTIVATE);   // hidden windows don't paint
    SampleOptions three;
    three.repeat = 3;
    DistTrialBalanceOptions lines;
    lines.from = 1;
    lines.voucherLines = true;
    StatementOptions everything;
    everything.comparePrior = everything.compareBudget = everything.showAccounts = true;
    // The sample report (several pages), F1, F2 for periods 1 to the current
    // one with voucher lines (landscape), F3 with every column and its
    // accounts (landscape), F4 and F5; then every original report.
    const ReportBuilder builders[] = {
        sampleReport(three, false).second,
        [](const ReportContext& ctx) { return TrialBalance(cb, ctx, {}); },
        [lines](const ReportContext& ctx) { return DistributionTrialBalance(cb, ctx, lines); },
        [everything](const ReportContext& ctx) { return IncomeStatement(cb, ctx, everything); },
        [](const ReportContext& ctx) { return BalanceSheet(cb, ctx, {}); },
        [](const ReportContext& ctx) { return CashFlowStatement(cb, ctx, {}); },
        [](const ReportContext& ctx) { return TrialBalanceByDept(cb, ctx); },
        [](const ReportContext& ctx) { return ChartOfAccountsReport(cb, ctx); },
        [](const ReportContext& ctx) { return BudgetReport(cb, ctx); },
        [](const ReportContext& ctx) { return VoucherSlips(cb, ctx); },
        [](const ReportContext& ctx) { return VoucherDetail(cb, ctx); },
        [](const ReportContext& ctx) { return VoucherSummary(cb, ctx); },
        [](const ReportContext& ctx) { return PrepostingDistribution(cb, ctx); },
        [](const ReportContext& ctx) { return PrePostingReport(cb, ctx); },
        [](const ReportContext& ctx) { return MedicalRepsListing(cb, ctx); },
        [](const ReportContext& ctx) { return ProductCodesListing(cb, ctx); },
        [](const ReportContext& ctx) { return DoctorCodesListing(cb, ctx); },
        [](const ReportContext& ctx) { return FixedAssetListing(cb, ctx); },
        [](const ReportContext& ctx) { return FixedAssetTypesListing(cb, ctx); },
        [](const ReportContext& ctx) { return ApCheques(cb, ctx); },
    };
    auto round = [&] {
        for (const ReportBuilder& b : builders)
            if (!stressOnce(frame, b)) return false;
        return true;
    };
    int growth = -1;
    // Two warm-ups: the first registers the classes and creates cached GDI
    // objects, and one of those is released during the second, so measuring
    // after a single warm-up reads -1.
    if (round() && round()) {
        const int before = guiObjects();
        bool ok = true;
        for (int i = 0; i < times && ok; ++i) ok = round();
        if (ok) growth = guiObjects() - before;
    }
    DestroyWindow(frame.hwnd());
    return growth;
}
