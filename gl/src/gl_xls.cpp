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



//void CALLBACK _export OnEndEdit	   (HSS, HWND, UINT);
//void CALLBACK _export OnRightClick (HSS hSS,HWND hWnd,UINT	event);
//void CALLBACK _export OnDblClick   (HSS, HWND, UINT);


extern HMENU HGLMenu,HGLExMenu;
extern char   CurDir[MAXPATH];
extern UINT drive;
extern char szApp[20];
extern HINSTANCE hInst;

static char FileName[MAXPATH] = "\0";
static void DisplayError(HWND	hwnd,SSERROR nError);


_CLASSDEF(TWnd)
class TWnd: public TWindow
{
 public:
 BOOL	bModified;
 COLORREF crColor;
 LOGFONT lfTemp;
 CHOOSEFONT cfTemp;
 HFONT hfFont;
 BOOL tfFontLoaded;
 HSS hSS;
 int	nFileType;
 SSERROR sErr;
 char	*szUntitled;
 BOOL   bRecalc;
 TWnd(PTWindowsObject AParent, LPSTR ATitle):TWindow(AParent,ATitle) { };
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
 virtual void WMMDIActivate  (TMessage& Msg);
 virtual void GetWindowClass (WNDCLASS& wc);
 virtual BOOL CanClose ();
 SSERROR Save(LPINT pFileType,LPSTR pFileName);
 void ShowButton(int ID, BOOL bShow);
 void ShowBar(BOOL bShow);
};

PTWnd WndXls;

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
 char buff[_MAX_PATH];
	if ( (sErr = SSSaveFileDlg(hSS, NULL, buff, _MAX_PATH - 1, &nFileType)) != SSERROR_NONE )
		{
 	         DisplayError(HWindow,sErr);
		}
	else
		{
		lstrcpy(FileName, buff);
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
 sErr = Save(&nFileType,FileName);
 return;
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
if ( (sErr = SSSaveWindowInfo(hSS)) == SSERROR_NONE && (sErr = SSWrite(hSS, pFileName, *pFileType)) == SSERROR_NONE )
	bModified = FALSE;
return sErr;
}


