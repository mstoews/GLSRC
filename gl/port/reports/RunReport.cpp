//
// RunReport.cpp - see RunReport.h.
//
#include "RunReport.h"

#include "ReportOptions.h"
#include "SampleReport.h"

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

int StressReports(int times)
{
    StressFrame frame;
    if (!frame.CreateFrame(hInst, L"GLReportStress", L"Report stress", nullptr, -1)) return -1;
    SetWindowPos(frame.hwnd(), nullptr, 0, 0, 900, 700, SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(frame.hwnd(), SW_SHOWNOACTIVATE);   // hidden windows don't paint
    SampleOptions three;
    three.repeat = 3;
    const ReportBuilder builder = sampleReport(three, false).second;
    int growth = -1;
    // Two warm-ups: the first registers the classes and creates cached GDI
    // objects, and one of those is released during the second, so measuring
    // after a single warm-up reads -1.
    if (stressOnce(frame, builder) && stressOnce(frame, builder)) {
        const int before = guiObjects();
        bool ok = true;
        for (int i = 0; i < times && ok; ++i) ok = stressOnce(frame, builder);
        if (ok) growth = guiObjects() - before;
    }
    DestroyWindow(frame.hwnd());
    return growth;
}
