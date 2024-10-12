
/*
	General Ledger  - (C) Copyright 1994 by Tegara Corporation
	Modify, add and delete accounts functions.

*/
#include "tbowl.h"
#include "d4all.hpp"


//#include "gl_strg.h"
#include "glclass.h"
#include "gl.h"
#include "gl_dlg.h"
#include "gl_pick.h"
#include "db_accts.h"
#include "tlist_w.h"
#include "ctype.h"

extern Code4 cb;
#define PROFILE "GL.INI"

#pragma argsused
void TListWindow::WMSize(RTMessage Msg)
{

}

BOOL TListWindow::SetPosition()
{
 RECT rc;
 GetWindowRect(Parent->HWindow,&rc);
 if(iRight == 0)
 iRight = rc.right;
 if(iBottom == 0)
 iBottom = rc.bottom;
 //return MoveWindow(Parent->HWindow,iLeft,iTop,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 return MoveWindow(Parent->HWindow,iLeft,iTop,iRight-iLeft,iBottom-iTop,TRUE);
}

TListWindow::~TListWindow()
{
 cb.closeAll();
}

void TListWindow::Cancel(RTMessage)
{
 if(CanClose())
  ShutDownWindow();
}

BOOL TListWindow::Seek(UINT ID)
{
  int len = (int) SendMessage(GetDlgItem(Parent->Parent->HWindow,ID),WM_GETTEXTLENGTH,NULL,NULL);
  char *szStr = new char [len+1];
  SendMessage(GetDlgItem(Parent->Parent->HWindow,ID),WM_GETTEXT,len+1,(LPARAM)(LPSTR)szStr);
  StrTrim(szStr);
  Str4large *szText = new Str4large;
  szText->setLen((int)len);
  szText->set(' ');
  szText->replace(Str4ptr(szStr));
  szText->trim();
  len = szText->len();
  delete [] szStr;
  BOOL bFound = FALSE;
  SetBool(SSB_REDRAW,FALSE);
  char szValue[11];
  SS_COORD lRow,lCol;
  GetLastValidCell(&lCol,&lRow);
  SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
  if(len > 0)
	for(int i=1; i <= lRow; i++)
	{
	 GetData(1,i,szValue);
	 Str4large *szCompare = new Str4large;
	 szCompare->set(' ');
	 szCompare->replace(Str4ptr(szValue));
	 szCompare->setLen(len);

	 if(strcmp(szText->ptr(),szCompare->ptr()) == 0)
	  {
		SetColor(SS_ALLCOLS,i,RGB(192,220,192), RGBCOLOR_BLUE);
		ShowCell(1,i,SS_SHOW_NEAREST);
		SetActiveCell(1,i);
		bFound = TRUE;
		break;
	  }
	}

 SetBool(SSB_REDRAW,TRUE);
 SetFocus(HWindow);
 if(!bFound)
  SetActiveCell(1,1);
 return bFound;
}


BOOL TListWindow::SavePosition()
{
 RECT rc;
 GetWindowRect(Parent->HWindow,&rc);
 Str4ten szLeft;
 Str4ten szTop;
 Str4ten szRight;
 Str4ten szBottom;
 szLeft.assignLong(rc.left,5);
 szTop.assignLong(rc.top,5);
 szRight.assignLong(rc.right,5);
 szBottom.assignLong(rc.bottom,5);
 WritePrivateProfileString(szlpAWindow,"LEFT",szLeft.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"TOP",szTop.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"RIGHT",szRight.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"BOTTOM",szBottom.str(),PROFILE);
 return TRUE;
}

_CLASSDEF (TSetQuery)
class TSetQuery : public TMainDialog
{
 public:
 char szWindow[12];
 char szQuery[128];
 TSetQuery(PTWindowsObject AParent, int AName,char *szWin):TMainDialog(AParent,AName)
 {
	strcpy(szWindow,szWin);
 };
 virtual void SetupWindow();
 virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
};

void TSetQuery::Ok (RTMessage)
{
 strcpy(szQuery,GetField(101));
 WritePrivateProfileString(szWindow,"QUERY",szQuery,PROFILE);
 ShutDownWindow();
}

