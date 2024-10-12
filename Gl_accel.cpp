
#include "std_inc.h"
#define ID_UPDATEWINDOW 1845
#define ID_RESET	      1094
#define ID_RECNO		   1173



extern HINSTANCE hInst;

void TAccessList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TAccessList::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  int iRows = (int)Access->recCount();

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

void TAccessList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption(GetString(hInst,6099));
 List->SetFields("access.psw","u",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(hInst,6100));
 List->SetData(2,SS_HEADER, GetString(hInst,6101));
 List->SetData(3,SS_HEADER, GetString(hInst,6102));
}


void TAccessList::HandleListBox( RTMessage Msg)
{

 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 string *szCode = new string;
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(char *)szCode->c_str());
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

 if(Access->seek(szCode))
  {
	SendMessage(Parent->HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

 Show(SW_HIDE);
 delete szCode;
 return;
}

void TAccessLevel::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TAccessLevel::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  int iRows = (int)Accessl->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  List->SetMaxCols(2);
  RECT rc;
  short d1Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  short ColWidth = ((rc.right - rc.left) - d1Width) ;
  List->SetColWidthInPixels(2,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);

}

void TAccessLevel::SetWndDetail()
{
 RECT rc;
 short d1Width;
 SetCaption(GetString(hInst,6103));
// List->SetFields("accessl.psw","l",3);
 List->SetFields();
 List->SetMaxCols(3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width );
 List->SetColWidthInPixels(2,ColWidth);
 List->SetData(1,SS_HEADER, GetString(hInst,6104));
 List->SetData(2,SS_HEADER, GetString(hInst,6102));
}


void TAccessLevel::HandleListBox( RTMessage Msg)
{

 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 string *szCode = new string;
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(char *)szCode->c_str());
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

 if(Accessl->seek(szCode))
  {
	SendMessage(Parent->HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

 Show(SW_HIDE);
 delete szCode;
 return;
}


