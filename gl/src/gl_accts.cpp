
/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Modify, add and delete accounts functions.
*/
#include "tbowl.h"
#include "d4all.hpp"
#include "gl_strg.h"
#include "glclass.h"
#include "gl.h"

#include "db_dept.h"

#include "gl_dlg.h"
#include "gl_pick.h"
#include "db_accts.h"
#include "gl_main.h"
#define  ID_SEEK 3005
#include "gl_accts.h"
#include "gl_budgt.h"
//#include "utildlg.h"
#include "gl_field.h"


extern Code4 cb;
extern HINSTANCE hInst;
extern char szApp[20];
BOOL  CheckFile(char *szFileName,HWND hWnd);

WORD FAR PASCAL _export PicCallBack(HANDLE hWnd, short ID, short nPos, char cChar);

#pragma argsused
WORD FAR PASCAL _export PicCallBack(HANDLE hWnd, short ID, short nPos, char cChar)
{ 
  //HANDLE hParent = GetParent(hWnd);
  //SendMessage(GetDlgItem(hWnd,ID),EPM_SETMASK,NULL,(LPARAM)(LPSTR)"XXXXXXX");
  //SendDlgItemMessage(hParent,ID,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)cChar);
  //MessageBox(hParent,"Call Back","",MB_OK);
  //SendMessage(hParent,WM_COMMAND,ID_SEARCH,NULL);
  return cChar;
}

void TGLAcctsAdd::Seek(RTMessage Msg)
{
 GetSeek(Msg);
}

