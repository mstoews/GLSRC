#include "ap.h"
#include "gl.h"
#include "db_dept.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "ap_class.h"
#include "ap_setup.h"

extern Code4 cb;
extern char szApp[20];

#define ID_CLEAR 2001

_CLASSDEF(TAPCashAcctsList)
class TAPCashAcctsList : public TDialog
{
 public:
 PTSpread List;
 TAPCashAcctsList(PTWindowsObject AParent, LPSTR AName):TDialog (AParent, AName)
  {   List = new TSpread(this,ID_DISTLIST);  }; 
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};

_CLASSDEF(TAPCashAccts)
class TAPCashAccts : public TMainDialog
{
 public:
 PTAPCashAcctsList AcctList;
 PTapsetup apsetup; 
 TAPCashAccts(PTWindowsObject AParent,LPSTR ATitle,PTapsetup s):TMainDialog(AParent,ATitle)
 {  apsetup = s; };
 virtual void SetupWindow();
 virtual void Add (RTMessage Msg)    = [ID_FIRST + ID_ADD];
 virtual void Search (RTMessage Msg) = [ID_FIRST + 7010];
 virtual void Delete (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void GoNext (RTMessage Msg) = [WM_FIRST + WM_LBUTTONDBLCLK];
 virtual void Update (RTMessage Msg) = [CM_FIRST + CM_SETDATA];
 virtual void Clear  (RTMessage Msg) = [CM_FIRST + ID_CLEAR];
 void Data(unsigned int col,unsigned int row,Field4 *field);
 BOOL Account(int ID, int ID_DEPT);
 void Redraw();
 void ClearSearch();
};

_CLASSDEF(TAPSetup)
class TAPSetup : public TMainDialog
{
 public:
 PTapsetup apsetup;
 TAPSetup(PTWindowsObject AParent,LPSTR ATitle):TMainDialog(AParent,ATitle){};
 BOOL Account(int ID, int ID_DEPT);
 virtual void SetupWindow();
 virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cash (RTMessage Msg) = [ID_FIRST + ID_ACCOUNTS];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};


void TAPCashAccts::Clear(RTMessage)
{
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_DESCRIPT,"");
}

void TAPCashAccts::GoNext(RTMessage)
{

 char* szAccountSeek = new char [apsetup->account->len() + apsetup->depart->len() + 1];
 strcpy(szAccountSeek,GetField(ID_ACCOUNTS));
 strcat(szAccountSeek,GetField(ID_DEPART));
 if(apsetup->seek(szAccountSeek) == 0)
   apsetup->skip();
 else
   apsetup->top();
   SendMessage(HWindow,WM_COMMAND,CM_SETDATA,NULL);
 return;
}

void TAPCashAccts::Search(RTMessage)
{
 
 ClearSearch(); 
 apsetup->top();
 unsigned int i = 0;
 for(apsetup->top();!apsetup->eof(); apsetup->skip())
  {
   ++i;
   Data(1,i,apsetup->account);
   Data(2,i,apsetup->depart);
   Data(3,i,apsetup->name1);
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
  apsetup->seek(szAccount);
 }
  SetField(ID_ACCOUNTS,apsetup->account);
  SetField(ID_DEPART,apsetup->depart);
  SetField(ID_DESCRIPT,apsetup->name1); 
}

void TAPCashAccts::ClearSearch()
{
 AcctList->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
}
void TAPCashAccts::Data(unsigned int col,unsigned int row, Field4 *field)
{
 AcctList->List->SetData(col,row,field->str());
}

void TAPCashAccts::Redraw()
{
 unsigned int i = 0;
 AcctList->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 AcctList->List->SetBool(SSB_REDRAW,FALSE);
 for(apsetup->top();!apsetup->eof(); apsetup->skip())
  {
   ++i;
   Data(1,i,apsetup->account);
   Data(2,i,apsetup->depart);
   Data(3,i,apsetup->name1);
  }
 AcctList->List->SetBool(SSB_REDRAW,TRUE);
}

void TAPCashAccts::SetupWindow()
{
 apsetup->SetIndex("cash");
 AcctList = new TAPCashAcctsList(this,"AP_VENDORLIST");
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);
 SetCaption("General Ledger Bank Accounts");
 Redraw();
 AcctList->Show(SW_SHOW);
}



void TAPCashAccts::Add(RTMessage Msg)
{
 if(!Account(ID_ACCOUNTS,ID_DEPART))
 {
  return;
 }
 
 char* szAccountSeek = new char [apsetup->account->len() + apsetup->depart->len() + 1];
 strcpy(szAccountSeek,GetField(ID_ACCOUNTS));
 strcat(szAccountSeek,GetField(ID_DEPART));
 if(apsetup->seek(szAccountSeek) != 0)
 {
  apsetup->appendStart();
  apsetup->name1->assign(GetField(ID_DESCRIPT));
  apsetup->account->assign(GetField(ID_ACCOUNTS));
  apsetup->depart->assign(GetField(ID_DEPART));
  apsetup->type->assign("CA");
  apsetup->append();
  Clear(Msg);
 }
 else if(MessageBox(HWindow,"Already added. Do you want to modify?","Modify",MB_ICONQUESTION|MB_YESNO) ==IDYES)
 {
  Modify(Msg);
  Clear(Msg);
 }
 Redraw();
 delete [] szAccountSeek;
}

