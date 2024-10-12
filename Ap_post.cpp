/*
	Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	Posting
	ap_post.cpp
*/
#include "std_inc.h"

#include "ap.h"
#include <math.h>
#include "db_dist.h"
#include "db_apbtc.h"
#include "ap_ctrl.h"
#include "gl_dlg.h"
#include "gl_postb.h"



extern HINSTANCE hInst;


void TManilaGL::APPostBatch   (RTMessage)
 {
  GetApplication()->ExecDialog(new TPostBatch(this,1061));
 }

void TManilaGL::APPostCash   (RTMessage)
 {
  //GetApplication()->ExecDialog(new TAPCashPost(this,1061));
 }

void TManilaGL::APPostAdj   (RTMessage){}


/*
void TPostBatch::HandleListBox( RTMessage Msg)
{
 string strMsg;
 dAPControl = 0;
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 if(apbtch.seek(lRecord)==0)
 {
  if(strcmp(apbtch.mode.str(),"3") == 0)
	{
	 strMsg = GetString(3093);
	 MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 return;
	}
	wsprintf(szMsg,GetString(8129),StrTrim(apbtch.batchno.str()));
	if(MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
	 if(CheckAPBalance(lRecord))
	  {
		char *szInvoice = strcpy(new char[apbtch.vendorno.len()+apbtch.invoiceno.len()+1],apbtch.vendorno.str());
		strcat(szInvoice,apbtch.invoiceno.str());
		AppendAPDist(szInvoice);
		AppendGLDist(&apbtch);
		delete [] szInvoice;
	  }
  }
 }

 SetField(ID_BATCHNO,"");
 SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
 int i = 1;
 for(apbtch.top();!apbtch.eof();apbtch.skip())
 {
		SSSetData(SHWindow,1,i,apbtch.batchno.str());
		SSSetData(SHWindow,2,i,apbtch.descript.str());
		SSSetData(SHWindow,3,i,apbtch.invoiceno.str());
		SSSetData(SHWindow,4,i,apbtch.payable.str());
	  i++;
  }

 if(i>10)
 SSSetMaxRows(SHWindow,i-1);
 else
 SSSetMaxRows(SHWindow,10);
 return;
}
*/


void TPostBatch::AP_AppendPriorPeriod(long lBatchno)
{
  Tgldist gldist(cb);
  if(apbtch.seek(lBatchno) == 0)
	{
		  //double dCurrBal = 0;
		  if(apbtchd.seek(apbtch.batchno.str())==0)
		  {
			while ((long)apbtchd.lBatchno() == (long)apbtch.lBatchno())
			 {
			  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd.batchno.str());
			  if(AP_UpdateGL(&apbtchd.account,&apbtchd.dept,apbtch.iPeriod()))
					{
						apbtch.mode.assign("4");
						apbtch.UpdateUserTime(szUser);
						gldist.appendStart();
						gldist.account.assign(apbtchd.account.str());
						gldist.type.assign("PP");
						gldist.dept.assign(apbtchd.dept.str());
						gldist.period.assign(apbtch.period.str());
						gldist.date.assign(apbtch.date.str());
						gldist.batchno.assignDouble(double(Str4ptr(apbtchd.batchno.str())));
						gldist.reference.assign(apbtch.reference.str());
						gldist.medrep.assign(apbtchd.depart.str());
						gldist.prod.assign(apbtchd.product.str());
						gldist.drcode.assign(apbtchd.drcodes.str());
						gldist.descript.assign(apbtchd.descript.str());
						gldist.debit.assignDouble(double(Str4ptr(apbtchd.debit.str())));
						gldist.credit.assignDouble(double(Str4ptr(apbtchd.credit.str())));
						gldist.UpdateUserTime(szUser);
						gldist.append();
				 }
				 apbtchd.skip(1);
			  }
			}
	}
}

