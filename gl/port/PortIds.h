//
// PortIds.h - IDs of the resources and commands the port adds (spec #3 3;
// plan S3-M1 task 6). Included by gl/res/port.rc and by C++.
//
// The range 20000-20999 is used by no original resource or header (checked
// in S3-M1 against gl/gl_new.rc, gl/gl.rc, gl/res/gl.rc, gl/header,
// include, gl/port and win32fx/include):
//
//   20000-20099  dialog templates
//   20100-20499  their controls
//   20500-20799  commands
//   20800-20999  strings
//
// Plain #defines only: rc.exe reads this file too (gl.rc and port.rc both
// include it, hence the classic guard).
//
#ifndef GL_PORT_PORTIDS_H
#define GL_PORT_PORTIDS_H

// ---- dialogs ---------------------------------------------------------------

#define IDD_SAMPLE_OPTIONS      20000   // Sample Report options (S3-M1: the engine's test report)
#define IDD_TB_OPTIONS          20001   // Trial Balance / Distribution Trial Balance options (S3-M2)
#define IDD_STMT_OPTIONS        20002   // Income Statement / Balance Sheet / Cash Flow options (S3-M3)
#define IDD_STMT_LAYOUT         20003   // Accounts > Statement Layout (S3-M4)

// ---- controls --------------------------------------------------------------

#define IDC_SAMPLE_ZEROS        20100   // include zero balances
#define IDC_SAMPLE_LANDSCAPE    20101   // landscape
#define IDC_SAMPLE_REPEAT       20102   // repeat the list n times (to make pages)

#define IDC_TB_FROM_LABEL       20110   // "Period:" (F1) / "From period:" (F2)
#define IDC_TB_FROM             20111   // period combo
#define IDC_TB_TO_LABEL         20112   // F2 only
#define IDC_TB_TO               20113   // F2 only
#define IDC_TB_LINES            20114   // F2: show voucher lines
#define IDC_TB_ZEROS            20115   // F1: zero balances; F2: accounts with no balance or activity

#define IDC_ST_PERIOD           20120   // period combo
#define IDC_ST_DEPT_LABEL       20121   // F3 only
#define IDC_ST_DEPT             20122   // F3: "All departments" or one
#define IDC_ST_PRIOR            20123   // F3: compare with the prior year
#define IDC_ST_BUDGET           20124   // F3: compare with the budget
#define IDC_ST_ACCOUNTS         20125   // show the accounts under each line
#define IDC_ST_ZEROS            20126   // show zero lines

#define IDC_SL_STATEMENT        20140   // Statement Layout (S3-M4): the statement picker
#define IDC_SL_GRID             20141   //   the lines (a tbSpread)
#define IDC_SL_LINENO           20142   //   the selected line's fields
#define IDC_SL_KIND             20143
#define IDC_SL_SIGN             20144
#define IDC_SL_INDENT           20145
#define IDC_SL_CAPTION          20146
#define IDC_SL_FORMULA          20147
#define IDC_SL_RANGES           20148
#define IDC_SL_ADD              20149   //   the line buttons
#define IDC_SL_INSERT           20150
#define IDC_SL_DELETE           20151
#define IDC_SL_APPLY            20152
#define IDC_SL_CHECK            20153   //   the layout buttons
#define IDC_SL_PREVIEW          20154
#define IDC_SL_RESET            20155
#define IDC_SL_SAVE             20156

#define IDC_FA_SUMMARY          20130   // Disposal (1001): cost, accumulated depreciation, book value (S4-M4)
#define IDC_FA_DELETE           20131   // Disposal (1001): Delete, for an asset never depreciated

#define IDC_FRAME_TOOLBAR       20160   // the frame's toolbar (UI refresh spec 4.3; ManilaGL.cpp)
#define IDC_FRAME_STATUS        20161   // the frame's status bar (UI refresh spec 4.4)

// ---- commands --------------------------------------------------------------

#define CM_SAMPLE_REPORT        20500   // gl.exe /samplereport, and /stress
#define CM_STATEMENT_LAYOUT     20502   // Accounts > Statement Layout (spec #3 6.4, S3-M4)
#define CM_REPORT_LIST          20501   // Reporting > Custom Report, GLSUB > Custom Report: the report list (D9, S3-M7)
#define CM_VOUCHER_LIST         20503   // Transactions > Voucher List: every voucher in an MDI grid (VoucherBrowser.h)

// ---- strings ---------------------------------------------------------------

// The frame's toolbar tooltips (UI refresh spec 4.3; port.rc), in button order.
#define IDS_TIP_NEW             20800
#define IDS_TIP_OPEN            20801
#define IDS_TIP_SAVE            20802
#define IDS_TIP_PRINT           20803
#define IDS_TIP_GLBATCH         20804
#define IDS_TIP_APBATCH         20805
#define IDS_TIP_CASH            20806
#define IDS_TIP_ARBATCH         20807
#define IDS_TIP_GLINQ           20808
#define IDS_TIP_GLPOST          20809
#define IDS_TIP_GLADD           20810
#define IDS_TIP_EXIT            20811

#endif // GL_PORT_PORTIDS_H
