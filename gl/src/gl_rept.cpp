/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Custom Report Dialog Box.
*/

#include <tbowl.h>
#include <commdlg.h>
#include "crpe.h"
#include "d4all.hpp"
#include "gl.h"
#include "glclass.h"
#include "gl_main.h"
#include "gl_dlg.h"
#include "gl_rept.h"
#include "gl_dlg.h"
#include "utildlg.h"
#include "stdlib.h"
#define ID_PRINTERROR 16009
#define ID_NAME 1001
#define ID_DESCR 1002
#define ID_FILEN 1003

#define LINENO 10
extern char szApp[20];
extern HWND hBar;
static COLORREF lpBack, lpFore;

static OPENFILENAME ofn;
static char PathName[128] = "\0";
static char FileName[128] = "\0";
extern Code4 cb;
extern BOOL crpeOpen;

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
  TGLAddReport(PTWindowsObject AParent, int AName,PTglcrept repts,PTModule AModule = NULL): TMainDialog(AParent,AName)
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
class TGLModifyReport : public TMainDialog
{
 public:
  PTglcrept glrept;
  TGLModifyReport(PTWindowsObject AParent, int AName,PTglcrept repts,PTModule AModule = NULL): TMainDialog(AParent,AName)
  {
	glrept = repts;
  };

  virtual void SetupWindow()
   {
	 SetWindowText(HWindow,GetString(6064));
	 SetWindowText(GetDlgItem(HWindow,IDOK),GetString(6032));
	 SetDlgItemText(HWindow,ID_NAME,(LPCSTR)glrept->cname->str());
	 SetDlgItemText(HWindow,ID_DESCR,(LPCSTR)glrept->descript->str());
	 SetDlgItemText(HWindow,ID_FILEN,(LPCSTR)glrept->fname->str());
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
  TGLDeleteReport(PTWindowsObject AParent, int AName,PTglcrept repts,PTModule AModule = NULL): TDialog(AParent,AName)
  {
	glrept = repts;
  };

  virtual void WMInitDialog (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG]{
	SetWindowText(HWindow,GetString(GetApplication()->hInstance,6065));
	SetWindowText(GetDlgItem(HWindow,IDOK),GetString(GetApplication()->hInstance,6033));
	SetDlgItemText(HWindow,ID_NAME,(LPCSTR)glrept->cname->str());
   SetDlgItemText(HWindow,ID_DESCR,(LPCSTR)glrept->descript->str());
	SetDlgItemText(HWindow,ID_FILEN,(LPCSTR)glrept->fname->str());
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
 if(*StrTrim(GetField(ID_NAME)) == 0 || *StrTrim(GetField(ID_FILEN)) == 0 || *StrTrim(GetField(ID_DESCR)) == 0){
  MessageBox(HWindow,GetString(6063),szApp,MB_ICONEXCLAMATION);
 }
 else
 {
 glrept->appendStart();
 glrept->cname->assign(GetField(ID_NAME));
 glrept->fname->assign(GetField(ID_FILEN));
 glrept->descript->assign(GetField(ID_DESCR));
 glrept->append();
 ShutDownWindow();
 }
 return;
}


void TGLAddReport::SetupWindow()
{
  TMainDialog::SetupWindow();
  ofn.lStructSize     = sizeof(OPENFILENAME);
  ofn.lpstrFile       = FileName ;          // Set in Open and Close functions
  ofn.nMaxFile        = _MAX_PATH ;
  ofn.lpstrFileTitle  = NULL ;          // Set in Open and Close functions
  ofn.hwndOwner       = HWindow;
  ofn.lpstrFilter     = GetString(6062);
  ofn.lpstrInitialDir = PathName;
  ofn.Flags           = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  ofn.lpfnHook        = NULL;

  if(GetOpenFileName(&ofn)!=0)
  {
	char szName[_MAX_FNAME];
	char szExt[_MAX_EXT];
	_splitpath(FileName,NULL,NULL,szName,szExt);
	strcpy(szFileName,szName);
	strcat(szFileName,szExt);
	SetDlgItemText(HWindow,ID_FILEN,(LPCSTR)szFileName);
  }
}

void TGLAddReport::Ok (RTMessage)
{
 if(*StrTrim(GetField(ID_NAME)) == 0 || *StrTrim(GetField(ID_FILEN)) == 0 || *StrTrim(GetField(ID_DESCR)) == 0){
  MessageBox(HWindow,GetString(6063),szApp,MB_ICONEXCLAMATION);
 }
 else
 {
 glrept->appendStart();
 glrept->cname->assign(GetField(ID_NAME));
 glrept->fname->assign(GetField(ID_FILEN));
 glrept->descript->assign(GetField(ID_DESCR));
 glrept->append();
 ShutDownWindow();
 }
 return;
}

void TGLCustomReport::SetupWindow   ()
{

 TMainDialog::SetupWindow();

 HWND SHWindow;
 RECT rc;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 HourGlassOn();
 cb.closeAll();
 cb.init();
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetClientRect(SHWindow,&rc);
 short d1Width,d3Width;
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 SSSetBool(SHWindow,SSB_AUTOSIZE,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);


 SSSetColWidth(SHWindow,1,4);
 SSSetColWidth(SHWindow,3,20);

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d3Width);



 short ColWidth = (rc.right - rc.left)
		  - d1Width;

 SSSetColWidthInPixels(SHWindow,2,ColWidth);


 //lfFont.lfHeight          = -13;//USER_CHAR_SIZE;
 lfFont.lfHeight          = -(GetFontSize());
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = USER_CHAR_SET; //default character set.
  lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);



