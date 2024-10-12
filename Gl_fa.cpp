#include "std_inc.h"

#define LINENO 10
#define ID_YEARS_LIFE 6033
#define ID_DECLINING  7024
#define ID_ACCUMULATED_DEP 6040
#define ID_NBV	6041


extern HINSTANCE hInst;



BOOL Tglfa::Journal()
{
 if(strcmp(mode.str(),"0") == 0)
  return TRUE;
 else
  return FALSE;
}

CHourGlass::CHourGlass(TWindow *pWnd)
 {
  m_hSaveCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
  m_pCapWnd = pWnd;
  if(m_pCapWnd)
	 SetCapture(pWnd->HWindow);
 }

CHourGlass::~CHourGlass()
{
 SetCursor(m_hSaveCursor);
 if(m_pCapWnd)
  ReleaseCapture();
}
void TAssetList::WMSize(RTMessage Msg)
{
  // NEW
  TWindow::WMSize(Msg);
  int iRows = (int)glfa->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);


}




_CLASSDEF(TGLFixedDelete)
class TGLFixedDelete : public TGLFixedAssets
{
 public:
 TGLFixedDelete(PTWindowsObject AWindow,int ATitle):TGLFixedAssets(AWindow,ATitle){};
 virtual void SetupWindow();
 virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
};

#define ID_ASSET_ACCOUNTS 2073
#define ID_ASSET_EXPENSE  2074
#define ID_ACCUM_DEP 2078




