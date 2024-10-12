/*
	Accounts Receivable  - (C) Copyright 1992 by Tegara Corporation
*/
#include "std_inc.h"




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
#define ID_VATACCT      8081
#define ID_VATDEPT      8082







int Tarcust::open( Code4& cb )
{
	Data4::open( cb,GetDbfName()) ;

	if( cb.errorCode != 0 )
	{
		if( isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
}




_CLASSDEF (TARVendorAdd)
class TARVendorAdd : public TMainDialog
{
 public:
 TARCustomerList *CustomerList;
 Tarcust   arcust;
 int iVat;
 TARVendorAdd(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
  CustomerList = NULL;
 }
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_VIEW];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void HandleVat      (RTMessage Msg) = [ID_FIRST + ID_VAT];
 virtual void Next	        (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous	     (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Find	        (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void SetDetail      (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR]{SetData();}
 virtual void Top            (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom 	     (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void New            (RTMessage Msg) = [ID_FIRST + CM_NEW];
 virtual void NewItem        (RTMessage Msg) = [ID_FIRST + ID_NEW];
 virtual void Options	     (RTMessage Msg) = [ID_FIRST + IDMAINOPTIONS];
 virtual void Clear          (RTMessage Msg) = [CM_FIRST + CM_NEW]{ New(Msg); }
 void Update();
 void SetData();
 void UpdateScreen();
 ~TARVendorAdd()
 {
  if(CustomerList != NULL)
	 delete CustomerList;
 }
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
  gl_setat.open(*cb);
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
  if(gl_setat.seek("WTXI") == 0)
  {
	SetField(ID_WITHTAX      , gl_setat.account.str() );
	SetField(ID_WITHDEPT     , gl_setat.dept.str() );
  }
  gl_setat.close();
}

void TARVendorOptions::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetDefaultAccounts();
 if(strlen(StrTrim(arcust->account.str())) > 0)
 {
  SetField(ID_AR_ACCT	    , arcust->ar_acct );
  SetField(ID_AR_DEPT	    , arcust->ar_dept );

 }
 if(strlen(StrTrim(arcust->ar_acct.str())) > 0)
 {
  SetField(ID_APCONTROL	    , arcust->account );
  SetField(ID_APDEPART       , arcust->depart );

 }

 if(strlen(StrTrim(arcust->vat_acct.str())) > 0)
 {
  SetField(ID_VATACCT	    , arcust->vat_acct );
  SetField(ID_VATDEPART	    , arcust->vat_dept );
 }

 if(strlen(StrTrim(arcust->withtax.str())) > 0)
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
 CHourGlass wait;
 Str4large szSales;
 szSales.assign(GetField(ID_APCONTROL));
 szSales.add(GetField(ID_APDEPART));

 if(!ValidGLAccount(szSales.str()))
  {
	 MessageBox(HWindow,GetString(4017),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_APCONTROL));
	 return;
  }

 Str4large szVat;
 szVat.assign(GetField(ID_VATACCT));
 szVat.add(GetField(ID_VATDEPT));

 if(!ValidGLAccount(szVat.str()))
  {
	 MessageBox(HWindow,GetString(4017),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	 return;
  }

 Str4large szAR;
 szAR.assign(GetField(ID_AR_ACCT));
 szAR.add(GetField(ID_AR_DEPT));

 if(!ValidGLAccount(szAR.str()))
  {
	 MessageBox(HWindow,GetString(4017),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_AR_ACCT));
	 return;
  }

 if(MessageBox(HWindow,GetString(1093),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION) == IDYES)
  {
	  arcust->terms.assignDouble(dGetField(ID_TERMS));
	  arcust->account.assign	(GetField(ID_APCONTROL));
	  arcust->depart.assign	(GetField(ID_APDEPART));
	  arcust->ar_acct.assign	(GetField(ID_AR_ACCT));
	  arcust->ar_dept.assign	(GetField(ID_AR_DEPT));
	  arcust->vat_acct.assign (GetField(ID_VATACCT));
	  arcust->vat_dept.assign (GetField(ID_VATDEPT));
	  arcust->withtax.assign  (GetField(ID_WITHTAX));
	  arcust->withdept.assign (GetField(ID_WITHDEPT));
	  arcust->vat.assignDouble(dGetField(ID_VAT));
	  arcust->contact.assign  (GetField(ID_VENDORCONTACT));
	  arcust->vin.assign      (GetField(ID_VIN_NUM));
	  arcust->UpdateUserTime(szUser);
	  arcust->flush();

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
	MessageBox(HWindow,GetString(4119),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }
 else if (arcust.seek(GetField(ID_VENDORNO)) == 0)
	GetApplication()->ExecDialog(new TARVendorOptions(this,1063,&arcust));
}


void TARVendorAdd::Top(RTMessage)
{
 arcust.top();
 UpdateScreen();
}

void TARVendorAdd::Bottom(RTMessage)
{
 arcust.bottom();
 UpdateScreen();
}

void TARVendorAdd::UpdateScreen()
{
  SetField(ID_VENDORNO       , arcust.vendorno );
  SetField(ID_VENDORSHORT    , arcust.vdshort );
  SetField(ID_VENDORNAME     , arcust.vdname );
  SetField(ID_VENDORADDRESS1 , arcust.address1 );
  SetField(ID_VENDORADDRESS2 , arcust.address2 );
  SetField(ID_VENDORADDRESS3 , arcust.address3 );
  SetField(ID_VENDORPOST     , arcust.postal );
  SetField(ID_PHONE	        , arcust.phone );
  SetField(ID_FAX	           , arcust.fax );
  SetField(ID_S_ADDRESS1     , arcust.s_add1 );
  SetField(ID_S_ADDRESS2     , arcust.s_add2 );
  SetField(ID_S_ADDRESS3     , arcust.s_add3 );
  SetField(ID_S_POSTAL       , arcust.s_postal );
  SetField(ID_S_PHONE	     , arcust.s_phone );
  SetField(ID_S_FAX	        , arcust.s_fax );
}


void TARVendorAdd::SetData()
{
  if(arcust.seek(GetField(ID_VENDORNO)) == 0)
	UpdateScreen();
}

void TARVendorAdd::Find(RTMessage)
{
 if(arcust.seek(GetField(ID_VENDORNO)) == 0)
 {
  UpdateScreen();
 }
else
 // Customer number not found.
MessageBox(HWindow,GetString(6017),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
}

void TARVendorAdd::Previous(RTMessage)
{
 if(strlen(StrTrim(GetField(ID_VENDORNO))) == 0)
 {
  arcust.top();
  SetData();
  return;
 }

 if(arcust.seek(GetField(ID_VENDORNO)) == 0 && !arcust.bof())
 {
  arcust.skip(-1);
  if(arcust.bof())
	arcust.top();
  SetField(ID_VENDORNO, arcust.vendorno);
  UpdateScreen();
 }

}

void TARVendorAdd::Delete(RTMessage)
{
 if(strlen(StrTrim(GetField(ID_VENDORNO))) == 0)
  {
	MessageBox(HWindow,GetString(6018),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	return;
  }
 if(arcust.eof())
  return;
 if(MessageBox(HWindow,GetString(6094),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION)==IDYES)
  {
	if(arcust.seek(GetField(ID_VENDORNO)) == 0)
	 {
	  arcust.deleteRec();
	  arcust.pack();
	  SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
	  CustomerList->List->Redraw();
	 }
  }
}

void TARVendorAdd::Next(RTMessage)
{
 if(*GetField(ID_VENDORNO) == 0)
 {
  arcust.top();
  SetField(ID_VENDORNO, arcust.vendorno);
  UpdateScreen();
  return;
 }
 if(arcust.seek(GetField(ID_VENDORNO)) == 0)
 {
  arcust.skip();
  if(arcust.eof())
	arcust.bottom();
  SetField(ID_VENDORNO, arcust.vendorno);
  UpdateScreen();

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
  if(CustomerList == NULL)
  {
	CHourGlass wait;
	CustomerList = new TARCustomerList(this,"");
	CustomerList->EnableAutoCreate();
	GetApplication()->MakeWindow(CustomerList);
  }
 // CustomerList->List->Seek(ID_VENDORNO);
  CustomerList->Show(SW_SHOW);
}

void TARVendorAdd::SetupWindow()
{
 TMainDialog::SetupWindow();

 if(arcust.open(*cb) < 0)
  {
	ErrorClose(arcust.GetDbfName());
	return;
  }
 /*
 arcust.top();
 while(!arcust.eof())
 {
  SendDlgItemMsg(8000,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)arcust.vendorno->str());
  arcust.skip();
 }
*/

 /*
 CustomerList = new TARCustomerList(this,"",&arcust);
 CustomerList->EnableAutoCreate();
 GetApplication()->MakeWindow(CustomerList);
 */

 iVat = 0;
 SetBitmap(ID_NEW);
 SetBitmap(IDMAINOPTIONS);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_SETDATA);
}




void TARVendorAdd::Update()
{
	  arcust.vendorno.assign(GetField(ID_VENDORNO));
	  arcust.vdshort.assign(GetField(ID_VENDORSHORT));
	  arcust.vdname.assign(GetField(ID_VENDORNAME));
	  arcust.address1.assign(GetField(ID_VENDORADDRESS1));
	  arcust.address2.assign(GetField(ID_VENDORADDRESS2));
	  arcust.address3.assign(GetField(ID_VENDORADDRESS3));
	  arcust.postal.assign(GetField(ID_VENDORPOST));
	  arcust.contact.assign(GetField(ID_VENDORCONTACT));
	  arcust.phone.assign(GetField(ID_PHONE));
	  arcust.fax.assign(GetField(ID_FAX));
	  arcust.s_add1.assign(GetField(ID_S_ADDRESS1));
	  arcust.s_add2.assign(GetField(ID_S_ADDRESS2));
	  arcust.s_add3.assign(GetField(ID_S_ADDRESS3));
	  arcust.s_postal.assign(GetField(ID_S_POSTAL));
	  arcust.s_phone.assign(GetField(ID_S_PHONE));
	  arcust.s_fax.assign(GetField(ID_S_FAX));
	  arcust.UpdateUserTime(szUser);
}


void TARVendorAdd::Ok (RTMessage Msg)
{
 CHourGlass wait;
 if(szEnterError(ID_VENDORNO))
  return;
 if(szEnterError(ID_VENDORSHORT))
  return;
 if(szEnterError(ID_VENDORNAME))
  return;
 if(arcust.seek(GetField(ID_VENDORNO))==0)
 {
	if(MessageBox(HWindow,GetString(1093),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
	  SetField(ID_VENDORNO,arcust.vendorno.str());
	  Update();
	}
 }
 else
 {
	 arcust.appendStart();
	  {
		Update();
	  }
	 arcust.append();
	 New(Msg);
 }
 arcust.top();
 if(CustomerList != NULL)
	 CustomerList->List->Redraw();
 SetFocus(GetDlgItem(HWindow,4001));
}

void TARVendorAdd::NewItem (RTMessage Msg)
{
  New(Msg);
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
 if(CustomerList != NULL)
	 CustomerList->List->SavePosition();
 arcust.close();
 ShutDownWindow();
}




     
// Maintain Vendor Accounts
void TManilaGL::ARAddVendors  (RTMessage)
 {
  GetApplication()->ExecDialog(new TARVendorAdd(this,1052));
 }


