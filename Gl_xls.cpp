//#include "std_inc.h"



#include <owl.h>
#include <commdlg.h>
#include <dir.h>
#include <direct.h>
#include "vtss.h"
#include "vtssmgr.h"
#include "gl.h"
#include "glclass.h"
#include "gl_main.h"

#include "string.h"
#include "gl_tooli.h"
#include "gl_tool.h"
#include "utildlg.h"

#define ID_ACCOUNT_SQL 1109
#define IDS_FILTERSTRING 6153

static char FileName[MAXPATH] = "\0";
extern  char PathName[128] = "\0";
extern char   CurDir[MAXPATH];
extern UINT drive;




void CALLBACK _export OnEndEdit	   (HSS, HWND, UINT);
void CALLBACK _export OnStartEdit	(HSS, HWND, UINT);
void CALLBACK _export OnRightClick  (HSS	hSS,HWND hWnd,UINT event);
void CALLBACK _export OnDblClick    (HSS, HWND, UINT);


extern HMENU HGLMenu,HGLExMenu;

extern HINSTANCE hInst;

_CLASSDEF(TSetAccountSQL)
class TSetAccountSQL : public TDialog
{
  public:
  string strFormula;
  void SetInfo(LPSTR szValue);
	TSetAccountSQL(PTWindowsObject AParent, int ATitle):TDialog(AParent,ATitle){};
	virtual void SetupWindow()
	{
	  TDialog::SetupWindow();
	  SendDlgItemMsg(4015,WM_SETTEXT,NULL,(LPARAM)(LPSTR)strFormula.c_str());
	}
};


void TSetAccountSQL::SetInfo(LPSTR szValue)
{
	 strFormula = szValue;
}


