#include "glh.h"
#include "gl_dlg.h"
extern Code4 cb;
extern char szApp[20];
extern HINSTANCE hInst;
void TMainDialog::Add         (RTMessage ){};
void TMainDialog::Top         (RTMessage ){};
void TMainDialog::Bottom      (RTMessage ){};
void TMainDialog::Forward     (RTMessage ){};
void TMainDialog::Back        (RTMessage ){};
void TMainDialog::Delete      (RTMessage ){};
void TMainDialog::Modify      (RTMessage ){};
void TMainDialog::UpdateWindow(){};
void TMainDialog::ClearScreen(void){};
BOOL TMainDialog::GetPosition(char *szWinName)
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 UINT iLeft = GetPrivateProfileInt(szWinName,"LEFT",0,"GL.INI");
 UINT iTop  = GetPrivateProfileInt(szWinName,"TOP",0,"GL.INI");
 return MoveWindow(HWindow,iLeft,iTop,rc.right-rc.left,rc.bottom-rc.top,TRUE);
}

BOOL TMainDialog::ValidGLAccount(char *szAccount)
{
 BOOL bValid = FALSE;
 Data4 glaccts(cb,"glaccts");
 glaccts.SetIndex("accounts");
 if(glaccts.seek(szAccount) == 0)
  bValid = TRUE;
 glaccts.close();
 return bValid;
}


BOOL TMainDialog::SetPosition(char *szWinName)
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 char *szLeft = new char[5];
  itoa(rc.left,szLeft,10);
 WritePrivateProfileString(szWinName,"LEFT",szLeft,"GL.INI");
 char *szTop  = new char[5];
  itoa(rc.top,szTop,10);
 WritePrivateProfileString(szWinName,"TOP",szTop,"GL.INI");
 return TRUE;
}


