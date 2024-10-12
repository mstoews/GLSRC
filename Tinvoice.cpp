/*  Project invoice

	 Copyright © 1994. All Rights Reserved.

	 SUBSYSTEM:    invoice.apx Application
	 FILE:         tinvoice.cpp
	 AUTHOR:


	 OVERVIEW
	 ========
	 Source file for implementation of TInvoice (TDialog).
*/

#include <owl\owlpch.h>
#pragma hdrstop

#include "tinvoice.h"
#include "d4all.hpp"
#include <dir.h>

Code4 cb;

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TInvoice, TDialog)
//{{TInvoiceRSP_TBL_BEGIN}}
	 EV_BN_CLICKED(ID_BUTTON1,CmChangeInvoice),
//{{TInvoiceRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TInvoice Implementation}}


TInvoice::TInvoice (TWindow* parent, TResId resId, TModule* module):
	 TDialog(parent, resId, module)
{
	 // INSERT>> Your constructor code here.

}


TInvoice::~TInvoice ()
{
	 Destroy();

	 // INSERT>> Your destructor code here.

}

void TInvoice::Close()
{
  dist.close();
  cb.initUndo();
  ShutDownWindow();

}

void TInvoice::SetupWindow()
{
  Parent->MoveWindow(100,100,374,175,TRUE);
  SendDlgItemMessage(IDC_AP_AR,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"AR");
  SendDlgItemMessage(IDC_AP_AR,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"AP");
  SendDlgItemMessage(IDC_AP_AR,CB_SETCURSEL,NULL,NULL);

  Vendor = new TEdit(this,IDC_VENDOR);
  OldInvoice = new TEdit(this,IDC_OLD_INVOICENO);
  NewInvoice = new TEdit(this,IDC_NEW_INVOICENO);
  cb.init();
}

void TInvoice::CmChangeInvoice ()
{
	 // INSERT>> Your code here.

  char CurDir[128];
  GetPrivateProfileString("GL_DIR","DIR","",CurDir,sizeof(CurDir),"GL.INI");
  chdir(CurDir);



  char szVendor[11];
  char szOldInvoice[15];
  char szNewInvoice[15];

  GetDlgItemText(IDC_VENDOR,szVendor,10);
  GetDlgItemText(IDC_OLD_INVOICENO,szOldInvoice,15);
  GetDlgItemText(IDC_NEW_INVOICENO,szNewInvoice,15);





  string strSeek;
  strSeek = szVendor;
  strSeek += szOldInvoice;
  string Msg;

  char szType[3];
  char szDbf[10];
  int indx = SendDlgItemMessage(IDC_AP_AR,CB_GETCURSEL,NULL,NULL);
  SendDlgItemMessage(IDC_AP_AR,CB_GETLBTEXT,indx,(LPARAM)(LPCSTR)szType);

  if(strcmp(szType,"AR") == 0)
	 strcpy(szDbf,"ar_dist");
  else
	 strcpy(szDbf,"ap_dist");

  dist.open (cb,szDbf);
  Tag4  invoice(dist,"invoiceno");
  dist.select(invoice);


  if(dist.seek ( (LPSTR)strSeek.c_str()) == 0)
  {
	 Field4 invoiceno;
	 invoiceno.init(dist,"invoiceno");
	 invoiceno.assign(szNewInvoice);
	 dist.flush();

	 Msg =  "The invoice: ";
	 Msg += szOldInvoice;
	 Msg += " has been changed to: ";
	 Msg += szNewInvoice;
	 MessageBox((LPSTR)Msg.c_str(),"Invoice Modified",MB_ICONEXCLAMATION);

  }
  else
  {
	 Msg = "Unable to find invoice number :";
	 Msg += strSeek;
	 MessageBox((LPSTR)Msg.c_str(),"Invoice Modification Failed",MB_ICONEXCLAMATION);
  }
  dist.close();
}


