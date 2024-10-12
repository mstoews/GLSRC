/*
	Accounts Payable  - (C) Copyright 1994 by Tegara Corporation
	Adjustment batch.
	gl_postb.cpp
*/
#include "std_inc.h"


#include "db_profi.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_dist.h"
#include "db_apbtc.h"
#include "db_arbtc.h"
#include "db_apdst.h"
#include "db_ardst.h"
#include "db_cash.h"
//#include "db_alloc.h"
#include "db_srce.h"

#include "gl_dlg.h"
#include "gl_main.h"
#include "widgets.h"




_CLASSDEF(TRecoverBatch)
class TRecoverBatch : public TMainDialog
{
 public:
 TSpread  *List;
 Tglbtch  glbtch;
 Tglbtchd glbtchd;
 Tglaccts glaccts;
 Tgldist  gldist;
 Tapbtch  apbtch;
 Tapbtchd  apbtchd;
 Tapdist  apdist;
 Tardist  ardist;
 Tapcash  apcash;
 Tapcashd  apcashd;
 Tarbtch  arbtch;
 Tarbtchd  arbtchd;
 int iCurrentTab;

 TRecoverBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  List = new TSpread (this,ID_DISTLIST);
 };


 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void HandleDoubleClick (RTMessage Msg ) = [WM_FIRST + SSM_DBLCLK];
 virtual void HasChanged     (RTMessage Msg) = [ID_FIRST + 1001];
 void SetPeriodAmount(int period,char *szAccountNo,double dBatchNo);

 void GLRedraw();
 void ARRedraw();
 void APRedraw();
 void CASHRedraw();

 void GLRecover(long lBatchno);
 void ARRecover(long lBatchno);
 void APRecover(long lBatchno);
 void CASHRecover(long lBatchno);

 ~TRecoverBatch()
 {
	delete List;
 }
};



void TRecoverBatch::GLRecover(long lBatchno)
{
  TMainDialog test(this,1016);
  GetApplication()->MakeWindow(&test);
  test.SetField(ID_TEXT,"Recovering batch entry...");
  test.Show(SW_SHOW);

  string strAccount;
  BOOL bDeleted = FALSE;
  if(glbtch.seek(lBatchno) ==0)
  {
	 if(strcmp(glbtch.mode.str(),"0")==0)
		{
		 if(glbtchd.seek(lBatchno) == 0)
			{
				while (long(Str4ptr(glbtchd.batchno.str())) == lBatchno )
					{
						strAccount = glbtchd.account.str();
						strAccount += glbtchd.dept.str();

						SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtchd.batchno.str());
						SetPeriodAmount(int (Str4ptr(glbtchd.period.str())),(LPSTR)strAccount.c_str(),lBatchno);
						glbtchd.skip();
						bDeleted = TRUE;
				  }
			}
		}
  }
 if(bDeleted)
 {
  if(gldist.seek(lBatchno) == 0 )
  {
	while (long (Str4ptr(gldist.batchno.str())) == lBatchno)
	 {
	  gldist.deleteRec();
	  gldist.skip();
	 }
	gldist.pack();
  }
 }
}


void TRecoverBatch::APRecover(long lBatchno)
{
 if(apbtch.seek(lBatchno) == 0)
  {
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtch.batchno.str());
	 apbtch.mode.assign("1");
	 if(apdist.seek(lBatchno) == 0 )
	  {
		 if(strcmp(apdist.paymentctl.str(),"O") == 0 || strcmp(apdist.paymentctl.str(),"H") == 0)
		  {
			apdist.deleteRec();
			apdist.pack();
		  }
	  }
  }
}


void TRecoverBatch::ARRecover(long lBatchno)
{
 if(arbtch.seek(lBatchno) == 0)
  {
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtch.batchno.str());
	 arbtch.mode.assign("1");
	 if(ardist.seek(lBatchno) == 0 )
	  {
		 if(strcmp(ardist.paymentctl.str(),"O") == 0 || strcmp(ardist.paymentctl.str(),"H") == 0)
		  {
			ardist.deleteRec();
			ardist.pack();
		  }
	  }
  }
}


void TRecoverBatch::CASHRecover(long lBatchno)
{
 if(apcash.seek(lBatchno) == 0)
  {
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtch.batchno.str());
	 apcash.mode.assign("1");
	 if(apcash.seek(lBatchno) == 0 )
	  {
			apcash.deleteRec();
			apcash.pack();
	  }
  }
}


void TRecoverBatch::HandleDoubleClick  (RTMessage Msg)
{
  char szBatch[11];
  long lBatchno;
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  List->GetData(1,Cell->Row,szBatch);
  lBatchno = atol(szBatch);
  List->DelRow(Cell->Row);
  switch(iCurrentTab)
  {
	 case 0 : GLRecover(lBatchno); break;
	 case 1 : ARRecover(lBatchno); break;
	 case 2 : APRecover(lBatchno); break;
	 case 3 : CASHRecover(lBatchno); break;
  }
}



