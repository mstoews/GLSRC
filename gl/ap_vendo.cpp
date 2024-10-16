/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Modify, add and delete vendor AP accounts functions.
*/
#include "ap.h"
#include <math.h>
#include "db_vend.h"
#include "gl_dlg.h"
#include "ap_vendo.h"
#include "gl_setat.hpp"

extern Code4 cb;
extern char szApp[20];
#define ID_DIS_DAYS 1037
#define ID_DIS_PERC 1038
#define ID_VIN_NUM 1039


void TAPVendorAdd::SetData(RTMessage)
{
 if(apvendor->seek(GetField(ID_VENDORNO) == 0))
 {
	SetField(ID_VENDORNO       , apvendor->vendorno );
	SetField(ID_VENDORSHORT    , apvendor->vdshort );
	SetField(ID_VENDORNAME     , apvendor->vdname );
	SetField(ID_VENDORADDRESS1 , apvendor->address1 );
	SetField(ID_VENDORADDRESS2 , apvendor->address2 );
	SetField(ID_VENDORADDRESS3 , apvendor->address3 );
	SetField(ID_VENDORPOST     , apvendor->postal );
	SetField(ID_VENDORCONTACT  , apvendor->contact );
	SetField(ID_VENDORSTATUS   , apvendor->status );
	SetField(ID_TERMS          , apvendor->terms );
	SetField(ID_VATPERCENTAGE  , apvendor->percentage );
	SetField(ID_REBATE         , apvendor->rebate );
	SetField(ID_PHONE	         , apvendor->phone );
	SetField(ID_FAX	         , apvendor->fax );
	SetField(ID_APCONTROL      , apvendor->account );
	SetField(ID_APDEPART       , apvendor->depart );
	SetField(ID_VATACCT        , apvendor->vat_acct );
	SetField(ID_VATDEPT        , apvendor->vat_dept );
	SetField(ID_VIN_NUM        , apvendor->vin );
	SetField(ID_MEDREP         , apvendor->medrep );
	SetField(ID_DIS_DAYS       , apvendor->discount_days );
	SetField(ID_DIS_PERC       , apvendor->discount_percent );

  if(strcmp(apvendor->type->str(),"C") == 0 )
  {
	SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,TRUE,NULL);
   SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,FALSE,NULL);
  }
  else
  {
   SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,TRUE,NULL);
	SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,FALSE,NULL);
  }
 }
else
MessageBox(HWindow,GetString(6017),szApp,MB_OK);

}

