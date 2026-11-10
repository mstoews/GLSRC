//
// ReportOptions.h - the reports' options dialogs (spec #3 3, 5.6). Each
// shows its options, starting from `opt`, and returns true after OK with
// `opt` updated. Their templates are in gl/res/port.rc.
//
// S3-M1 has only the sample report's; the finance set's come in S3-M2/M3.
//
#pragma once

#include "SampleReport.h"

#include <Windows.h>

// IDD_SAMPLE_OPTIONS.
bool ShowSampleOptions(HWND owner, SampleOptions& opt);
