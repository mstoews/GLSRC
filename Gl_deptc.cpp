/*
	General Ledger  - (C) Copyright 1993 by Tegara Corporation
	Delete general ledger accounts.
*/
#include "std_inc.h"
#define ID_PARENT 1120
#define ID_CHILD  1121
#define ID_DEPARTP 1119


#include "db_dept.h"
/*
#include "tbowl.h"
#include "d4all.hpp"
#include "gl.h"
#include "glclass.h"

#include "gl_main.h"

#include "glb.h"
#include "gl_dlg.h"
*/




// Gets the fields where every the database pointers is.

_CLASSDEF(TGLDeptCodes)
class TGLDeptCodes : public TMainDialog
{
 public:
 string strParent;
 string strDept;
 string strDesc;
 string strType;
 

 Tgldept    gldept;
 TDeptList   *DEPT;
 BOOL bDeptList;
 TGLDeptCodes(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 TGLDeptCodes(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Cancel     (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok         (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Delete     (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify	    (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Search 	 (RTMessage Msg) = [ID_FIRST + ID_GLINQ];
 virtual void Update     (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
 void Clear();

};

void TGLDeptCodes::Update(RTMessage)
{
	gldept.SetIndex("PC");
	string strSeek;
	strSeek = (LPSTR)strParent.c_str();
	strSeek += (LPSTR)strDept.c_str();
	if(gldept.seek(strSeek.c_str()) == 0)
	{
		SetField(ID_DEPARTP,(LPSTR)strParent.c_str());
		SetField(ID_DEPART,(LPSTR)strDept.c_str());
		SetField(ID_DESCRIPT,gldept.descript.str());
		if(strcmp(gldept.type.str(),"P") == 0 )
			{
			 SendDlgItemMsg(ID_PARENT,BM_SETCHECK,NULL,TRUE);
			 SendDlgItemMsg(ID_CHILD,BM_SETCHECK,NULL,FALSE);
			}
		else
			{
			 SendDlgItemMsg(ID_PARENT,BM_SETCHECK,NULL,FALSE);
			 SendDlgItemMsg(ID_CHILD,BM_SETCHECK,NULL,TRUE);
			}
	}
	else
	MessageBox(HWindow,"Couldn't find the it",(LPCSTR)strSeek.c_str(),MB_OK);
}


void TDeptList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TDeptList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1,iColWidth2;
 List->GetColWidth(1,&iColWidth1);
 List->GetColWidth(2,&iColWidth2);

 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(LPSTR)((TGLDeptCodes *)Parent)->strParent.c_str());
 List->GetData(2,Cell->Row,(LPSTR)((TGLDeptCodes *)Parent)->strDept.c_str());
 List->GetData(3,Cell->Row,(LPSTR)((TGLDeptCodes *)Parent)->strDesc.c_str());


 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);

 SendMessage(GetDlgItem(Parent->HWindow,1119),WM_SETTEXT,NULL,
		(LPARAM)(LPSTR)((TGLDeptCodes *)Parent)->strParent.c_str());
 SendMessage(GetDlgItem(Parent->HWindow,ID_DEPART),WM_SETTEXT,NULL,
		(LPARAM)(LPSTR)((TGLDeptCodes *)Parent)->strDept.c_str());

 SendMessage(GetDlgItem(Parent->HWindow,ID_DESCRIPT),WM_SETTEXT,NULL,
		(LPARAM)(LPSTR)((TGLDeptCodes *)Parent)->strDesc.c_str());

 //SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);


 if(bHide)
	Show(SW_HIDE);
 return;
}

void TDeptList::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  int iRows = (int)lRows;
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
 SetCaption(GetString(GetApplication()->hInstance,6092));
 List->SetFields();
}


// Maintain Department codes.
void TGlWindow::GLDeptCodes (RTMessage)
{
 GetApplication()->ExecDialog(new TGLDeptCodes(this,1023));
}

void TGLDeptCodes::Search(RTMessage	)
{
	DEPT->Show(SW_SHOW);
}

void TGLDeptCodes::SetupWindow()
{
 TMainDialog::SetupWindow();

 if(gldept.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	 gldept.close();
	 cb->init();
	 ShutDownWindow();
	 SetFocus(Parent->HWindow);
	 return;
  }

 gldept.init();
 DEPT = new TDeptList(this,"");
 DEPT->Attr.Style |= WS_THICKFRAME;
 DEPT->EnableAutoCreate();
 GetApplication()->MakeWindow(DEPT);
 bDeptList = TRUE;

}


void TGLDeptCodes::Ok (RTMessage Msg)
{
  CHourGlass wait;
  //size_t
  strParent = GetField(ID_DEPARTP);
  strDept   = GetField(ID_DEPART);
  strDesc   = GetField(ID_DESCRIPT);
  //gldept.SetIndex("DEPART");

  if(strlen(StrTrim((LPSTR)strParent.c_str())) == 0)
	{
	 MessageBox(HWindow,GetString(6049),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 SetFocus(GetDlgItem(HWindow,ID_DEPARTP));
	 return;
	}

  if(strlen(StrTrim((LPSTR)strDept.c_str())) == 0)
	{
	 MessageBox(HWindow,GetString(6049),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 SetFocus(GetDlgItem(HWindow,ID_DEPART));
	 return;
	}

  if(strlen(StrTrim((LPSTR)strDesc.c_str())) == 0)
	{
	 MessageBox(HWindow,GetString(6050),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
	 return;
	}

  strParent = GetField(ID_DEPARTP);
  strDept   = GetField(ID_DEPART);

  string szSeek;
  szSeek =  (LPSTR)strParent.c_str();
  szSeek +=  (LPSTR)strDept.c_str();


  if(gldept.seek((LPSTR)szSeek.c_str())!=0)
	{
	 gldept.appendStart();
	 gldept.deptp = strParent;
	 gldept.dept = strDept;
	 gldept.descript = strDesc;
	 gldept.UpdateUserTime(szUser);
	 gldept.append();
	}
	else
	Modify(Msg);
	Clear();
	return;
}

void TGLDeptCodes::Modify (RTMessage)
{
	 if(MessageBox(HWindow,GetString(6051),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO) == IDYES)
	 {
	  gldept.deptp = strParent;
	  gldept.dept = strDept;
	  gldept.descript = strDesc;
	  gldept.UpdateUserTime(szUser);
	  gldept.flush();
	  Clear();
	 }

}

void TGLDeptCodes::Delete  (RTMessage)
{
  strParent = GetField(ID_DEPARTP);
  strDept   = GetField(ID_DEPART);
  string strSeek;
  strSeek = (LPSTR)strParent.c_str();
  strSeek += (LPSTR)strDept.c_str();
  if(gldept.seek((LPSTR)strSeek.c_str()) == 0)
  {
	if(MessageBox(HWindow,GetString(6052),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO)==IDYES)
	 {
	  gldept.deleteRec();
	  gldept.pack();
	  gldept.unlock();
	  Clear();
	 }
  }
}

void TGLDeptCodes::Clear()
{
	  SetField(ID_DEPARTP,"");
	  SetField(ID_DEPART,"");
	  SetField(ID_DESCRIPT,"");
	  DEPT->List->SetFields();
	  SetFocus(GetDlgItem(HWindow,ID_DEPARTP));
}

void TGLDeptCodes::Cancel  (RTMessage)
{
  DEPT->List->SavePosition();
  gldept.close();
  ShutDownWindow();
 }