void TPostBatch::AP_UpdateRetainedEarnings(double dAdjustment)
{
  for(glaccts.top();!glaccts.eof();glaccts.skip())
	{
	  if(strcmp(glaccts.type.str(),"R") == 0)
		{
		  double dCurrentBalance = glaccts.dCurrbal();
		  dCurrentBalance += dAdjustment;
		  glaccts.curr_bal.assignDouble(dCurrentBalance);
		  glaccts.openbal.assignDouble(dCurrentBalance);
		  return;
		}
	}
}

#pragma argsused
BOOL TPostBatch::AP_UpdateGL(Field4 *account, Field4 *dept,int iPeriod)
{
		BOOL bValid = FALSE;
		//glaccts.SetIndexAccts();
		 if(glaccts.SetIndexAccts(1) < 0)
		  {
				ShutDownWindow();
				return bValid;
		  }

		string szSeek(account->str());
		szSeek += dept->str();
		if(glaccts.seek((LPSTR)szSeek.c_str()) == 0)
		{
				double credit = apbtchd.dCredit();
				double debit  = apbtchd.dDebit();
				double dNewOpenBal  = (glaccts.dCurrbal() - dCurrentYearTotal()) + debit - credit;
				double dCurrBal =  dNewOpenBal + dCurrentYearTotal();

			if(strcmp(glaccts.type.str(),"B") == 0)
			{
				glaccts.curr_bal.assignDouble(dCurrBal);
				double dOpenbal = glaccts.dOpenbal();
				dOpenbal += apbtchd.dDebit() - apbtchd.dCredit();
				glaccts.openbal.assignDouble(dOpenbal);
				bValid = TRUE;
			}
			else
			{
				double dPrevious12 = apbtchd.dDebit() - apbtchd.dCredit();
				AP_UpdateRetainedEarnings(dPrevious12);
				dPrevious12 += double(Str4ptr(glaccts.previous12.str()));
				glaccts.previous12.assignDouble(dPrevious12);
				bValid = TRUE;
			}
			glaccts.UpdateUserTime(szUser);
			apbtch.UpdateUserTime(szUser);
		}
		else
		  MessageBox(HWindow,GetString(3081),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
  return bValid;
}

double TPostBatch::AP_dCurrentYearTotal()
{
	return glaccts.dPeriod1() +
	glaccts.dPeriod2() +
	glaccts.dPeriod3() +
	glaccts.dPeriod4() +
	glaccts.dPeriod5() +
	glaccts.dPeriod6() +
	glaccts.dPeriod7() +
	glaccts.dPeriod8() +
	glaccts.dPeriod9() +
	glaccts.dPeriod10() +
	glaccts.dPeriod11() +
	glaccts.dPeriod12();
}

void TPostBatch::AP_PostAll ()
{
 if(MessageBox(HWindow,GetString(8130),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 CHourGlass wait;
 Relate4set rQuery(apbtch);
 string strQuery = GetQuery();
 rQuery.querySet((LPSTR)strQuery.c_str());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
	 char szInvoice[15];
	 long lBatchno = apbtch.lBatchno();
	 strcpy(szInvoice,apbtch.vendorno.str());
	 strcat(szInvoice,apbtch.invoiceno.str());
	 if(AP_CheckAPBalance(lBatchno))
	  {
		AP_AppendAPDist(szInvoice);
		if(strcmp(apbtch.mode.str(),"3") != 0)
			{
			 AP_AppendAPSummary(lBatchno);
			 apbtch.mode.assign("0");
			 apbtch.UpdateUserTime(szUser);
			}
		else
		 AP_AppendPriorPeriod(lBatchno);
	  }
  }
 rQuery.unlock();
 rQuery.free();
 APRedraw();
}

BOOL TPostBatch::AP_CheckAPBalance(long lBatchno)
{
  apbtchd.SetIndex("batchno");
  double dDebit  = 0 ;
  double dCredit = 0 ;
  if(apbtchd.seek(lBatchno) == 0)
	{
	 while((long)apbtchd.lBatchno() == (long)lBatchno)
	  {
		 dCredit  += apbtchd.dCredit();
		 dDebit   += apbtchd.dDebit();
		 apbtchd.skip();
	  }
	}
  if(round(dCredit,2) == round(dDebit,2))
  {
	return TRUE;
  }
  else
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	wsprintf(szMsg,GetString(1007),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return FALSE;
  }
}

#pragma argsused
void TPostBatch::AP_AppendAPDist(char *szInvoice)
{
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd.batchno.str());
	 apdist.appendStart();
	 apdist.apaccount.assign(apbtch.account.str());
	 apdist.apdepart.assign(apbtch.depart.str());
	 apdist.period.assignLong(long(Str4ptr(apbtch.period.str())));
	 apdist.date.assign(apbtch.invdate.str());
	 apdist.duedate.assign(apbtch.duedate.str());
	 apdist.batchno.assignDouble(double(Str4ptr(apbtch.batchno.str())));
	 apdist.paymentctl.assign("H");
	 apdist.chequeno.assign("");
	 apdist.datepaid.assign("");
	 apdist.invoiceno.assign(StrTrim(apbtch.invoiceno.str()));
	 apdist.vendorno.assign(apbtch.vendorno.str());
	 apdist.reference.assign(apbtch.reference.str());
	 apdist.descript.assign(apbtch.descript.str());
	 apdist.amountpaid.assignDouble(0);
	 apdist.paymentreq.assignDouble(0);
	 double dAmountdue = 0;
	 if(apbtch.dRebateamt() < 0)
	  dAmountdue = apbtch.dPayable() + apbtch.dRebateamt();
	 else
	  dAmountdue = apbtch.dPayable();
	 apdist.amount.assignDouble(dAmountdue);
	 apdist.rebateamt.assignDouble(double(Str4ptr(apbtch.rebateamt.str())));
	 apdist.vatamt.assignDouble(apbtch.dVatamt());
	 apdist.UpdateUserTime(szUser);
	 apdist.append();
}

// apjsum is really the summary database file to hold the entries that will be transferred to the GL.

void TPostBatch::AP_AppendAPSummary(long lBatchno)
{
	  if(apbtchd.seek(lBatchno) == 0)
		{
		 while (apbtchd.lBatchno() == lBatchno)
		  {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd.batchno.str());

		  apjsum.appendStart();
		  apjsum.account.assign(apbtchd.account.str());
		  apjsum.dept.assign(apbtchd.dept.str());
		  apjsum.period.assign(apbtch.period.str());
		  apjsum.date.assign(apbtch.date.str());
		  apjsum.mode.assign("1");
		  apjsum.type.assign("AP");
		  apjsum.batchno.assignDouble(double(Str4ptr(apbtch.batchno.str())));
		  apjsum.reference.assign(apbtch.reference.str());
		  apjsum.descript.assign(apbtch.descript.str());
		  apjsum.debit.assignDouble(double(Str4ptr(apbtchd.debit.str())));
		  apjsum.credit.assignDouble(double(Str4ptr(apbtchd.credit.str())));
		  apjsum.depart.assign(apbtchd.depart.str());
		  apjsum.append();
		  apbtchd.skip();
		}
	 }
	 else
	 {
	  MessageBox(HWindow,GetString(6110),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	  cb->closeAll();
	  //cb->exclusive = FALSE;
	  ShutDownWindow();
	 }
}




void TPostBatch::AP_AppendGLDist(PTapbtch apbtch)
{
  char szSeek[12];
  if(apbtchd.seek(apbtch->batchno.str())==0)
		{
		 while (long(Str4ptr(apbtchd.batchno.str())) == long(Str4ptr(apbtch->batchno.str())))
		  {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd.batchno.str());
		  strcpy(szSeek,apbtchd.account.str());
		  strcat(szSeek,apbtchd.dept.str());
		  apbtch->mode.assign("0");
		  apbtch->UpdateUserTime(szUser);
		  apjsum.appendStart();
		  apjsum.account.assign(apbtchd.account.str());
		  apjsum.dept.assign(apbtchd.dept.str());
		  apjsum.mode.assign("1");
		  apjsum.period.assign(apbtch->period.str());
		  apjsum.date.assign(apbtch->date.str());
		  apjsum.type.assign("AP");
		  apjsum.batchno.assignDouble(double(Str4ptr(apbtch->batchno.str())));
		  apjsum.reference.assign(apbtch->reference.str());
		  apjsum.descript.assign(apbtch->descript.str());
		  apjsum.debit.assignDouble(double(Str4ptr(apbtchd.debit.str())));
		  apjsum.credit.assignDouble(double(Str4ptr(apbtchd.credit.str())));
		  apjsum.depart.assign(apbtchd.depart.str());
		  apjsum.append();
		  apbtchd.skip(1);
	 }
  }
}

