#include <tbowl.h>
#include <ctl3d.h>
#include <dir.h>
#include <direct.h>
#include <commdlg.h>
#include "gl.h"
#include "gl_tooli.h"
#include "gl_tool.h"

void TToolBar::WMPaint(TMessage&)
{
  PAINTSTRUCT PS;
  HDC memDC,PaintDC;;
  RECT clientRect;
  HANDLE oldBmp;
  HBITMAP bmpButtonLine;

  PaintDC = BeginPaint(HWindow, &PS);
  GetClientRect(HWindow, &clientRect);
  memDC = CreateCompatibleDC(PaintDC);
  bmpButtonLine = LoadBitmap(GetApplication()->hInstance, "STATUSBAR");
  oldBmp = SelectObject(memDC, bmpButtonLine);
  StretchBlt(PaintDC,0,0, clientRect.right - clientRect.left,TOOLBAR_HEIGHT,memDC,0,0,10,TOOLBAR_HEIGHT, SRCCOPY);
  SelectObject(memDC, oldBmp);
  ReleaseDC(HWindow,memDC);
  ReleaseDC(HWindow,PaintDC);
  DeleteDC(memDC);
  DeleteDC(PaintDC);
  DeleteObject(bmpButtonLine);
  DeleteObject(oldBmp);
  EndPaint(HWindow, &PS);
}



void TToolBar::GL       (RTMessage){
	RECT rc;
   GetClientRect(HWindow,&rc);
	SendMessage(Parent->HWindow,WM_COMMAND,ID_GL,NULL);
 }


void TToolBar::GLAdd     (RTMessage){Mssg(ID_ACCTADD);}
void TToolBar::GLBatch   (RTMessage){Mssg(ID_MODBATCH);}
void TToolBar::APBatch   (RTMessage){Mssg(ID_APBATCH);}
void TToolBar::ARBatch   (RTMessage){Mssg(ID_ARBATCH);}
void TToolBar::APCash    (RTMessage){Mssg(ID_APCASH);}
void TToolBar::GLPost    (RTMessage){Mssg(ID_GLPOSTBATCH);}
void TToolBar::GLInquiry (RTMessage){Mssg(ID_GLCHARTINQ);}
void TToolBar::GLQuit    (RTMessage){Mssg(24340);}
void TToolBar::GLPrint   (RTMessage){Mssg(ID_PRINT);}
void TToolBar::GLOpen    (RTMessage){Mssg(ID_OPEN);}
void TToolBar::GLSave    (RTMessage){Mssg(ID_SAVE);}
void TToolBar::GLNew     (RTMessage){Mssg(111);}
void TToolBar::Cut	    (RTMessage){Mssg(ID_CUT);}
void TToolBar::Copy      (RTMessage){Mssg(ID_COPY);}
void TToolBar::Paste     (RTMessage){Mssg(ID_PASTE);}
void TToolBar::Border    (RTMessage){Mssg(ID_BORDER);}
void TToolBar::Font      (RTMessage){Mssg(ID_FONT);}
void TToolBar::Alignment (RTMessage){Mssg(ID_ALIGNMENT);}
void TToolBar::Number    (RTMessage){Mssg(ID_NUMBER);}
void TToolBar::Grids     (RTMessage){Mssg(ID_GRIDS);}

void TToolBar::Mssg( int ID)
{
 SendMessage(Parent->HWindow,WM_COMMAND,ID,NULL);
}

DWORD TToolBar::SetPic(int ID,double iPosition,BOOL bShow)
{
 RECT rect;
 GetClientRect(HWindow,&rect);
 int iTop = rect.top+2;
 MoveWindow(GetDlgItem(HWindow,ID) ,rect.left+iSpace+(iPosition*iWidth) ,iTop,iWidth,iHeight,TRUE);
 if(bShow)
  ShowWindow(GetDlgItem(HWindow,ID),SW_SHOW);
 else
  ShowWindow(GetDlgItem(HWindow,ID),SW_HIDE);
 SendDlgItemMsg(ID,SBM_SETSHADOWSIZE,1,NULL);
 return SendDlgItemMsg(ID,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)ID);
}

void TToolBar::SetupWindow()
{
 TDialog::SetupWindow();
 iSpace = 10;
 iWidth = 30;
 iHeight = 25;
 double dPosition = 0;
 SetPic(ID_NEW,       dPosition, TRUE); dPosition+=1;
 SetPic(ID_OPEN,      dPosition, TRUE); dPosition+=1;
 SetPic(ID_SAVE,      dPosition, TRUE); dPosition+=1;
 SetPic(ID_PRINT,     dPosition, TRUE); dPosition+=1.3;

 SetPic(ID_GLBATCH,   dPosition, TRUE); dPosition+=1;
 SetPic(ID_APBATCH,   dPosition, TRUE); dPosition+=1;
 SetPic(ID_CASH,      dPosition, TRUE); dPosition+=1;
 SetPic(ID_ARBATCHENT,dPosition, TRUE); dPosition+=1.3;

 SetPic(ID_GLINQ,     dPosition,TRUE);  dPosition+=1;
 SetPic(ID_GLPOST,    dPosition,TRUE);  dPosition+=1;
 SetPic(ID_GLADD,     dPosition,TRUE);  dPosition+=1.3;
 SetPic(ID_CANCEL,    dPosition,TRUE);  dPosition+=1.3;

 dPosition = 4.3;
 SetPic(ID_CUT,       dPosition,FALSE); dPosition+=1;
 SetPic(ID_COPY,      dPosition,FALSE); dPosition+=1;
 SetPic(ID_PASTE,     dPosition,FALSE); dPosition+=1.3;

 SetPic(ID_BORDER,    dPosition,FALSE); dPosition+=1;
 SetPic(ID_FONT,      dPosition,FALSE); dPosition+=1;
 SetPic(ID_ALIGNMENT, dPosition,FALSE); dPosition+=1;
 SetPic(ID_NUMBER,    dPosition,FALSE); dPosition+=1;
 SetPic(ID_GRIDS,     dPosition,FALSE);
}



void TVToolBar::SetupWindow()
{
 TDialog::SetupWindow();
 iSpace = 2;
 iWidth = 30;
 iHeight = 25;
 double dPosition = 0;
 SetPic(ID_GENERAL,   dPosition, TRUE); dPosition+=1.01;
 SetPic(2210,   dPosition, TRUE); dPosition+=1.01;
 SetPic(ID_RECEIVABLE,dPosition, TRUE);
}

void TVToolBar::GL  (RTMessage ) {Mssg(ID_GENERAL);}
void TVToolBar::AP  (RTMessage ) {Mssg(ID_AP);}
void TVToolBar::AR  (RTMessage ) {Mssg(ID_RECEIVABLE);}