void TAPCashAccts::Modify(RTMessage)
{
 apsetup->name1->assign(GetField(ID_DESCRIPT));
 apsetup->account->assign(GetField(ID_ACCOUNTS));
 apsetup->depart->assign(GetField(ID_DEPART));
 apsetup->type->assign("CA");
 apsetup->flush();

}

void TAPCashAccts::Delete(RTMessage)
{
 if(MessageBox(HWindow,"Do you want to delete the record?","Delete",MB_ICONQUESTION|MB_YESNO) ==IDYES)
  {
   apsetup->deleteRec();
   apsetup->pack();
   Redraw();
  } 
 SendMessage(HWindow,WM_COMMAND,CM_SETDATA,NULL);
}


void TAPCashAccts::Cancel(RTMessage)
{
 ShutDownWindow();
}



void TAPSetup::Cash(RTMessage)
{
 GetApplication()->ExecDialog(new TAPCashAccts(this,"AP_SETCASH",apsetup));
}


char  *GetProfile (char *szSection,char *Item)
{
  static char *szProfile = new char[40];
  GetPrivateProfileString(szSection,Item,"",szProfile,20,"GL.INI");
  return szProfile;
}

BOOL SetProfile(char *szSection,char *Item,char *szString)
{
 return WritePrivateProfileString(szSection,Item,szString,"GL.INI");
}


void TAPSetup::SetupWindow()
{
 apsetup = new Tapsetup(&cb);
 SetField(101,GetProfile("APCONTROL","DESC"));
 SetField(102,GetProfile("APCONTROL","ACCOUNT"));
 SetField(103,GetProfile("APCONTROL","DEPT"));

 SetField(104,GetProfile("VAT","DESC"));
 SetField(105,GetProfile("VAT","ACCOUNT"));
 SetField(106,GetProfile("VAT","DEPT"));

 SetField(107,GetProfile("COMPANY_REBATE","DESC"));
 SetField(108,GetProfile("COMPANY_REBATE","ACCOUNT"));
 SetField(109,GetProfile("COMPANY_REBATE","DEPT"));

 SetField(110,GetProfile("IND_REBATE","DESC"));
 SetField(111,GetProfile("IND_REBATE","ACCOUNT"));
 SetField(112,GetProfile("IND_REBATE","DEPT"));

}

BOOL TAPSetup::Account(int ID, int ID_DEPT)
 {
   BOOL bFound = FALSE;
   Data4 gl(&cb,"glaccts");
   Field4 account(gl,"account");
   Field4 dept(gl,"dept");
   gl.SetIndex("accounts");
   char *szAccount = new char[account.len()+dept.len()+1];
   strcpy(szAccount,GetField(ID));
   strcat(szAccount,GetField(ID_DEPT));
   if(gl.seek(szAccount) == 0)
    bFound = TRUE;
   else
    {

	  wsprintf(szMsg,StrTrim(GetString(8129)),StrTrim(GetField(ID)),StrTrim(GetField(ID_DEPT)));
	  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
    }
   gl.close();
   
   return bFound;
}

BOOL TAPCashAccts::Account(int ID, int ID_DEPT)
 {
   BOOL bFound = FALSE;
   Data4 gl(&cb,"glaccts");
   Field4 account(gl,"account");
   Field4 dept(gl,"dept");
   gl.SetIndex("accounts");
   char *szAccount = new char[account.len()+dept.len()+1];
   strcpy(szAccount,GetField(ID));
   strcat(szAccount,GetField(ID_DEPT));
   if(gl.seek(szAccount) == 0)
    bFound = TRUE;
   else
    {

	  wsprintf(szMsg,StrTrim(GetString(8129)),StrTrim(GetField(ID)),StrTrim(GetField(ID_DEPT)));
     MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
    }
   gl.close();   
   return bFound;
}


void TAPSetup::Ok(RTMessage)
{
if(MessageBox(HWindow,GetString(8124),szApp,MB_YESNO) == IDYES)
 {

  if(Account(102,103))
  {
   SetProfile("APCONTROL","ACCOUNT",GetField(102));
   SetProfile("APCONTROL","DEPT",GetField(103));
  }
  if(Account(105,106))
  {
   SetProfile("VAT","ACCOUNT",GetField(105));
   SetProfile("VAT","DEPT",GetField(106));
  }
  if(Account(108,109))
  {
   SetProfile("COMPANY_REBATE","ACCOUNT",GetField(108));
   SetProfile("COMPANY_REBATE","DEPT",GetField(109));
  }
  if(Account(111,112))
  {
   SetProfile("IND_REBATE","ACCOUNT",GetField(111));
   SetProfile("IND_REBATE","DEPT",GetField(112));
  }
 }
}

void TAPSetup::Cancel(RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGlWindow::APSetup   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPSetup(this,"AP_SETUP"));
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
 SetCaption("Account Payable Bank Accounts");
 HourGlassOff();
}

