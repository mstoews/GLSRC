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

// ---- commands --------------------------------------------------------------

#define CM_SAMPLE_REPORT        20500   // gl.exe /samplereport, and /stress

#endif // GL_PORT_PORTIDS_H