_CLASSDEF(TGLTypes)
class TGLTypes : public TGLFixedAssets
{
 public:
 TAccountList* ASSET;
 TAccountList* EXPENSES;
 TAccountList* ACCUM;
 TGLTypes(PTWindowsObject AWindow,int ATitle):TGLFixedAssets(AWindow,ATitle)
 {
	ASSET = NULL;
	EXPENSES = NULL;
	ACCUM = NULL;
 };
 virtual void SetupWindow();
 virtual void Ok  (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Modify(RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Next     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Top      (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom   (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void AssetAccounts (RTMessage Msg) = [ID_FIRST + ID_ASSET_ACCOUNTS];
 virtual void AssetExpenses (RTMessage Msg) = [ID_FIRST + ID_ASSET_EXPENSE];
 virtual void AccumlatedDep (RTMessage Msg) = [ID_FIRST + ID_ACCUM_DEP];
};

void TGLTypes::AssetAccounts(RTMessage)
{
  if(ASSET == NULL)
  {
	  ASSET = new TAccountList(this,"");
	  ASSET->SetAccountID(6023,6024,1078);
	  ASSET->Attr.Style |= WS_THICKFRAME;
	  ASSET->EnableAutoCreate();
	  GetApplication()->MakeWindow(ASSET);
	  ASSET->List->Redraw();
  }
  ASSET->Show(SW_SHOW);
}

void TGLTypes::AssetExpenses(RTMessage)
{
  if(EXPENSES == NULL)
  {
	  EXPENSES = new TAccountList(this,"");
	  EXPENSES->SetAccountID(6025,6026,6027);
	  EXPENSES->Attr.Style |= WS_THICKFRAME;
	  EXPENSES->EnableAutoCreate();
	  GetApplication()->MakeWindow(EXPENSES);
	  EXPENSES->List->Redraw();
  }
  EXPENSES->Show(SW_SHOW);
}

void TGLTypes::AccumlatedDep(RTMessage)
{
  if(ACCUM == NULL)
  {
	  ACCUM = new TAccountList(this,"");
	  ACCUM->SetAccountID(1076,6030,6028);
	  ACCUM->Attr.Style |= WS_THICKFRAME;
	  ACCUM->EnableAutoCreate();
	  GetApplication()->MakeWindow(ACCUM);
	  ACCUM->List->Redraw();
  }
  ACCUM->Show(SW_SHOW);
}

void TGLFixedAssets::SetupWindow()
{
 TMainDialog::SetupWindow();
 CHourGlass wait;
 bTypeList = FALSE;

 glfa    = new Tglfa(cb);
 if(!glfa->isValid())
  {
	ErrorClose(glfa->GetDbfName());
	return;
  }

 fatype  = new Tfatype(cb);
  if(!fatype->isValid())
  {
	ErrorClose(fatype->GetDbfName());
	return;
  }


 bTypes = FALSE;
 bAsset = FALSE;
 DWORD dwIndex = SendDlgItemMsg(ID_DEPRECIATION,CB_ADDSTRING,NULL,(LPARAM)(LPCSTR)"Straight");
 SendDlgItemMsg(ID_DEPRECIATION,CB_ADDSTRING,NULL,(LPARAM)(LPCSTR)"Declining");
 SendDlgItemMsg(ID_DEPRECIATION,CB_SETCURSEL,(WORD)dwIndex,NULL);
 ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,7025),SW_HIDE);

 TYPES = new TAssetTypesList(this,"",fatype);
 TYPES->Attr.Style |= WS_THICKFRAME;
 TYPES->EnableAutoCreate();
 GetApplication()->MakeWindow(TYPES);
 TYPES->List->Redraw();

 ASSET = new TAssetList(this,"",glfa);
 ASSET->Attr.Style |= WS_THICKFRAME;
 ASSET->EnableAutoCreate();
 GetApplication()->MakeWindow(ASSET);

 //List->SetBool(SSB_SHOWROWHEADERS,FALSE);

 IntSetDlgItemRange(HWindow,6022,0,1000);
 IntSetDlgItemRange(HWindow,6032,0,1000);
 IntSetDlgItemRange(HWindow,7024,0,1000);


 SetBitmap(ID_ASSETS);
 SetBitmap(ID_TYPES,"TYPES");

}


void TGLTypes::Delete(RTMessage Msg)
{
 if(MessageBox(HWindow,GetString(8109),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	if(fatype->seek(GetField(ID_ASSET_TYPE)) == 0)
	{
    fatype->deleteRec();
	 fatype->pack();
	 New(Msg);

	}
  }
}

void TGLTypes::SetupWindow()
{
 TGLFixedAssets::SetupWindow();
 bTypeList = TRUE;
 SetFocus(GetDlgItem(HWindow,ID_ASSET_TYPE));
 

}

void TGLTypes::Ok(RTMessage Msg)
{
 CHourGlass wait;
 string szGLAccount(GetField(ID_ASSET_ACCOUNT));
 string szDept(GetField(ID_ASSET_DEPT));
 if(!ValidateGL((LPSTR)szGLAccount.c_str(),(LPSTR)szDept.c_str()))
  {
	MessageBox(HWindow,GetString(6056),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

 szGLAccount = GetField(ID_EXPENSE_ACCOUNT);
 szDept      = GetField(ID_EXPENSE_DEPT);
 if(!ValidateGL((LPSTR)szGLAccount.c_str(),(LPSTR)szDept.c_str()))
  {
	MessageBox(HWindow,GetString(6057),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

 szGLAccount = GetField(ID_ACCUM_ACCOUNT);
 szDept      = GetField(ID_ACCUM_DEPT);

 if(!ValidateGL((LPSTR)szGLAccount.c_str(),(LPSTR)szDept.c_str()))
  {
	MessageBox(HWindow,GetString(6058),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

 if(strlen(StrTrim(GetField(ID_ASSET_TYPE))) == 0)
  return;

 if(fatype->seek(GetField(ID_ASSET_TYPE)) != 0)
  {
	fatype->appendStart();
	fatype->fatype.assign(GetField(ID_ASSET_TYPE));
	fatype->fadesc.assign(GetField(ID_DESCRIPT));
	fatype->asset_acct.assign(GetField(ID_ASSET_ACCOUNT));
	fatype->asset_dept.assign(GetField(ID_ASSET_DEPT));
	fatype->asset_desc.assign(GetField(ID_ASSET_DESC));
	fatype->expen_acct.assign(GetField(ID_EXPENSE_ACCOUNT));
	fatype->expen_dept.assign(GetField(ID_EXPENSE_DEPT));
	fatype->expen_desc.assign(GetField(ID_EXPENSE_DESC));
	fatype->accum_acct.assign(GetField(ID_ACCUM_ACCOUNT));
	fatype->accum_dept.assign(GetField(ID_ACCUM_DEPT));
	fatype->accum_desc.assign(GetField(ID_ACCUM_DESC));
	fatype->UpdateUserTime(szUser);
	fatype->append();
	TYPES->List->Redraw();
	//TYPES->WMSize(Msg);
	New(Msg);

  }
  else
  Modify(Msg);

}

void TGLTypes::Modify(RTMessage)
{
 if(MessageBox(HWindow,GetString(8108),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	fatype->fatype.assign(GetField(ID_ASSET_TYPE));
	fatype->fadesc.assign(GetField(ID_DESCRIPT));
	fatype->asset_acct.assign(GetField(ID_ASSET_ACCOUNT));
	fatype->asset_dept.assign(GetField(ID_ASSET_DEPT));
	fatype->asset_desc.assign(GetField(ID_ASSET_DESC));
	fatype->expen_acct.assign(GetField(ID_EXPENSE_ACCOUNT));
	fatype->expen_dept.assign(GetField(ID_EXPENSE_DEPT));
	fatype->expen_desc.assign(GetField(ID_EXPENSE_DESC));
	fatype->accum_acct.assign(GetField(ID_ACCUM_ACCOUNT));
	fatype->accum_dept.assign(GetField(ID_ACCUM_DEPT));
	fatype->accum_desc.assign(GetField(ID_ACCUM_DESC));
	fatype->UpdateUserTime(szUser);
	fatype->flush();


 }
}

 void TGLTypes::Next     (RTMessage Msg)
 {
  fatype->skip();
  UpdateType(Msg);
 }

 void TGLTypes::Previous (RTMessage Msg)
 {
  fatype->skip(-1);
  UpdateType(Msg);
 }

 void TGLTypes::Top      (RTMessage Msg)
 { 
	fatype->top();
	UpdateType(Msg);
 }

 void TGLTypes::Bottom   (RTMessage Msg)
 {
  fatype->bottom();
  UpdateType(Msg);
 }


void TGLFixedDelete::SetupWindow()
{
 TGLFixedAssets::SetupWindow();
 SetBitmap(IDOK,"ERASE");
}

void TGLFixedDelete::Ok(RTMessage Msg)
{

 if(MessageBox(HWindow,GetString(8107),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
 {
  CHourGlass wait;
  glfa->deleteRec();
  glfa->pack();
  New(Msg);

 }
}






BOOL TGLFixedAssets::TestDatabase()
 {
  BOOL bDatabase = TRUE;
  if(!CheckFile("GL_FA",Parent->HWindow))
	bDatabase = FALSE;
  if(!CheckFile("GL_FATYP",Parent->HWindow))
    bDatabase = FALSE;
  if(!bDatabase)
	BringWindowToTop(GetApplication()->MainWindow->HWindow);
  return bDatabase;
 }

void TGLFixedAssets::AssetList(RTMessage)
{
     ASSET->List->Seek(ID_FACODE);
	  ASSET->Show(SW_SHOW);
}




/*
BOOL TGLFixedAssets::SetField(int ID,Field4 *field)
{
 SendMessage(GetDlgItem(HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
 return TRUE;
}

BOOL TGLFixedAssets::SetField(int ID,double field)
{
 FloatSetValue(GetDlgItem(HWindow,ID),field);
 return TRUE;
}


BOOL TGLFixedAssets::SetField(int ID,char *field)
{
 SendMessage(GetDlgItem(HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
 return TRUE;
}
*/

void TGLFixedAssets::UpdateType (RTMessage)
{
  SetField(ID_ASSET_TYPE,&fatype->fatype);
  if(bTypeList)
	SetField(ID_DESCRIPT,&fatype->fadesc);
  SetField(ID_ASSET_ACCOUNT,&fatype->asset_acct);
  SetField(ID_ASSET_DEPT,&fatype->asset_dept);
  SetField(ID_ASSET_DESC,&fatype->asset_desc);
  SetField(ID_EXPENSE_ACCOUNT,&fatype->expen_acct);
  SetField(ID_EXPENSE_DEPT,&fatype->expen_dept);
  SetField(ID_EXPENSE_DESC,&fatype->expen_desc);
  SetField(ID_ACCUM_ACCOUNT,&fatype->accum_acct);
  SetField(ID_ACCUM_DEPT,&fatype->accum_dept);
  SetField(ID_ACCUM_DESC,&fatype->accum_desc);
  SetFocus(Parent->HWindow);
  if(TYPES !=NULL)
  TYPES->Show(SW_HIDE);
  bTypes = FALSE;
  SetFocus(HWindow);  
}

 void TGLFixedAssets::Search   (RTMessage) {}

 void TGLFixedAssets::Types    (RTMessage)
 {
	  TYPES->List->Seek(ID_ASSET_TYPE);
	  TYPES->Show(SW_SHOW);
 }

 BOOL TGLFixedAssets::szCheckField(int ID)
 {
  if(strlen(StrTrim(GetField(ID))) == 0)
  {

	wsprintf(szMsg,GetString(8101),GetString(ID));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
	return TRUE;
 }

 BOOL TGLFixedAssets::dCheckField(int ID)
 {
  if(dGetField(ID) == 0)
  {

	wsprintf(szMsg,GetString(8101),GetString(ID));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
	return TRUE;
 }


 BOOL TGLFixedAssets::CheckInput()
 {
  BOOL bOk = FALSE;

  if(strlen(StrTrim(GetField(ID_FACODE))) == 0
	||strlen(StrTrim(GetField(ID_ASSET_TYPE))) == 0
	||strlen(StrTrim(GetField(ID_DESCRIPT))) == 0
	||strlen(StrTrim(GetField(ID_REFERENCE))) == 0
	||dGetField(ID_LIFETIME) == 0
	||dGetField(ID_ASSET_COST)== 0
	||dGetField(ID_MONTHLY_DEP) == 0)
  bOk = FALSE;
  return bOk;
 }

void TGLFixedAssets::Ok       (RTMessage Msg)
{
 CHourGlass wait;
 string szGLAccount;
 string szDept;

 szGLAccount = GetField(ID_ASSET_ACCOUNT);
 szDept      = GetField(ID_ASSET_DEPT);

 if(!ValidateGL((char *)szGLAccount.c_str(),(char *)szDept.c_str()))
  {
	MessageBox(HWindow,GetString(6056),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

 szGLAccount = GetField(ID_EXPENSE_ACCOUNT);
 szDept      = GetField(ID_EXPENSE_DEPT);
 if(!ValidateGL((char *)szGLAccount.c_str(),(char *)szDept.c_str()))
  {
	MessageBox(HWindow,GetString(6057),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

 szGLAccount = GetField(ID_ACCUM_ACCOUNT);
 szDept      = GetField(ID_ACCUM_DEPT);

  if(!ValidateGL((char *)szGLAccount.c_str(),(char *)szDept.c_str()))
  {
	MessageBox(HWindow,GetString(6058),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

  if(dGetField(ID_SALVAGE) > (dGetField(ID_ASSET_COST)- dGetField(ID_ACCUMULATED_DEP)))
	 {
		MessageBox(HWindow,GetString(9097),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		return;
	 }
	if(dGetField(ID_LIFETIME) < dGetField(ID_LIFE_RMD))
	{
	  MessageBox(HWindow,GetString(9098),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	  return;
	}



  if(glfa->seek(GetField(ID_FACODE)) != 0)
  {
	 /*
	  glfa->cost->assignDouble(dGetField(ID_ASSET_COST));
	  glfa->salvage->assignDouble(dGetField(ID_SALVAGE));
	 */


	glfa->appendStart();
	UpdateData();
	glfa->append();

	New(Msg);
	ASSET->List->Redraw();
  }
  else
  Update(Msg);
 }

void TGLFixedAssets::DepMethod(RTMessage Msg)
{
 char* szCode = new char[11];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  DWORD Indx = SendDlgItemMsg(ID_DEPRECIATION,CB_GETCURSEL,0,0);
  if(Indx > 0)
	{
	  SendDlgItemMsg(ID_DEPRECIATION,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szCode);
	  if(strcmp(StrTrim(szCode),"Declining") == 0)
		{
		  //SetField(7021,"Average\n(Years)");
		  ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_SHOW);
		  ShowWindow(GetDlgItem(HWindow,7025),SW_SHOW);
		  //ShowWindow(GetDlgItem(HWindow,6022),SW_HIDE);
		}
	  else
	  {
		  //SetField(7021,"Life of Asset\n(Years)");
		  ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
		  ShowWindow(GetDlgItem(HWindow,7025),SW_HIDE);
		  //ShowWindow(GetDlgItem(HWindow,6022),SW_SHOW);
	  }
	}
 }
 delete [] szCode;
}

char *TGLFixedAssets::GetDepMethod()
{
	 char *szCode = new char[11];
	 int Indx = (int)SendDlgItemMsg(ID_DEPRECIATION,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	  {
		SendDlgItemMsg(ID_DEPRECIATION,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szCode);
	  }
	 return szCode;
}


void TGLFixedAssets::UpdateData()
{
	double dMonthly  = 0;
	Date4 dPurchased;
	dPurchased.assign(GetField(ID_DATE),GetString(4024));
	dPurchased.format(GetString(4023));
	glfa->facode.assign(GetField(ID_FACODE));
	glfa->date_pur.assign(dPurchased.str());
	glfa->fatype.assign(GetField(ID_ASSET_TYPE));
	glfa->descript.assign(GetField(ID_DESCRIPT));
	glfa->reference.assign(GetField(ID_REFERENCE));
	glfa->dep_meth.assign(GetDepMethod());
	glfa->cost.assignDouble(dGetField(ID_ASSET_COST));
	glfa->salvage.assignDouble(dGetField(ID_SALVAGE));
	glfa->mode.assign("1");

	/*
	 FA CORRECT
	 place code here to enter directly to the database if the user
	 doesn't leave the net book value and depreciation amounts blank.
	*/
	double dLifeRemaining = 0;
	double dLife = 0;

	if(dGetField(ID_LIFETIME) > 0)
	{
	  dLife = dGetField(ID_LIFETIME);
	  glfa->lifetime.assignDouble(dLife);
	}

	if(dGetField(ID_ACCUMULATED_DEP) >= 0)
		glfa->accum_dep.assignDouble(dGetField(ID_ACCUMULATED_DEP));

	if(strcmp(StrTrim(GetDepMethod()),"Straight") == 0)
	{
	 if(SendDlgItemMsg(ID_FULLYEAR,BM_GETCHECK,NULL,NULL))
	  dLifeRemaining = 12;
	 else
	  dLifeRemaining = dGetField(ID_LIFE_RMD);

	 glfa->lifeleft.assignDouble(dLifeRemaining);
	 if(dLife > 0)
	  dMonthly = (dGetField(ID_ASSET_COST) - dGetField(ID_SALVAGE))/dLife;
	glfa->curr_dep.assignDouble(dMonthly);
	SetField(ID_MONTHLY_DEP,dMonthly);
	}
	else
	{

	 glfa->lifetime.assignDouble(dLife);
	 if(SendDlgItemMsg(ID_FULLYEAR,BM_GETCHECK,NULL,NULL))
	  dLifeRemaining = 12;
	 else
	  dLifeRemaining = dGetField(ID_LIFE_RMD);
	 glfa->lifeleft.assignDouble(dLifeRemaining);
	 double dPercent = dGetField(ID_DECLINING);
	 glfa->percent.assignDouble(dPercent);

	 if(dPercent > 0)
	  {
		double dYearly = (dGetField(ID_ASSET_COST) - dGetField(ID_SALVAGE))*(dPercent/100);
		glfa->curr_dep.assignDouble(dYearly/12);
		SetField(ID_MONTHLY_DEP,dYearly/12);
	  }
	}
	glfa->UpdateUserTime(szUser);

}

void TGLFixedAssets::FullYear (RTMessage Msg)
{
  if(SendDlgItemMsg(Msg.WParam,BM_GETCHECK,NULL,NULL))
	SendDlgItemMsg(Msg.WParam,BM_SETCHECK,FALSE,NULL);
  else
	SendDlgItemMsg(Msg.WParam,BM_SETCHECK,TRUE,NULL);
}

void TGLFixedAssets::Update (RTMessage Msg)
{
 if(MessageBox(HWindow,GetString(8102),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	UpdateData();
	glfa->flush();

	//New(Msg);
	ASSET->List->Redraw();
   SetData(Msg);
  }
}

void TGLFixedAssets::SetData  (RTMessage Msg)
  {
	if(glfa->seek(GetField(ID_FACODE)) != 0)
	  glfa->top();
	char szDatePurchased[11];
	Date4 InvoiceDate;
	InvoiceDate.assign(glfa->date_pur.str(),"CCYYMMDD");
	InvoiceDate.format(szDatePurchased,GetString(4024));
	SetField(ID_DATE,szDatePurchased);
	SetField(ID_FACODE,&glfa->facode);
	SetField(ID_ASSET_TYPE,&glfa->fatype);
	SetField(ID_DESCRIPT,&glfa->descript);
	SetField(ID_REFERENCE,&glfa->reference);

	if(strcmp(StrTrim(glfa->dep_meth.str()),"Declining") == 0)
	  {
		SendDlgItemMsg(ID_DEPRECIATION,CB_SETCURSEL,1,NULL);
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_SHOW);
		ShowWindow(GetDlgItem(HWindow,7025),SW_SHOW);

		/*
		SetField(7021,"Avg");
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_SHOW);
		ShowWindow(GetDlgItem(HWindow,6022),SW_HIDE);
		*/
	  }
	else
	  {
		SendDlgItemMsg(ID_DEPRECIATION,CB_SETCURSEL,0,NULL);
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
		ShowWindow(GetDlgItem(HWindow,7025),SW_HIDE);

		/*
		SetField(7021,"Life");
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
		ShowWindow(GetDlgItem(HWindow,6022),SW_SHOW);
		*/
	  }

	if(fatype->seek(glfa->fatype.str()) == 0)
	 UpdateType(Msg);
	SetField(ID_LIFETIME,&glfa->lifetime);
	SetField(ID_ASSET_COST,glfa->dCost());
	SetField(ID_LIFE_RMD,&glfa->lifeleft);
	SetField(ID_SALVAGE,&glfa->salvage);
	SetField(ID_MONTHLY_DEP,&glfa->curr_dep);
	SetField(ID_YEARS_LIFE,dGetField(ID_LIFETIME)/12);
	SetField(ID_DECLINING,&glfa->percent);
	SetField(ID_ACCUMULATED_DEP,&glfa->accum_dep);

	if(GetField(ID_LIFE_RMD) >= 0)
	 {
	  //double dAccum = dGetField(ID_MONTHLY_DEP)*dGetField(ID_LIFE_RMD);
	  //SetField(ID_ACCUMULATED_DEP,dAccum);
	  SetField(ID_NBV, dAccum()); //glfa->dCost()- glfa->dSalvage() - dAccum());
	 }

 }

void TGLFixedAssets::UpdateWindow  (RTMessage Msg)
  {
	char szDatePurchased[11];
	Date4 InvoiceDate;
	InvoiceDate.assign(glfa->date_pur.str(),"CCYYMMDD");
	InvoiceDate.format(szDatePurchased,GetString(4024));
	SetField(ID_DATE,szDatePurchased);
	SetField(ID_FACODE,&glfa->facode);
	SetField(ID_ASSET_TYPE,&glfa->fatype);
	SetField(ID_DESCRIPT,&glfa->descript);
	SetField(ID_REFERENCE,&glfa->reference);

	if(strcmp(StrTrim(glfa->dep_meth.str()),"Declining") == 0)
	  {
		SendDlgItemMsg(ID_DEPRECIATION,CB_SETCURSEL,1,NULL);
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_SHOW);
		ShowWindow(GetDlgItem(HWindow,7025),SW_SHOW);

		/*
		SetField(7021,"Avg");
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_SHOW);
		ShowWindow(GetDlgItem(HWindow,6022),SW_HIDE);
		*/
	  }
	else
	  {
		SendDlgItemMsg(ID_DEPRECIATION,CB_SETCURSEL,0,NULL);
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
		ShowWindow(GetDlgItem(HWindow,7025),SW_HIDE);

		/*
		SetField(7021,"Life");
		ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
		ShowWindow(GetDlgItem(HWindow,6022),SW_SHOW);
		*/
	  }

	if(fatype->seek(glfa->fatype.str()) == 0)
	 UpdateType(Msg);
	SetField(ID_LIFETIME,&glfa->lifetime);
	SetField(ID_ASSET_COST,glfa->dCost());
	SetField(ID_LIFE_RMD,&glfa->lifeleft);
	SetField(ID_SALVAGE,&glfa->salvage);
	SetField(ID_MONTHLY_DEP,&glfa->curr_dep);
	SetField(ID_YEARS_LIFE,dGetField(ID_LIFETIME)/12);
	SetField(ID_DECLINING,&glfa->percent);
	SetField(ID_ACCUMULATED_DEP,&glfa->accum_dep);

	if(GetField(ID_LIFE_RMD) >= 0)
	 {
	  //double dAccum = dGetField(ID_MONTHLY_DEP)*dGetField(ID_LIFE_RMD);
	  //SetField(ID_ACCUMULATED_DEP,dAccum);
	  //SetField(ID_NBV,glfa->dCost()- glfa->dSalvage() - dAccum());
	  SetField(ID_NBV,dAccum());
	 }

 }


 void TGLFixedAssets::Next     (RTMessage Msg)
 {
  glfa->skip();
  UpdateWindow(Msg);
 }

 void TGLFixedAssets::Previous (RTMessage Msg)
 {
  glfa->skip(-1);
  UpdateWindow(Msg);
 }

 void TGLFixedAssets::Top      (RTMessage Msg)
 {
	glfa->top();
	UpdateWindow(Msg);
 }

 void TGLFixedAssets::Bottom   (RTMessage Msg)
 {
  glfa->bottom();
  UpdateWindow(Msg);
 }
 void TGLFixedAssets::New      (RTMessage)
 {
	SetField(ID_FACODE		,"");
	SetField(ID_ASSET_TYPE	,"");
	SetField(ID_DESCRIPT		,"");
   SetField(ID_REFERENCE	,"");
   SetField(ID_DEPRECIATION	,"");
	SetField(ID_LIFETIME		,"");
   SetField(ID_ASSET_COST	,"");
   SetField(ID_LIFE_RMD		,"");
   SetField(ID_SALVAGE		,"");
   SetField(ID_MONTHLY_DEP	,"");
	SetField(ID_ASSET_TYPE	,"");
   SetField(ID_ASSET_ACCOUNT	,"");
   SetField(ID_ASSET_DEPT	,"");
	SetField(ID_ASSET_DESC	,"");
	SetField(ID_EXPENSE_ACCOUNT	,"");
   SetField(ID_EXPENSE_DEPT	,"");
   SetField(ID_EXPENSE_DESC	,"");
   SetField(ID_ACCUM_ACCOUNT	,"");
   SetField(ID_ACCUM_DEPT	,"");
	SetField(ID_ACCUM_DESC	,"");
	SetField(6040           ,"");
	SetField(6041           ,"");
	SetField(ID_DECLINING   ,"");
	SetField(ID_YEARS_LIFE ,"");
	return;
}

void TGLFixedAssets::Cancel   (RTMessage)
{
 TYPES->List->SavePosition();
 ASSET->List->SavePosition();
 delete glfa;
 delete fatype;
 cb->closeAll();
 cb->init();
 ShutDownWindow();
}







void TAssetList::SetupWindow()
{
 TWindow::SetupWindow();
 List->SetFields();
}

void TAssetList::SetWndDetail()
{
}

void TAssetList::HandleListBox( RTMessage Msg)
{
 string strCode;
 BOOL bHide = TRUE;
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(LPSTR)strCode.c_str());
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 SendMessage(GetDlgItem(Parent->HWindow,ID_FACODE),WM_SETTEXT,NULL,(LPARAM)(LPSTR)strCode.c_str());
 SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
 if(bHide)
	Show(SW_HIDE);
 return;
}

void TAssetTypesList::WMSize(RTMessage Msg)
{
	 // NEW
  TWindow::WMSize(Msg);
  int iRows = (int)fatype->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);


}


void TAssetTypesList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TAssetTypesList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption(GetString(GetApplication()->hInstance,6061));

 List->SetFields("gl_fatyp","fatype",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6060));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6061));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6044));
}

void TAssetTypesList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);

 char *szBuffer1 = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);

 fatype->SetIndex("fatype");
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(fatype->seek(szBuffer1) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_UPDATETYPES,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(bHide)
   Show(SW_HIDE);
 delete [] szBuffer1;
 return;
}

void TManilaGL::GLFixedAdd    (RTMessage){GetApplication()->ExecDialog(new TGLFixedAssets(this,4004));}
void TManilaGL::GLFixedDisposal(RTMessage){GetApplication()->ExecDialog(new TGLFixedDelete(this,1001));}
void TManilaGL::GLFixedTypes   (RTMessage){GetApplication()->ExecDialog(new TGLTypes(this,4002));}

double TGLFixedAssets::dAccum(void)
  {
   double dAccum = 0;
	double dCost    = atof(glfa->cost.str());
	double dDepr    = atof(glfa->curr_dep.str());
	dAccum   = atof(glfa->accum_dep.str());
	int dLife  = atoi(glfa->lifetime.str());
	int dLeft  = atoi(glfa->lifeleft.str());
	dAccum = dCost - dAccum;
	return dAccum;

  };



/*
BOOL CheckFile(char *szFileName,PTWindowsObject AWindow);

BOOL CheckFile(char *szFileName,PTWindowsObject AWindow)
{
 BOOL bFile = TRUE;
 char *szMsg = new char [255];
 char *szFile = new char[12];
 strcat(szFile,".DBF");
 c4upper(szFile);
 HFILE hFile = _lopen(szFile,OF_SHARE_COMPAT);
 if(hFile == HFILE_ERROR)
  {
	wsprintf(szMsg,"File %s is locked or not found.",szFile);
	MessageBox(GetFocus(),szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   AWindow->ShutDownWindow();   
	bFile = FALSE;
  }
 _lclose(hFile);

 strcat(szFile,".MDX");
 c4upper(szFile);
 hFile = _lopen(szFile,OF_SHARE_COMPAT);
 if(hFile == HFILE_ERROR)
  {
	wsprintf(szMsg,"File %s is locked or not found.",szFile);
	MessageBox(GetFocus(),szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   AWindow->ShutDownWindow();   
   bFile = FALSE;
  }
 _lclose(hFile);

 delete [] szFile; 
 delete [] szMsg;

 return bFile;
}


*/

