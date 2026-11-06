
/*
   Accounts Receivable - (C) Copyright 1994 by Tegara Corporation
   Modify, add and delete accounts functions.
*/
#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "gl_dlg.h"
#include "gl_acct.h"
#include "gl_acctl.h"
#include "ar_custl.h"
#include "ar_btchl.h"
#define PROFILE "GL.INI"



/*
VOID     FAR cdecl Debug             (LPSTR,...);
void DebugW(LPSTR,...);
void DebugW(LPSTR,...)
{
 #ifdef DEBUG
  Debug(
}
*/

extern Code4 cb;
void TListWindow::WMSize(TMessage& Msg)
{
}

BOOL TListWindow::Seek(UINT ID)
{ 
  DWORD len =  SendMessage(GetDlgItem(Parent->Parent->HWindow,ID),WM_GETTEXTLENGTH,NULL,NULL);
  char *szStr = new char [(int)len+1];
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
    szCompare->setLen((int)len);

    if(strcmp(szText->ptr(),szCompare->ptr()) == 0)
     {
      SetColor(SS_ALLCOLS,i,RGB(192,220,192), RGBCOLOR_BLUE);
      ShowCell(1,i,SS_SHOW_NEAREST);
      SetActiveCell(1,i);      
      bFound = TRUE;
      break;
     }
    delete [] szCompare;
   }
 delete [] szText;

 SetFocus(HWindow);
 if(!bFound)
  SetActiveCell(1,1);
 return bFound;

}

void TListWindow::GetKey(RTMessage Msg)
{
 return;
}

BOOL TListWindow::SetPosition()
{
 RECT rc;
 GetWindowRect(Parent->HWindow,&rc);
 return MoveWindow(Parent->HWindow,iLeft,iTop,rc.right-rc.left,rc.bottom-rc.top,TRUE);
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

BOOL TListWindow::SavePosition()
{
 RECT rc;
 GetWindowRect(Parent->HWindow,&rc);
 Str4ten szLeft;
 Str4ten szTop;
 szLeft.assignLong(rc.left,5);
 szTop.assignLong(rc.top,5);
 WritePrivateProfileString(szlpAWindow,"LEFT",szLeft.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"TOP",szTop.str(),PROFILE);
 return TRUE;
}

void TListWindow::SetParamenters(RTMessage)
{
 GetApplication()->ExecDialog(new TDialog(this,1035));
}

void TListWindow::SetFields(char *szdBase,char *szIndex, int iNumberOfFields)
{ 
 iFieldNumber = iNumberOfFields;
 SS_CELLTYPE 	CellType;
 GetRowHeightInPixels(1,&iHeight);
 GetRowHeightInPixels(0,&iHeader);
 Rows = 7;
 Data4 DBF(cb,szdBase);
 DBF.SetIndex(szIndex);
 strcpy(szIdx,szIndex);
  
 //dBase = new Data4(&cb,szdBase);
 //dBase->SetIndex(szIndex);
 
 // First open the database and set the size of the windows based upon the
 // first three fields length.
 if(iFieldNumber == 0)
  iFieldNumber = DBF.numFields()+1;
 else
  iFieldNumber++;

 for (int j=1; j<iFieldNumber; j++)
  {
	 Field4 *fField = new Field4 (DBF,j);
	 SetData(j,0,fField->name());
	 SetColWidth(j,fField->len());
  }
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ ((Rows)*iHeight) + iHeader ;
 GetWindowRect(Parent->HWindow,&wd);

 Field4 *Field1 = new Field4(DBF,iField1);
 SetColWidth(1,Field1->len());
 Field4 *Field2= new Field4(DBF,iField2);
 SetColWidth(2,Field2->len());
 Field4 *Field3= new Field4(DBF,iField3);
 SetColWidth(3,Field3->len());

 short iWidth1,iWidth2,iWidth3,ColWidth;
 GetColWidthInPixels(iField1,(LPSHORT)&iWidth1);
 GetColWidthInPixels(iField2,(LPSHORT)&iWidth2);
 GetColWidthInPixels(iField3,(LPSHORT)&iWidth3);
 ColWidth = iWidth1+iWidth2+iWidth3;
 MoveWindow(Parent->HWindow,wd.left,wd.top,ColWidth,Height,TRUE);
 ::GetClientRect(Parent->HWindow,&rc);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER)-1,GetSystemMetrics(SM_CYBORDER));
 MoveWindow(HWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
 SetBool(SSB_SHOWROWHEADERS,FALSE);
 SetMaxCols(iFieldNumber);
 SetMaxRows(DBF.recCount());
 DBF.close();
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
 SetFocus(HWindow);
 if(!bFound)
  SetActiveCell(1,1);
}