_CLASSDEF(TWnd)
class TWnd: public TWindow
{
 public:
 string strFilename;
 BOOL	bModified;
 COLORREF crColor;
 LOGFONT lfTemp;
 CHOOSEFONT cfTemp;
 HFONT hfFont;
 BOOL tfFontLoaded;
 HSS hSS;
 HSSEDIT hSSEdit;
 int	nFileType;
 SSERROR sErr;
 char	*szUntitled;
 BOOL   bRecalc;
 void DisplayError(HWND	hwnd,SSERROR nError);
 TWnd(PTWindowsObject AParent, LPSTR ATitle):TWindow(AParent,ATitle)
 {
	hSS = NULL;
 };
 virtual void SetupWindow();
 virtual void WMSize         (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void DefineName     (RTMessage Msg) = [CM_FIRST + ID_RANGE];
 virtual void Print	        (RTMessage Msg) = [CM_FIRST + ID_PRINT];
 virtual void SetPrintArea   (RTMessage Msg) = [CM_FIRST + ID_SETAREA];
 virtual void SaveSheet      (RTMessage Msg) = [CM_FIRST + ID_SAVE];
 virtual void Save	        (RTMessage Msg) = [CM_FIRST + ID_SAVEAS];
 virtual void Font	        (RTMessage Msg) = [CM_FIRST + ID_FONT];
 virtual void Border 	     (RTMessage Msg) = [CM_FIRST + ID_BORDER];
 virtual void FormatNumber   (RTMessage Msg) = [CM_FIRST + ID_NUMBER];
 virtual void FormatPattern  (RTMessage Msg) = [CM_FIRST + ID_PATTERN];
 virtual void PageSetup      (RTMessage Msg) = [CM_FIRST + ID_PAGESETUP];
 virtual void SetRecalculate (RTMessage Msg) = [CM_FIRST + ID_RECALC];
 virtual void Copy           (RTMessage Msg) = [CM_FIRST + ID_COPY];
 virtual void Paste          (RTMessage Msg) = [CM_FIRST + ID_PASTE];
 virtual void Cut            (RTMessage Msg) = [CM_FIRST + ID_CUT];
 virtual void Clear          (RTMessage Msg) = [CM_FIRST + ID_CLEAR];
 virtual void SetAlignment   (RTMessage Msg) = [CM_FIRST + ID_ALIGNMENT];
 virtual void SetCalculation (RTMessage Msg) = [CM_FIRST + ID_CALC];
 virtual void SetRowHeight   (RTMessage Msg) = [CM_FIRST + ID_ROWHEIGHT];
 virtual void SetColWidth    (RTMessage Msg) = [CM_FIRST + ID_COLWIDTH];
 virtual void SetColor       (RTMessage Msg) = [CM_FIRST + ID_COLOR];
 virtual void Goto 	        (RTMessage Msg) = [CM_FIRST + ID_GOTO];
 virtual void AddPageBreak   (RTMessage Msg) = [CM_FIRST + ID_ADDBREAK];
 virtual void RemovePageBreak(RTMessage Msg) = [CM_FIRST + ID_REMOVEBREAK];
 virtual void SetPageTitles  (RTMessage Msg) = [CM_FIRST + ID_PAGETITLES];
 virtual void SetGridsOnOff  (RTMessage Msg) = [CM_FIRST + ID_GRIDS];
 virtual void SetAccountSQL  (RTMessage Msg) = [CM_FIRST + ID_ACCOUNT_SQL];
 virtual void WMMDIActivate  (TMessage& Msg);
 virtual void GetWindowClass (WNDCLASS& wc);
 virtual BOOL CanClose ();
 SSERROR Save(LPINT pFileType,LPSTR pFileName);
 void ShowButton(int ID, BOOL bShow);
 void ShowBar(BOOL bShow);
};



void TWnd::SetAlignment    (RTMessage ){SSFormatAlignmentDlg(hSS);}
void TWnd::SetCalculation  (RTMessage ){SSCalculationDlg(hSS);}
void TWnd::SetRowHeight    (RTMessage ){SSRowHeightDlg(hSS);}
void TWnd::SetColWidth     (RTMessage ){SSColWidthDlg(hSS);}
void TWnd::SetColor        (RTMessage ){SSColorPaletteDlg(hSS);}
void TWnd::Goto 	         (RTMessage ){SSGotoDlg(hSS);}
void TWnd::Cut		         (RTMessage ){SSEditCut(hSS);}
void TWnd::Copy		      (RTMessage ){SSEditCopy(hSS);}
void TWnd::Paste	         (RTMessage ){SSEditPaste(hSS);}
void TWnd::DefineName      (RTMessage ){SSDefinedNameDlg(hSS);}
void TWnd::SetPageTitles   (RTMessage ){SSSetPrintTitlesFromSelection(hSS);}
void TWnd::AddPageBreak    (RTMessage ){SSAddPageBreak(hSS);}
void TWnd::RemovePageBreak (RTMessage ){SSRemovePageBreak(hSS);}
void TWnd::Border          (RTMessage ){SSFormatBorderDlg(hSS);}
void TWnd::FormatNumber    (RTMessage ){SSFormatNumberDlg(hSS);};
void TWnd::FormatPattern   (RTMessage ){SSFormatPatternDlg(hSS);};
void TWnd::PageSetup       (RTMessage ){SSFilePageSetupDlg(hSS);};

void TWnd::SetGridsOnOff   (RTMessage )
{
 BOOL bGridLines;
 sErr = SSGetShowGridLines(hSS,&bGridLines);
 if(bGridLines)
 SSSetShowGridLines(hSS,FALSE);
  else
 SSSetShowGridLines(hSS,TRUE);
}
void TWnd::Clear (RTMessage){SSEditClear(hSS,0);}

void TWnd::SetAccountSQL(RTMessage )
{
	TSetAccountSQL* SetAccountSQL = new TSetAccountSQL(this,1009);
	SetAccountSQL->SetInfo("");
	GetApplication()->ExecDialog(SetAccountSQL);
}


void TWnd::SetupWindow()
{
 TWindow::SetupWindow();
 hSS = 0;
 nFileType = 2;
 szUntitled = strcpy(new char [11],GetString(hInst,4027));
 bModified = TRUE;
 CheckMenuItem(HGLExMenu,ID_RECALC,MF_CHECKED);
 SSSetAutoRecalc(hSS,TRUE);
 SSSetAllowEditHeaders(hSS,TRUE);
 SSSetShowVScrollBar(hSS,2);
 SSSetShowHScrollBar(hSS,2);
 SetFocus(HWindow);
}



void TWnd::SetRecalculate (RTMessage)
{
 if(GetMenuState(HGLExMenu,ID_RECALC,MF_BYCOMMAND) == MF_CHECKED)
 {
  CheckMenuItem(HGLExMenu,ID_RECALC,MF_UNCHECKED);
  SSSetAutoRecalc(hSS,FALSE);
 }
 else
 {
  CheckMenuItem(HGLExMenu,ID_RECALC,MF_CHECKED);
  SSSetAutoRecalc(hSS,TRUE);
 }
}


void TWnd::Save (RTMessage)
{

	if ( (sErr = SSSaveFileDlg(hSS, NULL,FileName, _MAX_PATH - 1, &nFileType)) != SSERROR_NONE )
		{
		 DisplayError(HWindow,sErr);
		}
	else
		{
		 SetCaption(FileName);
		}

// Save the view window information along with the worksheet information
if ( (sErr = SSSaveWindowInfo(hSS)) == SSERROR_NONE && (sErr = SSWrite(hSS,FileName, nFileType)) == SSERROR_NONE )
	bModified = FALSE;
 _chdrive(drive);
 chdir(CurDir);
}

void TWnd::SaveSheet(RTMessage)
{
 string strCaption;
 GetWindowText(HWindow,(LPSTR)strCaption.c_str(),strCaption.get_initial_capacity());
 sErr = Save(&nFileType,(LPSTR)strCaption.c_str());
 bModified = FALSE;
 return;
}



void TWnd::WMSize (RTMessage Msg)
{
 TWindow::WMSize(Msg);
 SSWndMoveView(hSS,0,20,LOWORD(Msg.LParam), HIWORD(Msg.LParam)-20);
 if(hSS != NULL)
	 SSEditBarMove(hSSEdit,0,0,LOWORD(Msg.LParam),20);
}

void TWnd::Print(RTMessage)
{
 SSFilePrint(hSS, TRUE);
 return;
}

void TWnd::SetPrintArea (RTMessage)
{
 if (( sErr = SSSetPrintAreaFromSelection(hSS)) != SSERROR_NONE)
	 DisplayError(HWindow, sErr);
	return;

}





void TWnd::DisplayError(HWND	hwnd,SSERROR 	nError)
{
 // Convert SSERROR to text
 if ( SSErrorNumberToText(nError, FileName,sizeof(FileName)) )
	strcpy(FileName,GetString(hInst,1078));
 MessageBox(hwnd,FileName,((TAcctModule *)GetApplication())->GetAppName(), MB_ICONEXCLAMATION | MB_OK);
}


void TWnd::WMMDIActivate(TMessage& Msg)
{

  TWindow::WMMDIActivate(Msg);
  DefWndProc(Msg);
  if ( Parent )
  {
	// ((PTGlWindow)Parent)->Wnd= (Msg.WParam ? this : NULL);
  }
  if(Msg.WParam == TRUE)
  {
	 HMENU HPopUpMenu = GetSubMenu(HGLExMenu,0); // Get the child menu position.
	 SendMessage(((PTGlWindow)Parent)->ClientWnd->HWindow,WM_MDISETMENU,0,MAKELONG(HGLExMenu,HPopUpMenu));
	 DrawMenuBar(GetApplication()->MainWindow->HWindow);
	 ShowBar(TRUE);
  }
  else if (Msg.WParam == NULL)
  {
	 HMENU HPopUpMenu = GetSubMenu(HGLMenu,0); // Get the child menu position.
	 SendMessage(((PTGlWindow)Parent)->ClientWnd->HWindow,WM_MDISETMENU,0,MAKELONG(HGLMenu,HPopUpMenu));
	 DrawMenuBar(GetApplication()->MainWindow->HWindow);
	 ShowBar(FALSE);
  }
  if ( !IsFlagSet(WB_ALIAS) )
  {
	 ActivationResponse(Msg.WParam, IsIconic(HWindow));
  }

}

void TWnd::ShowButton(int ID, BOOL bShow)
{
 if(bShow)
 ShowWindow(GetDlgItem((((PTGlWindow)Parent)->ToolBar->HWindow),ID),SW_SHOW);
 else
 ShowWindow(GetDlgItem((((PTGlWindow)Parent)->ToolBar->HWindow),ID),SW_HIDE);
}

void TWnd::ShowBar(BOOL bShow)
{
  BOOL bGLBAR;
  ShowButton(ID_BORDER,bShow);
  ShowButton(ID_FONT,bShow);
  ShowButton(ID_ALIGNMENT,bShow);
  ShowButton(ID_NUMBER,bShow);
  ShowButton(ID_GRIDS,bShow);
  ShowButton(ID_CUT,bShow);
  ShowButton(ID_COPY,bShow);
  ShowButton(ID_PASTE,bShow);
  if(bShow)
	bGLBAR = FALSE;
  else
	bGLBAR = TRUE;
  ShowButton(ID_APBATCH,bGLBAR);
  ShowButton(ID_CASH,bGLBAR);
  ShowButton(ID_ARBATCHENT,bGLBAR);
  ShowButton(ID_GLBATCH,bGLBAR);
  //ShowButton(ID_GLINQ,  bGLBAR);
  //ShowButton(ID_GLPOST, bGLBAR);
  //ShowButton(ID_GLADD,  bGLBAR);
  ShowButton(ID_CANCEL, bGLBAR);

}

void TWnd::Font (RTMessage){ SSFormatFontDlg(hSS);}

BOOL TWnd::CanClose ()
{

	// If the work sheet has been modified we should check to see if it should be saved.
	// If it should be saved does it have a correct name.
	// If the name is correct write to the disk and quit.
	// If the name is incorrect or there is no name open the save dialog box.
	// If an error occurs don't quit.
 string strCaption;
 GetWindowText(HWindow,(LPSTR)strCaption.c_str(),strCaption.get_initial_capacity());

 sErr = SSCheckModified(hSS);
 BOOL bCanClose = FALSE;
 if(sErr ==  0)
  switch(MessageBox(HWindow,GetString(hInst,1076),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNOCANCEL))
	{
	 case IDYES :{
				 if(Save(&nFileType,(LPSTR)strCaption.c_str()) == SSERROR_NONE){
						bCanClose = TRUE;
						break;
				 }
				 else{
						bCanClose = FALSE;
						break;
				 }
				}
	case IDCANCEL : bCanClose = FALSE; break;
	case IDNO : bCanClose = TRUE; break;
	default : break;
 }
 return bCanClose;
}

SSERROR TWnd::Save(LPINT pFileType,LPSTR pFileName)
{
char buff[_MAX_PATH];
// If we don't have a name or the name is (Untitled) then
// we will need to get a worksheet name.
if ( ! *pFileName || ! lstrcmp(pFileName, szUntitled) )
	{
	if ( (sErr = SSSaveFileDlg(hSS, NULL, buff, _MAX_PATH - 1, pFileType)) !=SSERROR_NONE )
		return sErr;
	else
		{
		lstrcpy(pFileName, buff);
		SetCaption(pFileName);
		}
	}

// Save the view window information along with the worksheet information
//if ( (sErr = SSSaveWindowInfo(hSS)) == SSERROR_NONE &&
sErr = SSWrite(hSS, pFileName, *pFileType);
if(sErr == SSERROR_NONE )
	bModified = FALSE;
return sErr;
}



void TWnd::GetWindowClass(WNDCLASS& wc)
{
 TWindow::GetWindowClass(wc);
 wc.hIcon=LoadIcon(wc.hInstance, "SHEET");
}

void CALLBACK _export OnEndEdit(
	HSS	hSS,
	HWND	hWnd,
	UINT	event)
{
 UNUSED(hSS);
 UNUSED(hWnd);
 UNUSED(event);

}

void CALLBACK _export OnStartEdit(
	HSS	hSS,
	HWND	hWnd,
	UINT	event)
{
UNUSED(hSS);
UNUSED(hWnd);
UNUSED(event);
//MessageBox(hWnd,"THIS IS A WINDOW","",MB_OK);
//bModified = TRUE;
}



void CALLBACK _export OnRightClick(HSS	hSS,HWND hWnd,UINT event)
{
 UNUSED(hWnd);
 UNUSED(event);
 POINT ptCurrent;
 HWND hwnd = SSWndGetWindow(hSS);
 GetCursorPos(&ptCurrent);
 //ClientToScreen(hwnd,&ptCurrent);
 HMENU hMenu = LoadMenu(hInst,"SPREAD");
 hMenu = GetSubMenu(hMenu,0);
 TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON ,ptCurrent.x,ptCurrent.y,0,GetParent(hwnd),NULL);
 DestroyMenu(hMenu);
 return;
}

