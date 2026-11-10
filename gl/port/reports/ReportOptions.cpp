//
// ReportOptions.cpp - see ReportOptions.h.
//
#include "ReportOptions.h"

#include "../Globals.h"
#include "../PortIds.h"

#include "win32fx.h"

namespace {

class SampleOptionsDialog : public fx::Dialog {
public:
    explicit SampleOptionsDialog(SampleOptions& opt) : opt_(opt) {}

protected:
    bool OnInitDialog() override
    {
        CheckDlgButton(hwnd(), IDC_SAMPLE_ZEROS, opt_.includeZeros ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd(), IDC_SAMPLE_LANDSCAPE, opt_.landscape ? BST_CHECKED : BST_UNCHECKED);
        SendDlgItemMessageW(hwnd(), IDC_SAMPLE_REPEAT, EM_LIMITTEXT, 2, 0);
        SetDlgItemInt(hwnd(), IDC_SAMPLE_REPEAT, static_cast<UINT>(opt_.repeat), FALSE);
        return true;
    }

    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != IDOK) return false;
        BOOL ok = FALSE;
        const UINT repeat = GetDlgItemInt(hwnd(), IDC_SAMPLE_REPEAT, &ok, FALSE);
        if (!ok || repeat < 1 || repeat > 50) {
            MessageBoxW(hwnd(), L"Enter a number from 1 to 50.", szApp, MB_OK | MB_ICONEXCLAMATION);
            HWND edit = GetDlgItem(hwnd(), IDC_SAMPLE_REPEAT);
            SetFocus(edit);
            SendMessageW(edit, EM_SETSEL, 0, -1);
            return true;
        }
        opt_.includeZeros = IsDlgButtonChecked(hwnd(), IDC_SAMPLE_ZEROS) == BST_CHECKED;
        opt_.landscape = IsDlgButtonChecked(hwnd(), IDC_SAMPLE_LANDSCAPE) == BST_CHECKED;
        opt_.repeat = static_cast<int>(repeat);
        End(IDOK);
        return true;
    }

private:
    SampleOptions& opt_;
};

} // namespace

bool ShowSampleOptions(HWND owner, SampleOptions& opt)
{
    SampleOptionsDialog dlg(opt);
    return dlg.DoModal(hInst, IDD_SAMPLE_OPTIONS, owner) == IDOK;
}
