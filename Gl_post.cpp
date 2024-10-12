/*
	Post Batch - (C) Copyright 1992 by Tegara Corporation
	gl_postb.cpp
*/

#include "std_inc.h"

// All of the database include files are in gl_postb.h
#include "widgets.h"
#include "gl_postb.h"
#include "ttab.h"
#include "lzexpand.h"

int ExclusiveLock(LPSTR szDBF);

extern char szMsg[256];
extern HINSTANCE hInst;
extern BOOL bAll;

#define ID_GLBTCH   8169
#define ID_GLBTCHD  8173
#define ID_GLDIST   8177
#define ID_ARBTCH   8170
#define ID_ARBTCHD  8174
#define ID_APBTCH   8171
#define ID_APBTCHD  8175
#define ID_DB_APCASH   8172
#define ID_APCASHD  8176
#define ID_LOCKED_FAILED 1080
#define ID_DATA_ERROR 60
#define ID_DATA_ERROR_MSG 61


void TPostBatch::SetupWindow()
{
 //Parent->Show(SW_HIDE);


 cb = &((TAcctModule *)GetApplication())->cb;
 //cb->exclusive = TRUE;
 TMainDialog::SetupWindow();
 SetBitmap(ID_VIEW);
 List = new TSpread(this,ID_DISTLIST);
 GetApplication()->MakeWindow(List);
 SetCaption(GetString(5117));
 CHourGlass wait;
 SS_CELLTYPE 	CellType;
 //((PTARCreatePayment)Parent)->bCancel = TRUE;



 BOOL bAllLocked = TRUE;

 if(ExclusiveLock(apcash.GetDbfName())< 0)
	bAllLocked = FALSE;

 if(apcash.open(*cb) < 0 )
  {
	ErrorClose(apcash.GetDbfName());
	return;
  }

 if(ExclusiveLock(apcashd.GetDbfName())< 0)
  bAllLocked = FALSE;


 if(apcashd.open(*cb) < 0)
  {
	ErrorClose(apcashd.GetDbfName());
	return;
  }

 if(ExclusiveLock(apbtch.GetDbfName())< 0)
  bAllLocked = FALSE;
 if(apbtch.open(*cb) < 0)
  {
	ErrorClose(apbtch.GetDbfName());
	return;
  }

 if(ExclusiveLock(apbtchd.GetDbfName())< 0)
  bAllLocked = FALSE;

 if(apbtchd.open(*cb) < 0)
  {
	ErrorClose(apbtchd.GetDbfName());

	return;
  }

 if(ExclusiveLock(apdist.GetDbfName())< 0)
  bAllLocked = FALSE;

 if(apdist.open(*cb) < 0)
  {
	ErrorClose(apdist.GetDbfName());
	return;
  }

 apdist.SetIndex(GetString(8145));

 if(ExclusiveLock(apjsum.GetDbfName())< 0)
  bAllLocked = FALSE;

 if(apjsum.open(*cb) < 0)
  {
	ErrorClose(apjsum.GetDbfName());
	return;
  }

 apjsum.SetIndex(GetString(8145));

 if(ExclusiveLock(glbtch.GetDbfName())< 0)
	bAllLocked = FALSE;

 if(glbtch.open(*cb) < 0)
  {
	ErrorClose(glbtch.GetDbfName());
	return;
  }
  glbtch.init();

 if(ExclusiveLock(glbtchd.GetDbfName())< 0)
	bAllLocked = FALSE;

 if(glbtchd.open(*cb) < 0)
  {
	ErrorClose(glbtchd.GetDbfName());
	return;
  }
  glbtchd.init();

 if(ExclusiveLock(glaccts.GetDbfName())< 0)
	 bAllLocked = FALSE;
 if(glaccts.open(*cb) < 0)
  {
	ErrorClose(glaccts.GetDbfName());

	return;
  }

 if(ExclusiveLock(gldist.GetDbfName())< 0)
  bAllLocked = FALSE;

 if(gldist.open(*cb) < 0)
  {
	ErrorClose("AP_DIST");
	return;
  }
 gldist.init();


 //glbtch.SetIndex("POST_GL");      // mode = '1' .and. mode = '3'
 glbtch.SetIndex(GetString(8140));

 if(ExclusiveLock(arbtch.GetDbfName())< 0)
	bAllLocked = FALSE;
 if(arbtch.open(*cb) < 0 )
  {
	ErrorClose(arbtch.GetDbfName());
	return;
  }
 arbtch.SetIndex("post_ar");


 if(ExclusiveLock(arbtchd.GetDbfName())< 0)
	bAllLocked = FALSE;
 if(arbtchd.open(*cb) < 0 )
  {
	ErrorClose(arbtchd.GetDbfName());
	return;
  }
 arbtchd.SetIndex(szBatchno);

 if(ExclusiveLock(gldist.GetDbfName())< 0)
	bAllLocked = FALSE;
 if(!ardist.open(*cb) < 0 )
  {
	ErrorClose(ardist.GetDbfName());

	return;
  }

 ardist.SetIndex("invoiceno");

 if(ExclusiveLock(arjsum.GetDbfName())< 0)
	bAllLocked = FALSE;
 if(!arjsum.open(*cb) < 0)
  {
	ErrorClose(arjsum.GetDbfName());
	return;
  }

 arjsum.SetIndex(szBatchno);

 if(bAllLocked)
 {
  HFONT   	hFont;
  hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
  List->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
  List->SetBool(SSB_PROTECT,TRUE);
  List->SetBool(SSB_HORZSCROLLBAR,FALSE);
  List->SetBool(SSB_VERTSCROLLBAR,TRUE);
  List->SetBool(SSB_SHOWROWHEADERS,FALSE);
  List->SetMaxCols(6);
  List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
  DeleteObject(hFont);
  List->SetData(1,SS_HEADER, GetString(5118));
  List->SetData(2,SS_HEADER, GetString(5119));
  List->SetData(3,SS_HEADER, "Update");
  List->SetData(4,SS_HEADER, "User");
  List->SetData(5,SS_HEADER, "Invoice No.");
  List->SetData(6,SS_HEADER, "Amount");
  List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
  RECT rc;
  List->GetClientRect(&rc);
  short d1Width,d3Width;
  List->SetColWidth(1,6);
  List->SetColWidth(3,10);
  List->SetColWidth(4,10);
  List->SetColWidth(5,10);
  List->SetColWidth(6,10);
  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(3,(LPSHORT)&d3Width);
  short ColWidth = (rc.right - rc.left)
		  - d1Width
		  - (4*d3Width);

  List->SetColWidthInPixels(2,ColWidth);

  List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
  List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
  List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
  List->SetCellType(6, SS_ALLROWS, &CellType);
  Show(SW_SHOW);
  //HASetActive(GetDlgItem(HWindow,1001),0);
  iCurrentTab = 0;
  PostMessage(HWindow,WM_COMMAND,ID_REDRAW,NULL);
 }
 else
 {
	MessageBox(HWindow,GetString(3038),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	PostMessage(HWindow,WM_COMMAND,IDCANCEL,NULL);
 }
}

