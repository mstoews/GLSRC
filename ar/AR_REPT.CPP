/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Custom Report Dialog Box.
*/
#include "ar.h"
#include "gl_dlg.h"
#include "db_rept.h"
_CLASSDEF(TGLCustomReport)
class TGLCustomReport : public TDialog
{
 public:
 PTListBox ReportList;
 PTStatic  ReportName;
 PTglcrept glrept;
 TGLCustomReport(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName)
 {
  ReportList = new TListBox(this,ID_ACCTLIST);
  ReportName = new TStatic(this,ID_PRINT,40);
 };
 virtual void SetupWindow();
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void HandlePrint    (RTMessage Msg) = [ID_FIRST + IDM_PRINT];
 virtual void HandleListBox  (RTMessage Msg) = [ID_FIRST + ID_ACCTLIST];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Add            (RTMessage Msg) = [ID_FIRST + ID_ACCTADD];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_ACCTDEL];
 virtual void Modify	     (RTMessage Msg) = [ID_FIRST + ID_ACCTMOD];
 virtual void ResetListBox();
};

#define ID_PRINTERROR 16009
#define ID_NAME 1001
	


static OPENFILENAME ofn;
static char szFileDialogText[] = "Worksheet (*.rpt)\0*.rpt\0All Files(*.*)\0*.*";
static char PathName[128] = "\0";
static char FileName[128] = "\0";
extern Code4 cb;

