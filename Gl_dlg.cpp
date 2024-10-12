#include "glh.h"
#include "gl_dlg.h"

void TMainDialog::Add         (RTMessage ){};
void TMainDialog::Top         (RTMessage ){};
void TMainDialog::Bottom      (RTMessage ){};
void TMainDialog::Forward     (RTMessage ){};
void TMainDialog::Back        (RTMessage ){};
void TMainDialog::Delete      (RTMessage ){};
void TMainDialog::Modify      (RTMessage ){};
void TMainDialog::UpdateWindow(){};
void TMainDialog::ClearScreen(void){};
void TMainDialog::SetupWindow(){ TDialog::SetupWindow();};
void TMainDialog::CreateDbf(char *DbfName, FIELD4INFO *fields, TAG4INFO *tag)
{
 DATA4 *create;
 cb.safety = 0;
 create = d4create(&cb,DbfName,fields,tag);
 d4close(create);
}


int TMainDialog::DlgMsg(int ID,int ID_MSG,UINT ICON)
{ 
 char *szTemp = new char[255];
 strcpy(szTemp,GetString(GetApplication()->hInstance,ID));
 int rc = BWCCMessageBox(HWindow,szTemp,GetString(GetApplication()->hInstance,ID_MSG),ICON);
 return rc;
}

double TMainDialog::dGetField(int ID)
{
 double dValue;
 FloatGetValue(GetDlgItem(HWindow,ID),(LPDOUBLE)&dValue);
 return dValue;
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

char *TMainDialog::GetField(int ID)
{
 DWORD len =  SendDlgItemMsg(ID,WM_GETTEXTLENGTH,NULL,NULL);
 char *szTemp = new char[(int)len + 1];
 SendDlgItemMsg(ID,WM_GETTEXT,len + 1,(LPARAM)(LPSTR)szTemp);
 return szTemp;
}

void TMainDialog::Cancel (RTMessage)
{

 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TMainDialog::CheckError(int ID)
{ 
 char *szMsg = new char[(int)150];
 HINSTANCE hInst = GetApplication()->hInstance;
 strcpy(szMsg,GetString(hInst,ID_INPUTERROR));
 strcat(szMsg,GetString(hInst,ID));
 BWCCMessageBox(HWindow,szMsg,GetString(hInst,ID_ERROR),MB_ICONEXCLAMATION);
 delete szMsg;
 return;
}

void TMainDialog::EnterError(int ID)
{
 char *szMsg = new char[(int)150];
 char *szField = new char[(int)20]; 
 HINSTANCE hInst = GetApplication()->hInstance;
 strcpy(szField,GetString(hInst,ID));
 wsprintf(szMsg,GetString(hInst,ID_BLANKFIELD),szField);
 BWCCMessageBox(HWindow,szMsg,GetString(hInst,ID_ERROR),MB_ICONEXCLAMATION);
 delete szMsg,szField;
 return;
}
