//
// PrinterSetup.cpp - see report/win32/PrinterSetup.h.
//
#include "report/win32/PrinterSetup.h"

#include <cderr.h>   // PDERR_*: not in <Windows.h> with WIN32_LEAN_AND_MEAN
#include <commdlg.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace rpt::win32 {

namespace {

constexpr int kTwipsPerInch = 1440;
constexpr int kDefaultMargin = 1080;   // 0.75 inch

struct Session {
    HGLOBAL devMode = nullptr;
    HGLOBAL devNames = nullptr;
    bool looked = false;   // the default printer has been asked for
    int margins[4] = {kDefaultMargin, kDefaultMargin, kDefaultMargin, kDefaultMargin};   // left, top, right, bottom
    std::vector<HWND> listeners;

    ~Session()
    {
        if (devMode) GlobalFree(devMode);
        if (devNames) GlobalFree(devNames);
    }
};

Session& session()
{
    static Session s;
    return s;
}

// The Windows default printer, once, if nothing was chosen yet.
void ensurePrinter()
{
    Session& s = session();
    if (s.looked || s.devNames) return;
    s.looked = true;
    PRINTDLGW pd{};
    pd.lStructSize = sizeof pd;
    pd.Flags = PD_RETURNDEFAULT;
    if (PrintDlgW(&pd)) {
        s.devMode = pd.hDevMode;
        s.devNames = pd.hDevNames;
    }
}

HGLOBAL copyGlobal(HGLOBAL h)
{
    if (!h) return nullptr;
    const SIZE_T size = GlobalSize(h);
    HGLOBAL out = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!out) return nullptr;
    void* from = GlobalLock(h);
    void* to = GlobalLock(out);
    if (from && to) std::memcpy(to, from, size);
    if (to) GlobalUnlock(out);
    if (from) GlobalUnlock(h);
    return out;
}

void setOrientation(HGLOBAL devMode, Orientation o)
{
    if (!devMode) return;
    if (auto* dm = static_cast<DEVMODEW*>(GlobalLock(devMode))) {
        dm->dmOrientation = static_cast<short>(o == Orientation::Landscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT);
        dm->dmFields |= DM_ORIENTATION;
        GlobalUnlock(devMode);
    }
}

void notify()
{
    for (HWND h : session().listeners)
        if (IsWindow(h)) PostMessageW(h, kPrinterChanged, 0, 0);
}

int toTwips(int deviceUnits, int dpi) { return dpi > 0 ? MulDiv(deviceUnits, kTwipsPerInch, dpi) : 0; }

} // namespace

bool ShowPrinterSetup(HWND owner)
{
    ensurePrinter();
    Session& s = session();
    PAGESETUPDLGW psd{};
    psd.lStructSize = sizeof psd;
    psd.hwndOwner = owner;
    psd.hDevMode = s.devMode;
    psd.hDevNames = s.devNames;
    psd.Flags = PSD_INTHOUSANDTHSOFINCHES | PSD_MARGINS | PSD_DISABLEORIENTATION;
    psd.rtMargin = {MulDiv(s.margins[0], 1000, kTwipsPerInch), MulDiv(s.margins[1], 1000, kTwipsPerInch),
                    MulDiv(s.margins[2], 1000, kTwipsPerInch), MulDiv(s.margins[3], 1000, kTwipsPerInch)};
    const BOOL ok = PageSetupDlgW(&psd);
    // The dialog may free the handles it was given and return new ones, whatever the outcome.
    s.devMode = psd.hDevMode;
    s.devNames = psd.hDevNames;
    if (!ok) {
        if (CommDlgExtendedError() == PDERR_NODEFAULTPRN)
            MessageBoxW(owner, L"No printer is installed.", L"Printer Setup", MB_OK | MB_ICONINFORMATION);
        return false;
    }
    s.margins[0] = MulDiv(psd.rtMargin.left, kTwipsPerInch, 1000);
    s.margins[1] = MulDiv(psd.rtMargin.top, kTwipsPerInch, 1000);
    s.margins[2] = MulDiv(psd.rtMargin.right, kTwipsPerInch, 1000);
    s.margins[3] = MulDiv(psd.rtMargin.bottom, kTwipsPerInch, 1000);
    notify();
    return true;
}