_CLASSDEF(TGLAddReport)
class TGLAddReport : public TMainDialog
{
 public:
  PTglcrept glrept;
  char szFileName[128];
  TGLAddReport(PTWindowsObject AParent, LPSTR AName,PTglcrept repts,PTModule AModule = NULL): TMainDialog(AParent,AName)
  {
   glrept = repts;
  };
  virtual void SetupWindow();
  virtual void Ok           (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel       (RTMessage Msg) = [ID_FIRST + IDCANCEL]{
   ShutDownWindow();
  }
};


_CLASSDEF(TGLModifyReport)
class TGLModifyReport : public TDialog
{
 public:
  PTglcrept glrept;
  TGLModifyReport(PTWindowsObject AParent, LPSTR AName,PTglcrept repts,PTModule AModule = NULL): TDialog(AParent,AName)
  {
   glrept = repts;
  };
  virtual void SetupWindow()
   {
    SetWindowText(HWindow,"Modify Report Definition");
    SetWindowText(GetDlgItem(HWindow,IDOK),"&Modify");
    SetDlgItemText(HWindow,ID_NAME,(LPCSTR)glrept->cname->str());
    SetDlgItemText(HWindow,ID_DESCRIPT,(LPCSTR)glrept->descript->str());
    SetDlgItemText(HWindow,ID_FNAME,(LPCSTR)glrept->fname->str());
    };
  virtual void Ok           (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel       (RTMessage Msg) = [ID_FIRST + IDCANCEL]{ShutDownWindow();}
};

_CLASSDEF(TGLDeleteReport)
class TGLDeleteReport : public TDialog
{
 public:
  PTglcrept glrept;
  TGLDeleteReport(PTWindowsObject AParent, LPSTR AName,PTglcrept repts,PTModule AModule = NULL): TDialog(AParent,AName)
  {
   glrept = repts;
  };
  virtual void SetupWindow(){
   SetWindowText(HWindow,"Delete Report Definition");
   SetWindowText(GetDlgItem(HWindow,IDOK),"&Delete");
   SetDlgItemText(HWindow,ID_NAME,(LPCSTR)glrept->cname->str());
   SetDlgItemText(HWindow,ID_DESCRIPT,(LPCSTR)glrept->descript->str());
   SetDlgItemText(HWindow,ID_FNAME,(LPCSTR)glrept->fname->str());
  };
  virtual void Ok           (RTMessage Msg) = [ID_FIRST + IDOK]{
   glrept->deleteRec();
   glrept->pack();
   Cancel(Msg);
  };
  virtual void Cancel       (RTMessage Msg) = [ID_FIRST + IDCANCEL]{
   ShutDownWindow();}
};

void TGLModifyReport::Ok           (RTMessage)
  {
   char szName[20];
   char szDescription[40];
   char szFName[128];
   GetDlgItemText(HWindow,ID_NAME,szName,sizeof(szName));
   GetDlgItemText(HWindow,ID_DESCRIPT,szDescription,sizeof(szDescription));
   GetDlgItemText(HWindow,ID_FNAME,szFName,sizeof(szFName));
   if(*szName == 0 || *szFName == 0 || *szDescription == 0){
   MessageBox(HWindow,"All fields must be entered.","ERROR",MB_ICONEXCLAMATION);
   return;
   }  
   glrept->cname->assign(szName);
   glrept->fname->assign(szFName);
   glrept->descript->assign(szDescription);
   glrept->flush();
   ShutDownWindow();
   return;
  }


void TGLAddReport::SetupWindow()
{

  ofn.lStructSize     = sizeof(OPENFILENAME);
  ofn.lpstrFile       = FileName ;          // Set in Open and Close functions
  ofn.nMaxFile        = _MAX_PATH ;
  ofn.lpstrFileTitle  = NULL ;          // Set in Open and Close functions
  ofn.hwndOwner       = HWindow;
  ofn.lpstrFilter     = szFileDialogText;
  ofn.lpstrInitialDir = PathName;
  ofn.Flags           = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  ofn.lpfnHook        = NULL;

  if(GetOpenFileName(&ofn)!=0)
  {
   strcpy(szFileName,FileName);
   SetDlgItemText(HWindow,ID_FNAME,(LPCSTR)szFileName);
  }

}

void TGLAddReport::Ok (RTMessage)
{
 char szName[20];
 char szDescription[40];
 char szFName[128];
 GetDlgItemText(HWindow,ID_NAME,szName,sizeof(szName));
 GetDlgItemText(HWindow,ID_DESCRIPT,szDescription,sizeof(szDescription));
 GetDlgItemText(HWindow,ID_FNAME,szFName,sizeof(szFName));
 if(*szName == 0 || *szFName == 0 || *szDescription == 0){
  MessageBox(HWindow,"All fields must be entered.","ERROR",MB_ICONEXCLAMATION);
 }
 else{
 glrept->appendStart();
 glrept->cname->assign(szName);
 glrept->fname->assign(szFName);
 glrept->descript->assign(szDescription);
 glrept->append();
 ShutDownWindow();
 } 
 return;
}


void TGLCustomReport::WMRButtonDown  (RTMessage){ }

void TGLCustomReport::SetupWindow(){
  glrept = new Tglcrept(&cb);
  glrept->top();
  while(!glrept->eof()) {
   SendDlgItemMsg(ID_ACCTLIST,LB_ADDSTRING,NULL,(LPARAM)(LPCSTR)glrept->descript->str());
   glrept->skip(1);
  }
   
  SetFocus(GetDlgItem(HWindow,ID_PRINT));
}

void TGLCustomReport::HandleListBox (RTMessage Msg) {
 switch (Msg.LP.Hi)
  { 
   case LBN_DBLCLK:
     char szBuf[85];
     DWORD dwIndex = SendDlgItemMsg(ID_ACCTLIST, LB_GETCURSEL,NULL, NULL);
     SendDlgItemMsg(ID_ACCTLIST,LB_SETCURSEL,dwIndex,0);
     SendDlgItemMsg(ID_ACCTLIST,LB_GETTEXT,dwIndex,(LONG)(LPSTR)szBuf);
 
     if(glrept->seek(szBuf)==0)
     {      
      SendDlgItemMsg(ID_PRINT,WM_SETTEXT,NULL,(LONG)(LPSTR)glrept->cname->str());
     }
     else     
     SetFocus(GetDlgItem(HWindow,ID_PRINT));
   break;
 }
}


void TGLCustomReport::HandlePrint   (RTMessage){  
  char temp[20];
  HINSTANCE hInst = GetApplication()->hInstance;
  GetDlgItemText(HWindow,ID_PRINT,temp,sizeof(temp));
  if(*temp != 0){
   char szRepName[35];
   char szFileName[128];
   strcpy(szRepName,glrept->cname->str());
   strcpy(szFileName,glrept->fname->str());
   short JobNumber = PEOpenPrintJob(szFileName);
   if(JobNumber != 0){
     EnableWindow(HWindow,FALSE);
     PEOutputToWindow(JobNumber,szRepName,
     CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,GetApplication()->MainWindow->HWindow);
     PEStartPrintJob(JobNumber,TRUE);
     PEClosePrintJob(JobNumber);
     EnableWindow(HWindow,TRUE);
     return;  
   }
   MessageBox(HWindow,GetString(hInst,ID_PRINTERROR),"Error",MB_ICONEXCLAMATION);
  }
  else
  MessageBox(HWindow,"Choose a report.","Error",MB_ICONINFORMATION);
  return;
}

void TGLCustomReport::Add (RTMessage)
{ 
 GetApplication()->ExecDialog(new TGLAddReport(this,"GL_ADDREPORT",glrept));
 ResetListBox();
 return;
}

void TGLCustomReport::Modify (RTMessage)
{
 char *temp = new char[20];
 GetDlgItemText(HWindow,ID_PRINT,temp,sizeof(temp));
 if(*temp != 0){
 GetApplication()->ExecDialog(new TGLModifyReport(this,"GL_ADDREPORT",glrept));
 ResetListBox();
 }
 else
 MessageBox(HWindow,"Choose a report.",GetString(GetApplication()->hInstance,ID_WARNING),MB_ICONINFORMATION);
 delete temp;
}


void TGLCustomReport::Delete  (RTMessage)
{
 GetApplication()->ExecDialog(new TGLDeleteReport(this,"GL_ADDREPORT",glrept));
 ResetListBox();
}

void TGLCustomReport::Cancel  (RTMessage){
 glrept->close();
 ShutDownWindow();
}

void TGLCustomReport::ResetListBox  ()
{
 SendDlgItemMsg(ID_ACCTLIST,LB_RESETCONTENT,NULL,NULL);
 glrept->top();
 while(!glrept->eof()) {
   SendDlgItemMsg(ID_ACCTLIST,LB_ADDSTRING,NULL,(LPARAM)(LPCSTR)glrept->descript->str());
   glrept->skip(1);
  }   
  SetFocus(GetDlgItem(HWindow,ID_PRINT));
}
void TGlWindow::APCustomReport (RTMessage)
{
  GetApplication()->ExecDialog(new TGLCustomReport(this,"GL_CUSTREPORT"));
}