void TPostBatch::AP_CreateGLSlip(void)
{
  dAPControl = 0;
  apbtch.top();
  while(!apbtch.eof())
  {
	AP_AppendGLDist(&apbtch);
	apbtch.skip(1);
  }
 return;
}

void TPostBatch::APRedraw()
{
 CHourGlass wait;

 TMainDialog test(this,1016);
 GetApplication()->MakeWindow(&test);
 test.SetField(ID_TEXT,GetString(336));
 test.Show(SW_SHOW);
 string strMsg1,strMsg;
 strMsg1 = GetString(335);


 HWND hSS = GetDlgItem(HWindow,ID_DISTLIST);
 SSSetMaxRows(hSS,apbtch.recCount());
 SSSetBool(hSS,SSB_REDRAW,FALSE);
 SSClearData(hSS,SS_ALLCOLS,SS_ALLROWS);
 long i=1;
 apbtch.SetIndexBatchno();
 Field4 update(apbtch,"update");
 Field4 user(apbtch,"user");
 Relate4set rQuery(apbtch);
 string strQuery = GetQuery();
 rQuery.querySet((LPSTR)strQuery.c_str());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
      strMsg = (LPSTR)strMsg1.c_str();
		SSSetData(hSS,1,i,apbtch.batchno.str());
		SSSetData(hSS,2,i,apbtch.descript.str());
		SSSetData(hSS,3,i,GetDate(update.str()));
		SSSetData(hSS,4,i,user.str());
		SSSetData(hSS,5,i,apbtch.invoiceno.str());
		SSSetData(hSS,6,i,apbtch.payable.str());
		strMsg += glbtch.batchno.str();
		test.SetField(ID_NUMBER,(LPSTR)strMsg.c_str());
		SetFocus(test.HWindow);

	  i++;
  }
 rQuery.unlock();
 rQuery.free();
 if(i>15)
 SSSetMaxRows(hSS,i-1);
 else
 SSSetMaxRows(hSS,15);
 SSSetBool(hSS,SSB_REDRAW,TRUE);
}

