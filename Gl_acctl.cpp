
/*
	General Ledger  - (C) Copyright 1994 by Tegara Corporation
	Modify, add and delete accounts functions.

*/
#include "std_inc.h"
extern HINSTANCE hInst;

#define PROFILE "GL.INI"
#define ID_MEMORY_ERROR 11


typedef TDoubleListImp <string> CList;


#pragma argsused
void TListWindow::WMSize(RTMessage Msg)
{

}

BOOL TListWindow::CanClose()
{
 SavePosition();
 return bCanClose;
}

BOOL TListWindow::SetPosition()
{
 RECT rc,rcDesktop;
 GetWindowRect(GetDesktopWindow(),&rcDesktop);
 GetWindowRect(Parent->HWindow,&rc);
 if(iRight == 0 || iRight > rcDesktop.right)
	iRight = rc.right;
 if(iBottom == 0 || iBottom > rcDesktop.bottom)
 iBottom = rc.bottom;
 int check = iBottom - iTop;
 if(check < 0)
  {
	 iBottom = 600;
	 iTop = 200;
  }
 //return MoveWindow(Parent->HWindow,iLeft,iTop,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 return MoveWindow(Parent->HWindow,iLeft,iTop,iRight-iLeft,iBottom-iTop,TRUE);
}

TListWindow::~TListWindow()
{
  //SavePosition();
}

void TListWindow::Cancel(RTMessage)
{
  bCanClose = TRUE;
  if(CanClose())
  ShutDownWindow();
}

