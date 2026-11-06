/*
	Accounts Payable  - (C) Copyright 1994 by Tegara Corporation
	Adjustment batch.
	gl_postb.cpp
*/

#include "tbowl.h"
#include "d4all.hpp"
#include "glclass.h"
#include "gl.h"
#include "db_accts.h"
#include "db_dept.h"
#include "db_profi.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_dist.h"
//#include "db_alloc.h"
#include "db_srce.h"

#include "gl_dlg.h"
#include "gl_main.h"


extern Code4 cb;

_CLASSDEF(TGLRecoverBatch)
class TGLRecoverBatch : public TMainDialog
{
 public:
 PTSpread  List;
 PTglbtch  glbtch;
 PTglbtchd glbtchd;
 PTglaccts glaccts;
 PTgldist  gldist;

 TGLRecoverBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  List = new TSpread (this,ID_DISTLIST);
 };


 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void HandleDoubleClick (RTMessage Msg ) = [WM_FIRST + SSM_DBLCLK]; 
 void SetPeriodAmount(int period,char *szAccountNo,double dBatchNo);
};


void TGlWindow::GLRepBatch   (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLRecoverBatch(this,1015));
 }


void TGLRecoverBatch::HandleDoubleClick  (RTMessage Msg)
{ 

  char szBatch[11];
  char szAccountNo[13];
  long dBatchNo;
  BOOL bDeleted = FALSE;
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  List->GetData(1,Cell->Row,szBatch);
  dBatchNo = atol(szBatch);
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSDelRow(SHWindow,Cell->Row);
 if(glbtch->seek(dBatchNo) ==0)
 {
  if(strcmp(glbtch->mode->str(),"0")==0)
  {

    if(glbtchd->seek(dBatchNo) == 0)
    {

     while (double(Str4ptr(glbtchd->batchno->str())) == dBatchNo )
     {
      strcpy(szAccountNo,glbtchd->account->str());
      strcat(szAccountNo,glbtchd->dept->str());
      SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtchd->batchno->str());     
      SetPeriodAmount(int (Str4ptr(glbtchd->period->str())),szAccountNo,dBatchNo);
      glbtchd->skip();
      bDeleted = TRUE;
     }
    }
  }
 }
 if(bDeleted)
 {
  if(gldist->seek(dBatchNo) == 0 )
  {
   while (double (Str4ptr(gldist->batchno->str())) == dBatchNo)
    {
     gldist->deleteRec();
     gldist->skip();
    }
   gldist->pack();
  }
 }
}

