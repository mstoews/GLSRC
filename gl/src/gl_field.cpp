#include "tbowl.h"
#include "d4all.hpp"
#include "gl_field.h"

extern HINSTANCE hInst;

void  Field::AssignField(HWND hWnd, UINT ID)
{
 switch(type())
   {
    case r4num  : assignDouble(dGetDouble(hWnd,ID));break;
    default     : assign(GetField(hWnd,ID));break;
   }  
 return;
}

void Field::SetField(HWND hWnd, UINT ID)
{                                     
  SendDlgItemMessage(hWnd,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)str());
  return;
}

double Field::dGetDouble(HWND hWnd, UINT ID)
{
 double dValue;
 FloatGetValue(GetDlgItem(hWnd,ID),(LPDOUBLE)&dValue);
 return dValue;
}

long Field::lGetLong(HWND hWnd,UINT ID)
{
 long dValue;
 IntGetValue(GetDlgItem(hWnd,ID),(LPLONG)&dValue);
 return dValue;
}


char *Field::GetField(HWND hWnd, UINT ID)
{
 DWORD len =  SendMessage(GetDlgItem(hWnd,ID),WM_GETTEXTLENGTH,NULL,NULL);
 static char szString[255];
 SendMessage(GetDlgItem(hWnd,ID),WM_GETTEXT,(WPARAM)len + 1,(LPARAM)(LPSTR)szString);
 return szString;
}

char *Field::GetString(HWND hWnd, UINT ID)
{
 DWORD len =  SendMessage(GetDlgItem(hWnd,ID),WM_GETTEXTLENGTH,NULL,NULL);
 static char *szString = new char [(int)len+1];
 SendMessage(GetDlgItem(hWnd,ID),WM_GETTEXT,(WPARAM)len + 1,(LPARAM)(LPSTR)szString);
 return szString;
}


char *Field::GetDate(HWND hWnd,UINT ID)
{
 Date4 dDate;
 dDate.assign(GetString(hWnd,ID),GetString(hWnd,4024));
 dDate.format(GetString(hWnd,4023));
 return dDate.str();
}
