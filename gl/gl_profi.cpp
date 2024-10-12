
#include <tbowl.h>
#include "glh.h"
#include "gl_dlg.h"
#include "glclass.h"
#include "gl_main.h"
#include "gl_profi.h"
#include "gl_prof.hpp"
#include "gl_pds.hpp"
#include "gl_setat.hpp"
#include "inputdia.h"


extern Code4 cb;
extern char szApp[20];
extern char CurDir[128];
extern HINSTANCE hInst;
GL_PROF gl_prof;
GL_PDS  gl_pds;
GL_SETAT gl_setat;
#define   ID_CLEAR 		2001
#define   ID_PREVIOUSYR 	6014
#define   PERIOD_LEN 		9
#define   DESCRIPT_LEN 		21
#define   ID_RETAINED 		1001
#define   ID_RETAINED_DP 	1002
#define   ID_VAT      		1003
#define   ID_VAT_DP   		1004
#define   ID_PREPAID_VAT 	1005
#define   ID_PREPAID_VAT_DP 	1006
#define   ID_COMPANY   		1007
#define   ID_COMPANY_DP 	1008
#define   ID_INDIVIDUAL 	1009
#define   ID_INDIVIDUAL_DP 	1010
#define   ID_AP_ACCT       	1011
#define   ID_AP_ACCT_DP    	1012
#define   ID_AR_ACCT       	1013
#define   ID_AR_ACCT_DP    	1014
#define   ID_SALES         	1015
#define   ID_SALES_DP      	1016
#define   ID_DIRECTORY        1017
#define   ID_WITHHOLDING      4026

_CLASSDEF(TAPCashAcctsList)
class TAPCashAcctsList : public TMainDialog
{
 public:
 PTSpread List;
 TAPCashAcctsList(PTWindowsObject AParent,int AName):TMainDialog (AParent, AName)
  {   List = new TSpread(this,ID_DISTLIST);  };
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};

