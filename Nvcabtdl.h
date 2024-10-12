#if !defined(__nvcabtdl_h)              // Sentry, use file only if it's not already included.
#define __nvcabtdl_h

/*  Project invoice
    
    Copyright © 1994. All Rights Reserved.

    SUBSYSTEM:    invoice.exe Application
    FILE:         nvcabtdl.h
    AUTHOR:       


    OVERVIEW
    ========
    Class definition for invoiceAboutDlg (TDialog).      
*/


#include <owl\owlpch.h>
#pragma hdrstop

#include "invocapp.rh"                  // Definition of all resources.


//{{TDialog = invoiceAboutDlg}}
class invoiceAboutDlg : public TDialog {
public:
    invoiceAboutDlg (TWindow *parent, TResId resId = IDD_ABOUT, TModule *module = 0);
    virtual ~invoiceAboutDlg ();

//{{invoiceAboutDlgVIRTUAL_BEGIN}}
public:
    void SetupWindow ();
//{{invoiceAboutDlgVIRTUAL_END}}
};    //{{invoiceAboutDlg}}


// Reading the VERSIONINFO resource.
class ProjectRCVersion {
public:
    ProjectRCVersion (TModule *module);
    virtual ~ProjectRCVersion ();

    BOOL GetProductName (LPSTR &prodName);
    BOOL GetProductVersion (LPSTR &prodVersion);
    BOOL GetCopyright (LPSTR &copyright);
    BOOL GetDebug (LPSTR &debug);

protected:
    LPBYTE      TransBlock;
    void FAR    *FVData;

private:
    // Don't allow this object to be copied.
    ProjectRCVersion (const ProjectRCVersion &);
    ProjectRCVersion & operator =(const ProjectRCVersion &);
};


#endif                                      // __nvcabtdl_h sentry.