void TListWindow::SetupWindow()
{
 TSpread::SetupWindow();
 iLeft = iTop = 0;
 iLeft = GetPrivateProfileInt(szlpAWindow,"LEFT",0,PROFILE);
 iTop  = GetPrivateProfileInt(szlpAWindow,"TOP",0,PROFILE);
 iField1 = GetPrivateProfileInt(szlpAWindow,"FIELD1",0,PROFILE);
 iField2 = GetPrivateProfileInt(szlpAWindow,"FIELD2",0,PROFILE);
 iField3 = GetPrivateProfileInt(szlpAWindow,"FIELD3",0,PROFILE);

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

void TListWindow::Redraw()
{
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
}

void TListWindow::Redraw(char *szDBF)
{
 unsigned int i = 1;
 Data4 DBF(cb,szDBF);
 Relate4set *rQuery = new Relate4set(DBF);
 rQuery->querySet(szQuery);
 SetBool(SSB_REDRAW,FALSE);
 int bINI = GetPrivateProfileInt(szlpAWindow,"CUSTOM",0,PROFILE);
 if(!bINI)
 {
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
	{
	  for(int j=1; j < iFieldNumber; j++)
		{
	Field4 fField(DBF,j);
	SetData(j,i,fField.str());
		}
	  i++;
	 }
 }
 else
 {
  for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
	{
	  for(int j=1; j < iFieldNumber; j++)
		{
		 Field4 fField1(DBF,iField1);
		 Field4 fField2(DBF,iField2);
		 Field4 fField3(DBF,iField3);
		 SetData(1,i,fField1.str());
		 SetData(2,i,fField2.str());
		 SetData(3,i,fField3.str());
	  }
	 i++;
  }
 }
 rQuery->unlock();
 rQuery->free();
 DBF.close();
 SetBool(SSB_REDRAW,TRUE);
 if(i>10)
  SetMaxRows(i);
 else
  SetMaxRows(10);
 return;
}


void TAccountList::SetupWindow()
{ 
 TWindow::SetupWindow(); 
 SetWndDetail();
}

void TAccountList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("General Ledger Accounts List"); 
 List->SetFields("glaccts","accounts",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Account");
 List->SetData(2,SS_HEADER, "Dept.");
 List->SetData(3,SS_HEADER, "Description");
 List->Redraw(GetClassName());
}

void TAccountList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1,iColWidth2;
 List->GetColWidth(1,&iColWidth1);
 List->GetColWidth(2,&iColWidth2);
 char *szBuffer1 = new char [iColWidth1+1];
 char *szBuffer2 = new char [iColWidth2+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);
 List->GetData(2,Cell->Row,szBuffer2);
 char *szSeek = new char [iColWidth1+iColWidth2+1];
 glaccts->SetIndex("accounts");
 wsprintf(szSeek,"%s%s",szBuffer1,szBuffer2);
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(glaccts->seek(szSeek) == 0)
  {
   SendMessage(GetDlgItem(Parent->HWindow,ID_JDESCRIPT),WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->descript->str());
   SendMessage(GetDlgItem(Parent->HWindow,ID_ACCOUNTS),WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->accounts->str());
   SendMessage(GetDlgItem(Parent->HWindow,ID_DEPART),WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->dept->str());
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
   Show(SW_HIDE);
 delete [] szBuffer1;
 delete [] szBuffer2;
 delete [] szSeek;
 return;
}

void TCustomerList::SetupWindow()
{ 
 TWindow::SetupWindow();
 SetWndDetail();
}

void TCustomerList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("Accounts Receivable Customers List");
 
 List->SetFields("ar_cust","vendorno",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Cust No.");
 List->SetData(2,SS_HEADER, "Description");
 List->SetData(3,SS_HEADER, "Name");
 List->Redraw(GetClassName());
}


void TCustomerList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1); 
 char *szVendorCode = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szVendorCode);
 arcust->SetIndex("vendorno");
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(arcust->seek(szVendorCode) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
   Show(SW_HIDE);
 delete [] szVendorCode;
 return;
}


void TBatchList::SetupWindow()
{ 
 TWindow::SetupWindow();
 SetWndDetail();
}

void TBatchList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("Accounts Receivable Batch List");
 
 List->SetFields("ar_btch","batchno");
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Batch No.");
 List->SetData(2,SS_HEADER, " ");
 List->SetData(3,SS_HEADER, "Description");
 List->Redraw(GetClassName());
}

void TBatchList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1); 
 char *szBatchno = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBatchno);
 ar_btch->SetIndex("batchno");
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(ar_btch->seek(szBatchno) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
   Show(SW_HIDE);
 delete [] szBatchno;
 return;
}

