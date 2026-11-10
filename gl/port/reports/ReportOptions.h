//
// ReportOptions.h - the reports' options dialogs (spec #3 3, 5.6). Each
// shows its options, starting from `opt`, and returns true after OK with
// `opt` updated. Their templates are in gl/res/port.rc.
//
// The period lists show the context's fiscal periods ("7  Oct 26"); a
// period of 0 in `opt` starts at the current period, and OK stores the
// period chosen.
//
#pragma once

#include "ReportContext.h"
#include "SampleReport.h"
#include "TrialBalances.h"

#include <Windows.h>

// IDD_SAMPLE_OPTIONS.
bool ShowSampleOptions(HWND owner, SampleOptions& opt);

// IDD_TB_OPTIONS, as F1 Trial Balance (period; zero balances) and F2
// Distribution Trial Balance (from / to period; voucher lines; inactive accounts).
bool ShowTrialBalanceOptions(HWND owner, const ReportContext& ctx, TrialBalanceOptions& opt);
bool ShowDistTrialBalanceOptions(HWND owner, const ReportContext& ctx, DistTrialBalanceOptions& opt);
