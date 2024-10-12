
/* ------------------------------------------------
	VTSSMGR.H -- VisualTools Formual One
					 Spreadsheet Manager	Header
					 Copyright (c) VisualTools, 1994
  ------------------------------------------------*/


#ifndef __VTSS_H__
#include "vtss.h"
#endif


// Quiet compiler warning.
#define UNUSED(x)	(x)=(x)

#define ISNULL(x)	((x) == NULL)

// Manager errors
#define SSERROR_BADEVENT		40

typedef void (CALLBACK FAR * EVENTFUNC)(HSS, HWND, UINT);

// Manager functions
#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus


SSERROR SSEXPORTAPI SSWndCreateView		   (HWND, HSS FAR *);
SSERROR SSEXPORTAPI SSWndDestroyView	   (HSS);
SSERROR SSEXPORTAPI SSWndMoveView		   (HSS,	int, int, int, int);
SSERROR SSEXPORTAPI SSWndRegisterEvent	   (HSS, UINT, EVENTFUNC);
HWND	  SSEXPORTAPI SSWndGetWindow		   (HSS);
void	  SSEXPORTAPI SSWndClickEventInfo   (HSS, LPRC, LPRC);
void	  SSEXPORTAPI SSWndEditEventInfo		(HSS, LPSTR FAR*, LPBOOL FAR*);
void	  SSEXPORTAPI SSWndObjectEventInfo	(HSS, LPSTR FAR*, LPOBJID);
		  
#ifdef __cplusplus
}
#endif	// __cplusplus
