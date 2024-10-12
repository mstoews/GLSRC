
#include <tbowl.h>
#include <commdlg.h>
#include <dir.h>
#include <direct.h>
#include "gl.h"
#include "glclass.h"
#include "gl_main.h"
#include "d4all.hpp"
#include "gl_manil.h"
#include "gl_dlg.h"
#include "db_fa.h"
#include "db_btch.h"
#include "db_btchd.h"



#include "gl_man.h"
#include <ctype.h>
#include "tlist_w.h"
#include "gl_asstl.h"
#include "gl_fatyp.h"
#include "gl_fa.h"
#include "utildlg.h"



#define LINENO 10
#define ID_YEARS_LIFE 6033
#define ID_DECLINING  7024



extern HINSTANCE hInst;
extern Code4 cb;
extern char szApp[20];

BOOL Tglfa::Journal()
{
 if(strcmp(mode->str(),"0") == 0)
  return TRUE;
 else
  return FALSE;
}


_CLASSDEF(TGLCreateDeprSlip)
class TGLCreateDeprSlip : public TMainDialog
{
 char *szPd;
 public:
 Tglfa* glfa;
 Tfatype* fatype;
 Tglbtch* glbtch;
 Tglbtchd* glbtchd;


 TGLCreateDeprSlip(PTWindowsObject AWindow,int ATitle,char *szPeriod):TMainDialog(AWindow,ATitle)
 {
  szPd = new char [20];
  strcpy(szPd,szPeriod);
 };
 virtual void SetupWindow();
 virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 void CreateDetailSlip(unsigned int iBatchno);
 void CreateBatchHeader(unsigned int iBatchno);
};


double TGLFixedAssets::dAccum(void)
  {

   double dAccum = 0;
   double dSalvage = atof(glfa->salvage->str());
	double dCost    = atof(glfa->cost->str());
	double dDepr    = atof(glfa->curr_dep->str());
   int dLife  = atoi(glfa->lifetime->str());
	int dLeft  = atoi(glfa->lifeleft->str());
   if(dLife-dLeft > 0)
	 dAccum = (dCost-dSalvage)-(dLife-dLeft)*dDepr;
   return dAccum;

  };

void TGLCreateDeprSlip::CreateBatchHeader(unsigned int iBatchno)
{
  Date4 dDate;
  dDate.today();
  dDate.format(GetString(4024));
  Str4large szMsg;
  Str4ten szBatchno;
  szBatchno.assignLong(iBatchno,5);
  szMsg.assign(GetString(8118));
  szMsg.add("...");
  szMsg.add(szBatchno.str());
  SetField(4095,szMsg.str());
  glbtch->appendStart();
  glbtch->batchno->assignLong(iBatchno);
  glbtch->mode->assign("1");
  glbtch->date->assign(dDate.str());
  glbtch->period->assignDouble(GetCurrentPeriodn());
  glbtch->descript->assign(GetString(8118));
  glbtch->debit->assignDouble(0);
  glbtch->credit->assignDouble(0);
  glbtch->entryno->assignLong(0);
  glbtch->cheque->assign("");
  glbtch->UpdateUserTime(szUser);
  glbtch->append();
}

