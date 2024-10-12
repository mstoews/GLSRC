#if !defined(__tinvoice_h)              // Sentry, use file only if it's not already included.
#define __tinvoice_h

/*  Project invoice

	 Copyright © 1994. All Rights Reserved.

	 SUBSYSTEM:    invoice.apx Application
	 FILE:         tinvoice.h
	 AUTHOR:


	 OVERVIEW
	 ========
	 Class definition for TInvoice (TDialog).
*/

#include <owl\owlpch.h>
#pragma hdrstop

#include <owl\dialog.h>
#include <owl\edit.h>
#include "d4all.hpp"

#include "invocapp.rh"            // Definition of all resources.

#define ID_BUTTON1 1001

//{{TDialog = TInvoice}}
class TInvoice : public TDialog {
public:

	 TInvoice (TWindow* parent, TResId resId = IDD_INVOICE, TModule* module = 0);
	 virtual ~TInvoice ();
	 virtual void SetupWindow();

//{{TInvoiceRSP_TBL_BEGIN}}
protected:
	 Data4  dist;
	 TEdit* Vendor;
	 TEdit* OldInvoice;
	 TEdit* NewInvoice;
	 void CmChangeInvoice ();
	 BOOL CanClose(){ return TRUE; }
	 void Close();
//{{TInvoiceRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TInvoice);
};    //{{TInvoice}}


#endif                                      // __tinvoice_h sentry.