void TMainDialog::SetupList()
{
 LOGFONT lfFont;

 HFONT   	hFont;
 HWND hDetail = GetDlgItem(HWindow,ID_DISTLIST);

 int Rows = 7;
 short iHeight,iHeader;
 SSGetRowHeightInPixels(hDetail,1,&iHeight);
 SSGetRowHeightInPixels(hDetail,0,&iHeader);  
 RECT wd,rc;

 int Height =  GetSystemMetrics(SM_CYCAPTION)+ 2*GetSystemMetrics(SM_CXDLGFRAME) + ((Rows)*iHeight) + iHeader + 1 ;
 GetWindowRect(HWindow,&wd);
 GetClientRect(HWindow,&rc); 
 MoveWindow(HWindow,wd.left,wd.top,wd.right-wd.left,Height,TRUE);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER)-1,GetSystemMetrics(SM_CYBORDER));
 MoveWindow(hDetail,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);


 //lfFont.lfHeight          = -13;
 lfFont.lfHeight          = -(GetFontSize());
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = 1; //default character set. 128 is Shift JIS
 lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);
 SSSetFont(hDetail,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 SSSetBool(hDetail,SSB_AUTOSIZE,FALSE);
 SSSetBool(hDetail,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(hDetail,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(hDetail,SSB_NOBEEP,TRUE);
 SSSetBool(hDetail,SSB_ALLOWUSERRESIZE,TRUE);
 SSSetBool(hDetail,SSB_PROTECT,TRUE);
 SSSetBool(hDetail,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(hDetail,SSB_NOBEEP,TRUE);




}
void TMainDialog::SetBitmap(int ID ,char *szBitmap)
{
 SendDlgItemMsg(ID,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)szBitmap);
}

void TMainDialog::SetupWindow()
{
TDialog::SetupWindow();
 LOGFONT lFont;
 HFONT  hfontDlg = (HFONT) NULL;
 memset(&lFont, 0, sizeof(lFont));
    if ((hfontDlg = (HFONT) SendMessage(HWindow, WM_GETFONT, 0, 0L))){
		if (GetObject(hfontDlg, sizeof(LOGFONT), (LPSTR) &lFont)){
		 lFont.lfHeight = -16;
		 lFont.lfWeight = FW_LIGHT;
		 lFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		 strcpy(lFont.lfFaceName, "MS Sans Serif");
		 if (hfontDlg = CreateFontIndirect((LPLOGFONT) &lFont)) {
		  SendMessage(HWindow,WM_SETFONT,(WPARAM)hfontDlg,(LPARAM)FALSE);
		 }
	  }
	}
 DeleteObject(hFont);

// SetBitmap(ID_NEW       ,"DEFAULT");
 SetBitmap(ID_NEWBATCH  ,"DEFAULT");
 SetBitmap(ID_TOP	,"TOP");
 SetBitmap(ID_BOTTOM	,"BOTTOM");
 SetBitmap(ID_NEXT	,"NEXT");
 SetBitmap(ID_PREVIOUS	,"PREVIOUS");
// SetBitmap(ID_VIEWLIST	,"LOOK");
// SetBitmap(ID_FA	,"LOOK");
 SetBitmap(ID_ADD	,"ADDLINE");
 SetBitmap(ID_DELETE	,"ERASE"); 
// SetBitmap(ID_VIEWBATCH	,"INSERT");
 SetBitmap(IDCANCEL	,"QUIT");
 SetBitmap(IDOK 	,"ADDLINE");
 SetBitmap(ID_SLIPLIST	,"INSERT");
// SetBitmap(ID_REPS,"SEEK");
// SetBitmap(ID_PRODS,"SEEK");
}





void TMainDialog::GetCalc(RTMessage)
{
 hCalculator = FindWindow("SciCalc",NULL);
 if(hCalculator == NULL)
  WinExec("calc.exe",SW_SHOWNA);
 else
  BringWindowToTop(hCalculator);
}


char *TMainDialog::GetString(int ID)
{
 static char szString[255];
 LoadString(hInst,ID,szString,sizeof(szString));
 return szString;
}

void TMainDialog::DatabaseError()
{
 MessageBox(HWindow,GetString(1367),"Error",MB_ICONEXCLAMATION);
 cb.closeAll();
 cb.init(); 
 ShutDownWindow();
}

void TMainDialog::CreateDbf(char *DbfName, FIELD4INFO *fields, TAG4INFO *tag)
{
}


int TMainDialog::DlgMsg(int ID,int ID_MSG,UINT ICON)
{ 
 char *szTemp1 = new char[255];
 char *szTemp2 = new char[255];
 strcpy(szTemp1,GetString(ID));
 strcpy(szTemp2,GetString(ID_MSG));
 int rc = MessageBox(HWindow,szTemp1,szTemp2,ICON);
 delete szTemp1;
 delete szTemp2;
 return rc;
}

double TMainDialog::dGetField(int ID)
{
 double dValue;
 FloatGetValue(GetDlgItem(HWindow,ID),(LPDOUBLE)&dValue);
 return dValue;
}

long   TMainDialog::lGetField(int ID)
{
 long lValue;
 IntGetValue(GetDlgItem(HWindow,ID),(LPLONG)&lValue);
 return lValue;
}


BOOL TMainDialog::SetField(int ID, Field4 *field)
{
   SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  return TRUE;
}      

BOOL TMainDialog::SetField(int ID, char *field)
{
  SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return TRUE;
}      

BOOL TMainDialog::SetField(int ID, double field)
{
  FloatSetDlgItemValue(Parent->HWindow,ID,field);
  return TRUE;
}

BOOL TMainDialog::SetField(int ID, int field)
{
 IntSetDlgItemValue(Parent->HWindow,ID,field);
 return TRUE;
}






char *TMainDialog::GetField(int ID)
{
 DWORD len =  SendDlgItemMsg(ID,WM_GETTEXTLENGTH,NULL,NULL);
 char *szTemp = new char[(int)len + 1];
 SendDlgItemMsg(ID,WM_GETTEXT,len + 1,(LPARAM)(LPSTR)szTemp);
 return szTemp;
}

void TMainDialog::Cancel (RTMessage)
{
 ShutDownWindow();
}

void TMainDialog::CheckError(int ID)
{ 
 //char *szMsg = new char[(int)85];
 //wsprintf(szMsg,GetString(ID_INPUTERROR));
 MessageBox(HWindow,GetString(ID_INPUTERROR),szApp,MB_ICONEXCLAMATION);
 //delete szMsg;
 return;
}

BOOL TMainDialog::szEnterError(int ID)
{
 BOOL bError = FALSE;
 if(*GetField(ID) == 0)
 {

  char *szField = new char[(int)20];
  strcpy(szField,GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
  delete szField;
  bError = TRUE;
 }
 return bError;
}

BOOL TMainDialog::iEnterError(int ID)
{
 BOOL bError = FALSE;
 if(lGetField(ID) == 0)
 {

  char *szField = new char[(int)20];
  strcpy(szField,GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
  delete szField;
  bError = TRUE;
 }
 return bError;
}


BOOL TMainDialog::dEnterError(int ID)
{
 BOOL bError = FALSE;
 if(dGetField(ID) == 0)
 {
  char *szField = new char[(int)20];
  strcpy(szField,GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
  delete szField;
  bError = TRUE;
 }
 return bError;
}