void CALLBACK _export OnDblClick(
	HSS	hSS,
	HWND	hWnd,
	UINT	event)
{

 UNUSED(hSS);
 UNUSED(event);
 UNUSED(hWnd);
 // For the Japanese edition
 // the only way to edit the spreadsheet
 // should be through the double click
 // function. When the user double clicks a dialog box is presented that
 // allows the user to change the value.
 // Set the values by determining which type of value should be sent to the
 // cell.

}


void TGlWindow::GLOpenSheet (RTMessage)
{

	OPENFILENAME ofn;
	char szFileTitle[256];
	UINT  i, cbString;
	char  chReplace;
	char  szFilter[256];
	if ((cbString = LoadString(hInst, IDS_FILTERSTRING,szFilter, sizeof(szFilter))) == 0) {
	 return ;
	}
	chReplace = szFilter[cbString - 1];

	for (i = 0; szFilter[i] != '\0'; i++) {
	 if (szFilter[i] == chReplace)
		 szFilter[i] = '\0';
	}

	//* Set all structure members to zero.
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = HWindow;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile= FileName;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = CurDir;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


	if(GetOpenFileName(&ofn)!=0)
		MakeChild(FileName);
	 _chdrive(drive);
	 chdir(CurDir);
}


void TGlWindow::New (RTMessage)
{
 MakeChild("");

}

