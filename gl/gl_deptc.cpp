/*
   General Ledger  - (C) Copyright 1993 by Tegara Corporation
   Delete general ledger accounts.
*/

#include "tbowl.h"
#include "d4all.hpp"
#include "gl.h"
#include "glclass.h"
#include "db_dept.h"
#include "gl_main.h"

#include "glb.h"
#include "gl_dlg.h"

extern Code4 cb;
extern char szApp[20];


// Gets the fields where every the database pointers is.



_CLASSDEF(TGLDeptCodes)
class TGLDeptCodes : public TMainDialog
{
 public:
 PTgldept    gldept;
 TDeptList   *DEPT;
 BOOL bDeptList;
 TGLDeptCodes(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 TGLDeptCodes(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok            (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify	        (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Search 	     (RTMessage Msg) = [ID_FIRST + ID_GLINQ];
};



void TDeptList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TDeptList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 char *szBuffer1 = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(gldept->seek(szBuffer1) == 0)
  {
	SendMessage(GetDlgItem(Parent->HWindow,ID_DEPART),WM_SETTEXT,NULL,(LPARAM)(LPSTR)gldept->dept->str());
	SendMessage(GetDlgItem(Parent->HWindow,ID_DESCRIPT),WM_SETTEXT,NULL,(LPARAM)(LPSTR)gldept->descript->str());
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
	Show(SW_HIDE);
 delete [] szBuffer1;
 return;
}

void TDeptList::WMSize(RTMessage Msg)
{

	 // NEW
  TWindow::WMSize(Msg);
  int iRows = gldept->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);


}

void TDeptList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption(GetString(GetApplication()->hInstance,6092));
 List->SetFields("gldept","depart",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6068));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6054));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6044));
}


// Maintain Department codes.
void TGlWindow::GLDeptCodes (RTMessage)
{
 GetApplication()->ExecDialog(new TGLDeptCodes(this,1023));
}

void TGLDeptCodes::Search(RTMessage	)
{
	DEPT->Show(SW_SHOW);
	DEPT->List->Seek(ID_DEPART);
}

void TGLDeptCodes::SetupWindow()
{
 TMainDialog::SetupWindow();
 gldept  =  new Tgldept(&cb);
 DEPT = new TDeptList(this,"",gldept);
 DEPT->Attr.Style |= WS_THICKFRAME;
 DEPT->EnableAutoCreate();
 GetApplication()->MakeWindow(DEPT);
 bDeptList = TRUE;
}


void TGLDeptCodes::Ok (RTMessage)
{
  char szDept[8];
  char szDescript[41];
  GetDlgItemText(HWindow,ID_DEPART,szDept,sizeof(szDept));
   if(*szDept==0)
   {
	 MessageBox(HWindow,GetString(6049),szApp,MB_OK);
	 return;
	}
	GetDlgItemText(HWindow,ID_DESCRIPT,szDescript,sizeof(szDescript));
  if(*szDescript==0)
	{
	 MessageBox(HWindow,GetString(6050),szApp,MB_OK);
	 return;
	}
  if(gldept->seek(szDept)==0)
	{
	 int rc = MessageBox(HWindow,GetString(6051),szApp,MB_YESNO);
    if(rc == IDYES)
    {
     gldept->dept->assign(szDept);
	  gldept->descript->assign(szDescript);
	  gldept->UpdateUserTime(szUser);
     gldept->flush();
    }
	 return;
   }

  gldept->appendStart();
  gldept->dept->assign(szDept);
  gldept->descript->assign(szDescript);
  gldept->UpdateUserTime(szUser);
  gldept->append();

  SetDlgItemText(HWindow,ID_DEPART,(LPSTR) "");
  SetDlgItemText(HWindow,ID_DESCRIPT,(LPSTR) "");
  SendDlgItemMsg(ID_DEPART,EM_LIMITTEXT,6,0L);
  SendDlgItemMsg(ID_DESCRIPT,EM_LIMITTEXT,40,0L);
 return;
}

void TGLDeptCodes::Modify (RTMessage)
{
  char szDept[8];
  char szDescript[41];
  GetDlgItemText(HWindow,ID_DEPART,szDept,sizeof(szDept));
  if(*szDept==0)
   {

	 return;
	}
	GetDlgItemText(HWindow,ID_DESCRIPT,szDescript,sizeof(szDescript));
  if(*szDescript==0)
	{

	 return;
	}
  if(gldept->seek(szDept)==0)
	{

	 if(MessageBox(HWindow,GetString(6051),szApp,MB_YESNO) == IDYES)
	 {
	  gldept->dept->assign(szDept);
	  gldept->descript->assign(szDescript);
	  gldept->UpdateUserTime(szUser);
	  gldept->flush();
	  SetDlgItemText(HWindow,ID_DEPART,(LPSTR) "");
	  SetDlgItemText(HWindow,ID_DESCRIPT,(LPSTR) "");
	  SendDlgItemMsg(ID_DEPART,EM_LIMITTEXT,6,0L);
	  SendDlgItemMsg(ID_DESCRIPT,EM_LIMITTEXT,40,0L);
	  SendDlgItemMsg(ID_DEPART,CB_ADDSTRING,gldept->dept->len(),(LONG)(LPSTR)gldept->dept->str());
    }
    else
    return;
   }
}

void TGLDeptCodes::Delete  (RTMessage)
{
  char szDept[8];
  GetDlgItemText(HWindow,ID_DEPART,szDept,sizeof(szDept));
  if(strlen(StrTrim(szDept))==0)
	{
	 MessageBox(HWindow,GetString(6049),szApp,MB_OK);
	 return;
	}
  if(MessageBox(HWindow,GetString(6052),szApp,MB_YESNO)==IDYES)
	 {
	  gldept->deleteRec();
	  gldept->pack();
	  gldept->unlock();
	  SetDlgItemText(HWindow,ID_DEPART,(LPSTR) "");
	  SetDlgItemText(HWindow,ID_DESCRIPT,(LPSTR) "");
	  SetFocus(GetDlgItem(HWindow,IDOK));
	  DEPT->List->Redraw();
	 }
}

void TGLDeptCodes::Cancel  (RTMessage)
{
 DEPT->List->SavePosition();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
 }