void TRecoverBatch::GLRedraw()
{
 CHourGlass wait;
 Relate4set rQuery(glbtch);
 string strQuery = GetString(1084);
 rQuery.querySet((LPSTR)strQuery.c_str());
 List->SetMaxRows(glbtch.recCount());
 List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 List->SetBool(SSB_REDRAW,FALSE);
 long i = 1;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,glbtch.batchno.str());
		List->SetData(2,i,glbtch.descript.str());
		List->SetData(3,i,GetDate(glbtch.date.str()));
		List->SetData(4,i,glbtch.credit.str());
	  i++;
 }
 rQuery.unlock();
 rQuery.free();

 if(i>13)
  List->SetMaxRows(i-1);
 else
  List->SetMaxRows(13);
  List->SetBool(SSB_REDRAW,TRUE);
}

void TRecoverBatch::ARRedraw()
{
 CHourGlass wait;
 Relate4set rQuery(arbtch);
 string strQuery = GetString(1084);
 rQuery.querySet((LPSTR)strQuery.c_str());

 List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 List->SetMaxRows(arbtch.recCount());
 List->SetBool(SSB_REDRAW,FALSE);
 long i = 1;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,arbtch.batchno.str());
		List->SetData(2,i,arbtch.descript.str());
		//List->SetData(
		List->SetData(3,i,GetDate(arbtch.date.str()));
		List->SetData(4,i,arbtch.payable.str());
	  i++;
 }
 rQuery.unlock();
 rQuery.free();

 if(i>13)
  List->SetMaxRows(i-1);
 else
  List->SetMaxRows(13);
  List->SetBool(SSB_REDRAW,TRUE);
}

void TRecoverBatch::APRedraw()
{
 CHourGlass wait;
 Relate4set rQuery(apbtch);
 string strQuery = GetString(1084); // Mode = '0' 
 rQuery.querySet((LPSTR)strQuery.c_str());
 List->SetMaxRows(apbtch.recCount());
 List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 List->SetBool(SSB_REDRAW,FALSE);
 long i = 1;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,apbtch.batchno.str());
		List->SetData(2,i,apbtch.descript.str());
		List->SetData(3,i,GetDate(apbtch.duedate.str()));
		List->SetData(4,i,apbtch.payable.str());
	  i++;
 }
 rQuery.unlock();
 rQuery.free();

 if(i>13)
  List->SetMaxRows(i-1);
 else
  List->SetMaxRows(13);
  List->SetBool(SSB_REDRAW,TRUE);

}

void TRecoverBatch::CASHRedraw()
{
 CHourGlass wait;
 Relate4set rQuery(apcash);
 string strQuery = GetString(1084);
 rQuery.querySet((LPSTR)strQuery.c_str());
 List->SetMaxRows(apcash.recCount());
 List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 List->SetBool(SSB_REDRAW,FALSE);
 long i = 1;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,apcash.batchno.str());
		List->SetData(2,i,apcash.descript.str());
		List->SetData(3,i,GetDate(apcash.date.str()));
		List->SetData(4,i,apcash.payable.str());
	  i++;
 }
 rQuery.unlock();
 rQuery.free();

 if(i>13)
  List->SetMaxRows(i-1);
 else
  List->SetMaxRows(13);
  List->SetBool(SSB_REDRAW,TRUE);

}


void TRecoverBatch::HasChanged(RTMessage)
{
		int iTab = HAGetActive(GetDlgItem(HWindow,1001));
		if(iCurrentTab != iTab)
		{
		 iCurrentTab = iTab;
		 switch(iCurrentTab)
			{
			 case 0 : GLRedraw(); break;
			 case 1 : ARRedraw(); break;
			 case 2 : APRedraw(); break;
			 case 3 : CASHRedraw(); break;
			}
		}
}


void TGlWindow::GLRepBatch   (RTMessage)
 {
  GetApplication()->ExecDialog(new TRecoverBatch(this,1015));
 }



