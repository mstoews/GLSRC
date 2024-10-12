/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Modify, add and delete vendor AP accounts functions.
*/
#include "std_inc.h"
#include "ap_vendo.h"


#define ID_DIS_DAYS 1037
#define ID_DIS_PERC 1038
#define ID_VIN_NUM  4010
#define ID_VAT_NUM  1039

#define ID_VATACCT      8081
#define ID_VATDEPT      8082
#define WM_OPEN_CHILDDLG 3066
#define POSITION 12

BOOL TAPVendorAdd::SetField(int ID, Field4 *field,HWND HWindow)
{

 string szClassName;
 szClassName.resize(40);
 ::GetClassName(GetDlgItem(HWindow,ID),(LPSTR)szClassName.c_str(),szClassName.length());
 szClassName.to_upper();
 if(szClassName == "TBFLOAT")
  {
	double d = atof(field->str());
	FloatSetDlgItemValue(HWindow,ID,d);
  }
 else
  if(szClassName == "TBINT")
  {
	int i = atoi(field->str());
	IntSetDlgItemValue(Parent->HWindow,ID,i);
  }
 else
  {
	SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  }
  return TRUE;
}

BOOL TAPVendorAdd::SetField(int ID, Field4& field,HWND HWindow)
{
 string szClassName;
 szClassName.resize(40);
 ::GetClassName(GetDlgItem(HWindow,ID),(LPSTR)szClassName.c_str(),szClassName.length());
 szClassName.to_upper();
 if(szClassName == "TBFLOAT")
  {
	double d = atof(field.str());
	FloatSetDlgItemValue(HWindow,ID,d);
  }
 else
  if(szClassName == "TBINT")
  {
	int i = atoi(field.str());
	IntSetDlgItemValue(Parent->HWindow,ID,i);
  }
 else
	SendMessage(GetDlgItem(HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field.str());
  return TRUE;
}


BOOL TAPVendorAdd::SetField(int ID, char *field)
{
  if(options != NULL)
    SendMessage(GetDlgItem(options->HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  SendMessage(GetDlgItem(HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return TRUE;
}

BOOL TAPVendorAdd::SetField(int ID, string* field)
{
  //SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->c_str());
  if(options != NULL)
  SendMessage(GetDlgItem(options->HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->c_str());
  SendMessage(GetDlgItem(HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->c_str());
  return TRUE;
}

BOOL TAPVendorAdd::SetField(int ID, double field)
{
	if(options != NULL)
     FloatSetDlgItemValue(options->HWindow,ID,field);
  FloatSetDlgItemValue(HWindow,ID,field);
  return TRUE;
}

BOOL TAPVendorAdd::SetField(int ID, int field)
{
 IntSetDlgItemValue(HWindow,ID,field);
  if(options != NULL)
	 IntSetDlgItemValue(options->HWindow,ID,field);
 return TRUE;
}




void TAPVendorAdd::HasChanged(RTMessage)
{
		int iTab = HAGetActive(GetDlgItem(HWindow,IDC_TAB));
		if(iCurrentTab != iTab)
		{
		 iCurrentTab = iTab;
		 switch(iCurrentTab)
			{
			 case 0 : options->Show(SW_HIDE); break;
			 case 1 :
			 {
				if(options == NULL)
				{
					options = new TDialog(this,1068);
					GetApplication()->MakeWindow(options);
					options->Show(SW_HIDE);
					// Create this window but don't let it be shown until you want to..
					// Currently the window flashes when it is being created.
					// Okay but not too cool...
				}
				if(options != NULL)
				{
						 RECT optionsRC,parentRC;
						 GetWindowRect(HWindow,&parentRC);
						 GetWindowRect(options->HWindow,&optionsRC);
						 MoveWindow(options->HWindow,parentRC.left+(POSITION/2),  // x position
									 parentRC.top+(4.5 * POSITION), // y position
									 optionsRC.right-optionsRC.left,  // width
									 optionsRC.bottom-optionsRC.top,  // height
									 TRUE);

						 options->Show(SW_SHOW);


				}
				break;
			 }
		  }
		}
  UpdateData();
}



void TAPVendorAdd::WMMove(RTMessage)
{
 RECT parentRC,optionsRC;
 GetWindowRect(HWindow,&parentRC);


 if(options != NULL)
 {
 GetWindowRect(options->HWindow,&optionsRC);
 MoveWindow(options->HWindow,parentRC.left+(POSITION/2),  // x position
									 parentRC.top+(4.5 * POSITION), // y position
									 optionsRC.right-optionsRC.left,  // width
									 optionsRC.bottom-optionsRC.top,  // height
									 TRUE);
 }
}



void TAPVendorAdd::UpdateData()
{
	SetField(ID_VENDORNO       , apvendor.vendorno	,HWindow );
	SetField(ID_VENDORSHORT    , apvendor.vdshort 	,HWindow);
	SetField(ID_VENDORNAME     , apvendor.vdname 	,HWindow);
	SetField(ID_VENDORADDRESS1 , apvendor.address1 	,HWindow);
	SetField(ID_VENDORADDRESS2 , apvendor.address2 	,HWindow);
	SetField(ID_VENDORADDRESS3 , apvendor.address3 	,HWindow);
	SetField(ID_VENDORPOST     , apvendor.postal 	,HWindow);
	SetField(ID_VENDORCONTACT  , apvendor.contact 	,HWindow);
	SetField(ID_VENDORSTATUS   , apvendor.status 	,HWindow);
	SetField(ID_VIN_NUM        , apvendor.vin 		,HWindow);
	SetField(ID_VAT_NUM        , apvendor.vat_num   ,HWindow);
	SetField(ID_PHONE	         , apvendor.phone 		,HWindow);
	SetField(ID_FAX	         , apvendor.fax 		,HWindow);
	if(options != NULL)
	{
		SetField(ID_TERMS          , apvendor.terms 		,options->HWindow);
		SetField(ID_APCONTROL      , apvendor.account 	,options->HWindow);
		SetField(ID_APDEPART       , apvendor.depart 	,options->HWindow);
		SetField(ID_VATACCT        , apvendor.vat_acct 	,options->HWindow);
		SetField(ID_VATDEPT        , apvendor.vat_dept 	,options->HWindow);

		SetField(ID_MEDREP         , apvendor.medrep 	,options->HWindow);
		SetField(ID_DIS_DAYS       , apvendor.discount_days ,options->HWindow);
		SetField(ID_DIS_PERC       , apvendor.discount_percent ,options->HWindow);
		SetField(IDC_EX_ACCT 		, apvendor.ex_acct	,options->HWindow);
		SetField(IDC_EX_DEPT       , apvendor.ex_dept	,options->HWindow);
		SetField(ID_VATPERCENTAGE  , apvendor.percentage,options->HWindow);
		SetField(ID_REBATE         , apvendor.rebate 	,options->HWindow);
  }


  if(strcmp(apvendor.type.str(),"C") == 0 )
  {
	SendMessage(GetDlgItem(HWindow,ID_COMPANY),BM_SETCHECK,TRUE,NULL);
	SendMessage(GetDlgItem(HWindow,ID_INDIVIDUAL),BM_SETCHECK,FALSE,NULL);
  }
  else
  {
	SendMessage(GetDlgItem(HWindow,ID_INDIVIDUAL),BM_SETCHECK,TRUE,NULL);
	SendMessage(GetDlgItem(HWindow,ID_COMPANY),BM_SETCHECK,FALSE,NULL);
  }

}

void TAPVendorAdd:: AddLine()
 {
	  apvendor.vendorno.assign(GetField(ID_VENDORNO));
	  apvendor.vdshort.assign(GetField(ID_VENDORSHORT));
	  apvendor.vdname.assign(GetField(ID_VENDORNAME));
	  apvendor.address1.assign(GetField(ID_VENDORADDRESS1));
	  apvendor.address2.assign(GetField(ID_VENDORADDRESS2));
	  apvendor.address3.assign(GetField(ID_VENDORADDRESS3));
	  apvendor.postal.assign(GetField(ID_VENDORPOST));
	  apvendor.contact.assign(GetField(ID_VENDORCONTACT));
	  apvendor.phone.assign(GetField(ID_PHONE));
	  apvendor.fax.assign(GetField(ID_FAX));
	  apvendor.type.assign(szType);
	  apvendor.vat_num.assign(GetField(ID_VAT_NUM));
	  apvendor.vin.assign(GetField(ID_VIN_NUM));

    // MessageBox(HWindow,GetField(ID_VAT_NUM),"Vat Number",MB_OK);

	  if(options == NULL)
	  {
		options = new TDialog(this,1068);
		GetApplication()->MakeWindow(options);
		options->Show(SW_HIDE);
	  }
	  if(options != NULL)
	  {
		  apvendor.terms.assignDouble(dGetField(options->HWindow,ID_TERMS));
		  apvendor.account.assign(GetField(options->HWindow,ID_APCONTROL));
		  apvendor.depart.assign(GetField(options->HWindow,ID_APDEPART));
		  apvendor.percentage.assignDouble(dGetField(options->HWindow,ID_VATPERCENTAGE));
		  apvendor.rebate.assignDouble(dGetField(options->HWindow,ID_REBATE));
		  apvendor.vat_acct.assign(GetField(options->HWindow,ID_VATACCT));
		  apvendor.vat_dept.assign(GetField(options->HWindow,ID_VATDEPT));
		  apvendor.medrep.assign(GetField(options->HWindow,ID_MEDREP));
		  apvendor.discount_days.assignDouble(dGetField(options->HWindow,ID_DIS_DAYS));
		  apvendor.discount_percent.assignDouble(dGetField(options->HWindow,ID_DIS_PERC));
		  string test(GetField(options->HWindow,IDC_EX_ACCT));
		  apvendor.ex_acct.assign(GetField(options->HWindow,IDC_EX_ACCT));
		  apvendor.ex_dept.assign(GetField(options->HWindow,IDC_EX_DEPT));
	  }
		  apvendor.UpdateUserTime(szUser);
 }


void TAPVendorAdd::SetData(RTMessage)
{

 SetField(ID_VENDORNO,(LPSTR)VendorList->strKey.c_str());

 if(apvendor.seek((LPSTR)VendorList->strKey.c_str()) == 0)
  UpdateData();
 else
  MessageBox(HWindow,GetString(6017),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);

}

void TAPVendorAdd::Previous(RTMessage)
{
 if(*GetField(ID_VENDORNO) == 0)
 {
  apvendor.top();
  UpdateData();
  return;
 }

 if(apvendor.seek(GetField(ID_VENDORNO)) == 0 && !apvendor.bof())
 {
  apvendor.skip(-1);
  SetField(ID_VENDORNO, apvendor.vendorno,HWindow);
	UpdateData();
 }

}

void TAPVendorAdd::Top(RTMessage)
{
 apvendor.top();
 SetField(ID_VENDORNO, apvendor.vendorno,HWindow);
 SendMessage(HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
}

void TAPVendorAdd::Bottom(RTMessage)
{
 apvendor.bottom();
 SetField(ID_VENDORNO, apvendor.vendorno,HWindow);
  UpdateData();
}


void TAPVendorAdd::Next(RTMessage)
{
 if(strlen(StrTrim(GetField(ID_VENDORNO))) == 0)
 {
  apvendor.top();
  SetField(ID_VENDORNO, apvendor.vendorno,HWindow);
	UpdateData();
  return;
 }
 if(apvendor.seek(GetField(ID_VENDORNO)) == 0)
 {
  apvendor.skip();
  SetField(ID_VENDORNO, apvendor.vendorno,HWindow);
	UpdateData();
 }
}


void TAPVendorAdd::Search  (RTMessage)
{
	 VendorList->List->Seek(ID_VENDORNO);
	 VendorList->Show(SW_SHOW);
}

void TAPVendorAdd::OpenChildWindow(RTMessage)
{

}

void TAPVendorAdd::WMInitDialog(RTMessage)
{
  SetupWindow();

}

void TAPVendorAdd::SetupWindow()
{
 TMainDialog::SetupWindow();
 RECT parentRC	;

 GetWindowRect(HWindow,&parentRC);


 iCurrentTab = 0;
 if(apvendor.open(*cb) < 0)
  {
	ErrorClose(apvendor.GetDbfName());
	return;
  }

  if(glaccts.open(*cb) < 0)
  {
	ErrorClose(glaccts.GetDbfName());
	return;
  }
 //apvendor.SetIndex("vendorno");
 apvendor.SetIndexVendor();

 VendorList = new TCustomerList(this,"");
 VendorList->Attr.Style |= WS_THICKFRAME;
 VendorList->EnableAutoCreate();


 options = new TDialog(this,1068);
 GetApplication()->MakeWindow(options);
 options->Show(SW_HIDE);

 SetBitmap(ID_SEARCHCUST,"LOOK");
 SetBitmap(CM_NEW,"DEFAULT");
 SetBitmap(ID_SETDATA);

 iVat = 0;
 bVendorList = TRUE;
 GL_SETAT gl_setat;
 gl_setat.open(*cb);
 gl_setat.SetIndex("gl_accts");

 if(gl_setat.seek("AP  ") == 0)
 {
  //szAPAccount = strcpy(new char[8] ,gl_setat.account.str());
  //szAPDept = strcpy(new char[8] ,gl_setat.dept.str());
  strAPAccount = gl_setat.account.str();
  strAPDept = gl_setat.dept.str();
 }
 else
 {
  //szAPAccount = strcpy(new char[8],"");
  //szAPDept    = strcpy(new char[5],"");
  strAPAccount = "";
  strAPDept = "";

 }


 if(gl_setat.seek("VATR") == 0)
 {
  //szVATAccount = strcpy(new char[8] ,gl_setat.account.str());
  //szVATDept    = strcpy(new char[5] ,gl_setat.dept.str());
  strVATAccount = gl_setat.account.str();
  strVATDept = gl_setat.dept.str();
 }
 else
 {
  //szVATAccount = strcpy(new char[8] ,"");
  //szVATDept    = strcpy(new char[5] ,"");
  strVATAccount = "";
  strVATDept    = "";
 }

 gl_setat.close();
 SetField(ID_APCONTROL , &strAPAccount );
 SetField(ID_APDEPART  , &strAPDept );
 SetField(ID_VATACCT   , &strVATAccount);
 SetField(ID_VATDEPT   , &strVATDept);
 GetApplication()->MakeWindow(VendorList);

}

void TAPVendorBatchAdd::SetupWindow()
{
 TMainDialog::SetupWindow();

  if(apvendor.open(*cb) < 0)
  {
	ErrorClose(apvendor.GetDbfName());
	return;
  }

  if(glaccts.open(*cb) < 0)
  {
	ErrorClose(glaccts.GetDbfName());
	return;
  }

 ShowWindow(GetDlgItem(HWindow,ID_SEARCHCUST),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,CM_NEW),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,ID_SETDATA),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,ID_NEXT),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,ID_PREVIOUS),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,ID_TOP),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,ID_BOTTOM),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,ID_DELETE),SW_HIDE);

 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);

 GL_SETAT gl_setat;
 gl_setat.open(*cb);
 gl_setat.SetIndex("gl_accts");

 if(gl_setat.seek("AP  ") == 0)
 {
  //szAPAccount = strcpy(new char[8] ,gl_setat.account.str());
  //szAPDept = strcpy(new char[8] ,gl_setat.dept.str());
  strAPAccount = gl_setat.account.str();
  strAPDept = gl_setat.dept.str();
 }
 else
 {
  //szAPAccount = strcpy(new char[8],"");
  //szAPDept    = strcpy(new char[5],"");
  strAPAccount = "";
  strAPDept = "";

 }


 if(gl_setat.seek("VATR") == 0)
 {
  //szVATAccount = strcpy(new char[8] ,gl_setat.account.str());
  //szVATDept    = strcpy(new char[5] ,gl_setat.dept.str());
  strVATAccount = gl_setat.account.str();
  strVATDept = gl_setat.dept.str();
 }
 else
 {
  //szVATAccount = strcpy(new char[8] ,"");
  //szVATDept    = strcpy(new char[5] ,"");
  strVATAccount = "";
  strVATDept    = "";
 }

 gl_setat.close();
 SetField(ID_VENDORNO  , szVendor);
 SetField(ID_APCONTROL , &strAPAccount );
 SetField(ID_APDEPART  , &strAPDept );
 SetField(ID_VATACCT   , &strVATAccount);
 SetField(ID_VATDEPT   , &strVATDept);

}

