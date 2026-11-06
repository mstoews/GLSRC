
// General Ledger  - (C) Copyright 1992 by Tegara Corporation

#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include <ctl3d.h>
#include <direct.h>
#include "utildlg.h"

#define IDD_STATUSBAR_MENUHELP 2
#define STATUSBAR_HEIGHT       23
#define FONT_SIZE 13
#define GWW_STATUSBAR          0
#define  BUTTON_WIDTH   45

Code4 cb;
LPSTR  path;
char   CurDir[MAXPATH];
extern PTWindowsObject GetObjectPtr;
LPSTR  dbf;
int  PeriodNo;
int X;
int Y;
HINSTANCE hTegara1,hTegara2,hInst;
char szApp[20];

#define TOOLBAR_WIDTH          32
#define TOOLBAR_HEIGHT         30
#define GWW_STATUSBAR          0
#define GWW_TOOLBOX            2
#define GWW_TOOLBAR            4

int FindGLStartUp(char *szFileSpec);

HINSTANCE hDLL;
HMENU  HAPMenu;
HWND  prev_wnd;
HWND hCalc,hGL,hAP;
FARPROC find_prev_wndi;
BOOL CALLBACK _export find_prev_wnd(HWND,LPARAM);

BOOL CALLBACK _export find_prev_wnd(HWND wnd, LPARAM lParam)
{
 /* Look for a top level window having the same instance. */
 if (!GetParent(wnd) && GetWindowWord(wnd, GWW_HINSTANCE) == (LPARAM) lParam)
 {
   prev_wnd = wnd;
   return FALSE;
 }
 return TRUE;
}

// Used to create the toolbar at the top of the application.
_CLASSDEF (TToolBar)
class TToolBar : public TDialog
{

  public:
  TToolBar(PTWindowsObject AParent,LPSTR ATitle):TDialog(AParent,ATitle){};
  virtual void SetupWindow();
  virtual void WMPaint       (RTMessage Msg) = [WM_FIRST + WM_PAINT];
  virtual void APAdd         (RTMessage Msg) = [ID_FIRST + ID_APADD];
  virtual void APBatch       (RTMessage Msg) = [ID_FIRST + ID_APBATCH];
  virtual void APPost        (RTMessage Msg) = [ID_FIRST + ID_APPOST];
  virtual void APInquiry     (RTMessage Msg) = [ID_FIRST + ID_APINQ];
  virtual void APCancelBatch (RTMessage Msg) = [ID_FIRST + ID_CHANGE];
  virtual void APCalculator  (RTMessage Msg) = [ID_FIRST + 1010];
  virtual void APQuit        (RTMessage Msg) = [ID_FIRST + ID_ADJCANCEL];
  DWORD SetPic(int ID, LPCSTR lpPic,double iPosition,BOOL bShow);
  void Mssg( int ID);

};

void TToolBar::Mssg( int ID){ SendMessage(Parent->HWindow,WM_COMMAND,ID,NULL);}

DWORD TToolBar::SetPic(int ID, LPCSTR lpPic,double iPosition,BOOL bShow)
{
 RECT rect;
 GetClientRect(HWindow,&rect); 
 int iSpace = 10;
 int iWidth = 30;
 int iHeight = 25;
 int iTop = rect.top+3;
 MoveWindow(GetDlgItem(HWindow,ID) ,rect.left+iSpace+(iPosition*iWidth) ,iTop,iWidth,iHeight,0);
 if(bShow)
  ShowWindow(GetDlgItem(HWindow,ID),SW_SHOW);
 else
  ShowWindow(GetDlgItem(HWindow,ID),SW_HIDE);
  SendDlgItemMsg(ID,SBM_SETSHADOWSIZE,1,NULL);
 return SendDlgItemMsg(ID,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)lpPic);
}


void TToolBar::APAdd         (RTMessage){Mssg(ID_APADDVENDORS);}
void TToolBar::APBatch       (RTMessage){Mssg(ID_APBATCH);}
void TToolBar::APPost        (RTMessage){Mssg(ID_APPOSTBATCH);}
void TToolBar::APInquiry     (RTMessage){Mssg(ID_OPENACCTS);}
void TToolBar::APCancelBatch (RTMessage){Mssg(ID_BATCHCANCEL);}
void TToolBar::APQuit        (RTMessage){Mssg(24340);}
void TToolBar::APCalculator  (RTMessage){Mssg(ID_CALCULATOR);}