void TSetQuery::SetupWindow()
{
 GetPrivateProfileString(szWindow,"QUERY","",szQuery,sizeof(szQuery),PROFILE);
 SetField(101,szQuery);
}

void TListWindow::SetParamenters(RTMessage)
{
 GetApplication()->ExecDialog(new TSetQuery(this,1035,szlpAWindow));
 Redraw();
}


void TListWindow::GetKey(RTMessage Msg)
{
  switch(Msg.WParam)
	{
	 //case  VK_BACK   : --iSeekLength;--pSeekBuffer;break;
	 case  VK_DELETE : iSeekLength = 0;  break;
	 case  VK_ESCAPE :
		{
		  iSeekLength = 0;
		  SetFocus(HWindow);
		  SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
		  SetActiveCell(1,1);
		  break;
		}
	 case  VK_RETURN : break;
	 default:
	  {
		char szSeek[2];
		char *pStorage;
		szSeek[0] = (char) Msg.WParam;
		szSeek[1] = 0;
		if(iSeekLength < 0)
		 iSeekLength = 0;
		if(iSeekLength > 0)
		 {
		  pStorage = (char *) malloc(iSeekLength);
		  strcpy(pStorage,pSeekBuffer);
		 }
		++iSeekLength;// Increment the seek buffer;
		pSeekBuffer = (char *) malloc(iSeekLength);
		char *p = (char *) malloc(1);
		p = szSeek;
		if(iSeekLength > 1)
		 {
		  strcpy(pSeekBuffer,pStorage);
		  strcat(pSeekBuffer,p);
		 }
		else
		strcpy(pSeekBuffer,p);
		//Debug("Seek Buffer = %s, Length = %d",pSeekBuffer,iSeekLength);
		SetFocus(HWindow);
		Seek(pSeekBuffer,iSeekLength);
		//Seek(szSeek,1);
	  }
  }
}


void TListWindow::SetFields(char *szdBase,char *szIndex, int iNumberOfFields)
{
 iFieldNumber = iNumberOfFields;
 SS_CELLTYPE 	CellType;
 GetRowHeightInPixels(1,&iHeight);
 GetRowHeightInPixels(0,&iHeader);
 Rows = 7; 
 dBase = new Data4(&cb,szdBase);
 strcpy(szIdx,szIndex);
 dBase->SetIndex(szIdx);

 // First open the database and set the size of the windows based upon the
 // first three fields length.
 if(iFieldNumber == 0)
  iFieldNumber = dBase->numFields();

 for (int j=0; j<iFieldNumber; j++)
  {
	 int iField = j+1;
	 Field4 *fField = new Field4 (*(Data4 *)dBase,iField);
    SetData(iField,0,fField->name());
	 SetColWidth(iField,fField->len());
  }
 
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ ((Rows)*iHeight) + iHeader ;
 GetWindowRect(Parent->HWindow,&wd);
 
 Field4 *Field1 = new Field4(*(Data4 *)dBase,iField1);
 SetColWidth(1,Field1->len());
 Field4 *Field2= new Field4(*(Data4 *)dBase,iField2);
 SetColWidth(2,Field2->len());
 Field4 *Field3= new Field4(*(Data4 *)dBase,iField3);
 SetColWidth(3,Field3->len());

 short iWidth1,iWidth2,iWidth3,ColWidth;
 GetColWidthInPixels(iField1,(LPSHORT)&iWidth1);
 GetColWidthInPixels(iField2,(LPSHORT)&iWidth2);
 GetColWidthInPixels(iField3,(LPSHORT)&iWidth3);
 ColWidth = iWidth1+iWidth2+iWidth3;
 MoveWindow(Parent->HWindow,wd.left,wd.top,ColWidth,Height,TRUE);
 ::GetClientRect(Parent->HWindow,&rc);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
 MoveWindow(HWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
 SetBool(SSB_SHOWROWHEADERS,FALSE);
 SetMaxCols(iFieldNumber);
 SetMaxRows(dBase->recCount());
 Redraw();
 SetPosition();
}

void TListWindow::Seek(char *szSeek, int len)
{
 BOOL bFound = FALSE;
 Str4large szCompare;
 char szValue[11];
 SS_COORD lRow,lCol;
 GetLastValidCell(&lCol,&lRow);
 SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 SetBool(SSB_REDRAW,FALSE);
 for(int i=1; i <= lRow; i++)
  {
	 GetData(1,i,szValue);
	 szCompare.set(' ');
	 szCompare.replace(Str4ptr(szValue));
	 szCompare.setLen(len);
	 if(strcmp(StrTrim(szSeek),StrTrim(szCompare.str())) == 0)
	  {
		ShowCell(1,i,SS_SHOW_NEAREST);
		SetColor(SS_ALLCOLS,i,RGB(192,220,192), RGBCOLOR_BLUE);
		SetActiveCell(1,i);
		bFound = TRUE;
		break;
	  }
  }
 if(!bFound)
  iSeekLength = 0;
 SetBool(SSB_REDRAW,TRUE);
 SetFocus(HWindow);
}

void TListWindow::SetupWindow()
{
 TSpread::SetupWindow();
 iSeekLength = 0;
 iLeft = iTop = iRight = iBottom = 0;
 iLeft = GetPrivateProfileInt(szlpAWindow,"LEFT",0,PROFILE);
 iTop     = GetPrivateProfileInt(szlpAWindow,"TOP",0,PROFILE);
 iRight   = GetPrivateProfileInt(szlpAWindow,"RIGHT",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szlpAWindow,"BOTTOM",0,PROFILE);
 iField1 = GetPrivateProfileInt(szlpAWindow,"FIELD1",0,PROFILE);
 iField2 = GetPrivateProfileInt(szlpAWindow,"FIELD2",0,PROFILE);
 iField3 = GetPrivateProfileInt(szlpAWindow,"FIELD3",0,PROFILE);

 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 100;
 }

 GetPrivateProfileString(szlpAWindow,"QUERY","",szQuery,sizeof(szQuery),PROFILE);
 if(strlen(szQuery) == 0)
 {
	strcpy(szQuery,".NOT. DELETED()");
 }

 if(iField1 == 0 || iField2 == 0 || iField3 == 0)
 {
  iField1 =  1;
  iField2 =  2;
  iField3 =  3;
 }
}

