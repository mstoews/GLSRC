#if !defined(__invocapp_h)              // Sentry, use file only if it's not already included.
#define __invocapp_h

/*  Project invoice
    
    Copyright © 1994. All Rights Reserved.

    SUBSYSTEM:    invoice.exe Application
    FILE:         invocapp.h
    AUTHOR:       


    OVERVIEW
    ========
    Class definition for invoiceApp (TApplication).      
*/


#include <owl\owlpch.h>
#pragma hdrstop

#include <owl\statusba.h>
#include <owl\controlb.h>
#include <owl\buttonga.h>
#include <owl\editfile.h>
#include <owl\opensave.h>

#include "invocapp.rh"            // Definition of all resources.


//
// FrameWindow must be derived to override Paint for Preview and Print.
//
//{{TDecoratedFrame = SDIDecFrame}}
class SDIDecFrame : public TDecoratedFrame {
public:
    SDIDecFrame (TWindow *parent, const char far *title, TWindow *clientWnd, BOOL trackMenuSelection = FALSE, TModule *module = 0);
    ~SDIDecFrame ();
};    //{{SDIDecFrame}}


//{{TApplication = invoiceApp}}
class invoiceApp : public TApplication {
private:
    TOpenSaveDialog::TData FileData;                    // Data to control open/saveas standard dialog.

private:
    void SetupSpeedBar (TDecoratedFrame *frame);

public:
    invoiceApp ();
    virtual ~invoiceApp ();

    void OpenFile (const char *fileName = 0);
//{{invoiceAppVIRTUAL_BEGIN}}
public:
    virtual void InitMainWindow();
//{{invoiceAppVIRTUAL_END}}

//{{invoiceAppRSP_TBL_BEGIN}}
protected:
	 void CmUpdateInvoice();
//{{invoiceAppRSP_TBL_END}}
  DECLARE_RESPONSE_TABLE(invoiceApp);
};    //{{invoiceApp}}


#endif                                      // __invocapp_h sentry.
