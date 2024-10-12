#include "ar.h"
#include "gl.h"
#include "db_dept.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "ar_class.h"

extern Code4 cb;

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
 void Data(unsigned int col,unsigned int row,Field4 *field);
 void ClearSearch();
};

_CLASSDEF(TAPSetup)
class TAPSetup : public TMainDialog
{
 public:
 PTapsetup apsetup;
 TAPSetup(PTWindowsObject AParent,LPSTR ATitle):TMainDialog(AParent,ATitle){};
 virtual void SetupWindow();
 virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cash (RTMessage Msg) = [ID_FIRST + ID_ACCOUNTS];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};

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
 Show(SW_HIDE);
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

void TAPCashAccts::SetupWindow()
{
 apsetup->SetIndex("cash");
 AcctList = new TAPCashAcctsList(this,"AR_VENDORLIST");
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);
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

void TAPCashAccts::Add(RTMessage)
{
 AcctList->Show(SW_HIDE);
 char* szAccountSeek = new char [apsetup->account->len() + apsetup->depart->len() + 1];
 strcpy(szAccountSeek,GetField(ID_ACCOUNTS));
 strcat(szAccountSeek,GetField(ID_DEPART));
 if(apsetup->seek(szAccountSeek) != 0)
 {
  apsetup->appendStart(0);
  apsetup->name1->assign(GetField(ID_DESCRIPT));
  apsetup->account->assign(GetField(ID_ACCOUNTS));
  apsetup->depart->assign(GetField(ID_DEPART));
  apsetup->type->assign("CA");
  apsetup->append();
 }
 if(MessageBox(HWindow,"Already added. Do you want to modify?","Modify",MB_ICONQUESTION|MB_YESNO) ==IDYES)
 {
  apsetup->name1->assign(GetField(ID_DESCRIPT));
  apsetup->account->assign(GetField(ID_ACCOUNTS));
  apsetup->depart->assign(GetField(ID_DEPART));
  apsetup->type->assign("CA");
  apsetup->flush();
 }
 delete [] szAccountSeek;
}

void TAPCashAccts::Modify(RTMessage)
{
 apsetup->name1->assign(GetField(ID_DESCRIPT));
 apsetup->account->assign(GetField(ID_ACCOUNTS));
 apsetup->depart->assign(GetField(ID_DEPART));
 apsetup->flush();
}

void TAPCashAccts::Delete(RTMessage)
{
 if(MessageBox(HWindow,"Do you want to delete the record?","Delete",MB_ICONQUESTION|MB_YESNO) ==IDYES)
  {
   apsetup->deleteRec();
   apsetup->pack();
  } 
 SendMessage(HWindow,WM_COMMAND,CM_SETDATA,NULL);
}


void TAPCashAccts::Cancel(RTMessage)
{
 ShutDownWindow();
}



void TAPSetup::Cash(RTMessage)
{
 GetApplication()->ExecDialog(new TAPCashAccts(this,"AR_SETCASH",apsetup));
}

void TAPSetup::SetupWindow()
{
 apsetup = new Tapsetup(&cb);
 apsetup->top();
 int ID = 100;
 while(!apsetup->eof())
 {
  ID++;
  SetField(ID,apsetup->name1);
  ID++;
  SetField(ID,apsetup->account);
  ID++;
  SetField(ID,apsetup->depart);
  apsetup->skip();
 }
}

void TAPSetup::Ok(RTMessage)
{
 int ID = 100;
 apsetup->top();
 while(!apsetup->eof())
 {
  ID++;
  apsetup->name1->assign(GetField(ID));
  ID++;
  apsetup->account->assign(GetField(ID));
  ID++;
  apsetup->depart->assign(GetField(ID));
  apsetup->flush();
  apsetup->skip();
 }
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TAPSetup::Cancel(RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGlWindow::APSetup   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPSetup(this,"AR_SETUP"));
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
 HourGlassOff();
}