void TAPVendorBatchAdd::Ok             (RTMessage Msg)
{
 CHourGlass wait;
 if(szEnterError(ID_VENDORNO))
  return;

 if(szEnterError(ID_VENDORNAME))
  return;

 if(szEnterError(ID_VENDORSHORT))
  return;


 if(SendMessage(GetDlgItem(HWindow,ID_INDIVIDUAL),BM_GETSTATE,NULL,NULL))
	 strcpy(szType,"I");
 else
 if(SendMessage(GetDlgItem(HWindow,ID_INDIVIDUAL),BM_GETSTATE,NULL,NULL))
	  strcpy(szType,"C");

 if(apvendor.seek(GetField(ID_VENDORNO))==0)
 {
	if(MessageBox(HWindow,GetString(6037),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
	  AddLine();
	  bVendorList = TRUE;
	}
	return;
 }

  apvendor.appendStart();
	AddLine();
  apvendor.append();
  Cancel(Msg);
}

void TAPVendorBatchAdd::Cancel(RTMessage)
{
 apvendor.close();
 glaccts.close();
 ShutDownWindow();
}




void TAPVendorAdd::Ok             (RTMessage)
{
 CHourGlass wait;
 if(szEnterError(ID_VENDORNO))
  return;


 if(szEnterError(ID_VENDORNAME))
  return;

 if(szEnterError(ID_VENDORSHORT))
  return;

 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 strcpy(szType,"I");
 else
 if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	  strcpy(szType,"C");

 if(apvendor.seek(GetField(ID_VENDORNO))==0)
 {
	if(MessageBox(HWindow,GetString(6037),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
	  AddLine();
	  apvendor.flush();
	  bVendorList = TRUE;
	}
	return;
 }

  apvendor.appendStart();
	AddLine();
  apvendor.append();

  VendorList->List->SetFields();

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
 SetField(ID_APCONTROL      , &strAPAccount );
 SetField(ID_APDEPART       , &strAPDept );
 SetField(ID_VATACCT	    , &strVATAccount);
 SetField(ID_VATDEPT	    , &strVATDept);
 SetField(ID_MEDREP         , "" );
 SetField(ID_VIN_NUM        , "" );
 SetField(ID_DIS_DAYS       , "");
 SetField(ID_DIS_PERC       , "" );

 SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,FALSE,NULL);
 SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,FALSE,NULL);
 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
}


