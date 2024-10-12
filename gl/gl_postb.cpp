/*
	Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
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
#include "gl_postb.h"
#include "gl_main.h"


extern Code4 cb;
extern char szApp[20];
extern char szMsg[256];
BOOL bAll;

void TGlWindow::GLPostBatch (RTMessage){GetApplication()->ExecDialog(new TGLPostBatch(this,1061));}

void TGLPostBatch::Ok             (RTMessage )
{
  bAll = TRUE;
  glbtch->top();
  while(!glbtch->eof())
  {
	if(glbtch->Open())
	 PostGLSlip(glbtch->Batchno());
	else if(glbtch->Adjustment())
	 PostAdjustment(glbtch->Batchno());
	glbtch->skip(1);
  }
  
  if(bAll)
   {
	 cb.closeAll();
	 cb.init();
    MessageBox(HWindow,GetString(8115),szApp,MB_ICONINFORMATION);
    ShutDownWindow();
   }
  else
  Redraw();
}

void TGLPostBatch::SetupWindow()
{
 TDialog::SetupWindow();
 HourGlassOn();
 SetBitmap(IDOK);
 SetBitmap(IDCANCEL);
 HWND SHWindow;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 glbtch = new Tglbtch(&cb);
 glbtchd = new Tglbtchd(&cb);
 glaccts = new Tglaccts(&cb);
 gldist  = new Tgldist(&cb);

 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);

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
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_HORZSCROLLBAR,FALSE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);

 SSSetMaxCols(SHWindow, 4);

 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 SSSetData(SHWindow,1,SS_HEADER, "Vch");
 SSSetData(SHWindow,2,SS_HEADER, "Description");
 SSSetData(SHWindow,3,SS_HEADER, "Debit Amount");
 SSSetData(SHWindow,5,SS_HEADER, "Credit Amount");

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 RECT rc;
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

 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 Redraw();
 HourGlassOff();

}

void TGLPostBatch::Redraw()
 {
  HourGlassOn();
  HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
  SSSetBool(SHWindow,SSB_REDRAW,FALSE);
  SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
  long i=1;
  glbtch->top();
  while(!glbtch->eof())
  {
	  if(strcmp(glbtch->mode->str(),"1") == 0)
	  {
		SSSetData(SHWindow,1,i,glbtch->batchno->str());
		SSSetData(SHWindow,2,i,glbtch->descript->str());
		SSSetData(SHWindow,3,i,glbtch->debit->str());
		SSSetData(SHWindow,4,i,glbtch->credit->str());
	  i++;
	  }
	  glbtch->skip(1);
  }

  if((i-1) > 7)
   SSSetMaxRows(SHWindow,i-1);
  else
   SSSetMaxRows(SHWindow,7);
  SSSetBool(SHWindow,SSB_REDRAW,FALSE);
  HourGlassOff();
 }



void TGLPostBatch::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}


void  TGLPostBatch::UpdateGL(Field4 *account, Field4 *dept)
{
		glaccts->SetIndex("accounts");
		char *szSeek = new char[account->len() + dept->len() +1];
		strcpy(szSeek,account->str());
		strcat(szSeek,dept->str());
		if(glaccts->seek(szSeek) ==0)
		{
		 int iPeriod = glbtchd->iPeriod();
		 double credit = glbtchd->Credit();
		 double debit  = glbtchd->Debit();
		 switch(iPeriod)
		 {
		  case 1  : glaccts->period1->assignDouble(glaccts->dPeriod1()+debit-credit);break;
		  case 2  : glaccts->period2->assignDouble(glaccts->dPeriod2()+debit-credit);break;
		  case 3  : glaccts->period3->assignDouble(glaccts->dPeriod3()+debit-credit);break;
		  case 4  : glaccts->period4->assignDouble(glaccts->dPeriod4()+debit-credit);break;
		  case 5  : glaccts->period5->assignDouble(glaccts->dPeriod5()+debit-credit);break;
		  case 6  : glaccts->period6->assignDouble(glaccts->dPeriod6()+debit-credit);break;
		  case 7  : glaccts->period7->assignDouble(glaccts->dPeriod7()+debit-credit);break;
		  case 8  : glaccts->period8->assignDouble(glaccts->dPeriod8()+debit-credit);break;
		  case 9  : glaccts->period9->assignDouble(glaccts->dPeriod9()+debit-credit);break;
		  case 10 : glaccts->period10->assignDouble(glaccts->dPeriod10()+debit-credit);break;
		  case 11 : glaccts->period11->assignDouble(glaccts->dPeriod11()+debit-credit);break;
		  case 12 : glaccts->period12->assignDouble(glaccts->dPeriod12()+debit-credit);break;
		  default : break;
		}
		glaccts->UpdateUserTime(szUser);
		glaccts->flush();
	 }
	 else
	 // "Unable to find GL Account"
	 MessageBox(HWindow,GetString(3081),szApp,MB_ICONINFORMATION);
  delete [] szSeek;
}


void TGLPostBatch::PostGLSlip(long lBatchno)
{
  glbtchd->SetIndex(GetString(8140));
  if(glbtch->seek(lBatchno) == 0)
	{
	 if(glbtch->Open())
	 {
	  if(double(Str4ptr(glbtch->debit->str())) == double(Str4ptr(glbtch->credit->str())))
	  {
		double dCurrBal = 0;
		 if(glbtchd->seek(glbtch->batchno->str())==0)
		  {
			while ((int)glbtchd->Batchno() == (int)glbtch->Batchno())
			 {
			  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtchd->batchno->str());
			  UpdateGL(glbtchd->account,glbtchd->dept);

			  dCurrBal  = double(Str4ptr(glaccts->curr_bal->str()))
				 + double(Str4ptr(glbtchd->debit->str())) -
					double(Str4ptr(glbtchd->credit->str()));

			  glaccts->curr_bal->assignDouble(dCurrBal);
			  glbtch->mode->assign("0");
			  glbtch->UpdateUserTime(szUser);
			  glbtch->flush();

			  glbtchd->mode->assign("0");
			  glbtchd->flush();

			  gldist->appendStart();
			  gldist->account->assign(glbtchd->account->str());
			  gldist->dept->assign(glbtchd->dept->str());
			  gldist->period->assign(glbtchd->period->str());
			  gldist->date->assign(glbtchd->date->str());
			  gldist->batchno->assignDouble(double(Str4ptr(glbtchd->batchno->str())));
			  gldist->reference->assign(glbtchd->reference->str());
			  gldist->cheque->assign(glbtchd->cheque->str());
			  gldist->medrep->assign(glbtchd->medrep->str());
			  gldist->prod->assign(glbtchd->product->str());
			  gldist->drcode->assign(glbtchd->drcode->str());
			  gldist->descript->assign(glbtchd->descript->str());
			  gldist->debit->assignDouble(double(Str4ptr(glbtchd->debit->str())));
			  gldist->credit->assignDouble(double(Str4ptr(glbtchd->credit->str())));
			  gldist->UpdateUserTime(szUser);
			  gldist->append();
			  glbtchd->skip(1);
		 }
		}
	 }
	else
			 {

		wsprintf(szMsg,GetString(8116),glbtch->batchno->str());
			  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
	   bAll = FALSE;
          }
	 }
  }
}

void TGLPostBatch::PostAdjustment(long lBatchno)
{
  if(glbtch->seek(lBatchno) == 0)
	{
	  if(double(Str4ptr(glbtch->debit->str())) == double(Str4ptr(glbtch->credit->str())))
     { 
      double dOpenBal = 0.0;
      double dCurrBal = 0.0;
		 if(glbtchd->seek(glbtch->batchno->str())==0)
		  {
	  while (double(Str4ptr(glbtchd->batchno->str())) == double(Str4ptr(glbtch->batchno->str())))
	  {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtchd->batchno->str());

		  char *szSeek = new char[glbtchd->account->len() + glbtchd->dept->len() +1];
             strcpy(szSeek,glbtchd->account->str());
				 strcat(szSeek,glbtchd->dept->str());
		 if(glaccts->seek(szSeek) == 0)
	    {
	     dOpenBal  = glaccts->Open_bal()+
	                 double(Str4ptr(glbtchd->debit->str())) -
		         double(Str4ptr(glbtchd->credit->str()));
	     glaccts->openbal->assignDouble(dOpenBal);

	     dCurrBal =  glaccts->Open_bal() +
			 glaccts->dPeriod1() +
			 glaccts->dPeriod2() +
			 glaccts->dPeriod3() +
			 glaccts->dPeriod4() +
			 glaccts->dPeriod5() +
			 glaccts->dPeriod6() +
			 glaccts->dPeriod7() +
			 glaccts->dPeriod8() +
			 glaccts->dPeriod9() +
			 glaccts->dPeriod10() +
			 glaccts->dPeriod11() +
			 glaccts->dPeriod12();
		  glaccts->curr_bal->assignDouble(dCurrBal);
		  glaccts->UpdateUserTime(szUser);
		  glaccts->flush();
		  glbtch->UpdateUserTime(szUser);
		  glbtch->mode->assign("2");
		  glbtch->flush();
		 }
	  glbtchd->skip(1);
		  }
		}
	 }
  }
}


void TGLPostBatch::HandleListBox(RTMessage Msg)
{
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 char *szBuffer = new char [10];
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 glbtch->SetIndex(GetString(8140));
 if(glbtch->seek(szBuffer) == 0)
 {
  SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  wsprintf(szMsg,GetString(8129),StrTrim(glbtch->batchno->str()));
  if(MessageBox(HWindow,szMsg,szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
	if(glbtch->Open() && glbtch->Debit() == glbtch->Credit() )
		{
		 PostGLSlip(glbtch->Batchno());
		 SSDelRow(SHWindow,Cell->Row);
		}
	else
	  {
		  wsprintf(szMsg,GetString(8116),glbtch->batchno->str());
		  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
	  }
  }
  else
  SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 }
 delete [] szBuffer;
}



void TGlWindow::GLYearEnd   (RTMessage)
 {
  if(MessageBox(HWindow,GetString(GetApplication()->hInstance,8111),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
	PTglaccts gl = new Tglaccts(&cb);
	gl->top();
	double dRetainedEarnings = 0;
	while(!gl->eof())
	 {
	  if(strcmp(gl->type->str(),"B") != 0)
	  {
		dRetainedEarnings += gl->Curr_bal();
		gl->curr_bal->assignDouble(0);
	  }
	  gl->previous1->assignDouble(gl->dPeriod1());
	  gl->previous2->assignDouble(gl->dPeriod2());
	  gl->previous3->assignDouble(gl->dPeriod3());
	  gl->previous4->assignDouble(gl->dPeriod4());
	  gl->previous5->assignDouble(gl->dPeriod5());
	  gl->previous6->assignDouble(gl->dPeriod6());
	  gl->previous7->assignDouble(gl->dPeriod7());
	  gl->previous8->assignDouble(gl->dPeriod8());
	  gl->previous9->assignDouble(gl->dPeriod9());
	  gl->previous10->assignDouble(gl->dPeriod10());
	  gl->previous11->assignDouble(gl->dPeriod11());
	  gl->previous12->assignDouble(gl->dPeriod12());

	  gl->period1->assignDouble(0);
	  gl->period2->assignDouble(0);
	  gl->period3->assignDouble(0);
	  gl->period4->assignDouble(0);
	  gl->period5->assignDouble(0);
	  gl->period6->assignDouble(0);
	  gl->period7->assignDouble(0);
	  gl->period8->assignDouble(0);
	  gl->period9->assignDouble(0);
	  gl->period10->assignDouble(0);
	  gl->period11->assignDouble(0);
	  gl->period12->assignDouble(0);
	  gl->UpdateUserTime(szUser);
	  gl->flush();
	  gl->skip();
	 }
	for(gl->top();!gl->eof();gl->skip())
	{
	 if(strcmp(gl->type->str(),"R") != 0)
	  {
			double dCurrentRetained = gl->Curr_bal();
			gl->curr_bal->assignDouble(dCurrentRetained + dRetainedEarnings);
			gl->flush();
	  }

	}
	gl->close();
  }
 }