void TWnd::WMSize (RTMessage Msg)
{
 TWindow::WMSize(Msg); 
 //SSWndMoveView(hSS, 0, WM_HEADER_HEIGHT,LOWORD(Msg.LParam), HIWORD(Msg.LParam));
 SSWndMoveView(hSS, 0,0,LOWORD(Msg.LParam), HIWORD(Msg.LParam));
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





static void DisplayError(HWND	hwnd,SSERROR 	nError)
{
 char	buff[_MAX_PATH];

 // Convert SSERROR to text
 if ( SSErrorNumberToText(nError, buff,sizeof(buff)) )
	strcpy(buff,GetString(hInst,1078));
 MessageBox(hwnd, buff,szApp, MB_ICONEXCLAMATION | MB_OK);
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
  ShowButton(ID_GLINQ,  bGLBAR);
  ShowButton(ID_GLPOST, bGLBAR);
  ShowButton(ID_GLADD,  bGLBAR);
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



 if(bModified)
  switch(MessageBox(HWindow,GetString(hInst,1076),szApp,MB_ICONQUESTION | MB_YESNOCANCEL))
	{
	 case IDYES :
				{
				if ( (sErr = SSSaveWindowInfo(hSS)) == SSERROR_NONE && (sErr = SSWrite(hSS, FileName,2)) == SSERROR_NONE )
				 bModified = FALSE;
				else
				if(Save(&nFileType,FileName) != SSERROR_NONE)
				 return FALSE;
				}
	case IDCANCEL : return FALSE;
	default : return TRUE;
 }
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
//bModified = TRUE;
}


void CALLBACK _export OnRightClick(
	HSS	hSS,
	HWND	hWnd,
	UINT	event
	)
{

 UNUSED(event);
 UNUSED(hWnd);
 UNUSED(event);
 //HMENU hMenu = LoadMenu(hInst,"SPREAD");
 //hMenu = GetSubMenu(hMenu,0);
 //POINT ptCurrent;

 //SSGetActiveCell(hSS,
 //SSRangeToTwips function could be used to get the coordinates of
 //the mouse click...

 //ClientToScreen(GetParent(hWnd),&ptCurrent);
 //TrackPopupMenu(hMenu,TPM_LEFTALIGN,ptCurrent.x,ptCurrent.y,0,hWnd,NULL);
 return;
}

void CALLBACK _export OnDblClick(
	HSS	hSS,
	HWND	hWnd,
	UINT	event)
{

 UNUSED(event);
 UNUSED(hWnd);
 // For the Japanese edition
 // the only way to edit the spreadsheet
 // should be through the double click
 // function. When the user double clicks a dialog box is presented that
 // allows the user to change the value.
 // Set the values by determining which type of value should be sent to the
 // cell.
 SSSortDlg(hSS);
}

void TGlWindow::GLOpenSheet (RTMessage)
{
 SSERROR sErr = SSOpenFileDlg(NULL,HWindow,FileName,_MAX_PATH);
		if ( sErr == SSERROR_NONE )
			MakeChild(FileName);
 _chdrive(drive);
 chdir(CurDir);
}


void TGlWindow::New (RTMessage)
{
 MakeChild("");
}

void TGlWindow::MakeChild(LPSTR szFileName)
{
 SSERROR sErr;
 WndXls = new TWnd(this,szFileName);
 GetApplication()->MakeWindow(WndXls);
 WndXls->Show(SW_HIDE);

 if(SSWndCreateView(WndXls->HWindow,&WndXls->hSS))
	{
	 MessageBox(HWindow,GetString(hInst,1079),szApp,MB_OK);
	 WndXls->ShutDownWindow();
	}

 //SSWndRegisterEvent(WndXls->hSS, SSM_ENDEDIT,(EVENTFUNC)OnEndEdit);
 //SSWndRegisterEvent(WndXls->hSS, SSM_RCLICK, (EVENTFUNC)OnRightClick);
 //SSWndRegisterEvent(WndXls->hSS, SSM_DBLCLICK, 	(EVENTFUNC)OnDblClick);

 if ( (sErr = SSRead(WndXls->hSS,szFileName, &WndXls->nFileType)) != SSERROR_NONE )
	  {

		WndXls->SetCaption("(untitled)");
	  }
 else
		WndXls->SetCaption(szFileName);

 RECT rc,top_wnd;
 GetClientRect(WndXls->HWindow,&rc);
 SSWndMoveView(WndXls->hSS, 0, 0,rc.right-rc.left,rc.bottom-rc.top);
 WndXls->Show(SW_SHOW);
}

void TGlWindow::GLIncomeSt (RTMessage)
{
 strcpy(FileName,CurDir);
 strcat(FileName,"\\");
 strcat(FileName,GetString(hInst,2080));
 MakeChild(FileName);
}

void TGlWindow::GLBalance (RTMessage)
{
 strcpy(FileName,CurDir);
 strcat(FileName,"\\");
 strcat(FileName,GetString(hInst,2081));
 MakeChild(FileName);
}


void TGlWindow::GLManAccounts    (RTMessage)
 {
 strcpy(FileName,CurDir);
 strcat(FileName,"\\");
 strcat(FileName,GetString(hInst,2082));
 MakeChild(FileName);
 }


void TGlWindow::GLFinRE    (RTMessage)
 {
 strcpy(FileName,CurDir);
 strcat(FileName,"\\");
 strcat(FileName,GetString(hInst,2083));
 MakeChild(FileName);

 }

void TGlWindow::GLFinFA    (RTMessage)
 {
 //strcpy(FileName,CurDir);
 //strcat(FileName,GetString(hInst,1084));
 //MakeChild(FileName);
  PrintReport("gl_fa.rpt",GetApplication()->MainWindow->HWindow);
 }

void TGlWindow::GLFinDep    (RTMessage)
 {
 //strcpy(FileName,CurDir);
 //strcat(FileName,GetString(hInst,1085));
 //MakeChild(FileName);
  PrintReport("gl_fatyp.rpt",GetApplication()->MainWindow->HWindow);
 }


/*
 void TGlWindow::GLOpenSheet   (RTMessage){}
 void TGlWindow::GLFinDep      (RTMessage){}
 void TGlWindow::GLFinRE       (RTMessage){}
 void TGlWindow::GLFinFA       (RTMessage){}
 void TGlWindow::GLManAccounts (RTMessage){}
 void TGlWindow::GLBalance     (RTMessage){}
 void TGlWindow::GLIncomeSt    (RTMessage){}
 void TGlWindow::New           (RTMessage){}
 void TGlWindow::MakeChild     (LPSTR szFileName){}
*/

