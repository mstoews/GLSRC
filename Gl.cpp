
// General Ledger  - (C) Copyright 1994 by Tegara Corporation
#include "std_inc.h"
#define STATUSBAR_HEIGHT       23
#define FONT_SIZE 	       15
#define GWW_STATUSBAR          0
#define ID_LOADING_AP       16007
#define ID_CUT		    3076
#define ID_PASTE	    3078
#define GWW_STATUSBAR          0
#define GWW_TOOLBOX            2
#define GWW_TOOLBAR            4
#define IDD_STATUSBAR_MENUHELP 2

// Define global variables for Codebase.
LPSTR  path;
char   CurDir[128];
UINT drive,iPeriod;

extern BOOL bMenuSetup;
extern PTWindowsObject GetObjectPtr;
extern unsigned _stklen = 10000 ; // for all Borland compilers

char szMsg[256];
char szPROFILE[] = "GL.INI";


HINSTANCE hInst,hTOOLS,hDBF,hWidgets; //,hXTable;
HMENU HGLMenu,HGLExMenu,HPrintMenu;
HWND  prev_wnd,hAP,hAR,hCalc,hBar;
FARPROC find_prev_wndi;
HFONT hfontDlg;


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
  string strTime(asctime (area));
  fTime.assign((LPSTR)strTime.c_str());
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



TAcctModule::~TAcctModule()
  {
 //	hCalc = FindWindow("SciCalc",NULL);
 //	if(hCalc != NULL)
 //	 SendMessage(hCalc,WM_CLOSE,NULL,NULL);
	Ctl3dUnregister(hInstance);
	DestroyMenu(HGLMenu);
	DestroyMenu(HGLExMenu);
	DestroyMenu(HPrintMenu);
	DeleteObject(hfontDlg);
	ToolBoxFreeAllDlls();
	cb.closeAll();
	cb.initUndo();
	FreeLibrary(hDBF);
	FreeLibrary(hWidgets);
	//FreeLibrary(hXTable);
	PECloseEngine();
  };

void TGlWindow::CMHelp (RTMessage)
{
  WinExec("winhelp gl.hlp",SW_SHOW);
}



void TTooltip::WMPaint(TMessage&)
{
}

void TManilaGL::WMTimer(RTMessage)
{
	string szString;
	int iLength = 85;
	szString.initial_capacity(iLength);
	HWND hCurrentWnd;
	POINT p;
	static HWND hLastWnd;
	if (GetActiveWindow() == HWindow)
	{
	  GetCursorPos(&p);
	  hCurrentWnd = WindowFromPoint(p);
	  if(hCurrentWnd != hLastWnd)
		{
		  KillTimer(HWindow,timer);
		  hLastWnd = hCurrentWnd;
		  UINT Button = GetDlgCtrlID(hCurrentWnd);
		  if(Button > 0)
		  {
			LoadString(GetApplication()->hInstance,Button,(char *)szString.c_str(),iLength);
			SendMessage(HStatusBar, SBRM_SETITEMTEXT, IDD_STATUSBAR_MENUHELP, (LPARAM)szString.c_str());
		  }
		  timer = SetTimer (HWindow,WM_TIMER,1000,NULL);
		}

	}

}

#pragma argsused
void  TManilaGL::DisplayToolTip(UINT ID){}

void TGlWindow::AccountsPayable (RTMessage)
{
}

void TGlWindow::AccountsReceive (RTMessage)
{
}