_CLASSDEF(TAPCashAccts)
class TAPCashAccts : public TMainDialog
{
 public:
 PTAPCashAcctsList AcctList;
 TAPCashAccts(PTWindowsObject AParent,int ATitle):TMainDialog(AParent,ATitle){};
 virtual void SetupWindow();
 virtual void Ok    (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Search (RTMessage Msg) = [ID_FIRST + 7010];
 virtual void Delete (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void GoNext (RTMessage Msg) = [WM_FIRST + WM_LBUTTONDBLCLK];
 virtual void Update (RTMessage Msg) = [CM_FIRST + CM_SETDATA];
 virtual void Clear  (RTMessage Msg) = [CM_FIRST + ID_CLEAR];
 BOOL Account(int ID, int ID_DEPARTMENT);
 void Data(unsigned int col,unsigned int row,Field4* field);
 void Redraw();
 void ClearSearch();
};


_CLASSDEF(TGLSetAccounts)
class TGLSetAccounts : public TMainDialog
{
 public:
 TGLSetAccounts(PTWindowsObject AParent, int ATitle): TMainDialog(AParent,ATitle){};
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cash           (RTMessage Msg) = [ID_FIRST + 2236];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 BOOL Account(int ID, int ID_DEPARTMENT);
};


void TGLSetAccounts::Cash(RTMessage)
{
  GetApplication()->ExecDialog(new TAPCashAccts(this,1034));
}

void TAPCashAccts::Clear(RTMessage)
{
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_DESCRIPT,"");
}

void TAPCashAccts::GoNext(RTMessage)
{

 char* szAccountSeek = new char [gl_setat.account->len() + gl_setat.dept->len() + 1];
 strcpy(szAccountSeek,GetField(ID_ACCOUNTS));
 strcat(szAccountSeek,GetField(ID_DEPART));
 if(gl_setat.seek(szAccountSeek) == 0)
	gl_setat.skip();
 else
	gl_setat.top();
	SendMessage(HWindow,WM_COMMAND,CM_SETDATA,NULL);
 return;
}

void TAPCashAccts::Search(RTMessage)
{

 ClearSearch();
 gl_setat.top();
 unsigned int i = 0;
 for(gl_setat.top();!gl_setat.eof(); gl_setat.skip())
  {
	++i;
	Data(1,i,gl_setat.account);
	Data(2,i,gl_setat.dept);
	Data(3,i,gl_setat.name);
  }
 AcctList->Show(SW_SHOW);
}

void TAPCashAcctsList::HandleListBox(RTMessage)
{
 SendMessage(Parent->HWindow,WM_COMMAND,CM_SETDATA,NULL);
 SetFocus(GetDlgItem(Parent->HWindow,ID_DESCRIPT));
}

void TAPCashAccts::Update(RTMessage)
{
 char szAccount[12];
 char szDept[5];
 if(IsWindowVisible(AcctList->HWindow))
 {
  SS_COORD Col,Row;
  AcctList->List->GetActiveCell(&Col,&Row);
  AcctList->List->GetData(1,Row,szAccount);
  AcctList->List->GetData(2,Row,szDept);
  strcat(szAccount,szDept);
  gl_setat.seek(szAccount);
 }
  SetField(ID_ACCOUNTS,gl_setat.account->str());
  SetField(ID_DEPART,gl_setat.dept->str());
  SetField(ID_DESCRIPT,gl_setat.name->str());
}

void TAPCashAccts::ClearSearch()
{
 AcctList->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
}
void TAPCashAccts::Data(unsigned int col,unsigned int row, Field4* field)
{
 AcctList->List->SetData(col,row,field->str());
}

void TAPCashAccts::Redraw()
{
 unsigned int i = 0;
 AcctList->List->SetBool(SSB_REDRAW,FALSE);
 AcctList->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 for(gl_setat.top();!gl_setat.eof(); gl_setat.skip())
  {
	++i;
	Data(1,i,gl_setat.account);
	Data(2,i,gl_setat.dept);
	Data(3,i,gl_setat.name);
  }
 AcctList->List->SetBool(SSB_REDRAW,TRUE);
}

void TAPCashAccts::SetupWindow()
{
  TMainDialog::SetupWindow();
 gl_setat.SetIndex("cash");
 AcctList = new TAPCashAcctsList(this,1011);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);
 //"General Ledger Bank Accounts"
 SetBitmap(2236);
 SetCaption(GetString(6023));
 Redraw();
 AcctList->Show(SW_SHOW);
}

BOOL TAPCashAccts::Account(int ID, int ID_DEPARTMENT)
 {
	BOOL bFound = FALSE;
	Data4 gl(&cb,"glaccts");
	Field4 account(gl,"account");
	Field4 dept(gl,"dept");
	gl.SetIndex("accounts");
	char *szAccount = new char[account.len()+dept.len()+1];
	strcpy(szAccount,GetField(ID));
	strcat(szAccount,GetField(ID_DEPARTMENT));
	if(gl.seek(szAccount) == 0)
	 bFound = TRUE;
	else
	 {

	  char *szDept = strcpy(new char [5],StrTrim(GetField(ID_DEPARTMENT)));
	  wsprintf(szMsg,StrTrim(GetString(8121)),StrTrim(GetField(ID)),szDept);
	  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);

	  delete szDept;
	 }
	gl.close();
	return bFound;
}


void TAPCashAccts::Ok(RTMessage Msg)
{
 if(!Account(ID_ACCOUNTS,ID_DEPART))
 {
  return;
 }

 char* szAccountSeek = new char [gl_setat.account->len() + gl_setat.dept->len() + 1];
 strcpy(szAccountSeek,GetField(ID_ACCOUNTS));
 strcat(szAccountSeek,GetField(ID_DEPART));
 if(gl_setat.seek(szAccountSeek) != 0)
 {
  gl_setat.appendStart();
  gl_setat.name->assign(GetField(ID_DESCRIPT));
  gl_setat.account->assign(GetField(ID_ACCOUNTS));
  gl_setat.dept->assign(GetField(ID_DEPART));
  gl_setat.type->assign("CA");
  gl_setat.append();
  Clear(Msg);
 }
 else if(MessageBox(HWindow,GetString(6022),szApp,MB_ICONQUESTION|MB_YESNO) ==IDYES)
 {
  Modify(Msg);
  Clear(Msg);
 }
 Redraw();
 delete [] szAccountSeek;
}