#pragma argsused
void TRecoverBatch::SetPeriodAmount(int period,char *szAccountNo,double dBatchNo)
 {
  if(glaccts.seek(szAccountNo) != 0)
  {
	return;
  }

  double dCurrBal,dPeriod;
  dCurrBal = dPeriod = 0;
  switch(period)
	{
	 case 1 :
	  {
			 dPeriod   = double(Str4ptr(glaccts.period1.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
			 glaccts.period1.assignDouble(dPeriod);
			 break;
	  }
	case 2 :
	  {
		dPeriod   = double(Str4ptr(glaccts.period2.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period2.assignDouble(dPeriod);
		break;
	  }
  case 3:
	  {
		dPeriod   = double(Str4ptr(glaccts.period3.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period3.assignDouble(dPeriod);
		break;
	  }
  case 4:
	  {
		dPeriod   = double(Str4ptr(glaccts.period4.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period4.assignDouble(dPeriod);
		break;
	  }
  case 5:
	  {
		dPeriod   = double(Str4ptr(glaccts.period5.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period5.assignDouble(dPeriod);
		break;
	  }
  case 6:
	  {
		dPeriod   = double(Str4ptr(glaccts.period6.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period6.assignDouble(dPeriod);
		break;
		}
  case 7:
	  {
		dPeriod   = double(Str4ptr(glaccts.period7.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period7.assignDouble(dPeriod);
		break;
	  }
  case 8:
	  {
		dPeriod   = double(Str4ptr(glaccts.period8.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
					+ double(Str4ptr(glbtchd.credit.str()));
		glaccts.period8.assignDouble(dPeriod);
		break;
	  }
  case 9:
	  {
		dPeriod   = double(Str4ptr(glaccts.period9.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period9.assignDouble(dPeriod);
		break;
	  }
  case 10:
	  {
		dPeriod   = double(Str4ptr(glaccts.period10.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period10.assignDouble(dPeriod);
		break;
	  }
  case 11:
	  {
		dPeriod   = double(Str4ptr(glaccts.period11.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period11.assignDouble(dPeriod);
		break;
	  }
  case 12:
	  {
		dPeriod   = double(Str4ptr(glaccts.period12.str()))
			 - double(Str4ptr(glbtchd.debit.str()))
			 + double(Str4ptr(glbtchd.credit.str()));
		glaccts.period12.assignDouble(dPeriod);
		break;
	  }
	}
	  dCurrBal  = double(Str4ptr(glaccts.curr_bal.str()));
	  dCurrBal  = dCurrBal - double(Str4ptr(glbtchd.debit.str())) + double(Str4ptr(glbtchd.credit.str()));
	  glaccts.curr_bal.assignDouble(dCurrBal);
     glaccts.UpdateUserTime(szUser);
	  glaccts.flush();
	  glbtch.mode.assign("1");
     glbtch.UpdateUserTime(szUser);
	  glbtch.flush();
 }


void TRecoverBatch::Cancel (RTMessage)
{
 glbtch.close();
 glbtchd.close();
 glaccts.close();
 gldist.close();
 ShutDownWindow();
}

void TRecoverBatch::SetupWindow()
{
 CHourGlass wait;
 TDialog::SetupWindow();
 iCurrentTab = 0;
 SetBitmap(IDCANCEL);
 RECT rc;
 SS_CELLTYPE 	CellType;
 HFONT   	hFont;

 List->GetClientRect(&rc);
 short d0Width,d1Width,d3Width,d4Width;

 List->SetColWidth(1,6);
 List->SetColWidth(3,10);
 List->SetColWidth(4,10);

 List->GetColWidthInPixels(0,(LPSHORT)&d0Width);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 List->GetColWidthInPixels(4,(LPSHORT)&d4Width);


 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d1Width
		  - d3Width
		  - d4Width;

 List->SetColWidthInPixels(2,ColWidth);

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 // Open General ledger header file.
 if(glbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glbtch.GetDbfName());
	return;
  }
  glbtch.SetIndexBatchno();

  if(glbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(glbtchd.GetDbfName());
	 return;
	}
  glbtchd.SetIndexBatchno();

 // Open GL accounts file

 if(glaccts.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glaccts.GetDbfName());
	return;
  }
   
	if(glaccts.SetIndexAccts(1) < 0)
	{
	  ShutDownWindow();
	  return;
	}


 // Open the Distribution file.
  if(gldist.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(gldist.GetDbfName());
	return;
  }
  gldist.SetIndex(GetString(8140));
  int error_code = ((TAcctModule *)GetApplication())->cb.errorCode;
  if(error_code < 0 )
  {
	 ErrorClose(gldist.GetDbfName());
	 return;
  }

  if(apbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(apbtchd.GetDbfName());
	 return;
	}

  if(apbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(apbtch.GetDbfName());
	 return;
	}

 if(arbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(arbtch.GetDbfName());
	 return;
	}

  if(arbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(arbtchd.GetDbfName());
	 return;
	}

  if(apcashd.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(apcashd.GetDbfName());
	 return;
	}

	if(apcash.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(apcash.GetDbfName());
	 return;
	}


  if(apdist.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(apdist.GetDbfName());
	 return;
	}

  if(ardist.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	 ErrorClose(ardist.GetDbfName());
	 return;
	}

 List->SetMaxCols( 4);
 List->SetData(1,SS_HEADER, GetString(6043));       //Batchno
 List->SetData(2,SS_HEADER, GetString(6044));       //Description
 List->SetData(3,SS_HEADER, GetString(1086));       //Date
 List->SetData(5,SS_HEADER, GetString(1087));       //Amount

 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(1, SS_ALLROWS, &CellType);
 List->SetCellType(2, SS_ALLROWS, &CellType);
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 List->SetCellType(4, SS_ALLROWS, &CellType);
 List->SetMaxRows(3000);
 GLRedraw();
}