void TGlWindow::Calculator (RTMessage)
{
 hCalc = FindWindow("SciCalc",NULL);
 if (hCalc == NULL)
  {
   WinExec("calc.exe",SW_SHOWNA);
  }
 else
 BringWindowToTop(hCalc);
}

void TToolBar::SetupWindow()
{
 double dPosition = 0;
 SetPic(ID_APBATCH,  "ENTER"		,dPosition, TRUE); dPosition+=1.3;
 SetPic(ID_APADD,    "ADD"		,dPosition, TRUE); dPosition+=1  ;
 SetPic(ID_APPOST,   "POST"             ,dPosition, TRUE); dPosition+=1  ;
 SetPic(ID_APINQ,    "LOOK"	        ,dPosition, TRUE); dPosition+=1  ;
 SetPic(ID_CHANGE,   "CANCEL"		,dPosition, TRUE); dPosition+=1  ;
 SetPic(1010,        "CALC"		,dPosition,TRUE);  dPosition+=1.3;
 SetPic(ID_ADJCANCEL, "QUIT"    	,dPosition,TRUE);

}



void TGlWindow::SendStatusMsg (int ID)
{
  SendMessage(HStatusBar, SBRM_SETITEMTEXT, IDD_STATUSBAR_MENUHELP,
   (LPARAM)GetString(GetApplication()->hInstance,ID));
}
// Define a class derived from TApplication
class TAcctModule :public TApplication
{
public:
  TAcctModule(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
    : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow){};
  virtual void InitMainWindow();
  ~TAcctModule()
  {
  DestroyMenu(HAPMenu);
  FreeLibrary(hTegara1);
  FreeLibrary(hTegara2);
  hCalc = FindWindow("SciCalc",NULL);
  if(hCalc != NULL)
  SendMessage(hCalc,WM_CLOSE,NULL,NULL);
  PECloseEngine();
  };
};


void TGlWindow::CMHelp (RTMessage)
{
  WinExec("winhelp winhelp.hlp",SW_SHOW);
}

extern "C"
{
 int GetTest(PTWindowsObject ParentWnd);
}  

extern "C"
{
 int GetAlias(HWND ParenthWnd);
}


void TGlWindow::CMAbout (RTMessage)
{
 GetApplication()->ExecDialog(new TDialog(this,"ABOUT"));
}




void TGlWindow::GetWindowClass(WNDCLASS& wc)
{
 TWindow::GetWindowClass(wc);
 wc.hIcon=LoadIcon(wc.hInstance, "GL");
}




// DIALOGS #5-7: Common printer, setup, and options dialogs
void TGlWindow::CMDialogPrint(RTMessage)
{
 DlgPrinterSetup(HWindow);
}

void TGlWindow::SetupWindow()
 {
  TMDIFrame::SetupWindow(); 
  ToolBar = (TToolBar *) new TToolBar(this,"TOOLBAR");
  ToolBar->EnableAutoCreate();
  GetApplication()->MakeWindow(ToolBar);
  ToolBar->Show(SW_SHOW); 
  HStatusBar = GLCreateStatusBar();
  SendMessage(HStatusBar, SBRM_SETITEMTEXT,3,(LPARAM)GetCurrentPeriod());
  SendMessage(HStatusBar, SBRM_SETITEMTEXT,1,(LPARAM)GetCompanyName());
  cb.exclusive = 0;
  cb.readOnly = 0;
  cb.readLock = 0;
  cb.lockAttempts =5;
  SetHandleCount(40);
}

char  *TGlWindow::GetCompanyName ()
{
  return GetCompany();
}


char  *TGlWindow::GetCurrentPeriod ()
{
  static char szCurrPeriod[20];
  strcpy(szCurrPeriod,szGetPeriod());
  return szCurrPeriod;
}

double GetCurrentPeriodn()
{  
  return iGetPeriod();
}


TGlWindow::TGlWindow(LPSTR ATitle,LPSTR MenuName) : TMDIFrame(ATitle,MenuName)
 {
   // Should add the start up code here for finding the current directory for a ini file.
   //

   ChildMenuPos = 4;
   Attr.X=0;
   Attr.Y=0;
   Attr.W=CW_USEDEFAULT;
   Attr.H=0;
   getcwd(CurDir,MAXPATH);
   UINT drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,"GLDR.INI");
   int rc = GetPrivateProfileString("GL_DIR","DIR","",CurDir,sizeof(CurDir),"GLDR.INI");

   if(rc <= 0)
   {   
     exit(1);
   }
   _chdrive(drive);
   if(chdir(CurDir) != 0)
    exit(1);
 }

