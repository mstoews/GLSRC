#include "std_inc.h"


#define ID_LOOK 4054
#define ID_LOCKED_FAILED 1080
#define ID_DATA_ERROR 60
#define ID_DATA_ERROR_MSG 61



extern char szMsg[256];
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
void TMainDialog::MsgBox(string strMessage,UINT ID)
{
  MessageBox(HWindow,(LPSTR)strMessage.c_str(),((TAcctModule *)GetApplication())->GetAppName(),ID);
}

int Data4::SetIndex(char *tagName,Code4* cb)
{
	d4tag_select ( data,d4tag(data,tagName));
	int rc = cb->errorCode;
			if (cb->errorCode < 0)
			{
			 string strMsg,strError;
			 strError = GetString(cb->hInst,cb->errorCode);
			 wsprintf(szMsg,GetString(cb->hInst,ID_DATA_ERROR),(LPSTR)strError.c_str(),alias());
			 strMsg = szMsg;
			 MessageBox(GetFocus(),(LPSTR)strMsg.c_str(),"",MB_ICONINFORMATION);
			 cb->closeAll();
			 cb->init();
			 }
	return rc;
}


void TMainDialog::ErrorClose(LPSTR szFile)
{
  string strMsg,strError;
  strError = GetString(((TAcctModule *)GetApplication())->cb.errorCode);
  wsprintf(szMsg,GetString(ID_DATA_ERROR),(LPSTR)strError.c_str(),szFile);
  strMsg = szMsg;
  MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
  ((TAcctModule *)GetApplication())->cb.closeAll();
  ((TAcctModule *)GetApplication())->cb.init();
  ShutDownWindow();
  SetFocus(Parent->HWindow);
}


BOOL TMainDialog::GetPosition(char *szWinName)
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 UINT iLeft = GetPrivateProfileInt(szWinName,"L",0,"GL.INI");
 UINT iTop  = GetPrivateProfileInt(szWinName,"T",0,"GL.INI");
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
 WritePrivateProfileString(szWinName,"L",szLeft,"GL.INI");
 char *szTop  = new char[5];
  itoa(rc.top,szTop,10);
 WritePrivateProfileString(szWinName,"T",szTop,"GL.INI");
 return TRUE;
}


void TMainDialog::SetupList()
{
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
 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
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

void TMainDialog::SetBitmap(UINT ID)
{
 SendDlgItemMsg(ID,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)ID);
}


void TMainDialog::SetupWindow()
{
 TDialog::SetupWindow();
 //cb->closeAll();
 cb = &((TAcctModule *)GetApplication())->cb;
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
 SetBitmap(ID_NEW);
 SetBitmap(ID_TOP);
 SetBitmap(ID_BOTTOM);
 SetBitmap(ID_NEXT);
 SetBitmap(ID_PREVIOUS);
 SetBitmap(ID_DELETE);
 SetBitmap(ID_SLIPLIST);
 SetBitmap(ID_LOOK);
 RECT rc;
 RECT parent;
 int pos = 80;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+3,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);
}





void TMainDialog::GetCalc(RTMessage)
{
 hCalculator = FindWindow("SciCalc",NULL);
 if(hCalculator == NULL)
  WinExec("calc.exe",SW_SHOWNA);
 else
  BringWindowToTop(hCalculator);
}


char *TMainDialog::GetDate(int ID)
{
 Date4 dDate;
 dDate.assign(GetField(ID),GetString(4024));
 dDate.format(GetString(4023));
 return dDate.str();
}

char *TMainDialog::GetDate(char *date)
{
  static char szDate[11];
  Date4 dDate(date);
  dDate.format(szDate,GetString(4024));
  return szDate;
}

char *TMainDialog::GetString(int ID)
{
 static char szString[255];
 LoadString(hInst,ID,szString,sizeof(szString));
 return szString;
}

void TMainDialog::DatabaseError()
{
}

#pragma argsused
void TMainDialog::CreateDbf(char *DbfName, FIELD4INFO *fields, TAG4INFO *tag)
{
}

#pragma argsused
int TMainDialog::DlgMsg(int ID,int ID_MSG,UINT ICON)
{ 
 return TRUE;
}

double TMainDialog::dGetField(int ID)
{
 double dValue;
 FloatGetValue(GetDlgItem(HWindow,ID),(LPDOUBLE)&dValue);
 return dValue;
}

double TMainDialog::dGetField(HWND hWnd,int ID)
{
 double dValue;
 FloatGetValue(GetDlgItem(hWnd,ID),(LPDOUBLE)&dValue);
 return dValue;
}