void TGlWindow::GLBackup(RTMessage)
{
 Code4 *cb = &((TAcctModule *)GetApplication())->cb;
 string* s = new string(((TAcctModule *)GetApplication())->GetAppName());
 if(MessageBox(HWindow,GetString(GetApplication()->hInstance,1073),s->c_str(),MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	cb->closeAll();
	if(WinExec("savegl.pif",SW_SHOW) == ERROR_FILE_NOT_FOUND)
	 MessageBox(HWindow,GetString(hInst,8153),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  }
 delete s;
}

void TGlWindow::GLRestore(RTMessage)
{
 Code4 *cb = &((TAcctModule *)GetApplication())->cb;
 if(MessageBox(HWindow,GetString(GetApplication()->hInstance,1074),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
	cb->closeAll();
	if(WinExec("restore.pif",SW_SHOW) == ERROR_FILE_NOT_FOUND)
	 MessageBox(HWindow,GetString(hInst,8154),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);

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
  cb = &((TAcctModule *)GetApplication())->cb;
  cb->hWnd = HWindow;
  cb->hInst = ((TAcctModule *)GetApplication())->hInstance;
  timer = SetTimer (HWindow,WM_TIMER,1000,NULL);
  SetMenu(GetApplication()->MainWindow->HWindow,HGLMenu);
  SetPosition();
  ((TAcctModule *)GetApplication())->SetFontStyle();
}

void TAcctModule::SetFontStyle()
{
  lfFont.lfHeight          = -(GetFontSize());
  lfFont.lfWidth           = 0;
  lfFont.lfEscapement      = 0;
  lfFont.lfOrientation     = 0;
  lfFont.lfWeight          = FW_NORMAL;
  lfFont.lfItalic          = 0;
  lfFont.lfUnderline       = 0;
  lfFont.lfStrikeOut       = 0;
  lfFont.lfCharSet         = 1;
  lfFont.lfOutPrecision    = 0;
  lfFont.lfClipPrecision   = 0;
  lfFont.lfQuality         = 0;
  lfFont.lfPitchAndFamily  = FF_SWISS;
  StrCpy (lfFont.lfFaceName, "HELV");
}

void TGlWindow::SetupWindow()
 {
  TMDIFrame::SetupWindow();
  ToolBar = (TToolBar *) new TToolBar(this,1032);
  Code4 *cb = &((TAcctModule *)GetApplication())->cb;
  GetApplication()->MakeWindow(ToolBar);
  HStatusBar = GLCreateStatusBar();
  cb->exclusive = 0;
  cb->lockAttempts = 1;
  SetHandleCount(99);
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
  SendMessage(HStatusBar,SBRM_SETITEMTEXT,3,(LPARAM)(LPSTR)GetCurrentPeriod());
  ToolBar->Show(SW_SHOW);
}

char *TGlWindow::GetCompanyName()
{
  static char szCompany[40];
  return strcpy(szCompany,((TAcctModule *)GetApplication())->szCompanyName);
}

char  *TGlWindow::GetCurrentPeriod ()
{
 return szGetPeriod();
}

TGlWindow::TGlWindow(LPSTR ATitle,LPSTR MenuName) : TMDIFrame(ATitle,MenuName)
{
  printJob = NULL;
}

void TGlWindow::WMSize(RTMessage)
{
 RECT rect;
 GetClientRect(HWindow, &rect);
 MoveWindow(GetClient()->HWindow,0,TOOLBAR_HEIGHT,rect.right,rect.bottom-(STATUSBAR_HEIGHT+TOOLBAR_HEIGHT),TRUE);
 MoveWindow(ToolBar->HWindow,-7,0,rect.right-rect.left+7,TOOLBAR_HEIGHT,1);
 MoveWindow(HStatusBar,0, rect.bottom-STATUSBAR_HEIGHT, rect.right,STATUSBAR_HEIGHT, TRUE);
}

void TGLClient::WMRButtonDown (RTMessage Msg)
{
 POINT point = MAKEPOINT(Msg.LParam);
 ClientToScreen(HWindow,&point);
 ((TAcctModule *)GetApplication())->SetPoint(point.x,point.y);
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
	SendMessage(HStatusBar, SBRM_SETITEMTEXT, IDD_STATUSBAR_MENUHELP, (LPARAM)szString->c_str());
  }
 delete szString;
}

// "Takeda General Ledger Accounting Module"
void TGlWindow::PopUpMenu (RTMessage)
{
	HMENU hMenu = LoadMenu(GetApplication()->hInstance,"GLSUB");
	hMenu = GetSubMenu(hMenu,0);
	POINT pt = ((TAcctModule *)GetApplication())->GetPoint();
	TrackPopupMenu(hMenu,TPM_LEFTBUTTON,pt.x,pt.y,0,GetApplication()->MainWindow->HWindow,NULL);
	DestroyMenu(hMenu);
	return;
}

void TManilaGL::SetPosition()
{

 int iLeft   = GetPrivateProfileInt(GetClassName(),"L",0,szPROFILE);
 int iTop    = GetPrivateProfileInt(GetClassName(),"T",0,szPROFILE);
 int iRight  = GetPrivateProfileInt(GetClassName(),"R",0,szPROFILE);
 int iBottom = GetPrivateProfileInt(GetClassName(),"B",0,szPROFILE);

 if(iBottom == 0 || iRight == 0)
 {
	iBottom = 480;
 }
 MoveWindow(HWindow,iLeft,iTop,iRight-iLeft,iBottom-iTop,TRUE);
}


void TManilaGL::CMExit(RTMessage)
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 char   szString[25];
 string szLeft(itoa(rc.left,szString,10));
 string szTop(itoa(rc.top,szString,10));
 string szRight(itoa(rc.right,szString,10));
 string szBottom(itoa(rc.bottom,szString,10));
 WritePrivateProfileString(GetClassName(),"L",(char *)szLeft.c_str(),szPROFILE);
 WritePrivateProfileString(GetClassName(),"R",(char *)szRight.c_str(),szPROFILE);
 WritePrivateProfileString(GetClassName(),"B",(char *)szBottom.c_str(),szPROFILE);
 WritePrivateProfileString(GetClassName(),"T",(char *)szTop.c_str(),szPROFILE);
 SendMessage(GetApplication()->MainWindow->HWindow,WM_CLOSE,NULL,NULL);
}

TManilaGL::TManilaGL(LPSTR ATitle,LPSTR MenuName) : TGlWindow(ATitle,MenuName)
 {
	ChildMenuPos = 6;
	Attr.Style |= WS_MAXIMIZE;
	getcwd(CurDir,MAXPATH);
	drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,"GL.INI");
	int rc = GetPrivateProfileString("GL_DIR","DIR","",CurDir,sizeof(CurDir),"GL.INI");
	string strMsg(GetApplication()->hInstance,9101);
	strMsg += CurDir;
	//strcpy(CurDir,"C:\\GL\\SAMPLE");
	//if(rc <= 0)
	//{
	//  MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	//  exit(1);
	//}

	_chdrive(drive);
	if(chdir(CurDir) != 0)
	 {
		 MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
		 exit(1);
	 }

 }


void TAcctModule::InitMainWindow()
{

  SetAppName(GetString(hInstance,2));
  GetPrivateProfileString("GL_DIR","COMPANY","",szCompanyName,sizeof(szCompanyName),"GL.INI");
  cb.init();
  cb.lockAttempts = 1;
  cb.exclusive = 0;
  MainWindow = new TManilaGL(GetAppName(),"");
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
  HPrintMenu = LoadMenu(hInstance,MAKEINTRESOURCE(1));
  HGLMenu = LoadMenu(hInstance,MAKEINTRESOURCE(2));
  HGLExMenu = LoadMenu(hInstance,MAKEINTRESOURCE(3));

  ToolBoxLoadDlls(DLL_TBPRO2 | DLL_TBPRO3 | DLL_TBPRO5 );
  hDBF   = LoadLibrary("DBF.DLL");
  hWidgets = LoadLibrary("TABS.DLL");
  //hXTable  = LoadLibrary("xtable.dll");
  AcctModule.Run();
  return AcctModule.Status;
}

HWND TGlWindow::GLCreateStatusBar()
{
  /*
  SendMessage(GetDlgItem(VToolBar->HWindow,3222),WM_SETTEXT,NULL, (LPARAM)(LPSTR)GetCompany());

  return NULL;
 */
  HWND hWndSBR;
  RECT RectClient;


  static STATUSBARITEM Items[] =
	  {
	  {1, 6, 250,GetCompanyName(),SBRS_TEXTLEFT},
	  {2, 6, SBR_AUTOSIZE, ""    ,SBRS_TEXTLEFT},
	  {3, 6, 140,  ""            ,SBRS_TEXTCENTER},
	  };

  GetClientRect(HWindow, &RectClient);

  hWndSBR = tbCreateStatusBar(HWindow, WS_CHILD | WS_VISIBLE | SBRS_DRAWTOPLINE |SBRS_PIXELS,  0,
						RectClient.bottom - STATUSBAR_HEIGHT,
						RectClient.right, STATUSBAR_HEIGHT,
		  Items, sizeof(Items) / sizeof(STATUSBARITEM));

 LOGFONT lFont;
 hfontDlg = (HFONT) NULL;
 memset(&lFont, 0, sizeof(lFont));
 hfontDlg = (HFONT) SendMessage(hWndSBR, WM_GETFONT, 0, 0L);
 if (hfontDlg)
 {
	  if (GetObject(hfontDlg, sizeof(LOGFONT), (LPSTR) &lFont)){
		  lFont.lfHeight = -15;
		  lFont.lfWeight = FW_LIGHT;
		  lFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		  strcpy(lFont.lfFaceName,"MS Sans Serif");
		  hfontDlg = CreateFontIndirect((LPLOGFONT) &lFont);
		 if (hfontDlg)
		 {
		  SendMessage(hWndSBR,WM_SETFONT,(WPARAM)CreateFontIndirect((LPLOGFONT) &lFont),(LPARAM)FALSE);
		 }
	}

 }
 return (hWndSBR);
}