void TGLAcctsDelete::Ok (RTMessage)
{
if(MessageBox(HWindow,GetString(1021),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
{
 glaccts->SetIndex("accounts");
 if(*GetField(ID_ACCOUNTS) == 0)
 {
  MessageBox(HWindow,GetString(1025),szApp,MB_OK);
  return;
 }
//char *szSeek = strcat(strcpy(new char[strlen(GetField(ID_ACCOUNTS)) + strlen(GetField(ID_DEPART)) + 1],GetField(ID_ACCOUNTS)),GetField(ID_DEPART));
string szSeek;
szSeek = GetField(ID_ACCOUNTS);
szSeek += GetField(ID_DEPART);
MessageBox(HWindow,szSeek.c_str(),"",MB_OK);
if(glaccts->seek((char *)szSeek.c_str())!=0)
 {
  MessageBox(HWindow,GetString(1026),szApp,MB_OK);
  return;
 }
if(
		  glaccts->dPeriod1() != 0
	  || glaccts->dPeriod2() != 0
	  || glaccts->dPeriod3() != 0
	  || glaccts->dPeriod4() != 0
	  || glaccts->dPeriod5() != 0
	  || glaccts->dPeriod6() != 0
	  || glaccts->dPeriod7() != 0
	  || glaccts->dPeriod8() != 0
	  || glaccts->dPeriod9() != 0
	  || glaccts->dPeriod10() != 0
	  || glaccts->dPeriod11() != 0
	  || glaccts->dPeriod12() != 0
	  || glaccts->dPrevious12() != 0
	  )
 {
  MessageBox(HWindow,GetString(1027),szApp,MB_OK);
  return;
 }
 glaccts->deleteRec();
 glaccts->pack();
 bAcctList = TRUE;
 SetDlgItemText(HWindow,ID_ACCOUNTS ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_DEPART   ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_DEPTCODE ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_JDESCRIPT ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_CURRBAL  ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_OPENBAL  ,(LPSTR) "" );
 SetFocus(GetDlgItem(HWindow,2013));
 GLACCOUNT->List->Redraw();
 }
 return;
}


void TGLAcctsAdd::SetData(RTMessage Msg)
{
 char *szBuff = strcat(strcpy(new char[strlen(GetField(ID_ACCOUNTS))],GetField(ID_ACCOUNTS)),GetField(ID_DEPART));
 if(*GetField(ID_ACCOUNTS) == 0)
 glaccts->top();
 else
 {  
  glaccts->seek(szBuff);
 }
 Update(Msg);
}

void TGLAcctsAdd::Delete(RTMessage Msg)
{
if(MessageBox(HWindow,GetString(1021),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
{
 glaccts->SetIndex("accounts");
 if(*GetField(ID_ACCOUNTS) == 0)
 {
  MessageBox(HWindow,GetString(1025),szApp,MB_OK);
  return;
 }
char *szSeek = strcat(strcpy(new char[strlen(GetField(ID_ACCOUNTS)) + strlen(GetField(ID_DEPART)) + 1],GetField(ID_ACCOUNTS)),GetField(ID_DEPART));
if(glaccts->seek(szSeek)!=0)
 {
  MessageBox(HWindow,GetString(1026),szApp,MB_OK);
  delete szSeek;
  return;
 }
if(
		  glaccts->dPeriod1() != 0
	  || glaccts->dPeriod2() != 0
	  || glaccts->dPeriod3() != 0
	  || glaccts->dPeriod4() != 0
	  || glaccts->dPeriod5() != 0
	  || glaccts->dPeriod6() != 0
	  || glaccts->dPeriod7() != 0
	  || glaccts->dPeriod8() != 0
	  || glaccts->dPeriod9() != 0
	  || glaccts->dPeriod10() != 0
	  || glaccts->dPeriod11() != 0
	  || glaccts->dPeriod12() != 0
	  || glaccts->dPrevious12() != 0
	  )
 {
  MessageBox(HWindow,GetString(1027),szApp,MB_OK);
  delete szSeek;
  return;
 }
 glaccts->deleteRec();
 glaccts->pack();
 bAcctList = TRUE;
 SetDlgItemText(HWindow,ID_ACCOUNTS ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_DEPART   ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_DEPTCODE ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_JDESCRIPT ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_CURRBAL  ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_OPENBAL  ,(LPSTR) "" );
 SetFocus(GetDlgItem(HWindow,2013));
 GLACCOUNT->List->Redraw();
 }
}

void TGLAcctsAdd::Update(RTMessage)
{
		glaccts->accounts->SetField(HWindow,ID_ACCOUNTS);
      glaccts->dept->SetField(HWindow,ID_DEPART);
      glaccts->descript->SetField(HWindow,ID_JDESCRIPT);
      glaccts->fdescript->SetField(HWindow,ID_DESCRIPT);
		glaccts->previous12->SetField(HWindow,ID_OPENBAL);
      glaccts->curr_bal->SetField(HWindow,ID_CURRBAL);

     if(lstrcmp(glaccts->type->str(),"B")==0)
      {
      SendDlgItemMsg(ID_BS,BM_SETCHECK , 1 ,0L);
      SendDlgItemMsg(ID_RE,BM_SETCHECK , 0 ,0L);
      SendDlgItemMsg(ID_IS,BM_SETCHECK , 0 ,0L);
      }
     else if(lstrcmp(glaccts->type->str(),"R")==0)
      {
		SendDlgItemMsg(ID_BS,BM_SETCHECK , 0 ,0L);
      SendDlgItemMsg(ID_IS,BM_SETCHECK , 0 ,0L);
      SendDlgItemMsg(ID_RE,BM_SETCHECK , 1 ,0L);
      }
     else if(lstrcmp(glaccts->type->str(),"I")==0)
      {
      SendDlgItemMsg(ID_BS,BM_SETCHECK , 0 ,0L);
      SendDlgItemMsg(ID_IS,BM_SETCHECK , 1 ,0L);
      SendDlgItemMsg(ID_RE,BM_SETCHECK , 0 ,0L);
      }
      SetFocus(GetDlgItem(HWindow,IDOK));

}

void TGLAcctsAdd::GetSeek (RTMessage)
{
    GLACCOUNT->List->Seek(ID_ACCOUNTS);
    GLACCOUNT->Show(SW_SHOW);
}


void TGLAcctsAdd::Search  (RTMessage)
{
    GLACCOUNT->List->Seek(ID_ACCOUNTS);
	 GLACCOUNT->Show(SW_SHOW);
}

void TGLAcctsDelete::SetupWindow()
{
 TGLAcctsAdd::SetupWindow();
 SetField(IDOK,GetString(8095));
 SetCaption(GetString(8096));
}

void TGLAcctsAdd::SetupWindow()
{
 TMainDialog::SetupWindow();
 if(!CheckFile("GLACCTS",Parent->HWindow) || !CheckFile("GLDEPT",Parent->HWindow))
  {
   ShutDownWindow();
   return;
  }
 lpfnSetPic = MakeProcInstance ((FARPROC) PicCallBack, GetApplication()->hInstance);
 SendMessage(GetDlgItem(HWindow,ID_CURRBAL),EPM_SETMASK,NULL,(LPARAM)(LPSTR)"???????");
 SendMessage(GetDlgItem(HWindow,ID_CURRBAL),EPM_SETCALLBACK,NULL,(LPARAM)lpfnSetPic);
 HourGlassOn();
 HWND hForeignLanguage = GetDlgItem(HWindow,ID_DESCRIPT);
 ShowWindow(hForeignLanguage,SW_HIDE);
 glaccts = new Tglaccts(&cb);
 glaccts->SetIndex("accounts");
 gldept  = new Tgldept(&cb);

 GLACCOUNT = new TAccountList(this,"",glaccts);
 GLACCOUNT->Attr.Style |= WS_THICKFRAME;
 GLACCOUNT->EnableAutoCreate();
 GetApplication()->MakeWindow(GLACCOUNT);
 bAcctList = TRUE;
 SetFocus(GetDlgItem(HWindow,2013));
 SendDlgItemMsg(ID_FOREIGN,BM_SETCHECK,0,0L);
 SetBitmap(ID_GLINQ);
 SetBitmap(4076);
 SetBitmap(IDOK);
 SetBitmap(IDCANCEL);
 HourGlassOff();
}



void TGLAcctsAdd::Ok (RTMessage Msg)
{

 if(szEnterError(ID_ACCOUNTS))
  return;

 if(szEnterError(ID_JDESCRIPT))
  return;

 glaccts->SetIndex("accounts");

 string szS;
 szS = GetField(ID_ACCOUNTS);
 szS += GetField(ID_DEPART);

 if(glaccts->seek((char *)szS.c_str()) != 0 && strcmp(Type(),"R") == 0)
	{
	 MessageBox(HWindow,GetString(ID_RETAINEDERROR),szApp,MB_OK);
	 return;
	}

 if(glaccts->seek((char *) szS.c_str())== 0)
 {
  Modify();
  return;
 }

 if(strcmp(Type(),"N") == 0)
  {
	MessageBox(HWindow,GetString(4071),szApp,MB_ICONINFORMATION);
	return;
  }

 // Before adding to the database make sure that the record hasn't been added
 // previously or that there is anything in the current balance of previous balance for the year.

 glaccts->appendStart();
 Field4 field;
 for(int fieldNum = 1; fieldNum<=glaccts->numFields(); fieldNum++){
  field.init(*(Data4 *)glaccts,fieldNum);
  switch(field.type()){
	  case r4num: field.assignDouble(0); break;
	  default: break;
  }
 }
 glaccts->accounts->AssignField(HWindow,ID_ACCOUNTS);
 glaccts->dept->AssignField(HWindow,ID_DEPART);
 glaccts->descript->AssignField(HWindow,ID_JDESCRIPT);
 glaccts->fdescript->AssignField(HWindow,ID_DESCRIPT);
 glaccts->type->assign(Type());
 glaccts->curr_bal->AssignField(HWindow,ID_CURRBAL);
 glaccts->previous12->AssignField(HWindow,ID_OPENBAL);
 glaccts->openbal->AssignField(HWindow,ID_OPENBAL);
 glaccts->UpdateUserTime(szUser);
 glaccts->append();

 SetField(ID_ACCOUNTS ,"" );
 SetField(ID_DEPART   ,"" );
 SetField(ID_DEPTCODE ,"" );
 SetField(ID_JDESCRIPT,"" );
 SetField(ID_DESCRIPT,"" );
 FloatSetDlgItemValue(HWindow,ID_CURRBAL  ,0);
 FloatSetDlgItemValue(HWindow,ID_OPENBAL  ,0);
 GLACCOUNT->List->Redraw();
 SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));
}

void TGLAcctsAdd::Modify()
{
	 //if(MessageBox(HWindow,GetString(ID_NOTZERO),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
	 //{
	  glaccts->descript->AssignField(HWindow,ID_JDESCRIPT);
	  glaccts->fdescript->AssignField(HWindow,ID_DESCRIPT);
	  glaccts->previous12->AssignField(HWindow,ID_OPENBAL);
	  glaccts->curr_bal->AssignField(HWindow,ID_CURRBAL);
     glaccts->UpdateUserTime(szUser);
	  glaccts->flush();
	  glaccts->unlock();
	  GLACCOUNT->List->Redraw();
	  SendMessage(HWindow,WM_COMMAND,ID_GLINQ,NULL);
	// }
	 return;
}



void TGLAcctsAdd::Cancel         (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}


char *TGLAcctsAdd::Type()
 {
  static char *szType;
  DWORD dwIndex = SendDlgItemMsg(ID_BS,BM_GETCHECK,0,0);
  if(dwIndex==1)
   {
    szType = strcpy(new char[2],"B");
	 return szType;
   }

  dwIndex = SendDlgItemMsg(ID_IS,BM_GETCHECK,0,0);
  if(dwIndex==1)
   {
    szType = strcpy(new char[2],"I");
    return szType;
   }
 
  dwIndex = SendDlgItemMsg(ID_RE,BM_GETCHECK,0,0);
  if(dwIndex==1)
   {
    szType = strcpy(new char[2],"R");
    return szType;
   }
  else
   {
    szType = strcpy(new char[2],"N");
    return szType;
   }
 }

void TGLAcctsAdd::ShowForeign(RTMessage)
{
 DWORD dwIndex = SendDlgItemMsg(ID_FOREIGN,BM_GETCHECK,NULL,NULL);
 if(dwIndex == 1)
 {
  ShowWindow(GetDlgItem(HWindow,ID_DESCRIPT),SW_HIDE);
  SendDlgItemMsg(ID_FOREIGN,BM_SETCHECK,0,0L);
 }
 else
 {
  ShowWindow(GetDlgItem(HWindow,ID_DESCRIPT),SW_SHOW);
  SendDlgItemMsg(ID_FOREIGN,BM_SETCHECK,1,0L);
 }


}

void TGLAcctsAdd::BS (RTMessage )
  {
  SendDlgItemMsg ( ID_BS, BM_SETCHECK , 1 ,0L);
  SendDlgItemMsg ( ID_IS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_RE, BM_SETCHECK , 0 ,0L);
  glaccts->type->assign("B");
  SetFocus(GetDlgItem(HWindow, IDOK));
  }

void TGLAcctsAdd::IS (RTMessage )
  {
  SendDlgItemMsg ( ID_BS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_IS, BM_SETCHECK , 1 ,0L);
  SendDlgItemMsg ( ID_RE, BM_SETCHECK , 0 ,0L);
  glaccts->type->assign("I");
  SetFocus(GetDlgItem(HWindow, IDOK));
  }

void TGLAcctsAdd::RE (RTMessage )
  {
  SendDlgItemMsg ( ID_BS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_IS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_RE, BM_SETCHECK , 1 ,0L);
  glaccts->type->assign("R");
  SetFocus(GetDlgItem(HWindow, IDOK));
  }




// Function called from the main  - Add accounts from the chart of accts.
void TGlWindow::GLAcctAdd (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLAcctsAdd(this,1010));
 }


 
// Function called from the main  - Delete accounts from the chart of accts.
void TGlWindow::GLAcctDel (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLAcctsDelete(this,1010));
 }

