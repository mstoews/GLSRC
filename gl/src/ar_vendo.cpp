/*
	Accounts Receivable  - (C) Copyright 1992 by Tegara Corporation
*/
#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"

#include <math.h>
#include "db_cust.h"
#include "gl.h"
#include "gl_dlg.h"
#include "ar_custl.h"
#include "gl_setat.hpp"

#define ID_APPEND 4015
#define ID_S_PHONE    1076
#define ID_S_FAX      1077
#define ID_S_ADDRESS1 1078
#define ID_S_ADDRESS2 1079
#define ID_S_ADDRESS3 1080
#define ID_S_POSTAL   1081
#define ID_VATDEPART  1083
#define ID_VIN_NUM    1005
#define ID_WITHTAX    8083
#define ID_WITHDEPT   8084



extern Code4 cb;
extern char szApp[20];



_CLASSDEF (TARVendorAdd)
class TARVendorAdd : public TMainDialog
{
 public:
 BOOL bVendorList;
 //PTARVendorList VendorList;
 TARCustomerList *VendorList;
 Tarcust   *arcust;
 Tglaccts  *glaccts;
 int iVat;
 TARVendorAdd(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
 };
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_VIEW];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void HandleVat      (RTMessage Msg) = [ID_FIRST + ID_VAT];
 virtual void Next	        (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous	     (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Find	        (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void SetDetail      (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR]{SetData();};
 virtual void Top            (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom 	     (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void New            (RTMessage Msg) = [ID_FIRST + CM_NEW];
 virtual void Options	     (RTMessage Msg) = [ID_FIRST + IDMAINOPTIONS];
 virtual void Clear          (RTMessage Msg) = [CM_FIRST + CM_NEW]{ New(Msg); };
 void Update();
 void SetData();

};

_CLASSDEF (TARVendorOptions)
class TARVendorOptions : public TMainDialog
{
 public:
 PTarcust arcust;
 TARVendorOptions(PTWindowsObject AParent,int AName,PTarcust ar): TMainDialog (AParent,AName)
 {
  arcust =ar;
 };
  virtual void SetupWindow();  
  virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void SetDefaultAccounts(void);
};

void TARVendorOptions::SetDefaultAccounts(void)
{
  GL_SETAT gl_setat;
  gl_setat.open(cb);
  gl_setat.SetIndex("gl_accts");
  if(gl_setat.seek("SALE") == 0)
  {
  
  SetField(ID_AR_ACCT	    , gl_setat.account->str() );
  SetField(ID_AR_DEPT	    , gl_setat.dept->str() );

  }
  if(gl_setat.seek("AR  ") == 0)
  {
	SetField(ID_APCONTROL    , gl_setat.account->str() );
	SetField(ID_APDEPART     , gl_setat.dept->str() );
  }
  if(gl_setat.seek("VATP") == 0)
  {
	SetField(ID_VATACCT	    , gl_setat.account->str() );
	SetField(ID_VATDEPART    , gl_setat.dept->str() );
  }
  if(gl_setat.seek("WTXI") == 0)
  {
	SetField(ID_WITHTAX      , gl_setat.account->str() );
	SetField(ID_WITHDEPT     , gl_setat.dept->str() );
  }
  gl_setat.close();
}

void TARVendorOptions::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetDefaultAccounts();
 if(strlen(StrTrim(arcust->account->str())) > 0)
 {
  SetField(ID_AR_ACCT	    , arcust->ar_acct );
  SetField(ID_AR_DEPT	    , arcust->ar_dept );

 }
 if(strlen(StrTrim(arcust->ar_acct->str())) > 0)
 {
  SetField(ID_APCONTROL	    , arcust->account );
  SetField(ID_APDEPART       , arcust->depart );

 }

 if(strlen(StrTrim(arcust->vat_acct->str())) > 0)
 {
  SetField(ID_VATACCT	    , arcust->vat_acct );
  SetField(ID_VATDEPART	    , arcust->vat_dept );
 }

 if(strlen(StrTrim(arcust->withtax->str())) > 0)
 {
  SetField(ID_WITHTAX	    , arcust->withtax  );
  SetField(ID_WITHDEPT	    , arcust->withdept );
 }

  SetField(ID_VAT            , arcust->vat );
  SetField(ID_VENDORCONTACT  , arcust->contact );
  SetField(ID_TERMS          , arcust->terms );
  SetField(ID_VIN_NUM        , arcust->vin );
}

void TARVendorOptions::Ok(RTMessage)
{

 Str4large szSales;
 szSales.assign(GetField(ID_APCONTROL));
 szSales.add(GetField(ID_APDEPART));

 if(!ValidGLAccount(szSales.str()))
  {
	 MessageBox(HWindow,GetString(4017),szApp,MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_APCONTROL));
	 return;
  }

 Str4large szVat;
 szVat.assign(GetField(ID_VATACCT));
 szVat.add(GetField(ID_VATDEPT));

 if(!ValidGLAccount(szVat.str()))
  {
	 MessageBox(HWindow,GetString(4017),szApp,MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	 return;
  }

 Str4large szAR;
 szAR.assign(GetField(ID_AR_ACCT));
 szAR.add(GetField(ID_AR_DEPT));

 if(!ValidGLAccount(szAR.str()))
  {
	 MessageBox(HWindow,GetString(4017),szApp,MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_AR_ACCT));
	 return;
  }

 if(MessageBox(HWindow,GetString(1093),szApp,MB_YESNO|MB_ICONQUESTION) == IDYES)
  {
	  arcust->terms->assignDouble(dGetField(ID_TERMS));
	  arcust->account->assign	(GetField(ID_APCONTROL));
	  arcust->depart->assign	(GetField(ID_APDEPART));
	  arcust->ar_acct->assign	(GetField(ID_AR_ACCT));
	  arcust->ar_dept->assign	(GetField(ID_AR_DEPT));
	  arcust->vat_acct->assign (GetField(ID_VATACCT));
	  arcust->vat_dept->assign (GetField(ID_VATDEPT));
	  arcust->withtax->assign  (GetField(ID_WITHTAX));
	  arcust->withdept->assign (GetField(ID_WITHDEPT));
	  arcust->vat->assignDouble(dGetField(ID_VAT));
	  arcust->contact->assign  (GetField(ID_VENDORCONTACT));
	  arcust->vin->assign      (GetField(ID_VIN_NUM));
	  arcust->UpdateUserTime(szUser);
	  arcust->flush();
	  arcust->unlock();
	  ShutDownWindow();
  }

}

void TARVendorOptions::Cancel(RTMessage)
{
 ShutDownWindow();
}

void TARVendorAdd::Options(RTMessage)
{
 if(strcmp(StrTrim(GetField(ID_VENDORNO)),"") == 0)
  {
   MessageBox(HWindow,GetString(4119),szApp,MB_ICONINFORMATION);
   return;
  }
 else if (arcust->seek(GetField(ID_VENDORNO)) == 0)
 GetApplication()->ExecDialog(new TARVendorOptions(this,1063,arcust));
}


void TARVendorAdd::Top(RTMessage)
{
 arcust->top();
 SetData();
}

void TARVendorAdd::Bottom(RTMessage)
{
 arcust->bottom();
 SetData();
}


void TARVendorAdd::SetData()
{

  SetField(ID_VENDORNO       , arcust->vendorno );
  SetField(ID_VENDORSHORT    , arcust->vdshort );
  SetField(ID_VENDORNAME     , arcust->vdname );
  SetField(ID_VENDORADDRESS1 , arcust->address1 );
  SetField(ID_VENDORADDRESS2 , arcust->address2 );
  SetField(ID_VENDORADDRESS3 , arcust->address3 );
  SetField(ID_VENDORPOST     , arcust->postal );
  SetField(ID_PHONE	     , arcust->phone );
  SetField(ID_FAX	     , arcust->fax );
  SetField(ID_S_ADDRESS1     , arcust->s_add1 );
  SetField(ID_S_ADDRESS2     , arcust->s_add2 );
  SetField(ID_S_ADDRESS3     , arcust->s_add3 );
  SetField(ID_S_POSTAL       , arcust->s_postal );
  SetField(ID_S_PHONE	     , arcust->s_phone );
  SetField(ID_S_FAX	     , arcust->s_fax );
}

void TARVendorAdd::Find(RTMessage)
{
 char *szVendorCode = strcpy(new char[(int)arcust->vendorno->len() + 1],GetField(ID_VENDORNO));
 if(arcust->seek(szVendorCode) == 0)
 {
  SetData();
 }
else
// Customer number not found.
MessageBox(HWindow,GetString(6017),szApp,MB_OK);
}

void TARVendorAdd::Previous(RTMessage)
{
 if(*GetField(ID_VENDORNO) == 0)
 {
  arcust->top();
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
  return;
 }

 if(arcust->seek(GetField(ID_VENDORNO)) == 0 && !arcust->bof())
 {
  arcust->skip(-1);
  if(arcust->bof())
	arcust->top();
  SetField(ID_VENDORNO, arcust->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
 }

}

void TARVendorAdd::Delete(RTMessage)
{
 if(strlen(StrTrim(GetField(ID_VENDORNO))) == 0)
  {
	MessageBox(HWindow,GetString(6018),szApp,MB_OK);
	return;
  }
 if(arcust->eof())
  return;
 if(MessageBox(HWindow,GetString(6094),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
  {
   if(arcust->seek(GetField(ID_VENDORNO)) == 0)
    {
     arcust->deleteRec();
     arcust->pack();  
     SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
     VendorList->List->Redraw();
    }
    else
    MessageBox(HWindow,"Vendor code not found.",szApp,MB_OK);
  }
}

void TARVendorAdd::Next(RTMessage)
{
 if(*GetField(ID_VENDORNO) == 0)
 {
  arcust->top();
  SetField(ID_VENDORNO, arcust->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
  return;
 }
 if(arcust->seek(GetField(ID_VENDORNO)) == 0)
 {
  arcust->skip();
  if(arcust->eof())
   arcust->bottom();
  SetField(ID_VENDORNO, arcust->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
 } 
}



void TARVendorAdd::HandleVat (RTMessage)
{
 if(SendDlgItemMsg(ID_VAT,BM_GETCHECK,NULL,NULL))
  {
   iVat = 1;
  }
 else
  {
   iVat = 0;
  }
}


void TARVendorAdd::Search  (RTMessage)
{
  VendorList->List->Seek(ID_VENDORNO);
  VendorList->Show(SW_SHOW);
}

void TARVendorAdd::SetupWindow()
{
 TMainDialog::SetupWindow();
 arcust = new Tarcust(&cb,"ap_vend");
 glaccts  = new Tglaccts(&cb,"glaccts");
 arcust->SetIndex("vendorno");

 /*
 arcust->top();
 while(!arcust->eof())
 {
  SendDlgItemMsg(8000,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)arcust->vendorno->str());
  arcust->skip();
 }
*/

 VendorList = new TARCustomerList(this,"",arcust);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);


 iVat = 0;
 bVendorList = TRUE;
 SetBitmap(ID_NEW);
 SetBitmap(IDMAINOPTIONS);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_SETDATA);
}




void TARVendorAdd::Update()
{
     arcust->vendorno->assign(GetField(ID_VENDORNO));
     arcust->vdshort->assign(GetField(ID_VENDORSHORT));
     arcust->vdname->assign(GetField(ID_VENDORNAME));
     arcust->address1->assign(GetField(ID_VENDORADDRESS1));
     arcust->address2->assign(GetField(ID_VENDORADDRESS2));
     arcust->address3->assign(GetField(ID_VENDORADDRESS3));
     arcust->postal->assign(GetField(ID_VENDORPOST));
     arcust->contact->assign(GetField(ID_VENDORCONTACT));
     arcust->phone->assign(GetField(ID_PHONE));
     arcust->fax->assign(GetField(ID_FAX));
     arcust->s_add1->assign(GetField(ID_S_ADDRESS1));
     arcust->s_add2->assign(GetField(ID_S_ADDRESS2));
     arcust->s_add3->assign(GetField(ID_S_ADDRESS3));
     arcust->s_postal->assign(GetField(ID_S_POSTAL));
     arcust->s_phone->assign(GetField(ID_S_PHONE));
	  arcust->s_fax->assign(GetField(ID_S_FAX));
	  arcust->UpdateUserTime(szUser);

}


void TARVendorAdd::Ok             (RTMessage)
{
 if(szEnterError(ID_VENDORNO))
  return;
 if(szEnterError(ID_VENDORSHORT))
  return;
 if(szEnterError(ID_VENDORNAME))
  return;


 if(arcust->seek(GetField(ID_VENDORNO))==0)
 {
	if(MessageBox(HWindow,GetString(1093),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
	  Update();
	  arcust->flush();
	  bVendorList = TRUE;
	}


 }
 else
 {
    arcust->appendStart();
     {
      Update();
      arcust->terms->assignDouble(0);
      arcust->account->assign("");
      arcust->depart->assign("");
      arcust->ar_acct->assign("");
      arcust->ar_dept->assign("");
      arcust->vat_acct->assign("");
		arcust->vat_dept->assign("");
		arcust->withtax->assign("");
		arcust->withdept->assign("");
      arcust->vat->assignDouble(0);
      arcust->contact->assign("");
		arcust->vin->assign("");
		arcust->UpdateUserTime(szUser);
	  }
	 arcust->append();
 }
 arcust->top();
 VendorList->List->Redraw();
 //SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
}

void TARVendorAdd::New (RTMessage)
{
 SetField(ID_VENDORNO       , "" );
 SetField(ID_VENDORSHORT    , "" );
 SetField(ID_VENDORNAME     , "" );
 SetField(ID_VENDORADDRESS1 , "" );
 SetField(ID_VENDORADDRESS2 , "" );
 SetField(ID_VENDORADDRESS3 , "" );
 SetField(ID_VENDORPOST     , "" );
 SetField(ID_PHONE          , "" );
 SetField(ID_FAX            , "" );
 SetField(ID_S_ADDRESS1     , "" );
 SetField(ID_S_ADDRESS2     , "" );
 SetField(ID_S_ADDRESS3     , "" );
 SetField(ID_S_POSTAL       , "" );
 SetField(ID_S_PHONE	    , "" );
 SetField(ID_S_FAX	    , "" );
}

     
void TARVendorAdd::Cancel         (RTMessage)
{
 VendorList->List->SavePosition();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}




     
// Maintain Vendor Accounts
void TManilaGL::ARAddVendors  (RTMessage)
 {
  GetApplication()->ExecDialog(new TARVendorAdd(this,1052));
 }