PrinterHandles CopyPrinter(Orientation o)
{
    ensurePrinter();
    PrinterHandles h;
    h.devMode = copyGlobal(session().devMode);
    h.devNames = copyGlobal(session().devNames);
    setOrientation(h.devMode, o);
    return h;
}

void SetSessionPrinter(HGLOBAL devMode, HGLOBAL devNames)
{
    Session& s = session();
    if (devMode != s.devMode && s.devMode) GlobalFree(s.devMode);
    if (devNames != s.devNames && s.devNames) GlobalFree(s.devNames);
    s.devMode = devMode;
    s.devNames = devNames;
    s.looked = true;
    notify();
}

HDC CreatePrinterIC(Orientation o)
{
    PrinterHandles h = CopyPrinter(o);
    HDC ic = nullptr;
    if (h.devNames) {
        if (auto* dn = static_cast<DEVNAMES*>(GlobalLock(h.devNames))) {
            const wchar_t* base = reinterpret_cast<const wchar_t*>(dn);
            const std::wstring driver = base + dn->wDriverOffset;
            const std::wstring device = base + dn->wDeviceOffset;
            GlobalUnlock(h.devNames);
            auto* dm = h.devMode ? static_cast<DEVMODEW*>(GlobalLock(h.devMode)) : nullptr;
            ic = CreateICW(driver.empty() ? nullptr : driver.c_str(), device.c_str(), nullptr, dm);
            if (dm) GlobalUnlock(h.devMode);
        }
    }
    if (h.devMode) GlobalFree(h.devMode);
    if (h.devNames) GlobalFree(h.devNames);
    return ic;
}

PageSetup PageSetupFromDC(HDC printer)
{
    const int dpiX = GetDeviceCaps(printer, LOGPIXELSX), dpiY = GetDeviceCaps(printer, LOGPIXELSY);
    const int physW = GetDeviceCaps(printer, PHYSICALWIDTH), physH = GetDeviceCaps(printer, PHYSICALHEIGHT);
    const int offX = GetDeviceCaps(printer, PHYSICALOFFSETX), offY = GetDeviceCaps(printer, PHYSICALOFFSETY);
    const int resX = GetDeviceCaps(printer, HORZRES), resY = GetDeviceCaps(printer, VERTRES);
    PageSetup p;
    p.width = toTwips(physW, dpiX);
    p.height = toTwips(physH, dpiY);
    const int* m = session().margins;
    p.left = (std::max)(m[0], toTwips(offX, dpiX));
    p.top = (std::max)(m[1], toTwips(offY, dpiY));
    p.right = (std::max)(m[2], toTwips(physW - resX - offX, dpiX));
    p.bottom = (std::max)(m[3], toTwips(physH - resY - offY, dpiY));
    return p;
}

PageSetup CurrentPageSetup(Orientation o)
{
    if (HDC ic = CreatePrinterIC(o)) {
        PageSetup p = PageSetupFromDC(ic);
        DeleteDC(ic);
        if (p.width > 0 && p.height > 0) return p;
    }
    PageSetup p = PageSetup::A4(o);
    const int* m = session().margins;
    p.left = m[0];
    p.top = m[1];
    p.right = m[2];
    p.bottom = m[3];
    return p;
}

POINT PaperOrigin(HDC printer)
{
    return {-GetDeviceCaps(printer, PHYSICALOFFSETX), -GetDeviceCaps(printer, PHYSICALOFFSETY)};
}

void AddPrinterListener(HWND hwnd)
{
    auto& l = session().listeners;
    if (std::find(l.begin(), l.end(), hwnd) == l.end()) l.push_back(hwnd);
}

void RemovePrinterListener(HWND hwnd)
{
    auto& l = session().listeners;
    l.erase(std::remove(l.begin(), l.end(), hwnd), l.end());
}

} // namespace rpt::win32