/*
void TPostBatch::Cancel (RTMessage)
{
 cb.closeAll();
 cb.exclusive = FALSE;
 ShutDownWindow();
}


void TPostBatch::SetupWindow()
{
 cb.exclusive = TRUE;
 TMainDialog::SetupWindow();
 SetCaption(GetString(333));
 CHourGlass wait;
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
 lfFont.lfCharSet         = iStyle;
 lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);

SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
SSSetBool(SHWindow,SSB_PROTECT,TRUE);
SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
SSSetBool(SHWindow,SSB_SHOWROWHEADERS, FALSE);


 if(apbtch.open(cb) < 0)
  {
	ErrorClose(apbtch.alias());
	cb.exclusive = FALSE;
	return;
  }



 if(apbtchd.open(cb))
  {
	ErrorClose(GL_BTCHD);
	cb.exclusive = FALSE;
	return;
  }
 apbtchd.SetIndex("batchno");

 if(glaccts.open(cb) < 0)
  {
	cb.exclusive = FALSE;
	ErrorClose(GLACCTS);
	return;
  }
  glaccts.SetIndexAccts();  			//glaccts.SetIndex(GetString(8145));


 if(apdist.open(cb)< 0)
  {
	ErrorClose(AP_DIST);
	cb.exclusive = FALSE;
	return;
  }

 apdist.SetIndex(GetString(8145));


 if(apjsum.open(cb) < 0)
  {
	ErrorClose(apjsum.alias());
	cb.exclusive = FALSE;
	return;
  }

 apjsum.SetIndex(GetString(8145));

//apjsum.pack();
//apdist.pack();

 //SetField(1001,"Press OK to Post.");

 SSSetMaxCols(SHWindow, 4);


 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, GetString(8141));
 SSSetData(SHWindow,2,SS_HEADER, GetString(8142));
 SSSetData(SHWindow,3,SS_HEADER, GetString(8143));
 SSSetData(SHWindow,4,SS_HEADER, GetString(8144));


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0.00,atof(GetString(6107)));
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;
 //Relate4set *rQuery = new Relate4set(apbtch);
 apbtch.SetIndex("post_ap");
 //rQuery.querySet("MODE = '1'");


 for(apbtch.top();!apbtch.eof();apbtch.skip())
 {
		SSSetData(SHWindow,1,i,apbtch.batchno.str());
		SSSetData(SHWindow,2,i,apbtch.descript.str());
		SSSetData(SHWindow,3,i,apbtch.invoiceno.str());
		SSSetData(SHWindow,4,i,apbtch.payable.str());
	  i++;
  }

 if(i>10)
 SSSetMaxRows(SHWindow,i-1);
 else
 SSSetMaxRows(SHWindow,10);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
}
*/