char *TMainDialog::GetField(HWND hWnd, int ID)
{
 static string szTemp;
 DWORD len =  SendMessage(GetDlgItem(hWnd,ID),WM_GETTEXTLENGTH,NULL,NULL);
 SendMessage(GetDlgItem(hWnd,ID),WM_GETTEXT,(WPARAM)(len + 1),(LPARAM)(LPSTR)szTemp.c_str());
 return (char *) szTemp.c_str();
}


long   TMainDialog::lGetField(int ID)
{
 long lValue;
 IntGetValue(GetDlgItem(HWindow,ID),(LPLONG)&lValue);
 return lValue;
}


BOOL TMainDialog::SetField(int ID, Field4 *field)
{

 string szClassName;
 szClassName.resize(40);
 ::GetClassName(GetDlgItem(HWindow,ID),(LPSTR)szClassName.c_str(),szClassName.length());
 szClassName.to_upper();
 if(szClassName == "TBFLOAT")
  {
	double d = atof(field->str());
	FloatSetDlgItemValue(HWindow,ID,d);
  }
 else
  if(szClassName == "TBINT")
  {
	int i = atoi(field->str());
	IntSetDlgItemValue(Parent->HWindow,ID,i);
  }
 else
  {
	SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  }
  return TRUE;
}

BOOL TMainDialog::SetField(int ID, Field4& field)
{
 string szClassName;
 szClassName.resize(40);
 ::GetClassName(GetDlgItem(HWindow,ID),(LPSTR)szClassName.c_str(),szClassName.length());
 szClassName.to_upper();
 if(szClassName == "TBFLOAT")
  {
	double d = atof(field.str());
	FloatSetDlgItemValue(HWindow,ID,d);
  }
 else
  if(szClassName == "TBINT")
  {
	int i = atoi(field.str());
	IntSetDlgItemValue(Parent->HWindow,ID,i);
  }
 else
  {
	SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field.str());
  }
  return TRUE;
}


BOOL TMainDialog::SetField(int ID, char *field)
{
  SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return TRUE;
}

BOOL TMainDialog::SetField(int ID, string* field)
{
  SendDlgItemMsg(ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->c_str());
  return TRUE;
}

BOOL TMainDialog::SetField(int ID, double field)
{
  FloatSetDlgItemValue(HWindow,ID,field);
  return TRUE;
}

BOOL TMainDialog::SetField(int ID, int field)
{
 IntSetDlgItemValue(Parent->HWindow,ID,field);
 return TRUE;
}


char *TMainDialog::GetField(int ID)
{
 static string szTemp;
 DWORD len =  SendDlgItemMsg(ID,WM_GETTEXTLENGTH,NULL,NULL);
 SendDlgItemMsg(ID,WM_GETTEXT,(WPARAM)(len + 1),(LPARAM)(LPSTR)szTemp.c_str());
 return (char *) szTemp.c_str();
}

void TMainDialog::Cancel (RTMessage)
{
  cb->init();
 ShutDownWindow();
}

void TMainDialog::CheckError(int ID)
{
 char szField[40];
 strcpy(szField,GetString(ID));
 wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
 MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
 SetFocus(GetDlgItem(HWindow,ID));
 return;
}

BOOL TMainDialog::szEnterError(int ID)
{
 BOOL bError = FALSE;
 DWORD len =  SendMessage(GetDlgItem(HWindow,ID),WM_GETTEXTLENGTH,(WPARAM)NULL,NULL);
 Str4large szText;
 szText.setLen((int)len);
 szText.set(' ');
 szText.replace(Str4ptr(GetField(ID)));
 szText.trim();
 int iLen = szText.len();
 if(iLen == 0)
 {
  Str4large szField;
  szField.setLen(40);
  szField.assign(GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField.str());
  //MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  bError = TRUE;
 }
 SetFocus(GetDlgItem(HWindow,ID));
 return bError;
}

BOOL TMainDialog::iEnterError(int ID)
{
 BOOL bError = FALSE;
 long dValue;
 FloatGetValue(GetDlgItem(HWindow,ID),(LPDOUBLE)&dValue);
 if(dValue == 0)
 {
  Str4large szField;
  szField.setLen(40);
  szField.assign(GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField.str());
  MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  bError = TRUE;
 }
 return bError;
}


BOOL TMainDialog::dEnterError(int ID)
{
 BOOL bError = FALSE;
 double dValue;
 FloatGetValue(GetDlgItem(HWindow,ID),(LPDOUBLE)&dValue);
 if(dValue == 0)
 {
  char szField[40];
  strcpy(szField,GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
  MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  bError = TRUE;
 }
 SetFocus(GetDlgItem(HWindow,ID));
 return bError;
}