FindGLStartUp(char* szFileSpec)
 {
      struct ffblk fb;
      if(findfirst(szFileSpec,&fb,FA_ARCH)==0)
      {  
       return TRUE;
      }
     return FALSE;
 }


void TGlWindow::WMSize(RTMessage)
{
 RECT rect;
 GetClientRect(HWindow, &rect);
 MoveWindow(GetClient()->HWindow,0,TOOLBAR_HEIGHT,rect.right,rect.bottom-(TOOLBAR_HEIGHT+STATUSBAR_HEIGHT),1);
 MoveWindow(ToolBar->HWindow,-7,0,rect.right-rect.left+7,38,1);
 MoveWindow(HStatusBar, 0, rect.bottom-STATUSBAR_HEIGHT, rect.right, STATUSBAR_HEIGHT, TRUE);
}

void TToolBar::WMPaint(TMessage&)
{
  PAINTSTRUCT PS;
  HDC PaintDC;
  PaintDC = BeginPaint(HWindow, &PS);
  HDC memDC;
  RECT clientRect;
  HANDLE oldBmp;
  HBITMAP bmpButtonLine;
  GetClientRect(HWindow, &clientRect);
  memDC = CreateCompatibleDC(PaintDC);
  bmpButtonLine = LoadBitmap(GetApplication()->hInstance, "STATUSBAR");
  oldBmp = SelectObject(memDC, bmpButtonLine);
  StretchBlt(PaintDC,0,0, clientRect.right - clientRect.left,TOOLBAR_HEIGHT,memDC,0,0,10,TOOLBAR_HEIGHT, SRCCOPY);
  SelectObject(memDC, oldBmp);
  DeleteDC(memDC);
  EndPaint(HWindow, &PS);
}







void TGLClient::WMRButtonDown (RTMessage Msg)
{ 
 POINT point = MAKEPOINT(Msg.LParam);
 ClientToScreen(HWindow,&point);
 X=point.x;
 Y=point.y;
 SendMessage(GetApplication()->MainWindow->HWindow,WM_POPUPMENU,(unsigned)Msg.LParam,NULL);
}

void TGlWindow::WMMenuSelect(RTMessage Msg)
{
  char Str[85];
  int nFlags = Msg.LP.Lo;
  int hSysMenu = Msg.LP.Hi;
  if (HStatusBar != 0)
  {
   if((unsigned)nFlags == 0xFFFF && hSysMenu == NULL)
    {
     LoadString(GetApplication()->hInstance, IDS_READY, Str, sizeof(Str));
    }
   else if(nFlags & MF_POPUP )
    {
     LoadString(GetApplication()->hInstance, IDS_MENUITEM, Str, sizeof(Str));
    }
   else if(nFlags & MF_SYSMENU )
    {
     LoadString(GetApplication()->hInstance, IDS_SYSTEM, Str, sizeof(Str));
    }
   else 
    LoadString(GetApplication()->hInstance, Msg.WParam, Str, sizeof(Str));
    SendMessage(HStatusBar, SBRM_SETITEMTEXT, IDD_STATUSBAR_MENUHELP, (LPARAM)Str);
  }
}


void TGlWindow::PopUpMenu (RTMessage) 
{
   HMENU hMenu = LoadMenu(GetApplication()->hInstance,"ARSUB");
   hMenu = GetSubMenu(hMenu,0);
   TrackPopupMenu(hMenu,TPM_LEFTBUTTON,X,Y,0,GetApplication()->MainWindow->HWindow,NULL);
   return;
}



/*
void TGlWindow::APMenu(RTMessage)
{                      
 HMENU HPopUpMenu = GetSubMenu(HAPMenu,0); // Get the child menu position.
 SendMessage(ClientWnd->HWindow,WM_MDISETMENU,0,MAKELONG(HAPMenu,HPopUpMenu));
 DrawMenuBar(HWindow);
 SetWindowText(HWindow,szApp);
}
*/