void Tglaccts::UpdateAccount(char *szSeek,int iPeriod, double dDebit, double dCredit)
{
	 if(seek(szSeek) == 0)
		{
		 curr_bal.assignDouble(dCurrbal() + dDebit - dCredit);
		 switch(iPeriod)
		  {
	 case 1: period1.assignDouble(dPeriod1()+ dDebit - dCredit);   break;
	 case 2: period2.assignDouble(dPeriod2()+ dDebit - dCredit);   break;
	 case 3: period3.assignDouble(dPeriod3() + dDebit - dCredit); break;
	 case 4: period4.assignDouble(dPeriod4()+ dDebit - dCredit);   break;
	 case 5: period5.assignDouble(dPeriod5()+ dDebit - dCredit);   break;
	 case 6: period6.assignDouble(dPeriod6()+ dDebit - dCredit);   break;
	 case 7: period7.assignDouble(dPeriod7()+ dDebit - dCredit);   break;
	 case 8: period8.assignDouble(dPeriod8()+ dDebit - dCredit);   break;
	 case 9: period9.assignDouble(dPeriod9()+ dDebit - dCredit);   break;
	 case 10: period10.assignDouble(dPeriod10()+ dDebit - dCredit); break;
	 case 11: period11.assignDouble(dPeriod11()+ dDebit - dCredit); break;
	 case 12: period12.assignDouble(dPeriod12()+ dDebit - dCredit); break;
	 default: return;
		 }
	 UpdateUserTime(szUser);

	 }
 return;
}

double Tglaccts::dPeriod(char *szAccount, char *szDept, int iPeriod)
{
  string strSeek = szAccount;
  strSeek += szDept;
  double dPeriod = 0;
  SetIndex(GetString(hInst,8145));
  if(seek((LPSTR)strSeek.c_str()) == 0)
	{
		 switch(iPeriod)
		  {
	 case 1: dPeriod = dPeriod1();   break;
	 case 2: dPeriod = dPeriod2();   break;
	 case 3: dPeriod = dPeriod3();   break;
	 case 4: dPeriod = dPeriod4();   break;
	 case 5: dPeriod = dPeriod5();   break;
	 case 6: dPeriod = dPeriod6();   break;
	 case 7: dPeriod = dPeriod7();   break;
	 case 8: dPeriod = dPeriod8();   break;
	 case 9: dPeriod = dPeriod9();   break;
	 case 10: dPeriod = dPeriod10(); break;
	 case 11: dPeriod = dPeriod11(); break;
	 case 12: dPeriod = dPeriod12(); break;
	 default: return dPeriod;
		 }
	}
  return dPeriod;
}