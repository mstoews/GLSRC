
// General Ledger  - (C) Copyright 1994 by Tegara Corporation


#include <tbowl.h>
#include <ctl3d.h>
#include <dir.h>
#include <direct.h>
#include <commdlg.h>

#include "d4all.hpp"


#include "glclass.h"

#include "gl.h"


#include "crpe.h"
#include "gl_dlg.h"
//#include "gl_pick.h"


#include "gl_main.h"
#include "gl_strg.h"
#include "gl_mains.h"
//#include "gl_btche.h"

//#include "gl_repts.h"
#include "gl_tooli.h"
#include "gl_tool.h"
#include "gl_manil.h"
#include "gl_man.h"
#include "utildlg.h"
#include "time.h"
#include "cstring.h"
#include "winerror.h"

#define IDD_STATUSBAR_MENUHELP 3222

#define STATUSBAR_HEIGHT       23
#define FONT_SIZE 	       15
#define GWW_STATUSBAR          0
#define ID_LOADING_AP       16007
#define ID_CUT		    3076
#define ID_PASTE	    3078
#define GWW_STATUSBAR          0
#define GWW_TOOLBOX            2
#define GWW_TOOLBAR            4



// Define global variables for Codebase.
Code4 cb;
LPSTR  path;
char   CurDir[128];
UINT drive;
BOOL crpeOpen;
extern BOOL bMenuSetup;
extern PTWindowsObject GetObjectPtr;
LPSTR  dbf;
char szApp[20];
char szMsg[256];
int  PeriodNo;
int X;
int Y;

HINSTANCE hInst,hTOOLS,hDBF;
HMENU HGLMenu,HGLExMenu;
HWND  prev_wnd,hAP,hAR,hCalc,hBar;
FARPROC find_prev_wndi;
BOOL CALLBACK _export find_prev_wnd(HWND,LPARAM);
BOOL CALLBACK _export find_prev_wnd(HWND wnd, LPARAM lParam)
{
 /* Look for a top level window having the same instance. */
 if (!GetParent(wnd) && (HINSTANCE)GetWindowWord(wnd, GWW_HINSTANCE) == (HINSTANCE) lParam)
 {
	prev_wnd = wnd;
	return FALSE;
 }
 return TRUE;
}


int Data4::UpdateUserTime(char* szUser)
{
  Str4large current_time;
  time_t t;
  struct tm *area;
  t = time(NULL);
  area = localtime(&t);
  Date4 d;
  d.today();
  Field4 fDate(data,"update");
  if(!fDate.isValid())
	return FALSE;
  Field4 fUser(data,"user");
  if(!fUser.isValid())
	return FALSE;
  Field4 fTime(data,"time");
  if(!fTime.isValid())
	return FALSE;

  fDate.assign(d.str());
  fUser.assign(szUser);
  current_time.assign(asctime(area));
  fTime.assign(asctime(area));
  return TRUE;
}


LPSTR GetString(HINSTANCE hInstance, int ID)
{
 GLOBALHANDLE hString = GlobalAlloc(GMEM_MOVEABLE,85);
 LPSTR lpString = (LPSTR)GlobalLock(hString);
 LoadString(hInstance,ID,lpString,85);
 GlobalUnlock(hString);
 return lpString;
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
  ~TAcctModule();
};

TAcctModule::~TAcctModule()
  {
	hCalc = FindWindow("SciCalc",NULL);
	if(hCalc != NULL)
	SendMessage(hCalc,WM_CLOSE,NULL,NULL);
	Ctl3dUnregister(hInstance);
	DestroyMenu(HGLMenu);
	DestroyMenu(HGLExMenu);
	ToolBoxFreeAllDlls();
	FreeLibrary(hDBF);
	PECloseEngine();
	cb.initUndo();
  };

void TGlWindow::CMHelp (RTMessage)
{
  WinExec("winhelp gltemp.hlp",SW_SHOW);
}


void TGlWindow::AccountsPayable (RTMessage)
{
}

void TGlWindow::AccountsReceive (RTMessage)
{
}