void TAPCashAccts::Modify(RTMessage)
{
 gl_setat.name->assign(GetField(ID_DESCRIPT));
 gl_setat.account->assign(GetField(ID_ACCOUNTS));
 gl_setat.dept->assign(GetField(ID_DEPART));
 gl_setat.type->assign("CA");
 gl_setat.flush();

}

void TAPCashAccts::Delete(RTMessage)
{
 //"Do you want to delete the record?"
 if(MessageBox(HWindow,GetString(6024),szApp,MB_ICONQUESTION|MB_YESNO) ==IDYES)
  {
   gl_setat.deleteRec();
   gl_setat.pack();
   Redraw();
  } 
 SendMessage(HWindow,WM_COMMAND,CM_SETDATA,NULL);
}


void TAPCashAccts::Cancel(RTMessage)
{
 ShutDownWindow();
}


void TGLSetAccounts::Cancel (RTMessage)
{
 gl_setat.close();
 ShutDownWindow();
}

BOOL TGLSetAccounts::Account(int ID, int ID_DEPARTMENT)
 {
   BOOL bFound = FALSE;
   Data4 gl(&cb,"glaccts");
   Field4 account(gl,"account");
   Field4 dept(gl,"dept");
   gl.SetIndex("accounts");
	char *szAccount = new char[account.len()+dept.len()+1];
	strcpy(szAccount,GetField(ID));
	strcat(szAccount,GetField(ID_DEPARTMENT));
   if(gl.seek(StrTrim(szAccount)) == 0)
	 bFound = TRUE;
	else
	 {

	  char *szDept = strcpy(new char [5],StrTrim(GetField(ID_DEPARTMENT)));
	  wsprintf(szMsg,StrTrim(GetString(8121)),StrTrim(GetField(ID)),szDept);
	  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);

	  delete szDept;
	 }
   gl.close();   
   return bFound;
}


void TGLSetAccounts::Ok(RTMessage)
{

 int i = 0;
 int j  =0;
 for(i=1001;i<1009;i++)
 {
  j=i+2000;
  if(!Account(i,j))
   return;
 }

 gl_setat.SetIndex("GL_ACCTS");
 i=1001;
 gl_setat.top();
 while(!gl_setat.eof())
  {
	j=i+2000;
	gl_setat.account->assign(GetField(i));
	gl_setat.dept->assign(GetField(j));
	gl_setat.flush();
	gl_setat.skip();
	i++;
  }
 gl_setat.unlock();
 gl_setat.close();
 ShutDownWindow();
}

void TGLSetAccounts::SetupWindow()
{
 TMainDialog::SetupWindow();
 gl_setat.open(cb);
 gl_setat.SetIndex("gl_accts");
 int i =1001;
 int n =0;
 int j = 0;
 gl_setat.top();
 while (!gl_setat.eof())
 {
  n=i+1000;
  j=i+2000;
  SetField(n,gl_setat.name->str());     // 2000's
  SetField(i,gl_setat.account->str());  // 1000's
  SetField(j,gl_setat.dept->str());     // 3000's
  gl_setat.skip();
  i++;
 }
 
}



void TGLSetPeriodDates::ResetDepreciation (RTMessage)
{
  Data4 glfa(cb,"gl_fa");
  Field4 mode(glfa,"mode");
  // "Reset the depreciation journal flag."
  if(MessageBox(HWindow,GetString(6025),szApp,MB_ICONINFORMATION | MB_YESNO) == IDYES)
  {
   for (glfa.top(); !glfa.eof(); glfa.skip())
   {
      mode.assign(" ");
      glfa.flush();
   }
  }
}