#pragma argsused
void TGLRecoverBatch::SetPeriodAmount(int period,char *szAccountNo,double dBatchNo)
 {
  if(glaccts->seek(szAccountNo) != 0)
  {
	return;
  }

  double dCurrBal,dPeriod;
  dCurrBal = dPeriod = 0;
  switch(period)
	{
	 case 1 :
	  {
			 dPeriod   = double(Str4ptr(glaccts->period1->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
			 glaccts->period1->assignDouble(dPeriod);
			 break;
	  }
	case 2 :
	  {
		dPeriod   = double(Str4ptr(glaccts->period2->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period2->assignDouble(dPeriod);
		break;
	  }
  case 3:
	  {
		dPeriod   = double(Str4ptr(glaccts->period3->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period3->assignDouble(dPeriod);
		break;
	  }
  case 4:
	  {
		dPeriod   = double(Str4ptr(glaccts->period4->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period4->assignDouble(dPeriod);
		break;
	  }
  case 5:
	  {
		dPeriod   = double(Str4ptr(glaccts->period5->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period5->assignDouble(dPeriod);
		break;
	  }
  case 6:
	  {
		dPeriod   = double(Str4ptr(glaccts->period6->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period6->assignDouble(dPeriod);
		break;
		}
  case 7:
	  {
		dPeriod   = double(Str4ptr(glaccts->period7->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period7->assignDouble(dPeriod);
		break;
	  }
  case 8:
	  {
		dPeriod   = double(Str4ptr(glaccts->period8->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
					+ double(Str4ptr(glbtchd->credit->str()));
		glaccts->period8->assignDouble(dPeriod);
		break;
	  }
  case 9:
	  {
		dPeriod   = double(Str4ptr(glaccts->period9->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period9->assignDouble(dPeriod);
		break;
	  }
  case 10:
	  {
		dPeriod   = double(Str4ptr(glaccts->period10->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period10->assignDouble(dPeriod);
		break;
	  }
  case 11:
	  {
		dPeriod   = double(Str4ptr(glaccts->period11->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period11->assignDouble(dPeriod);
		break;
	  }
  case 12:
	  {
		dPeriod   = double(Str4ptr(glaccts->period12->str()))
			 - double(Str4ptr(glbtchd->debit->str()))
			 + double(Str4ptr(glbtchd->credit->str()));
		glaccts->period12->assignDouble(dPeriod);
		break;
	  }
	}
	  dCurrBal  = double(Str4ptr(glaccts->curr_bal->str()));
	  dCurrBal  = dCurrBal - double(Str4ptr(glbtchd->debit->str())) + double(Str4ptr(glbtchd->credit->str()));
	  glaccts->curr_bal->assignDouble(dCurrBal);
     glaccts->UpdateUserTime(szUser);
	  glaccts->flush();
	  glbtch->mode->assign("1");
     glbtch->UpdateUserTime(szUser);
	  glbtch->flush();
 }


void TGLRecoverBatch::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGLRecoverBatch::SetupWindow()
{
 TDialog::SetupWindow();
 HourGlassOn();
 SetBitmap(IDCANCEL);
 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetClientRect(SHWindow,&rc);
 short d0Width,d1Width,d3Width,d4Width;

 SSSetColWidth(SHWindow,1,6);
 SSSetColWidth(SHWindow,3,10);
 SSSetColWidth(SHWindow,4,10);

 SSGetColWidthInPixels(SHWindow,0,(LPSHORT)&d0Width);
 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d3Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d4Width);


 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d1Width
		  - d3Width
		  - d4Width;

 SSSetColWidthInPixels(SHWindow,2,ColWidth);


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


SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
SSSetBool(SHWindow,SSB_PROTECT,TRUE);
SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);

 glbtch =  new Tglbtch(&cb);
 glbtch->SetIndex(GetString(8140));


 glbtchd =  new Tglbtchd(&cb);
 glbtchd->SetIndex(GetString(8140));


 glaccts =  new Tglaccts(&cb);
 glaccts->SetIndex(GetString(8145));


 gldist  =  new Tgldist(&cb);
 gldist->SetIndex(GetString(8140));

 SSSetMaxCols(SHWindow, 4);



 SSSetData(SHWindow,1,SS_HEADER, GetString(6043));       //Batchno
 SSSetData(SHWindow,2,SS_HEADER, GetString(6044));   //Description
 SSSetData(SHWindow,3,SS_HEADER, GetString(6069));       //Debit
 SSSetData(SHWindow,5,SS_HEADER, GetString(6070));       //Credit

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);

 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;
 glbtch->top();
 while(!glbtch->eof())
 {
     if(StrCmp(glbtch->mode->str(),"0")==0)
     {
      SSSetData(SHWindow,1,i,glbtch->batchno->str());
      SSSetData(SHWindow,2,i,glbtch->descript->str());
      SSSetData(SHWindow,3,i,glbtch->debit->str());
      SSSetData(SHWindow,4,i,glbtch->credit->str());
     i++;
     }
     glbtch->skip(1);
 }

 if(i>13)
  SSSetMaxRows(SHWindow,i-1);
 else
  SSSetMaxRows(SHWindow,12);


 SSBuildDependencies(SHWindow);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSReCalc(SHWindow);
 SSSetLock(SHWindow,5,SS_ALLROWS,TRUE);
 SSSetLock(SHWindow,6,SS_ALLROWS,TRUE);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 SSSetBool(SHWindow, SSB_ARROWSEXITEDITMODE, TRUE);
 SSSetEditEnterAction(SHWindow, SS_ENTERACTION_NEXT);

 HourGlassOff();
}