void TGlWindow::GLBackup(RTMessage)
{
 string* s = new string(szApp);
 if(MessageBox(HWindow,GetString(GetApplication()->hInstance,1073),s->c_str(),MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	cb.closeAll();
	if(WinExec("savegl.pif",SW_SHOW) == ERROR_FILE_NOT_FOUND)
	 MessageBox(HWindow,GetString(hInst,8153),szApp,MB_OK);
  }
 delete s;
}

void TGlWindow::GLRestore(RTMessage)
{
 cb.closeAll();
 if(MessageBox(HWindow,GetString(GetApplication()->hInstance,1074),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	cb.closeAll();
	if(WinExec("restore.pif",SW_SHOW) == ERROR_FILE_NOT_FOUND)
	 MessageBox(HWindow,GetString(hInst,8154),szApp,MB_OK);

  }
}


void TGlWindow::CMAbout (RTMessage)
{
  GetApplication()->ExecDialog(new TDialog(this,1008));
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

void TManilaGL::SetupWindow()
{
  TGlWindow::SetupWindow();
  wSelection = ID_GL;
  CheckMenuItem(HGLMenu,wSelection,MF_CHECKED);
  SetMenu(GetApplication()->MainWindow->HWindow,HGLMenu);
}

void TGlWindow::SetupWindow()
 {
  TMDIFrame::SetupWindow();
  ToolBar = (TToolBar *) new TToolBar(this,1032);
  GetApplication()->MakeWindow(ToolBar);
  ToolBar->Show(SW_SHOW);
  VToolBar =  new TVToolBar(this,2057);
  GetApplication()->MakeWindow(VToolBar);
  VToolBar->Show(SW_SHOW);

  cb.exclusive = 0;
  cb.readOnly = 0;
  cb.readLock = 0;
  cb.lockAttempts = -1;
  SetHandleCount(50);
  char szProfile[2];
  GetPrivateProfileString("ACCESS","PSW","",szProfile,sizeof(szProfile),"GL.INI");
  if(strcmp(StrTrim(szProfile),"T") == 0)
	{
	 bMenuSetup = FALSE;
	 PostMessage(HWindow,WM_COMMAND,ID_SYSACCESS,NULL);
	}
  else
	{
	 strcmp(szUser,"NOLOGIN");
	 bMenuSetup = TRUE;
	}
}

char *TGlWindow::GetCompanyName()
{
  static char szCompanyName[40];
  strcpy(szCompanyName,GetCompany());
  return szCompanyName;
}

char  *TGlWindow::GetCurrentPeriod ()
{
 return szGetPeriod();
}

TGlWindow::TGlWindow(LPSTR ATitle,LPSTR MenuName) : TMDIFrame(ATitle,MenuName){}

void TGlWindow::WMSize(RTMessage)
{
 RECT rect,last,first,third;  // last is the last button, first = 1st text, etc.
 GetClientRect(HWindow, &rect);
 MoveWindow(GetClient()->HWindow,0,TOOLBAR_HEIGHT,rect.right,rect.bottom-(2*TOOLBAR_HEIGHT),TRUE);
 MoveWindow(ToolBar->HWindow,-7,0,rect.right-rect.left+7,TOOLBAR_HEIGHT,1);
 MoveWindow(VToolBar->HWindow, 0, rect.bottom-TOOLBAR_HEIGHT, rect.right, TOOLBAR_HEIGHT, TRUE);
 int iHeight = 25;
 int iSpace  = 2;
 int iTop = rect.top+4;
 GetClientRect(GetDlgItem(VToolBar->HWindow,ID_RECEIVABLE),&last);
 MoveWindow(GetDlgItem(VToolBar->HWindow,ID_STATUSHELP),iSpace,iTop,8*iHeight,iHeight-iSpace,TRUE);
 int iThird = rect.right-102;
 MoveWindow(GetDlgItem(VToolBar->HWindow,3224),iThird,iTop,100,iHeight-iSpace,TRUE);
 GetWindowRect(GetDlgItem(VToolBar->HWindow,3224),&third);
 GetWindowRect(GetDlgItem(VToolBar->HWindow,ID_STATUSHELP),&first);
 int iSecondWidth = third.left-first.right-10;
 MoveWindow(GetDlgItem(VToolBar->HWindow,3223),first.right+1,iTop,iSecondWidth,iHeight-iSpace,TRUE);
 GLCreateStatusBar();
}

void TGLClient::WMRButtonDown (RTMessage Msg)
{
 POINT point = MAKEPOINT(Msg.LParam);
 ClientToScreen(HWindow,&point);
 X=point.x;
 Y=point.y;
 SendMessage(GetApplication()->MainWindow->HWindow,WM_POPUPMENU,(WPARAM) Msg.LParam,NULL);
}

void TGlWindow::WMMenuSelect(RTMessage Msg)
{
  string* szString = new string;
  int nFlags = Msg.LP.Lo;
  int hSysMenu = Msg.LP.Hi;
  int iLength = 85;
  szString->initial_capacity(iLength);
  if (HStatusBar != 0)
  {
	if((unsigned)nFlags == 0xFFFF && hSysMenu == NULL)
	 {
	  LoadString(GetApplication()->hInstance, IDS_READY,(char *)szString->c_str(),iLength);
	 }
	else if(nFlags & MF_POPUP )
	 {
	  LoadString(GetApplication()->hInstance, IDS_MENUITEM,(char *)szString->c_str(),iLength);
	 }
	else if(nFlags & MF_SYSMENU )
	 {
	  LoadString(GetApplication()->hInstance, IDS_SYSTEM,(char *)szString->c_str(),iLength);
	 }
	else
	 LoadString(GetApplication()->hInstance, Msg.WParam,(char *)szString->c_str(),iLength);

	SendMessage(GetDlgItem(VToolBar->HWindow,3223),WM_SETTEXT,NULL, (LPARAM)(LPSTR)szString->c_str());
  }
 delete szString;
}

// "Takeda General Ledger Accounting Module"
void TGlWindow::PopUpMenu (RTMessage)
{
	HMENU hMenu = LoadMenu(GetApplication()->hInstance,"GLSUB");
	hMenu = GetSubMenu(hMenu,0);
	TrackPopupMenu(hMenu,TPM_LEFTBUTTON,X,Y,0,GetApplication()->MainWindow->HWindow,NULL);
	DestroyMenu(hMenu);
	return;
}

TManilaGL::TManilaGL(LPSTR ATitle,LPSTR MenuName) : TGlWindow(ATitle,MenuName)
 {

	ChildMenuPos = 7;
	Attr.X=0;
	Attr.Y=0;
	Attr.W=CW_USEDEFAULT;
	Attr.H=0;
	getcwd(CurDir,MAXPATH);
	drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,"GLDR.INI");
	int rc = GetPrivateProfileString("GL_DIR","DIR","",CurDir,sizeof(CurDir),"GLDR.INI");
	if(rc <= 0)
	{
	  MessageBox(HWindow,"Can't find ini file.","",MB_OK);
	  exit(1);
	}
	_chdrive(drive);
	if(chdir(CurDir) != 0)
	 exit(1);
 }



void TAcctModule::InitMainWindow()
{
  strcpy(szApp,"TIAS");
  MainWindow = new TManilaGL(szApp,"");
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  TAcctModule AcctModule ("AcctApp", hInstance, hPrevInstance,lpCmdLine, nCmdShow);
  if(hPrevInstance)
  {
	find_prev_wndi = (FARPROC)MakeProcInstance((FARPROC)find_prev_wnd,hInstance);
	EnumWindows((WNDENUMPROC)find_prev_wndi,(LPARAM)hPrevInstance);
	FreeProcInstance(find_prev_wndi);
	if (!prev_wnd)
	  return 0;
	prev_wnd = GetLastActivePopup(prev_wnd);
	ShowWindow(prev_wnd, SW_SHOWNORMAL);
	SetActiveWindow(prev_wnd);
	return 0;
  }
  hInst = hInstance;
  Ctl3dRegister(hInstance);
  Ctl3dAutoSubclass(hInstance);

  HGLMenu = LoadMenu(hInstance,"manila");
  HGLExMenu = LoadMenu(hInstance,"gle");
  ToolBoxLoadDlls(DLL_TBPRO2 | DLL_TBPRO3 | DLL_TBPRO5 );

  hDBF   = LoadLibrary("DBF.DLL");
  crpeOpen = FALSE;
  AcctModule.Run();
  return AcctModule.Status;
}

HWND TGlWindow::GLCreateStatusBar()
{
  SendMessage(GetDlgItem(VToolBar->HWindow,3222),WM_SETTEXT,NULL, (LPARAM)(LPSTR)GetCompany());
  SendMessage(GetDlgItem(VToolBar->HWindow,3224),WM_SETTEXT,NULL, (LPARAM)(LPSTR)GetCurrentPeriod());
  return NULL;
}