void TListWindow::Redraw(void)
{
 unsigned int i = 1;
 dBase->SetIndex(szIdx);
 Relate4set *rQuery = new Relate4set(dBase);
 rQuery->querySet(szQuery);
 SetBool(SSB_REDRAW,FALSE);
 int bINI = GetPrivateProfileInt(szlpAWindow,"CUSTOM",0,PROFILE);
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
	 {
	  for(int j=1; j < iFieldNumber; j++)
		{
		 Field4 fField1(*(Data4 *)dBase,iField1);
		 Field4 fField2(*(Data4 *)dBase,iField2);
		 Field4 fField3(*(Data4 *)dBase,iField3);
		 SetData(1,i,fField1.str());
		 SetData(2,i,fField2.str());
		 SetData(3,i,fField3.str());
		}
	  i++;
	 }
 rQuery->unlock();
 rQuery->free();
 SetBool(SSB_REDRAW,TRUE);
 if(i>10)
  SetMaxRows(i);
 else
  SetMaxRows(10);
 return;


 /*
 SetupWindow();
 dBase->SetIndex(szIdx);
 SetBool(SSB_REDRAW,FALSE);
 SetMaxRows(dBase->recCount());
 unsigned int i = 1;
 Relate4set *rQuery = new Relate4set(dBase);
 rQuery->querySet(szQuery);

 ClearData(SS_ALLCOLS,SS_ALLROWS);
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
	{
	 for(int j=0; j < iFieldNumber; j++)
		{
		 int iFieldNum = j+1;
		 Field4 fField(*(Data4 *)dBase,iFieldNum);
		 SetData(iFieldNum,i,fField.str());
      }
	  i++;
    }     
 rQuery->unlock();
 rQuery->free();
 if(i>10)
  SetMaxRows(i);
 else
  SetMaxRows(10);

 SetBool(SSB_REDRAW,TRUE);
 return;
 */
}