void TAcctModule::InitMainWindow()
{
  MainWindow = new TGlWindow(szApp,"ar");
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  strcpy(szApp,"Accounts Receivable");
  TAcctModule AcctModule ("AcctAR", hInstance, hPrevInstance,lpCmdLine, nCmdShow);  
  if(hPrevInstance)
  {
		find_prev_wndi = MakeProcInstance((FARPROC)find_prev_wnd,hInstance);
		EnumWindows((WNDENUMPROC)find_prev_wndi,(LPARAM)hPrevInstance);
		FreeProcInstance(find_prev_wndi);
   if (!prev_wnd)
     return 0;
   prev_wnd = GetLastActivePopup(prev_wnd);
   ShowWindow(prev_wnd, SW_SHOWNORMAL);
   SetActiveWindow(prev_wnd);
   return 0;
  }
  Ctl3dRegister(hInstance);
  Ctl3dAutoSubclass(hInstance);

  hInst = hInstance;
  HAPMenu = LoadMenu(hInstance,"ap");
  PEOpenEngine();
  AcctModule.Run();
  return AcctModule.Status;
}

HWND TGlWindow::GLCreateStatusBar()
{
  HWND hWndSBR;
  RECT RectClient;

  static STATUSBARITEM Items[] =
     {
     {1, 6, 240, "Company Name",SBRS_TEXTLEFT},
     {2, 6, SBR_AUTOSIZE, ""   ,SBRS_TEXTLEFT},
     {3, 6, 140,  ""           ,SBRS_TEXTLEFT},
     };

  GetClientRect(HWindow, &RectClient);

  hWndSBR = tbCreateStatusBar(HWindow, WS_CHILD | WS_VISIBLE | SBRS_DRAWTOPLINE |SBRS_PIXELS,  0,
                  RectClient.bottom - STATUSBAR_HEIGHT,
                  RectClient.right, STATUSBAR_HEIGHT,
		  Items, sizeof(Items) / sizeof(STATUSBARITEM));

 LOGFONT lFont;
 HFONT  hfontDlg = (HFONT) NULL;
 memset(&lFont, 0, sizeof(lFont));
    if ((hfontDlg = (HFONT) SendMessage(hWndSBR, WM_GETFONT, 0, 0L))){
	if (GetObject(hfontDlg, sizeof(LOGFONT), (LPSTR) &lFont)){
	    lFont.lfHeight = -(15);
	    lFont.lfWeight = FW_LIGHT;
	    lFont.lfCharSet = 1;
	    lFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	    strcpy(lFont.lfFaceName, "MS Sans Serif");
	    if (hfontDlg = CreateFontIndirect((LPLOGFONT) &lFont)) {
	     SendMessage(hWndSBR,WM_SETFONT,(WPARAM)hfontDlg,(LPARAM)FALSE);
	    }
	}
    }

 return (hWndSBR);
}



//Adjustment journal entry
void TGlWindow::APAdjPosting  (RTMessage)
 {
  SendMessage(HWindow,WM_COMMAND,ID_APPOSTBATCH,NULL);
  
 }

void TGlWindow::APVenPayment  (RTMessage)
 {
  GetApplication()->ExecDialog(new TDialog(this,"AR_VENPAYMENT"));
 }

void TGlWindow::APExternalBat (RTMessage)
 {
  GetApplication()->ExecDialog(new TDialog(this,"AR_EXTBATCH"));
 }



void TGlWindow::APYearEnd     (RTMessage)
 {
  GetApplication()->ExecDialog(new TDialog(this,"AR_YEAREND"));
 }



void TGlWindow::ModifyBatch   (RTMessage){}
void TGlWindow::NewBatch      (RTMessage){}
void TGlWindow::SearchGL      (RTMessage){}
void TGlWindow::CancelBatch   (RTMessage){}



void TGlWindow::APInqVendors  (RTMessage)
 {
  GetApplication()->ExecDialog(new TDialog(this,"AR_INQVENDORS"));
 }


void TGlWindow::APVendorInquiry  (RTMessage) {  SendMessage(HWindow,WM_COMMAND,ID_OPENACCTS,NULL); }

void TGlWindow::GeneralLedger (RTMessage)
{ 
 hGL = FindWindow("gl.exe",NULL);
 if(hGL == NULL)
  WinExec("gl.exe",SW_SHOWNA);
 else
  BringWindowToTop(hGL);
}

void TGlWindow::AccountsPayable (RTMessage)
{
 hAP = FindWindow("ap.exe",NULL);
 if(hAP == NULL)
  WinExec("ap.exe",SW_SHOWNA);
 else
  BringWindowToTop(hAP);
}

