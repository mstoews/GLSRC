//---------------------------------------------------------------------------
//
//  Module:   mdiapp.h
//
//  Purpose:
//     Simple MDI interface to demonstrate MDI dialog functions.
//
//  Development Team:
//     Bryan A. Woodruff
//
//  History:   Date       Author      Comment
//              4/13/92   BryanW      Wrote it.
//
//
//---------------------------------------------------------------------------
//
//  Written by Microsoft Product Support Service, Windows Developer Support.
//  Copyright (c) 1992 Microsoft Corporation.  All Rights Reserved.
//
//---------------------------------------------------------------------------

#include <windows.h>

#include "mdidlg.h"
#include "resource.h"

// constant definitions

#define MAXLEN_DIALOGS  16

// notification from modeless dialog

#define DN_ENDDIALOG    0x1000

// atom definitions

#define ATOM_NOTIFYWND  0x0100

// data structures

typedef struct tagMODELESSDIALOGSTRUCT
{
   HWND     hDlg ;
   DLGPROC  lpDialogProc ;

} MODELESSDIALOGSTRUCT ;

// global defintions
#ifdef GLOBALDEFS
HWND                  hMDIClient, hMDIFrame ;
MODELESSDIALOGSTRUCT  ModelessDialogs[ MAXLEN_DIALOGS ] ;
#else
extern HWND                  hMDIClient, hMDIFrame ;
extern MODELESSDIALOGSTRUCT  ModelessDialogs[ MAXLEN_DIALOGS ] ;
#endif

// macros

#define SET_PROP( x, atom,  y )  SetProp( x, MAKEINTATOM( atom ), y )
#define GET_PROP( x, atom )      GetProp( x, MAKEINTATOM( atom ) )
#define REMOVE_PROP( x, atom )   RemoveProp( x, MAKEINTATOM( atom ) )

// function prototypes

int FAR PASCAL WinMain( HINSTANCE, HINSTANCE, LPSTR, int ) ;
BOOL NEAR InitApplication( HINSTANCE ) ;
BOOL NEAR InitInstance( HINSTANCE, LPSTR, int ) ;
VOID NEAR CloseAllChildren( VOID ) ;

BOOL NEAR AddModelessDialog( HWND, DLGPROC ) ;
DLGPROC NEAR RemoveModelessDialog( HWND ) ;

LRESULT FAR PASCAL MDIFrameWndProc( HWND, UINT, WPARAM, LPARAM ) ;
BOOL FAR PASCAL DialogProc( HWND, UINT, WPARAM, LPARAM ) ;

//---------------------------------------------------------------------------
//  End of File: mdiapp.h
//---------------------------------------------------------------------------

