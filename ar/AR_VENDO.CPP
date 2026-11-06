/*
   Accounts Receivable  - (C) Copyright 1992 by Tegara Corporation
*/
#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include <math.h>
#include "db_vend.h"
#include "gl_dlg.h"
#include "ar_vendl.h"
#include "ar_custl.h"
#include "gl_setat.hpp"

#define ID_APPEND 4015
#define ID_S_PHONE    1076
#define ID_S_FAX      1077	
#define ID_S_ADDRESS1 1078
#define ID_S_ADDRESS2 1079
#define ID_S_ADDRESS3 1080
#define ID_S_POSTAL   1081
#define ID_VATACCT    1082
#define ID_VATDEPART  1083
#define ID_VIN_NUM    1005



extern Code4 cb;
static int iVendorList;
extern char szApp[20];


void TAPVendorList::SetParentField(int ID, Field4 *field)
{
 SendDlgItemMessage(Parent->HWindow,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str()); 
}


_CLASSDEF (TAPVendorAdd)
class TAPVendorAdd : public TMainDialog
{
 public:
 BOOL bVendorList;
 //PTAPVendorList VendorList;
 TCustomerList *VendorList;
 PTapvendor apvendor;
 PTglaccts  glaccts;
 int iVat; 
 TAPVendorAdd(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
 };
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void HandleVat      (RTMessage Msg) = [ID_FIRST + ID_VAT];
 virtual void Next	     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous	     (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Find	     (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void SetDetail      (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR]{SetData();};
 virtual void Top            (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom 	     (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void New            (RTMessage Msg) = [ID_FIRST + CM_NEW];
 virtual void Options	     (RTMessage Msg) = [ID_FIRST + IDMAINOPTIONS];
 virtual void Clear          (RTMessage Msg) = [CM_FIRST + CM_NEW]{ New(Msg); };
 void Update();         
 void SetData();
					  
};

_CLASSDEF (TAPVendorOptions)
class TAPVendorOptions : public TMainDialog
{
 public:
 PTapvendor arcust;
 TAPVendorOptions(PTWindowsObject AParent, LPSTR AName,PTapvendor ar): TMainDialog (AParent,AName)
 {
  arcust =ar;
 };
  virtual void SetupWindow();  
  virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void SetDefaultAccounts(void);
};

void TAPVendorOptions::SetDefaultAccounts(void)
{
  GL_SETAT gl_setat;
  gl_setat.open(cb);
  gl_setat.SetIndex("gl_accts");
  if(gl_setat.seek("SALE") == 0)
  {
  
  SetField(ID_AR_ACCT	    , gl_setat.account.str() );
  SetField(ID_AR_DEPT	    , gl_setat.dept.str() );

  }
  if(gl_setat.seek("AR  ") == 0)
  {
   SetField(ID_APCONTROL    , gl_setat.account.str() );
   SetField(ID_APDEPART     , gl_setat.dept.str() );
  }
  if(gl_setat.seek("VATP") == 0)
  {
   SetField(ID_VATACCT	    , gl_setat.account.str() );
   SetField(ID_VATDEPART    , gl_setat.dept.str() );
  }
  gl_setat.close();
}

void TAPVendorOptions::SetupWindow()
{
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
  SetField(ID_VAT            , arcust->vat );
  SetField(ID_VENDORCONTACT  , arcust->contact );
  SetField(ID_TERMS          , arcust->terms );
  SetField(ID_VIN_NUM        , arcust->vin );
}

void TAPVendorOptions::Ok(RTMessage)
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
 szVat.add(GetField(ID_VATDEPART));

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

 if(MessageBox(HWindow,GetString(ID_APPEND),szApp,MB_YESNO|MB_ICONQUESTION) == IDYES)
  {
     arcust->terms->assignDouble(dGetField(ID_TERMS));
     arcust->account->assign(GetField(ID_APCONTROL));
     arcust->depart->assign(GetField(ID_APDEPART));
     arcust->ar_acct->assign(GetField(ID_AR_ACCT));
     arcust->ar_dept->assign(GetField(ID_AR_DEPT));
     arcust->vat_acct->assign(GetField(ID_VATACCT));
     arcust->vat_dept->assign(GetField(ID_VATDEPART));
     arcust->vat->assignDouble(dGetField(ID_VAT));
     arcust->contact->assign(GetField(ID_VENDORCONTACT));
     arcust->vin->assign(GetField(ID_VIN_NUM));
     arcust->flush();
     arcust->unlock();
     ShutDownWindow();
  }

}

void TAPVendorOptions::Cancel(RTMessage)
{
 ShutDownWindow();
}

void TAPVendorAdd::Options(RTMessage)
{
 if(strcmp(StrTrim(GetField(ID_VENDORNO)),"") == 0)
  {
   MessageBox(HWindow,GetString(4119),szApp,MB_ICONINFORMATION);
   return;
  }
 else if (apvendor->seek(GetField(ID_VENDORNO)) == 0)
 GetApplication()->ExecDialog(new TAPVendorOptions(this,"AR_VENDOROPTIONS",apvendor));
}


void TAPVendorAdd::Top(RTMessage)
{
 apvendor->top();
 SetData();
}

void TAPVendorAdd::Bottom(RTMessage)
{
 apvendor->bottom();
 SetData();
}


void TAPVendorAdd::SetData()
{

  SetField(ID_VENDORNO       , apvendor->vendorno );
  SetField(ID_VENDORSHORT    , apvendor->vdshort );
  SetField(ID_VENDORNAME     , apvendor->vdname );
  SetField(ID_VENDORADDRESS1 , apvendor->address1 );
  SetField(ID_VENDORADDRESS2 , apvendor->address2 );
  SetField(ID_VENDORADDRESS3 , apvendor->address3 );
  SetField(ID_VENDORPOST     , apvendor->postal );
  SetField(ID_PHONE	     , apvendor->phone );
  SetField(ID_FAX	     , apvendor->fax );
  SetField(ID_S_ADDRESS1     , apvendor->s_add1 );
  SetField(ID_S_ADDRESS2     , apvendor->s_add2 );
  SetField(ID_S_ADDRESS3     , apvendor->s_add3 );
  SetField(ID_S_POSTAL       , apvendor->s_postal );
  SetField(ID_S_PHONE	     , apvendor->s_phone );
  SetField(ID_S_FAX	     , apvendor->s_fax );
}

void TAPVendorAdd::Find(RTMessage)
{
 char *szVendorCode = strcpy(new char[(int)apvendor->vendorno->len() + 1],GetField(ID_VENDORNO));
 if(apvendor->seek(szVendorCode) == 0)
 {
  SetData();
 }
else
MessageBox(HWindow,"Customer Number not found.",szApp,MB_OK);
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
  if(apvendor->bof())
   apvendor->top();
  SetField(ID_VENDORNO, apvendor->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
 }
 
}

void TAPVendorAdd::Delete(RTMessage)
{
 if(strlen(StrTrim(GetField(ID_VENDORNO))) == 0)
  {
   MessageBox(HWindow,"Enter a vendor code.",szApp,MB_OK);
   return;
  }
 if(apvendor->eof())
  return;
 if(MessageBox(HWindow,GetString(1096),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
  {
   if(apvendor->seek(GetField(ID_VENDORNO)) == 0)
    {
     apvendor->deleteRec();
     apvendor->pack();  
     SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
     VendorList->List->Redraw("ar_cust");
    }
    else
    MessageBox(HWindow,"Vendor code not found.",szApp,MB_OK);
  }
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
  if(apvendor->eof())
   apvendor->bottom();
  SetField(ID_VENDORNO, apvendor->vendorno);
  SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
 } 
}



void TAPVendorAdd::HandleVat (RTMessage)
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

 /*
 apvendor->top();
 while(!apvendor->eof())
 {
  SendDlgItemMsg(8000,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)apvendor->vendorno->str());
  apvendor->skip();
 }
*/

 VendorList = new TCustomerList(this,"",apvendor);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);


 iVat = 0;
 bVendorList = TRUE;
 SetBitmap(4082,"NEW");
 SetBitmap(3043,"LOOK");
 SetBitmap(4076,"SEEK");
 SetBitmap(IDMAINOPTIONS,"OPTIONS");
}




void TAPVendorAdd::Update()
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
     apvendor->s_add1->assign(GetField(ID_S_ADDRESS1));
     apvendor->s_add2->assign(GetField(ID_S_ADDRESS2));
     apvendor->s_add3->assign(GetField(ID_S_ADDRESS3));
     apvendor->s_postal->assign(GetField(ID_S_POSTAL));
     apvendor->s_phone->assign(GetField(ID_S_PHONE));
     apvendor->s_fax->assign(GetField(ID_S_FAX));

}


void TAPVendorAdd::Ok             (RTMessage)
{
 if(szEnterError(ID_VENDORNO))
  return;
 if(szEnterError(ID_VENDORSHORT))
  return;
 if(szEnterError(ID_VENDORNAME))
  return;


 if(apvendor->seek(GetField(ID_VENDORNO))==0)
 {
   if(MessageBox(HWindow,GetString(1093),szApp,MB_YESNO|MB_ICONQUESTION)==IDYES)
   {
     Update();
     apvendor->flush();
     bVendorList = TRUE;
   }
   
  
 }
 else
 {
    apvendor->appendStart();
     {
      Update();
      apvendor->terms->assignDouble(0);
      apvendor->account->assign("");
      apvendor->depart->assign("");
      apvendor->ar_acct->assign("");
      apvendor->ar_dept->assign("");
      apvendor->vat_acct->assign("");
      apvendor->vat_dept->assign("");
      apvendor->vat->assignDouble(0);
      apvendor->contact->assign("");
      apvendor->vin->assign("");
     }
    apvendor->append();
 }
 apvendor->top();
 VendorList->List->Redraw("ar_cust");
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
 SetField(ID_PHONE          , "" );
 SetField(ID_FAX            , "" );
 SetField(ID_S_ADDRESS1     , "" );
 SetField(ID_S_ADDRESS2     , "" );
 SetField(ID_S_ADDRESS3     , "" );
 SetField(ID_S_POSTAL       , "" );
 SetField(ID_S_PHONE	    , "" );
 SetField(ID_S_FAX	    , "" );
}

     
void TAPVendorAdd::Cancel         (RTMessage)
{
 cb.closeAll();
 ShutDownWindow();
}




     
// Maintain Vendor Accounts
void TGlWindow::APAddVendors  (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPVendorAdd(this,"AR_ADDVENDORS"));
 }

void TGlWindow::APModVendors  (RTMessage){}

void TGlWindow::APDelVendors  (RTMessage){}