  glrept = new Tglcrept(&cb);
  glrept->top();

 SSSetData(SHWindow,1,SS_HEADER, "No.");
 SSSetData(SHWindow,2,SS_HEADER, "Description");

 SSSetMaxCols(SHWindow, 2);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 long i=1;
 Str4ten szLine;
 glrept->top();
 while(!glrept->eof())
 {
		//itoa(i,szLine,10);
		szLine.assignLong(i,4);
		SSSetData(SHWindow,1,i,szLine.str());
		SSSetData(SHWindow,2,i,glrept->descript->str());
	  i++;
	  glrept->skip(1);
 }

 if(i>LINENO)
  SSSetMaxRows(SHWindow,i-1);
 else
  SSSetMaxRows(SHWindow,LINENO);
  HourGlassOff();
  SetFocus(GetDlgItem(HWindow,ID_DISTLIST));
  SetBitmap(ID_PRINT);
  SetBitmap(ID_ADD);
  SetBitmap(ID_MODIFY);
}


void TGLCustomReport::HandlePrint (RTMessage)
{
  char *temp = new char [ glrept->descript->len() + 1];
  HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
  SS_CELLCOORD Cell;
  SSGetActiveCell(SHWindow,&Cell.Col,&Cell.Row);
  SSGetData(SHWindow,2,Cell.Row,temp);
  if(glrept->seek(temp) == 0)
  {
   char *szFileName = new char[glrept->fname->len()+1];
   strcpy(szFileName,glrept->fname->str());
   PrtReport(StrTrim(szFileName));
   SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
   delete [] szFileName;
  }
  delete [] temp;
};


void TGLCustomReport::HandleDblclk   (RTMessage Msg)
{  

  char *temp = new char [ glrept->descript->len() + 1];
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
  SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SSGetData(SHWindow,2,Cell->Row,temp);
  SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  if(glrept->seek(temp) == 0)
  {
	char *szFileName = new char[glrept->fname->len()+1];
	strcpy(szFileName,glrept->fname->str());
	PrtReport(StrTrim(szFileName));
	SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
	delete [] szFileName;
  }
 delete temp; 
}

void TGLCustomReport::Add (RTMessage)
{
 GetApplication()->ExecDialog(new TGLAddReport(this,1012,glrept));
 ResetListBox();
 return;
}

void TGLCustomReport::Modify (RTMessage)
{
  char *temp = new char [ glrept->descript->len() + 1];
  HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
  SS_CELLCOORD Cell;
  SSGetActiveCell(SHWindow,&Cell.Col,&Cell.Row);
  SSGetData(SHWindow,2,Cell.Row,temp);
  if(glrept->seek(temp) == 0)
  {
   char *szFileName = new char[glrept->fname->len() + 1];
   strcpy(szFileName,glrept->fname->str());
	GetApplication()->ExecDialog(new TGLModifyReport(this,1012,glrept));
   ResetListBox();
   delete [] szFileName;
  }
  delete [] temp;
}


void TGLCustomReport::Delete  (RTMessage)
{
  char *temp = new char [ glrept->descript->len() + 1];
  HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
  SS_CELLCOORD Cell;
  SSGetActiveCell(SHWindow,&Cell.Col,&Cell.Row);
  SSGetData(SHWindow,2,Cell.Row,temp);
  if(glrept->seek(temp) == 0)
  {
   char *szFileName = new char[glrept->fname->len() + 1];
	strcpy(szFileName,glrept->fname->str());
   GetApplication()->ExecDialog(new TGLDeleteReport(this,1012,glrept));
	ResetListBox();
   delete [] szFileName;
  }
  delete [] temp;
}

void TGLCustomReport::Cancel  (RTMessage){
 glrept->close();
 ShutDownWindow();
}

void TGLCustomReport::ResetListBox  ()
{
 HourGlassOn();
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSSetBool(SHWindow,SSB_REDRAW,FALSE);
 long i=1;

 Str4ten szLine;
 SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
 for(glrept->top();!glrept->eof();glrept->skip())
 {
      szLine.assignLong(i,4);
      SSSetData(SHWindow,1,i,szLine.str());
		SSSetData(SHWindow,2,i,glrept->descript->str());
      i++;
 }

 if(i>LINENO)
  SSSetMaxRows(SHWindow,i-1);
 else
  SSSetMaxRows(SHWindow,LINENO);

  SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SSSetBool(SHWindow,SSB_REDRAW,TRUE);
  HourGlassOff();
}

char  *szCurrentPeriod();

void TGLCustomReport::PrtReport(char *szFileName)
{
  PrintReport(szFileName,GetApplication()->MainWindow->HWindow);
}

void TGlWindow::GLCustomReport (RTMessage)
{
  GetApplication()->ExecDialog(new TGLCustomReport(this,1022));
}