void TAssetList::WMSize(RTMessage Msg)
{
  // NEW
  TWindow::WMSize(Msg);
  int iRows = glfa->recCount();

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

void TGLCreateDeprSlip::CreateDetailSlip(unsigned int iBatchno)
{
  Date4 dDate;
  dDate.today();
  dDate.format("MM/DD/CCYY");
  fatype->SetIndex("fatype");

  if(fatype->seek(glfa->fatype->str()) == 0)
  {
	glbtchd->appendStart();
	glbtchd->batchno->assignLong(iBatchno);
	glbtchd->journal->assignLong(0);
	glbtchd->sourcecode->assign("");
	glbtchd->date->assign(dDate.str());
	glbtchd->period->assignDouble(GetCurrentPeriodn());
	glbtchd->reference->assign(glfa->reference->str());
	glbtchd->descript->assign(fatype->expen_desc->str());
	glbtchd->account->assign(fatype->expen_acct->str());
	glbtchd->cheque->assign("");
	glbtchd->dept->assign(fatype->expen_dept->str());
	glbtchd->debit->assignDouble(glfa->dCurr_dep());
   glbtchd->credit->assignDouble(0);
	glbtchd->append();

   glbtchd->appendStart();
	glbtchd->batchno->assignLong(iBatchno);
   glbtchd->journal->assignLong(0);
	glbtchd->sourcecode->assign("");
	glbtchd->date->assign(dDate.str());
   glbtchd->period->assignDouble(GetCurrentPeriodn());
   glbtchd->reference->assign(glfa->reference->str());
	glbtchd->descript->assign(fatype->accum_desc->str());
   glbtchd->account->assign(fatype->accum_acct->str());
	glbtchd->cheque->assign("");
   glbtchd->dept->assign(fatype->accum_dept->str());
	glbtchd->debit->assignDouble(0);
	glbtchd->credit->assignDouble(glfa->dCurr_dep());
   glbtchd->append();
  }
  else
  {

	wsprintf(szMsg,GetString(8120),glfa->fatype->str());
	MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
  }
}


void TGLCreateDeprSlip::SetupWindow()
{
 SetField(4001,szPd);
 glfa    = new Tglfa(&cb);
 fatype  = new Tfatype(&cb);
 glbtch  = new Tglbtch(&cb);
 glbtchd = new Tglbtchd(&cb);

}

void TGLCreateDeprSlip::Ok(RTMessage)
{
 double dBatchTotal = 0;
 glbtch->bottom();
 int iBatchno = (int) glbtch->Batchno();
 iBatchno++;
 glbtch->top();
 glfa->top();
 unsigned int iEntry = 0;
 if(MessageBox(HWindow,GetString(8110),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
   while (!glfa->eof())
   {
	 iEntry++;
    if(!glfa->Journal())
	  {
      CreateDetailSlip(iBatchno);
		glbtchd->journal->assignLong(iEntry);
		glbtchd->flush();
      double dLife = glfa->dLeft();
		dLife++;
      glfa->mode->assign("0");
		glfa->lifeleft->assignDouble(dLife);
		glfa->UpdateUserTime(szUser);
		glfa->flush();
      dBatchTotal = 1;
     }     
	 glfa->skip();
   }
	if(dBatchTotal > 0)				// if detail has been created
    CreateBatchHeader(iBatchno);		// create the header amount for the same total
  }
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGLCreateDeprSlip::Cancel(RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
 return;
}


_CLASSDEF(TGLFixedDelete)
class TGLFixedDelete : public TGLFixedAssets
{
 public:
 TGLFixedDelete(PTWindowsObject AWindow,int ATitle):TGLFixedAssets(AWindow,ATitle){};
 virtual void SetupWindow();
 virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
};

_CLASSDEF(TGLTypes)
class TGLTypes : public TGLFixedAssets
{
 public: 
 TGLTypes(PTWindowsObject AWindow,int ATitle):TGLFixedAssets(AWindow,ATitle){};
 virtual void SetupWindow();
 virtual void Ok  (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Modify(RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Next     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Top      (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom   (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
};

void TGLFixedAssets::SetupWindow()
{
 TMainDialog::SetupWindow();
 HourGlassOn();
 bTypeList = FALSE;
 if(!TestDatabase())
  return;

 glfa    = new Tglfa(&cb);
 fatype  = new Tfatype(&cb);

 bTypes = FALSE;
 bAsset = FALSE;
 DWORD dwIndex = SendDlgItemMsg(ID_DEPRECIATION,CB_ADDSTRING,NULL,(LPARAM)(LPCSTR)"Straight");
 SendDlgItemMsg(ID_DEPRECIATION,CB_ADDSTRING,NULL,(LPARAM)(LPCSTR)"Declining");
 SendDlgItemMsg(ID_DEPRECIATION,CB_SETCURSEL,dwIndex,NULL);
 ShowWindow(GetDlgItem(HWindow,ID_DECLINING),SW_HIDE);
 ShowWindow(GetDlgItem(HWindow,7025),SW_HIDE);

 TYPES = new TAssetTypesList(this,"",fatype);
 TYPES->Attr.Style |= WS_THICKFRAME;

 TYPES->EnableAutoCreate();
 GetApplication()->MakeWindow(TYPES);

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
 HourGlassOff();
}


void TGLTypes::Delete(RTMessage Msg)
{
 if(MessageBox(HWindow,GetString(8109),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
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
 char *szGLAccount = new char [fatype->asset_acct->len() + 1];
 char *szDept      = new char [fatype->asset_dept->len() + 1];

 szGLAccount = GetField(ID_ASSET_ACCOUNT);
 szDept      = GetField(ID_ASSET_DEPT);
 if(!ValidateGL(szGLAccount,szDept))
  {
	MessageBox(HWindow,GetString(6056),szApp,MB_ICONEXCLAMATION);
	return;
  }
 szGLAccount = GetField(ID_EXPENSE_ACCOUNT);
 szDept      = GetField(ID_EXPENSE_DEPT);
 if(!ValidateGL(szGLAccount,szDept))
  {
	MessageBox(HWindow,GetString(6057),szApp,MB_ICONEXCLAMATION);
	return;
  }

 szGLAccount = GetField(ID_ACCUM_ACCOUNT);
 szDept      = GetField(ID_ACCUM_DEPT);

 if(!ValidateGL(szGLAccount,szDept))
  {
	MessageBox(HWindow,GetString(6058),szApp,MB_ICONEXCLAMATION);
	return;
  }

 if(strlen(StrTrim(GetField(ID_ASSET_TYPE))) == 0)
  return;
  
 if(fatype->seek(GetField(ID_ASSET_TYPE)) != 0)
  {
	fatype->appendStart();
	fatype->fatype->assign(GetField(ID_ASSET_TYPE));
	fatype->fadesc->assign(GetField(ID_DESCRIPT));
	fatype->asset_acct->assign(GetField(ID_ASSET_ACCOUNT));
	fatype->asset_dept->assign(GetField(ID_ASSET_DEPT));
	fatype->asset_desc->assign(GetField(ID_ASSET_DESC));
	fatype->expen_acct->assign(GetField(ID_EXPENSE_ACCOUNT));
	fatype->expen_dept->assign(GetField(ID_EXPENSE_DEPT));
	fatype->expen_desc->assign(GetField(ID_EXPENSE_DESC));
	fatype->accum_acct->assign(GetField(ID_ACCUM_ACCOUNT));
	fatype->accum_dept->assign(GetField(ID_ACCUM_DEPT));
	fatype->accum_desc->assign(GetField(ID_ACCUM_DESC));
	fatype->UpdateUserTime(szUser);
	fatype->append();
	fatype->unlock();
	New(Msg);

  }
  else
  Modify(Msg);

}

void TGLTypes::Modify(RTMessage)
{
 if(MessageBox(HWindow,GetString(8108),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	fatype->fatype->assign(GetField(ID_ASSET_TYPE));
	fatype->fadesc->assign(GetField(ID_DESCRIPT));
	fatype->asset_acct->assign(GetField(ID_ASSET_ACCOUNT));
	fatype->asset_dept->assign(GetField(ID_ASSET_DEPT));
	fatype->asset_desc->assign(GetField(ID_ASSET_DESC));
	fatype->expen_acct->assign(GetField(ID_EXPENSE_ACCOUNT));
	fatype->expen_dept->assign(GetField(ID_EXPENSE_DEPT));
	fatype->expen_desc->assign(GetField(ID_EXPENSE_DESC));
	fatype->accum_acct->assign(GetField(ID_ACCUM_ACCOUNT));
	fatype->accum_dept->assign(GetField(ID_ACCUM_DEPT));
	fatype->accum_desc->assign(GetField(ID_ACCUM_DESC));
	fatype->UpdateUserTime(szUser);
	fatype->flush();
	fatype->unlock();

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
 if(MessageBox(HWindow,GetString(8107),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
 {
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


void TGLFixedAssets::UpdateType (RTMessage)
{
  SetField(ID_ASSET_TYPE,fatype->fatype);
  if(bTypeList)
   SetField(ID_DESCRIPT,fatype->fadesc);
  SetField(ID_ASSET_ACCOUNT,fatype->asset_acct);
  SetField(ID_ASSET_DEPT,fatype->asset_dept);
  SetField(ID_ASSET_DESC,fatype->asset_desc);
  SetField(ID_EXPENSE_ACCOUNT,fatype->expen_acct);
  SetField(ID_EXPENSE_DEPT,fatype->expen_dept);
  SetField(ID_EXPENSE_DESC,fatype->expen_desc);
  SetField(ID_ACCUM_ACCOUNT,fatype->accum_acct);
  SetField(ID_ACCUM_DEPT,fatype->accum_dept);
  SetField(ID_ACCUM_DESC,fatype->accum_desc);
  SetFocus(Parent->HWindow);
  TYPES->Show(SW_HIDE);
  bTypes = FALSE;  
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
	MessageBox(HWindow,szMsg,szApp,MB_ICONINFORMATION);
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
	MessageBox(HWindow,szMsg,szApp,MB_ICONINFORMATION);
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
  char *szGLAccount = new char [fatype->asset_acct->len() + 1];
  char *szDept      = new char [fatype->asset_dept->len() + 1];

 szGLAccount = GetField(ID_ASSET_ACCOUNT);
 szDept      = GetField(ID_ASSET_DEPT);
 if(!ValidateGL(szGLAccount,szDept))
  {
	MessageBox(HWindow,GetString(6056),szApp,MB_ICONEXCLAMATION);
	return;
  }
 szGLAccount = GetField(ID_EXPENSE_ACCOUNT);
 szDept      = GetField(ID_EXPENSE_DEPT);
 if(!ValidateGL(szGLAccount,szDept))
  {
	MessageBox(HWindow,GetString(6057),szApp,MB_ICONEXCLAMATION);
	return;
  }

 szGLAccount = GetField(ID_ACCUM_ACCOUNT);
 szDept      = GetField(ID_ACCUM_DEPT);

 if(!ValidateGL(szGLAccount,szDept))
  {
	MessageBox(HWindow,GetString(6058),szApp,MB_ICONEXCLAMATION);
	return;
  }

  	if(dGetField(ID_SALVAGE) > dGetField(ID_ASSET_COST))
	 {
		MessageBox(HWindow,GetString(9097),szApp,MB_ICONINFORMATION);
		return;
	 }
	if(dGetField(ID_LIFETIME) < dGetField(ID_LIFE_RMD))
	{
	  MessageBox(HWindow,GetString(9098),szApp,MB_ICONINFORMATION);
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
	glfa->unlock();
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
  if(Indx != CB_ERR)
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
	glfa->facode->assign(GetField(ID_FACODE));
	glfa->date_pur->assign(dPurchased.str());
	glfa->fatype->assign(GetField(ID_ASSET_TYPE));
	glfa->descript->assign(GetField(ID_DESCRIPT));
	glfa->reference->assign(GetField(ID_REFERENCE));
	glfa->dep_meth->assign(GetDepMethod());
	glfa->cost->assignDouble(dGetField(ID_ASSET_COST));
	glfa->salvage->assignDouble(dGetField(ID_SALVAGE));

	if(strcmp(StrTrim(GetDepMethod()),"Straight") == 0)
	{
	 double dLife = 0;
	 if(dGetField(ID_LIFETIME) > 0)
	  dLife = dGetField(ID_LIFETIME);
	 else
	  SetField(ID_LIFETIME,dLife);
	 glfa->lifetime->assignDouble(dLife);
	 double dLifeRemaining = 0;
	 if(SendDlgItemMsg(ID_FULLYEAR,BM_GETCHECK,NULL,NULL))
	  dLifeRemaining = 12;
	 else
	  dLifeRemaining = dGetField(ID_LIFE_RMD);

	 glfa->lifeleft->assignDouble(dLifeRemaining);
	 if(dLife > 0)
	  dMonthly = (dGetField(ID_ASSET_COST) - dGetField(ID_SALVAGE))/dLife;
	glfa->curr_dep->assignDouble(dMonthly);
	SetField(ID_MONTHLY_DEP,dMonthly);
	}
	else
	{
	  double dLife =  0;
	  if(dGetField(ID_LIFETIME) > 0)
		dLife = dGetField(ID_LIFETIME);
	  else
		SetField(ID_LIFETIME,dLife);

	 glfa->lifetime->assignDouble(dLife);
	 double dLifeRemaining = 0;
	 if(SendDlgItemMsg(ID_FULLYEAR,BM_GETCHECK,NULL,NULL))
	  dLifeRemaining = 12;
	 else
	  dLifeRemaining = dGetField(ID_LIFE_RMD);
	 glfa->lifeleft->assignDouble(dLifeRemaining);
	 double dPercent = dGetField(ID_DECLINING);
	 glfa->percent->assignDouble(dPercent);

	 if(dPercent > 0)
	  {
		double dYearly = (dGetField(ID_ASSET_COST) - dGetField(ID_SALVAGE))*(dPercent/100);
		glfa->curr_dep->assignDouble(dYearly/12);
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
 if(MessageBox(HWindow,GetString(8102),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	UpdateData();
	glfa->flush();
	glfa->unlock();
	//New(Msg);
	ASSET->List->Redraw();
   SetData(Msg);
  }
}

void TGLFixedAssets::SetData  (RTMessage Msg)
  {
	char szDatePurchased[11];
	Date4 InvoiceDate;
	InvoiceDate.assign(glfa->date_pur->str(),"CCYYMMDD");
	InvoiceDate.format(szDatePurchased,GetString(4024));
	SetField(ID_DATE,szDatePurchased);
	SetField(ID_FACODE,glfa->facode);
	SetField(ID_ASSET_TYPE,glfa->fatype);
	SetField(ID_DESCRIPT,glfa->descript);
	SetField(ID_REFERENCE,glfa->reference);

	if(strcmp(StrTrim(glfa->dep_meth->str()),"Declining") == 0)
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

	if(fatype->seek(glfa->fatype->str()) == 0)
	 UpdateType(Msg);
	SetField(ID_LIFETIME,glfa->lifetime);
	SetField(ID_ASSET_COST,glfa->dCost());
	SetField(ID_LIFE_RMD,glfa->lifeleft);
	SetField(ID_SALVAGE,glfa->salvage);
	SetField(ID_MONTHLY_DEP,glfa->curr_dep);
	SetField(ID_YEARS_LIFE,dGetField(ID_LIFETIME)/12);
	SetField(ID_DECLINING,glfa->percent);

	if(GetField(ID_LIFE_RMD) > 0)
	 {
	  double dAccum = dGetField(ID_MONTHLY_DEP)*dGetField(ID_LIFE_RMD);
	  SetField(6040,dAccum);
	  SetField(6041,glfa->dCost()- double (Str4ptr(glfa->salvage->str())) - dAccum);
	 }

 }

 

 void TGLFixedAssets::Next     (RTMessage Msg)
 {
  glfa->skip();
  SetData(Msg);
 }

 void TGLFixedAssets::Previous (RTMessage Msg)
 {
  glfa->skip(-1);
  SetData(Msg);
 }

 void TGLFixedAssets::Top      (RTMessage Msg)
 { 
   glfa->top();
	SetData(Msg);
 }

 void TGLFixedAssets::Bottom   (RTMessage Msg)
 {
  glfa->bottom();
  SetData(Msg);
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
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}







void TAssetList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TAssetList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 HINSTANCE hInst = GetApplication()->hInstance;
 SetCaption(GetString(GetApplication()->hInstance,6059));

 List->SetFields("gl_fa","facode",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6060));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6093));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6044));
}

void TAssetList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 char *szBuffer1 = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);
 glfa->SetIndex("facode");
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(glfa->seek(szBuffer1) == 0)
  {
	SendMessage(Parent->HWindow,WM_COMMAND,CM_UPDATE,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
	Show(SW_HIDE);
 delete [] szBuffer1;
 return;
}

void TAssetTypesList::WMSize(RTMessage Msg)
{
	 // NEW
  TWindow::WMSize(Msg);
  int iRows = fatype->recCount();

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
 HINSTANCE hInst = GetApplication()->hInstance;
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
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(fatype->seek(szBuffer1) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_UPDATETYPES,NULL);
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
   Show(SW_HIDE);
 delete [] szBuffer1;
 return;
}

void TManilaGL::GLFixedAdd    (RTMessage){GetApplication()->ExecDialog(new TGLFixedAssets(this,4004));}
void TManilaGL::GLFixedDisposal(RTMessage){GetApplication()->ExecDialog(new TGLFixedDelete(this,1001));}
void TManilaGL::GLFixedTypes   (RTMessage){GetApplication()->ExecDialog(new TGLTypes(this,4002));}
void TManilaGL::GLFixedSlip    (RTMessage){GetApplication()->ExecDialog(new TGLCreateDeprSlip(this,4003,GetCurrentPeriod()));}


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
	MessageBox(GetFocus(),szMsg,szApp,MB_ICONINFORMATION);
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
	MessageBox(GetFocus(),szMsg,szApp,MB_ICONINFORMATION);
   AWindow->ShutDownWindow();   
   bFile = FALSE;
  }
 _lclose(hFile);

 delete [] szFile; 
 delete [] szMsg;

 return bFile;
}


*/