#pragma argsused

void TGlWindow::MakeChild(LPSTR szFileName)
{

 WndXls  = new TWnd(this,szFileName);
 GetApplication()->MakeWindow(WndXls);

 WndXls->Show(SW_HIDE);

 if(SSWndCreateView(WndXls->HWindow,&WndXls->hSS))
	{
	 MessageBox(HWindow,GetString(hInst,1079),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 WndXls->ShutDownWindow();
	}
  //SSWndRegisterEvent(WndXls->hSS, SSM_ENDEDIT,(EVENTFUNC)OnEndEdit);
  //SSWndRegisterEvent(WndXls->hSS, SSM_STARTEDIT,(EVENTFUNC)OnStartEdit);
  SSWndRegisterEvent(WndXls->hSS, SSM_RCLICK,      (EVENTFUNC)OnRightClick);
  SSWndRegisterEvent(WndXls->hSS, SSM_DBLCLICK, 	(EVENTFUNC)OnDblClick);

 if ( (SSRead(WndXls->hSS,szFileName, &WndXls->nFileType)) != SSERROR_NONE )
	  {
		WndXls->SetCaption(GetString(hInst,6158));
	  }
 else
		WndXls->SetCaption(szFileName);
 WndXls->strFilename = szFileName;
 RECT rc;
 GetClientRect(WndXls->HWindow,&rc);
 SSWndMoveView(WndXls->hSS, 0,20,rc.right-rc.left,rc.bottom-rc.top-20);
 SSEditBarNew(WndXls->HWindow,&WndXls->hSSEdit);
 SSSetSSEdit(WndXls->hSS,WndXls->hSSEdit);
 SSEditBarMove(WndXls->hSSEdit,0,0,rc.right-rc.left,20);
 WndXls->Show(SW_SHOW);
 WndXls->bModified = FALSE;
}


void TGlWindow::GLFinFA    (RTMessage)
 {
  PrintReport(GetString(hInst,6154),GetApplication()->MainWindow->HWindow);
 }

void TGlWindow::GLFinDep    (RTMessage)
 {
  PrintReport(GetString(hInst,6155),GetApplication()->MainWindow->HWindow);
 }



