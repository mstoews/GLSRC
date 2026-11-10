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

// ---- controls --------------------------------------------------------------

#define IDC_SAMPLE_ZEROS        20100   // include zero balances
#define IDC_SAMPLE_LANDSCAPE    20101   // landscape
#define IDC_SAMPLE_REPEAT       20102   // repeat the list n times (to make pages)

// ---- commands --------------------------------------------------------------

#define CM_SAMPLE_REPORT        20500   // gl.exe /samplereport, and /stress

#endif // GL_PORT_PORTIDS_H