void TGLSetPeriodDates::Ok           (RTMessage)
{
 Date4 szStart;
 Date4 szEnd;
 gl_pds.top();
 for(int i=4001;i<4013;i++)
 {
  szStart.assign(GetField(i),GetString(4024));
  gl_pds.periods.assign(szStart.str());

  szEnd.assign(GetField(i+1000),GetString(4024));
  gl_pds.periode.assign(szEnd.str());

  gl_pds.pd_desc.assign(GetField(i+2000));

  if(SendDlgItemMsg(i+2012,BM_GETCHECK,NULL,NULL) == 1)
	gl_pds.status.assign("1");
  else
	gl_pds.status.assign("0");

  gl_pds.flush();
  gl_pds.skip();
 }
 gl_pds.unlock();
 ShutDownWindow();
}

void TGLSetPeriodDates::Cancel       (RTMessage)
{
 ShutDownWindow();
}

void TGLSetPeriodDates::SetupWindow()
{
 TMainDialog::SetupWindow();
 gl_pds.top();
 Date4 szStart;
 Date4 szEnd;
 char szStartDate[11];
 char szEndDate[11];
 for(int i=4001;i<4013;i++)
  {
	szStart.assign(gl_pds.periods.str(),"CCYYMMDD");
	szStart.format(szStartDate,GetString(4024));
	szEnd.assign(gl_pds.periode.str(),"CCYYMMDD");
	szEnd.format(szEndDate,GetString(4024));
	SetField(i,szStartDate);
	SetField(i+1000,szEndDate);
	SetField(i+2000,gl_pds.pd_desc.str());
	if(strcmp(gl_pds.status.str(),"1") == 0)
	{
	SendDlgItemMsg(i+2012,BM_SETCHECK,1,NULL);
	}
	gl_pds.skip();
  }
}


void TGLProfile::GetCalc(RTMessage)
{
 //"Not fooling around here.\nThis will kill all the data....\nOnly one chance."
 if(MessageBox(HWindow,GetString(6026),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	Data4 gl_sys(cb,"clear");
	gl_sys.SetIndex("fdbf");
	gl_sys.top();
	Data4 data;
	Field4 fDbf(gl_sys,"fdbf");
	while(!gl_sys.eof())
	 {
	  data.open(cb,StrTrim(fDbf.str()));
	  data.zap();
     data.pack();
     data.reindex();
     data.close();
     gl_sys.skip();
    }
   gl_sys.close();

  Data4 glaccts(cb,"glaccts");
  glaccts.SetIndex("accounts");
  for(glaccts.top();!glaccts.eof();glaccts.skip())
   {
     Field4 field;
      for(int fieldNum = 1; fieldNum<=glaccts.numFields(); fieldNum++)
       {
        field.init(glaccts,fieldNum);
         switch(field.type())
         {
          case r4num: field.assignDouble(0); break;
          default: break;
			}
		}
  }
 }
}

void TGLProfile::SetAccounts(RTMessage)
{
 GetApplication()->ExecDialog(new TGLSetAccounts(this,1033));
}

void TGLProfile::SetupWindow()
{
 TMainDialog::SetupWindow();
 gl_prof.open(cb);

 // Set the opening and closing periods
 gl_pds.open(cb);
 gl_pds.SetIndex("period");
 gl_pds.top();

 char szDate[11];
 Date4 szStartDate(gl_pds.periods.str());
 szStartDate.format(szDate,GetString(4024));

 SetField(ID_PERIODS,szDate);
 gl_pds.bottom();
 Date4 szEndDate(gl_pds.periode.str());
 szEndDate.format(szDate,GetString(4024));
 SetField(ID_PERIODE,szDate);

 gl_prof.top();
 SetField(ID_VENDORNO,gl_prof.company.str());
 SetField(ID_ADDRESS1,gl_prof.address1.str());
 SetField(ID_ADDRESS2,gl_prof.address2.str());
 SetField(ID_ADDRESS3,gl_prof.address3.str());
 SetField(ID_POST,    gl_prof.postal.str());
 SetField(ID_TEL,     gl_prof.telephone.str());
 SetField(ID_FAX,     gl_prof.fax.str());
 SetField(ID_CONTACT, gl_prof.contact.str());
 SetField(ID_DIRECTORY, CurDir);
 GetWithHolding();
 SetBitmap(4000);
 SetBitmap(1125);
}

void TGLProfile::GetWithHolding()
{
  char szWithHolding[2];
  GetPrivateProfileString(GetString(8150),"TAX","T",szWithHolding,sizeof(szWithHolding),"GL.INI");
  if(strcmp(szWithHolding,"T") == 0)
	SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,TRUE,NULL);
  else
	SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);
}