void TAPVendorAdd::Cancel         (RTMessage)
{
 VendorList->List->SavePosition();
 apvendor.close();
 glaccts.close();
 ShutDownWindow();
}

void TAPVendorAdd::Delete (RTMessage)
{
 if(apvendor.eof())
	return;
 if(MessageBox(HWindow,GetString(6034),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION)==IDYES)
  {
  apvendor.deleteRec();
  apvendor.pack();
  VendorList->List->Redraw();
  SendMessage(HWindow,WM_COMMAND,CM_NEW,NULL);
  }
}

/*
	Modify the GetField function show that it returns the correct window
	item by passing the handle to the window you want to extract from.

*/


void TAPVendorBatchAdd:: AddLine()
 {
	  apvendor.vendorno.assign(GetField(ID_VENDORNO));
	  apvendor.vdshort.assign(GetField(ID_VENDORSHORT));
	  apvendor.vdname.assign(GetField(ID_VENDORNAME));
	  apvendor.address1.assign(GetField(ID_VENDORADDRESS1));
	  apvendor.address2.assign(GetField(ID_VENDORADDRESS2));
	  apvendor.address3.assign(GetField(ID_VENDORADDRESS3));
	  apvendor.postal.assign(GetField(ID_VENDORPOST));
	  apvendor.contact.assign(GetField(ID_VENDORCONTACT));
	  apvendor.phone.assign(GetField(ID_PHONE));
	  apvendor.fax.assign(GetField(ID_FAX));
	  apvendor.type.assign(szType);
	  if(options != NULL)
	  {
		  apvendor.terms.assignDouble(dGetField(options->HWindow,ID_TERMS));
		  apvendor.account.assign(GetField(options->HWindow,ID_APCONTROL));
		  apvendor.depart.assign(GetField(options->HWindow,ID_APDEPART));
		  apvendor.percentage.assignDouble(dGetField(options->HWindow,ID_VATPERCENTAGE));
		  apvendor.rebate.assignDouble(dGetField(options->HWindow,ID_REBATE));
		  apvendor.vat_acct.assign(GetField(options->HWindow,ID_VATACCT));
		  apvendor.vat_dept.assign(GetField(options->HWindow,ID_VATDEPT));
		  apvendor.vin.assign(GetField(options->HWindow,ID_VIN_NUM));
		  apvendor.medrep.assign(GetField(options->HWindow,ID_MEDREP));
		  apvendor.discount_days.assignDouble(dGetField(options->HWindow,ID_DIS_DAYS));
		  apvendor.discount_percent.assignDouble(dGetField(options->HWindow,ID_DIS_PERC));
	  }
	  apvendor.UpdateUserTime(szUser);
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