void TPickList::SetListSize(int Rows)
{
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 short iHeight,iHeader;
 SSGetRowHeightInPixels(SHWindow,1,&iHeight);
 SSGetRowHeightInPixels(SHWindow,0,&iHeader); 
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ 2*GetSystemMetrics(SM_CXDLGFRAME) + ((Rows-1)*iHeight) + iHeader + 1 ;
 GetWindowRect(HWindow,&wd);
 GetClientRect(HWindow,&rc); 
 MoveWindow(HWindow,wd.left,wd.top,wd.right-wd.left,Height,TRUE);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
 MoveWindow(SHWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

/*
TGLAcctList::~TGLAcctList()
{
 delete ListArray;
}


void TGLAcctList::HandleListBox (RTMessage Msg)
{
 int nIndx;
 int nRecord = 0;
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 RArrayIterator ListIterator =(RArrayIterator)(ListArray->initIterator());

   while (int (ListIterator) != 0)
    {
      RObject AnObject = ListIterator++;
		nIndx = ((PTListItem)(&AnObject))->index;
      if( nIndx == Cell->Row)
       nRecord = ((PTListItem)(&AnObject))->rec;
    }
  if(nRecord != 0)
   gl->go(nRecord);
  SendMessage(GetDlgItem(Parent->HWindow,ID_ACCOUNTS) ,WM_SETTEXT,NULL,(LONG)(LPSTR)gl->accounts->str());
  if(strlen(gl->dept->str()) != 0)
  {
   SendMessage(GetDlgItem(Parent->HWindow,ID_DEPART)  ,WM_SETTEXT,NULL,(LONG)(LPSTR)gl->dept->str());
  }
  else
  SendMessage(GetDlgItem(Parent->HWindow,ID_DEPART)   ,WM_SETTEXT,NULL,(LONG)(LPSTR)"    ");
  SendMessage(GetDlgItem(Parent->HWindow,ID_JDESCRIPT),WM_SETTEXT,NULL,(LONG)(LPSTR)gl->descript->str());
  SendMessage(Parent->HWindow,WM_COMMAND,ID_ACCTLIST,NULL);
  SetFocus(GetDlgItem(Parent->HWindow,ID_DEBIT));
  Show(SW_HIDE);
}


void TGLAcctList::SetupWindow ()
{
 TDialog::SetupWindow();
 RECT rc;
 SS_CELLTYPE 	CellType;
 HourGlassOn();
 SetListSize(8);
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 GetClientRect(HWindow,&rc);  
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));            
 MoveWindow(SHWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 short d1Width,d2Width;
 SSSetColWidth(SHWindow,1,7);
 SSSetColWidth(SHWindow,2,5);
 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width); 
 short ColWidth = (rc.right - rc.left) - d1Width - d2Width - GetSystemMetrics(SM_CXHSCROLL);
 SSSetColWidthInPixels(SHWindow,3,ColWidth);
 
 gl->bottom();
 SSSetMaxCols(SHWindow, 3);
 SSSetMaxRows(SHWindow,gl->recCount());


 SSSetData(SHWindow,1,SS_HEADER, "Account");
 SSSetData(SHWindow,2,SS_HEADER, "Dept");
 SSSetData(SHWindow,3,SS_HEADER, "Description");
 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,SS_ALLCOLS, SS_ALLROWS, &CellType);
 GetPosition("ACCOUNTLIST");
 HourGlassOff();
}


void TGLAcctList::Redraw()
{
 HWND hWnd = GetDlgItem(HWindow,ID_DISTLIST);
 SSSetBool(hWnd,SSB_REDRAW,FALSE);
 SSClearData(hWnd,SS_ALLCOLS,SS_ALLROWS);
 long i=1;
 Relate4set *rQuery = new Relate4set(gl);
 rQuery->querySet("ACCOUNT <> ' '");
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
   {
   SSSetData(hWnd,1,i,gl->accounts->str());
   SSSetData(hWnd,2,i,gl->dept->str());
   SSSetData(hWnd,3,i,gl->descript->str());
   DWORD nRec_no = gl->recNo();
   ListArray->add(* (new TListItem((int)i,(int)nRec_no)));
   i++;
  }
 rQuery->unlock();
 rQuery->free();
 if(i> 12)
  SSSetMaxRows(hWnd,i);
 else
  SSSetMaxRows(hWnd,12);
 SSSetBool(hWnd,SSB_REDRAW,TRUE);
}
*/