void TGLProfile::SetWithHolding()
{
  char szWithHolding[2];
  if(SendDlgItemMsg(ID_WITHHOLDING,BM_GETCHECK,NULL,NULL))
	strcpy(szWithHolding,"T");
  else
	strcpy(szWithHolding,"F");
  WritePrivateProfileString(GetString(8150),"TAX",szWithHolding,"GL.INI");
}

void TGLProfile::WithHolding(RTMessage)
{
 if(SendDlgItemMsg(ID_WITHHOLDING,BM_GETCHECK,NULL,NULL))
	SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);
 else
	SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,TRUE,NULL);
}

void TGLProfile::Ok  (RTMessage)
{
 gl_prof.top();
 if(gl_prof.eof())
 gl_prof.appendStart();
 gl_prof.company.assign(GetField(ID_VENDORNO));
 gl_prof.address1.assign(GetField(ID_ADDRESS1));
 gl_prof.address2.assign(GetField(ID_ADDRESS2));
 gl_prof.address3.assign(GetField(ID_ADDRESS3));
 gl_prof.postal.assign(GetField(ID_POST));
 gl_prof.telephone.assign(GetField(ID_TEL));
 gl_prof.fax.assign(GetField(ID_FAX));
 gl_prof.contact.assign(GetField(ID_CONTACT));
 gl_prof.UpdateUserTime(szUser);
 if(gl_prof.eof())
  gl_prof.append();
 else
  gl_prof.flush();
 gl_prof.unlock();
 SetWithHolding();

 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGLProfile::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGLProfile::SetPeriodDates (RTMessage)
{
 GetApplication()->ExecDialog(new TGLSetPeriodDates(this,1028));
}

void TGlWindow::GLCompanySet (RTMessage)
{
 GetApplication()->ExecDialog(new TGLProfile(this,1021));
 SendMessage(HStatusBar, SBRM_SETITEMTEXT,1,(LPARAM)GetCompanyName());
}





void TAPCashAcctsList::SetupWindow()
{
 TDialog::SetupWindow();
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;
 HourGlassOn(); 
 GetClientRect(HWindow,&rc);  
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));            
 MoveWindow(List->HWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 short d1Width,d2Width;
 List->SetColWidth(1,7);
 List->SetColWidth(2,5);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 short ColWidth = (rc.right - rc.left) - d1Width - d2Width - GetSystemMetrics(SM_CXHSCROLL);
 List->SetColWidthInPixels(3,ColWidth);
 
 List->SetBool(SSB_SHOWROWHEADERS,FALSE);
 List->SetBool(SSB_ALLOWUSERSELBLOCK,TRUE);
 List->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_ALLOWUSERRESIZE,FALSE);
 List->SetBool(SSB_AUTOSIZE,TRUE);


 //lfFont.lfHeight          = -13;
 lfFont.lfHeight          = -(GetFontSize());
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = 1; //default character set.
 lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);

 List->SetMaxCols( 3);
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 List->SetData(1,SS_HEADER, "Account");
 List->SetData(2,SS_HEADER, "Dept");
 List->SetData(3,SS_HEADER, "Description");
 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
 // "Account Payable Bank Accounts"
 SetCaption(GetString(6027));
 HourGlassOff();
}

