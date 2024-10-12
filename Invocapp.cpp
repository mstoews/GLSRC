/*  Project invoice

	 Copyright © 1994. All Rights Reserved.

	 SUBSYSTEM:    invoice.exe Application
	 FILE:         invocapp.cpp
	 AUTHOR:


	 OVERVIEW
	 ========
	 Source file for implementation of invoiceApp (TApplication).
*/


#include <owl\owlpch.h>
#pragma hdrstop


#include "invocapp.h"
#include "nvcabtdl.h"                        // Definition of about dialog.
#include "tinvoice.h"
#include "ctl3d.h"


//{{invoiceApp Implementation}}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(invoiceApp, TApplication)
//{{invoiceAppRSP_TBL_BEGIN}}
	 EV_COMMAND(CM_UPDATEINVOICE, CmUpdateInvoice),
	 //{{invoiceAppRSP_TBL_END}}
END_RESPONSE_TABLE;




//////////////////////////////////////////////////////////
// invoiceApp
// =====
//
invoiceApp::invoiceApp () : TApplication("invoice")
{

    // Common file file flags and filters for Open/Save As dialogs.  Filename and directory are
    // computed in the member functions CmFileOpen, and CmFileSaveAs.
    FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    FileData.SetFilter("All Files (*.*)|*.*|");

	 // INSERT>> Your constructor code here.

}


invoiceApp::~invoiceApp ()
{
    // INSERT>> Your destructor code here.

}


void invoiceApp::SetupSpeedBar (TDecoratedFrame *frame)
{ 
    //
    // Create default toolbar New and associate toolbar buttons with commands.
    //   
	 TControlBar* cb = new TControlBar(frame);
	 cb->Insert(*new TButtonGadget(CM_UPDATEINVOICE, CM_UPDATEINVOICE));
	 // Add fly-over help hints.
	 cb->SetHintMode(TGadgetWindow::EnterHints);

    frame->Insert(*cb, TDecoratedFrame::Top);
}


//////////////////////////////////////////////////////////
// invoiceApp
// =====
// Application intialization.
//
void invoiceApp::InitMainWindow ()
{
	 /*
	 SDIDecFrame *frame = new SDIDecFrame(0, GetName(), 0, TRUE);

	 nCmdShow = nCmdShow != SW_SHOWMINIMIZED ? SW_SHOWNORMAL : nCmdShow;

	 //
	 // Assign ICON w/ this application.
	 //
	 frame->SetIcon(this, IDI_SDIAPPLICATION);

	 //
	 // Menu associated with window and accelerator table associated with table.
	 //
	 frame->AssignMenu(SDI_MENU);

	 //
	 // Associate with the accelerator table.
	 //
	 frame->Attr.AccelTable = SDI_MENU;

	 SetupSpeedBar(frame);

	 TStatusBar *sb = new TStatusBar(frame, TGadget::Recessed,
												TStatusBar::CapsLock        |
												TStatusBar::NumLock         |
												TStatusBar::ScrollLock      |
												TStatusBar::Overtype);
	 frame->Insert(*sb, TDecoratedFrame::Bottom);

	 SetMainWindow(frame);
	 */
	 SetMainWindow(new TFrameWindow(0, "Change Invoice Number",new TInvoice(0,IDD_INVOICE)));
	 //RECT desktop;
	 //GetWindowRect(GetDesktopWindow(),&desktop);
	 MoveWindow(MainWindow->HWindow,100,100,300,200,TRUE);
    //
    // Windows 3-D controls.
    //
	 //EnableCtl3d(TRUE);
	 Ctl3dRegister(HInstance);
	 Ctl3dAutoSubclass(HInstance);


}



void invoiceApp::CmUpdateInvoice ()
{
	TInvoice Invoice(MainWindow,IDD_INVOICE);
	Invoice.DoExecute();

}

//////////////////////////////////////////////////////////
// invoiceApp
// ===========
// Menu File Open command


SDIDecFrame::SDIDecFrame (TWindow *parent, const char far *title, TWindow *clientWnd, BOOL trackMenuSelection, TModule *module)
    : TDecoratedFrame(parent, title, clientWnd == 0 ? new TEditFile(0, 0, 0) : clientWnd, trackMenuSelection, module)
{
    // INSERT>> Your constructor code here.

}


SDIDecFrame::~SDIDecFrame ()
{
    // INSERT>> Your destructor code here.

}


//////////////////////////////////////////////////////////
// invoiceApp
// ===========
// Menu Help About invoice.exe command

int OwlMain (int , char* [])
{
	 invoiceApp     App;
	 int             result;

	 result = App.Run();

    return result;
}