void TGlWindow::GLAcctBudget  (RTMessage)
{
  GetApplication()->ExecDialog(new TGLBudget(this,1017));
}

void TGLBudget::SetupWindow()
{
 TGLAcctsAdd::SetupWindow();
 SetCaption(GetString(8097));
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 RECT rc;
 SSGetClientRect(SHWindow,&rc);
 short d1Width;

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 SSSetBool(SHWindow,SSB_AUTOSIZE,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, 12);
 
 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetColWidth(SHWindow,1,6);

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);


 short ColWidth = ((rc.right - rc.left) - d1Width)/3;

 SSSetColWidthInPixels(SHWindow,2,ColWidth);
 SSSetColWidthInPixels(SHWindow,3,ColWidth);
 SSSetColWidthInPixels(SHWindow,4,ColWidth);

 //lfFont.lfHeight          = -13;
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
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, "Period");
 SSSetData(SHWindow,2,SS_HEADER, "Budget"); 
 SSSetData(SHWindow,3,SS_HEADER, "Current Year");
 SSSetData(SHWindow,4,SS_HEADER, "Previous Year");


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,SS_ALLCOLS, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,4,0,0,99);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);

}

void TGLBudget::Update(RTMessage Msg)
{
 TGLAcctsAdd::Update(Msg);

 
 Field4 *budget;
 Field4 *period;
 Field4 *previous;
 char *szPeriod = new char [3];
 SSSetBool(SHWindow,SSB_REDRAW,FALSE);
 for(int i = 1;i<13;i++)
 {
  period   = (Field4 *) new Field4(*(Data4 *)glaccts,(i+7));
  previous = (Field4 *) new Field4(*(Data4 *)glaccts,(i+20));
  budget   = (Field4 *) new Field4(*(Data4 *)glaccts,(i+32));
  SSSetData(SHWindow,2,i,budget->str());
  SSSetData(SHWindow,3,i,period->str());
  SSSetData(SHWindow,4,i,previous->str());
  itoa(i,szPeriod,10);
  SSSetData(SHWindow,1,i,szPeriod);
 } 
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
}

void TGLBudget::Ok(RTMessage)
{
 if(MessageBox(HWindow,GetString(8098),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
 {
   char *szBudget = new char [glaccts->budget1->len() + 1];
    for(int i=1;i<13;i++)
     {
      	SSGetData(SHWindow,2,i,szBudget);
  	double dValue = atof(szBudget);
	  switch (i)
	  {
	    case  1:  glaccts->budget1->assignDouble(dValue); break;
	    case  2:  glaccts->budget2->assignDouble(dValue); break;
	    case  3:  glaccts->budget3->assignDouble(dValue); break;
	    case  4:  glaccts->budget4->assignDouble(dValue); break;
	    case  5:  glaccts->budget5->assignDouble(dValue); break;
	    case  6:  glaccts->budget6->assignDouble(dValue); break;
	    case  7:  glaccts->budget7->assignDouble(dValue); break;
	    case  8:  glaccts->budget8->assignDouble(dValue); break;
	    case  9:  glaccts->budget9->assignDouble(dValue); break;
	    case 10:  glaccts->budget10->assignDouble(dValue); break;
	    case 11:  glaccts->budget11->assignDouble(dValue); break;
	    case 12:  glaccts->budget12->assignDouble(dValue); break;
	  }
    }
 }
}