BOOL TListWindow::Seek(UINT ID)
{
  int len = (int) SendMessage(GetDlgItem(Parent->Parent->HWindow,ID),WM_GETTEXTLENGTH,NULL,NULL);
  char szStr[255];
  SendMessage(GetDlgItem(Parent->Parent->HWindow,ID),WM_GETTEXT,len+1,(LPARAM)(LPSTR)szStr);
  StrTrim(szStr);
  Str4large szText;
  szText.setLen((int)len);
  szText.set(' ');
  szText.replace(Str4ptr(szStr));
  szText.trim();
  len = szText.len();
  BOOL bFound = FALSE;
  SetBool(SSB_REDRAW,FALSE);
  char szValue[255];
  SS_COORD lRow,lCol;
  GetLastValidCell(&lCol,&lRow);
  SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
  if(len > 0)
	for(int i=1; i <= lRow; i++)
	{
	 GetData(1,i,szValue);
	 Str4large szCompare;
	 szCompare.set(' ');
	 szCompare.replace(Str4ptr(szValue));
	 szCompare.setLen(len);

	 if(strcmp(szText.ptr(),szCompare.ptr()) == 0)
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
 WritePrivateProfileString(szlpAWindow,"L",StrTrim(szLeft.str()),PROFILE);
 WritePrivateProfileString(szlpAWindow,"T",StrTrim(szTop.str()),PROFILE);
 WritePrivateProfileString(szlpAWindow,"R",StrTrim(szRight.str()),PROFILE);
 WritePrivateProfileString(szlpAWindow,"B",StrTrim(szBottom.str()),PROFILE);
 return TRUE;
}

_CLASSDEF (TSetQuery)
class TSetQuery : public TMainDialog
{
 public:
 Code4* cb;
 char* szWindow;
 char szQuery[128];
 char szTitle[128];
 char szNumber[3];
 char szFields[128];
 char szColumns[128];
 TSetQuery(PTWindowsObject AParent, int AName):TMainDialog(AParent,AName) {};
 virtual void SetupWindow();
 virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel(RTMessage Msg) = [ID_FIRST + IDCANCEL] {  ShutDownWindow(); }
};

void TSetQuery::Ok (RTMessage)
{

 Data4 dbf(cb,szWindow);
 strcpy(szQuery,GetField(101));
 if(strlen(szQuery) == 0)
	strcpy(szQuery,GetString(15)); // ".NOT. DELETED()"
 int returnCode = 1;
 Expr4 exp(dbf,szQuery);
 if(!exp.isValid())
		{
			string szMsg;
			szMsg = GetString(13);
			szMsg += szQuery;
			szMsg += GetString(14);
			MessageBox(HWindow,(LPSTR)szMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
			returnCode = exp.isValid();
		}

 exp.free();
 dbf.close();

 if(returnCode == 1)
 {
	((TListWindow *)Parent)->bRedraw = TRUE;
	WritePrivateProfileString(szWindow,GetString(16),GetField(101),PROFILE); // "QUERY"
	WritePrivateProfileString(szWindow,GetString(17),GetField(102),PROFILE);
	WritePrivateProfileString(szWindow,GetString(18),GetField(103),PROFILE);
	WritePrivateProfileString(szWindow,GetString(19),GetField(104),PROFILE);
	WritePrivateProfileString(szWindow,GetString(21),GetField(105),PROFILE);
  }
 ShutDownWindow();
}

void TSetQuery::SetupWindow()
{
 ((TListWindow *)Parent)->bRedraw = FALSE;
 GetPrivateProfileString(szWindow,GetString(16),"",szQuery,sizeof(szQuery),PROFILE);
 SetField(101,szQuery);
 GetPrivateProfileString(szWindow,GetString(17),"",szTitle,sizeof(szTitle),PROFILE);
 SetField(102,szTitle);
 GetPrivateProfileString(szWindow,GetString(18),"",szFields,sizeof(szFields),PROFILE);
 SetField(103,szFields);
 GetPrivateProfileString(szWindow,GetString(19),"",szNumber,sizeof(szNumber),PROFILE);
 SetField(104,szNumber);
 GetPrivateProfileString(szWindow,GetString(21),"",szColumns,sizeof(szColumns),PROFILE);
 SetField(105,szColumns);
}

void TListWindow::SetParamenters(RTMessage)
{
 TSetQuery* SetQuery = new TSetQuery(this,1035);
 //TSetQuery SetQuery(this,1035);
 SetQuery->szWindow = szlpAWindow;
 SetQuery->cb = cb;
 GetApplication()->ExecDialog(SetQuery);
 if(bRedraw == TRUE)
 {
	/*
	 Data4 dbf(cb,szlpAWindow);
	 GetPrivateProfileString(szlpAWindow,"QUERY","",szQuery,sizeof(szQuery),PROFILE);

	 if(strlen(szQuery) == 0)
			strcpy(szQuery,".NOT. DELETED()");

	 int returnCode = 1;
	 Expr4 exp(dbf,szQuery);
	 if(!exp.isValid())
		{
			string szMsg;
			szMsg = "The expression ";
			szMsg += szQuery;
			szMsg += " in the GL.INI file is invalid.";
			MessageBox(HWindow,(LPSTR)szMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
			returnCode = exp.isValid();
		}

	 exp.free();
	 dbf.close();
	 if(returnCode =! 0)
	*/
	Redraw();
 }
}


void TListWindow::GetKey(RTMessage Msg)
{
 switch(Msg.WParam)
	{
	 case  VK_DELETE : iSeekLength = 0;  break;
	 case  VK_ESCAPE :
		{
		  iSeekLength = 0;
		  SetBool(SSB_REDRAW,FALSE);
		  SetFocus(HWindow);
		  SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
		  SetActiveCell(1,1);
		  SetBool(SSB_REDRAW,TRUE);
		  break;
		}
	 case  VK_RETURN : break;
	 default:
	  {
		char szSeek[2];
		string strStorage;
		szSeek[0] = (char) Msg.WParam;
		szSeek[1] = 0;
		if(iSeekLength < 0)
		 iSeekLength = 0;
		if(iSeekLength > 0)
		 strStorage = strSeekBuffer;

		++iSeekLength;// Increment the seek buffer;

		if(iSeekLength > 1)
		 {
		  strSeekBuffer = strStorage;
		  strSeekBuffer += szSeek;
		  strSeekBuffer.resize(iSeekLength);
		 }
		else
		 {
		  strSeekBuffer = szSeek;
		  strSeekBuffer.resize(iSeekLength);
		 }

		//char szValue[11];
		int i = 1;
		SS_COORD lRow,lCol;
		GetLastValidCell(&lCol,&lRow);
		SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
		SetBool(SSB_REDRAW,FALSE);
		string strCompare;
		BOOL bFound = FALSE;
		string p;
		while(i <= lRow)
		 {
			GetData(1,i,(LPSTR)p.c_str());
			string strCompare((LPSTR)p.c_str(),0,iSeekLength);
			if(strCompare == strSeekBuffer)
			{
				ShowCell(1,i,SS_SHOW_NEAREST);
				SetColor(SS_ALLCOLS,i,RGB(192,220,192), RGBCOLOR_BLUE);
				SetActiveCell(1,i);
				bFound = TRUE;
				break;
			 }
		  i++;
		 }
	  if(!bFound)
	  {
		iSeekLength = 0;
		SetActiveCell(1,1);
	  }
	  SetBool(SSB_REDRAW,TRUE);
	  SetFocus(HWindow);
	  }
  }

}


void TListWindow::SetDesc()
{
  char strTitles[80];
  GetPrivateProfileString(szlpAWindow,GetString(GetApplication()->hInstance,17),"",strTitles,254,PROFILE);
  SetWindowText(Parent->HWindow,strTitles);

  char strHeaders[255];
  char token[]=",";
  char *p;
  GetPrivateProfileString(szlpAWindow,GetString(GetApplication()->hInstance,21),"",strHeaders,254,PROFILE);
  p=strHeaders;
  p = strtok(strHeaders,token); // Find the first token.

  CList LDescList;
  string strDesc;
  while(p != NULL)
  {
	 strDesc = p;
	 LDescList.AddAtTail(strDesc);
	 p = strtok(NULL,token); // Find the next token.
  }

  int i = 1;
  while( !LDescList.IsEmpty() )
  {
	 string Field = LDescList.PeekHead();
	 SetData(i,SS_HEADER,(LPSTR)Field.c_str());
	 LDescList.DetachAtHead();
	 i++;
  }
}

void TListWindow::SetFieldType(int iType,long nColumn)
{
	switch(iType)
		 {
			case r4num:    SetTypeStaticText(&CellType,SS_TEXT_RIGHT); break;
			case r4date:
			default:       SetTypeStaticText(&CellType,SS_TEXT_LEFT); break;
		 }
	 SetCellType(nColumn,SS_ALLROWS, &CellType);
}

BOOL TListWindow::SetFields()
{
 SetBool(SSB_REDRAW,FALSE);

 ClearData(SS_ALLCOLS,SS_ALLROWS);
 GetRowHeightInPixels(1,&iHeight);
 GetRowHeightInPixels(0,&iHeader);
 Rows = 7;
 Data4 dbf(((TAcctModule *)GetApplication())->GetCodeBase(),szlpAWindow);




 if(!dbf.isValid())
  {
	cb->closeAll();
	cb->init();
	ShutDownWindow();
	SetFocus(Parent->HWindow);
	return FALSE;
  }
 SetMaxRows(dbf.recCount());

 Tag4  tag;
 tag.initDefault(dbf);
 dbf.select(tag);

 long j = 0;
 char strFields[255];
 char token[]=",";
 char *p;
 iFieldNumber = GetPrivateProfileInt(szlpAWindow,GetString(GetApplication()->hInstance,19),0,PROFILE);
 GetPrivateProfileString(szlpAWindow,GetString(GetApplication()->hInstance,16),"",szQuery,sizeof(szQuery),PROFILE);
 if(strlen(szQuery) == 0)
 {
	strcpy(szQuery,GetString(GetApplication()->hInstance,15)); //".NOT. DELETED()"
 }
 /*
 int returnCode = 1;
 Expr4 exp(dbf,szQuery);
 if(!exp.isValid())
  {
	 string szMsg;
	 szMsg = "The expression ";
	 szMsg += szQuery;
	 szMsg += " in the GL.INI file is invalid.";
	 MessageBox(HWindow,(LPSTR)szMsg.c_str(),((TAcctModule *)GetString(GetApplication())->GetAppName(),MB_OK);
	 returnCode = exp.isValid();
  }

 exp.free();
 if(returnCode == 0)
  {
	dbf.close();
	return FALSE;
  }
 */
 Relate4set rQuery(dbf);
 rQuery.querySet(szQuery);

 long i = 0;
 if(iFieldNumber > 0)
 {
	GetPrivateProfileString(szlpAWindow,GetString(GetApplication()->hInstance,18),"",strFields,40,PROFILE);
	p=strFields;
	p = strtok(strFields,token); // Find the first token.
	CList iList;
	string szFields;
	i = 0;
	while(p != NULL && i < iFieldNumber)
	{
	 szFields = p;
	 iList.AddAtTail(szFields);
	 p = strtok(NULL,token); // Find the next token.
	 i++;
	}
	i = 1;

	while( !iList.IsEmpty() )
	{
		SetBool(SSB_REDRAW,FALSE);
		string Field = iList.PeekHead();
		Field4 fld(dbf,(LPSTR)Field.c_str());
		if(fld.isValid())
		{
			SetColWidth(i,fld.len());
			SetFieldType(fld.type(),i);
         j = 0;
			for(int qc = rQuery.top();qc!=r4eof;qc=rQuery.skip(1L) )
			{
				j++;
				SetData(i,j,StrTrim(fld.str()));
			}
			i++;
		}
		else
		{
		 dbf.close();
		 cb->init();
		 string strMsg(hInst,3091);
		 strMsg += szlpAWindow;
		 strMsg += "\nField : ";
		 strMsg += Field;
		 MessageBox(Parent->HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
		 ShutDownWindow();
		 dbf.close();
		 return FALSE;
		}
		iList.DetachAtHead();
		SetBool(SSB_REDRAW,FALSE);
	}
  SetDesc();

 }
 else
 {

	iFieldNumber = dbf.numFields();
	if(iFieldNumber > 3)
	 iFieldNumber = 3;

	i = 0;
	while(i < iFieldNumber)
	{
	 i++;
	 Field4 fld(dbf,(int)i);
	 SetData(i,SS_HEADER,(LPSTR)fld.str());
	 SetColWidth(i,fld.len());
	 j = 0;
	 for(int qc = rQuery.top();qc!=r4eof;qc=rQuery.skip(1L) )
		{
		 j++;
		 SetData(i,j,fld.str());
		}
	}
	if(dbf.numFields() < 3)
	 SetMaxCols(dbf.numFields());
	else
	 SetMaxCols(3);
 }
 // Clean up.

 rQuery.unlock();
 rQuery.free();
 dbf.close();

 ::GetClientRect(Parent->HWindow,&rc);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
 MoveWindow(HWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 SetBool(SSB_SHOWROWHEADERS,FALSE);
 SetMaxCols(iFieldNumber);
 SetMaxRows(j);
 // SetPosition(); Moved to the window setup...
 SetBool(SSB_REDRAW,TRUE);
 return TRUE;
}

#pragma argsused
BOOL TListWindow::SetFields(char *szdBase,char *szIndex, int iNumberOfFields)
{
 return TRUE;

}


void TListWindow::SetupWindow()
{
 TSpread::SetupWindow();
 cb = &((TAcctModule *)GetApplication())->cb;
 iSeekLength = 0;
 iLeft = iTop = iRight = iBottom = 0;
 iLeft    = GetPrivateProfileInt(szlpAWindow,"L",0,PROFILE);
 iTop     = GetPrivateProfileInt(szlpAWindow,"T",0,PROFILE);
 iRight   = GetPrivateProfileInt(szlpAWindow,"R",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szlpAWindow,"B",0,PROFILE);

 RECT rcDesktop;
 GetWindowRect(GetDesktopWindow(),&rcDesktop);
 if((iBottom - iTop)  > (rcDesktop.bottom - rcDesktop.top) ||  (iRight -iLeft) > (rcDesktop.right - rcDesktop.left))
	 {
	  iTop = 0;
	  iBottom = 0;
	  iLeft = 0;
	  iRight = 300;
	 }


 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 500;
 }

 GetPrivateProfileString(szlpAWindow,GetString(GetApplication()->hInstance,16),"",szQuery,sizeof(szQuery),PROFILE); // "QUERY"
 if(strlen(szQuery) == 0)
 {
	strcpy(szQuery,GetString(GetApplication()->hInstance,15));
 }

	 HFONT   	hFont;
	 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
	 SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
	 DeleteObject(hFont);
	 SetPosition();
	 //Redraw();

}

void TListWindow::Redraw(void)
{
 CHourGlass wait;
 SetFields();
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