void TAPVendorAdd::Previous(RTMessage)
{
 if(*GetField(ID_VENDORNO) == 0)
 {
  apvendor->top();
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
  return;
 }

 if(apvendor->seek(GetField(ID_VENDORNO)) == 0 && !apvendor->bof())
 {
  apvendor->skip(-1);
  SetField(ID_VENDORNO, apvendor->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
 }
 
}

void TAPVendorAdd::Top(RTMessage)
{
 apvendor->top();
 SetField(ID_VENDORNO, apvendor->vendorno);
 SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
}
void TAPVendorAdd::Bottom(RTMessage)
{
 apvendor->bottom();
 SetField(ID_VENDORNO, apvendor->vendorno);
 SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
}


void TAPVendorAdd::Next(RTMessage)
{
 if(*GetField(ID_VENDORNO) == 0)
 {
  apvendor->top();
  SetField(ID_VENDORNO, apvendor->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
  return;
 }
 if(apvendor->seek(GetField(ID_VENDORNO)) == 0)
 {
  apvendor->skip();
  SetField(ID_VENDORNO, apvendor->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
 } 
}


void TAPVendorAdd::Search  (RTMessage)
{
    VendorList->List->Seek(ID_VENDORNO);
    VendorList->Show(SW_SHOW);
}

void TAPVendorAdd::SetupWindow()
{
 TMainDialog::SetupWindow();
 apvendor = new Tapvendor(&cb,"ap_vend");
 glaccts  = new Tglaccts(&cb,"glaccts");
 apvendor->SetIndex("vendorno");

 VendorList = new TCustomerList(this,"",apvendor);
 VendorList->Attr.Style |= WS_THICKFRAME;
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);

 SetBitmap(ID_SEARCHCUST,"LOOK");
 SetBitmap(CM_NEW,"DEFAULT");
 SetBitmap(ID_SETDATA);

 iVat = 0;
 bVendorList = TRUE;
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("gl_accts");

 if(gl_setat.seek("AP  ") == 0)
 {
  szAPAccount = strcpy(new char[8] ,gl_setat.account->str());
  szAPDept = strcpy(new char[8] ,gl_setat.dept->str());
 }
 else
 {
  szAPAccount = strcpy(new char[8],"");
  szAPDept    = strcpy(new char[5],"");
 }


 if(gl_setat.seek("VATR") == 0)
 {
  szVATAccount = strcpy(new char[8] ,gl_setat.account->str());
  szVATDept    = strcpy(new char[5] ,gl_setat.dept->str());
 }
 else
 {
  szVATAccount = strcpy(new char[8] ,"");
  szVATDept    = strcpy(new char[5] ,"");

 }
 gl_setat.close();
 SetField(ID_APCONTROL , szAPAccount );
 SetField(ID_APDEPART  , szAPDept );
 SetField(ID_VATACCT,szVATAccount);
 SetField(ID_VATDEPT,szVATDept);

}

void TAPVendorBatchAdd::SetupWindow()
{
 TMainDialog::SetupWindow();
 apvendor->SetIndex("vendorno");

 /*
 VendorList = new TCustomerList(this,"AP_VENDORLIST",apvendor);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);
 */

 SetBitmap(ID_SEARCHCUST,"LOOK");
 SetBitmap(CM_NEW,"DEFAULT");
 SetBitmap(ID_SETDATA);
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);

 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("gl_accts");

 if(gl_setat.seek("AP  ") == 0)
 {
  szAPAccount = strcpy(new char[8] ,gl_setat.account->str());
  szAPDept = strcpy(new char[8] ,gl_setat.dept->str());
 }
 else
 {
  szAPAccount = strcpy(new char[8],"");
  szAPDept    = strcpy(new char[5],"");
 }


 if(gl_setat.seek("VATP") == 0)
 {
  szVATAccount = strcpy(new char[8] ,gl_setat.account->str());
  szVATDept    = strcpy(new char[5] ,gl_setat.dept->str());
 }
 else
 {
  szVATAccount = strcpy(new char[8] ,"");
  szVATDept    = strcpy(new char[5] ,"");

 }
 gl_setat.close();
 SetField(ID_APCONTROL      , szAPAccount );
 SetField(ID_APDEPART       , szAPDept );
 SetField(ID_VATACCT,szVATAccount);
 SetField(ID_VATDEPT,szVATDept);

}

void TAPVendorBatchAdd::Ok             (RTMessage Msg)
{

 if(szEnterError(ID_VENDORNO))
  return;

 if(szEnterError(ID_VENDORSHORT))
  return;

 if(szEnterError(ID_VENDORNAME))
  return;

 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 strcpy(szType,"I");
 else
 if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	  strcpy(szType,"C");

 if(apvendor->seek(GetField(ID_VENDORNO))==0)
 {
	if(MessageBox(HWindow,GetString(6037),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
	  AddLine();
	  apvendor->flush();
	  bVendorList = TRUE;
	}
	return;
 }

  apvendor->appendStart();
	AddLine();
  apvendor->append();
  Cancel(Msg);
}

void TAPVendorBatchAdd::Cancel(RTMessage)
{
 VendorList->List->SetPosition();
 ShutDownWindow();
}




void TAPVendorAdd::Ok             (RTMessage)
{

 if(szEnterError(ID_VENDORNO))
  return;

 if(szEnterError(ID_VENDORSHORT))
  return;

 if(szEnterError(ID_VENDORNAME))
  return;

 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 strcpy(szType,"I");
 else
 if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	  strcpy(szType,"C");

 if(apvendor->seek(GetField(ID_VENDORNO))==0)
 {
	if(MessageBox(HWindow,GetString(6037),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
	  AddLine();
	  apvendor->flush();
	  bVendorList = TRUE;
	}
	return;
 }

  apvendor->appendStart();
	AddLine();
  apvendor->append();
  VendorList->List->Redraw();
  SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
}



void TAPVendorAdd::New (RTMessage)
{
 SetField(ID_VENDORNO       , "" );
 SetField(ID_VENDORSHORT    , "" );
 SetField(ID_VENDORNAME     , "" );
 SetField(ID_VENDORADDRESS1 , "" );
 SetField(ID_VENDORADDRESS2 , "" );
 SetField(ID_VENDORADDRESS3 , "" );
 SetField(ID_VENDORPOST     , "" );
 SetField(ID_VENDORTEL      , "" );
 SetField(ID_VENDORCONTACT  , "" );
 SetField(ID_VENDORSTATUS   , "" );
 SetField(ID_VATPERCENTAGE  , "" );
 SetField(ID_TERMS          , "" );
 SetField(ID_REBATE         , "" );
 SetField(ID_PHONE          , "" );
 SetField(ID_FAX            , "" );
 SetField(ID_APCONTROL      , szAPAccount );
 SetField(ID_APDEPART       , szAPDept );
 SetField(ID_VATACCT	    , szVATAccount);
 SetField(ID_VATDEPT	    , szVATDept);
 SetField(ID_MEDREP         , "" );
 SetField(ID_VIN_NUM        , "" );
 SetField(ID_DIS_DAYS       , "");
 SetField(ID_DIS_PERC       , "" );

 SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,FALSE,NULL);
 SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,FALSE,NULL);
}


void TAPVendorAdd::Cancel         (RTMessage)
{
 VendorList->List->SavePosition();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TAPVendorAdd::Delete (RTMessage)
{
 if(apvendor->eof())
	return;
 if(MessageBox(HWindow,GetString(6034),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
  {
  apvendor->deleteRec();
  apvendor->pack();
  VendorList->List->Redraw();
  SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
  }
}

void TAPVendorAdd:: AddLine()
 {
	  apvendor->vendorno->assign(GetField(ID_VENDORNO));
	  apvendor->vdshort->assign(GetField(ID_VENDORSHORT));
	  apvendor->vdname->assign(GetField(ID_VENDORNAME));
	  apvendor->address1->assign(GetField(ID_VENDORADDRESS1));
	  apvendor->address2->assign(GetField(ID_VENDORADDRESS2));
	  apvendor->address3->assign(GetField(ID_VENDORADDRESS3));
	  apvendor->postal->assign(GetField(ID_VENDORPOST));
	  apvendor->contact->assign(GetField(ID_VENDORCONTACT));
	  apvendor->phone->assign(GetField(ID_PHONE));
	  apvendor->fax->assign(GetField(ID_FAX));
	  apvendor->type->assign(szType);
	  apvendor->terms->assignDouble(dGetField(ID_TERMS));
	  apvendor->account->assign(GetField(ID_APCONTROL));
	  apvendor->depart->assign(GetField(ID_APDEPART));
	  apvendor->percentage->assignDouble(dGetField(ID_VATPERCENTAGE));
	  apvendor->rebate->assignDouble(dGetField(ID_REBATE));
	  apvendor->vat_acct->assign(GetField(ID_VATACCT));
	  apvendor->vat_dept->assign(GetField(ID_VATDEPT));
	  apvendor->vin->assign(GetField(ID_VIN_NUM));
	  apvendor->medrep->assign(GetField(ID_MEDREP));
	  apvendor->discount_days->assignDouble(dGetField(ID_DIS_DAYS));
	  apvendor->discount_percent->assignDouble(dGetField(ID_DIS_PERC));
	  apvendor->UpdateUserTime(szUser);
 }



// Maintain Vendor Accounts
void TManilaGL::APAddVendors  (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPVendorAdd(this,1041));
 }
void TManilaGL::APModVendors  (RTMessage)
 {
  //GetApplication()->ExecDialog(new TAPVendorMod(this,1041));
 }

void TManilaGL::APDelVendors  (RTMessage)
 {
  //GetApplication()->ExecDialog(new TAPVendorDel(this,1041));
 }
