#include "std_inc.h"
#include "ar_clear.h"
#include "db_arclr.h"
#include "ar_clr.h"
#include "ar_paymt.h"

#define ID_INVALID_ACCOUNT 112

/******************************************************************************
Create  process globally allows the user to select as many items from the
outstanding AR list as they wish before the do any processing. The processing
is then done all at once. By pressing the global processing button.
At least one search must be preformed before the user can use the global processing
button.
******************************************************************************/



void TARCreatePayment::SeekMsg  (RTMessage Msg)
{
  Seek(Msg);
}

void TARCreatePayment::ProcessClearGlobally  (RTMessage)
{
  if(!bHasInvoices)
	 return;
  if(SendDlgItemMsg(ID_GLOBAL,BM_GETCHECK,NULL,NULL))
  {
	CHourGlass wait;
	char   szInvoiceno[21];
	char   szVendorno[20];
	HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
	SSGetData(SHWindow,3,1,szVendorno);
	SSGetData(SHWindow,4,1,szInvoiceno);
	strcat(szVendorno,szInvoiceno);
	if(ardist.SetIndex("invoiceno",cb) < 0)
	{
     ShutDownWindow();
	  return;
	}
	if(ardist.seek(szVendorno)== 0)
	{
	  if(strlen(StrTrim(szInvoiceno)) == 0)
		{
		 MessageBox(HWindow,GetString(6129),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
		 return;
		}

	  if(strcmp(ardist.paymentctl.str(),"C")== 0)
		{
		 MessageBox(HWindow,GetString(6130),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
		 return;
		}

	  TGlobalClearing* GlobalClearing = new TGlobalClearing(this,1163,"");
	  GlobalClearing->ardist = &ardist;
	  GlobalClearing->arcust = &arcust;
	  GlobalClearing->szSales = &szSales;
	  GlobalClearing->szSalesDept = &szSalesDept;
	  GlobalClearing->szVAT = &szVAT;
	  GlobalClearing->szVATDept = &szVATDept;
	  GlobalClearing->szAR = &szAR;
	  GlobalClearing->szARDept = &szARDept;
	  GlobalClearing->szWithAcct = &szWithAcct;
	  GlobalClearing->szWithDept = &szWithDept;
	  GetApplication()->ExecDialog(GlobalClearing);
	  if(!bCancel)  //If cancelled return to the parent window.
		ProcessReceivablesGlobally();
	}
  }
}

void TGlobalClearing::SetupWindow()
{
  //TARClear::SetupWindow();
  SetBitmap(IDOK);
  SetBitmap(IDCANCEL);
  SetCashAccounts();
  SetField(ID_VENDORNO,ardist->vendorno);
  SetField(ID_INVOICENO,ardist->invoiceno);
  Str4large szDue,szPaid;
  szDue.assignDouble(ardist->dRemainder(),16,2);
  szPaid.assignDouble(ardist->dAmount()-ardist->dRemainder(),16,2);
  SetField(ID_DEBIT,ardist->amount.str());
  SetField(ID_PAIDTODATE,szPaid.ptr());
  SetField(ID_DUEAMOUNT,szDue.str());
  SetField(ID_AMOUNT,szDue.str());
  SetField(ID_ACCOUNTS,ardist->account);
  SetField(ID_DEPART,ardist->dept);
  char szDate[11];
  Date4 dDate(ardist->datepaid.str());
  dDate.format(szDate,GetString(4024));
  SetField(ID_DATE,szDate);
  if(strlen(StrTrim(ardist->datepaid.str())) == 0)
 {
  Date4 Today;
  Today.today();
  SetField(ID_DATE,Today.str());
 }

 arcust->SetIndexVendorno();
 if(arcust->seek(ardist->vendorno.str()) == 0)
  {
	SetData(arcust,2016,19);
	SetData(arcust,2017,18);
	SetData(arcust,2018,20);
	SetData(arcust,2019,21);
	SetData(arcust,2020,17);
	SetData(arcust,2021,16);
	SetData(arcust,2122,30);
	SetData(arcust,2123,31);
  }
  else
  {
	 MessageBox(HWindow,GetString(5128),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONSTOP);
	 ShutDownWindow();
  }
  SetBitmap(ID_RESET);
  SetBitmap(4102);
}


void TGlobalClearing::Ok(RTMessage)
{
  string strAccount;
  strAccount = GetField(2013);
  strAccount += GetField(2014);

  Tglaccts glaccts;
  if(glaccts.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	 ErrorClose(glaccts.GetDbfName());
	 return;
  }
  //glaccts.SetIndexAccts();
  glaccts.SetIndexAccts(1);
  if(glaccts.seek((LPSTR)strAccount.c_str()) != 0)
	{
	  string strMsg(((TAcctModule *)GetApplication())->hInstance,ID_INVALID_ACCOUNT);
	  MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	  glaccts.close();
	  return;
	}
  glaccts.close();

  ((PTARCreatePayment)Parent)->strAccount = GetField(2013);
  ((PTARCreatePayment)Parent)->strDept = GetField(2014);
  ((PTARCreatePayment)Parent)->bCancel = FALSE;
  
  ShutDownWindow();
}



void  TARCreatePayment::ProcessReceivablesGlobally()
{
	 if(bCancel)  //If cancelled return to the parent window.
	  return;
	 if(ar_clr.open(*cb) < 0)
	 {
		ErrorClose(ar_clr.GetDbfName());
		return;
	 }

	 string strQuery;
	 strQuery = "PAYMENTCTL = 'G'";

	 Relate4set GlobalClear(ardist);
	 GlobalClear.querySet((LPSTR)strQuery.c_str());

	 for (int qc = GlobalClear.top();qc != r4eof; qc = GlobalClear.skip(1L))
	  {
		SetGlobalPaid();
	  }
};


void TARCreatePayment::SetGlobalPaid()
{



  string strBatchno(GetString(6161)); //strQuery =  "BATCHNO = ";
  string strType(GetString(6162)); //strQuery += " .AND. TYPE = 'CH'";
  string strQuery;
  strQuery = strBatchno;
  strQuery += ardist.batchno.str();
  strQuery += strType;

  Relate4set QueryDiscount(ar_clr);
  QueryDiscount.querySet((LPSTR)strQuery.c_str());
  for(int qc=QueryDiscount.top();qc!=r4eof;qc=QueryDiscount.skip(1L))
	 {
		ar_clr.deleteRec();
	 }
  ar_clr.pack();
  QueryDiscount.unlock();
  QueryDiscount.free();

  ardist.SetIndex("vendorno");

  string szDescript(GetDescription(ardist.account.str(),""));
  double dAmount = atof(ardist.amountpaid.str());
  Str4large strAmount;
  strAmount.assignDouble(fabs(dAmount),16,2);

  ardist.mode.assign("1");
  ardist.paymentctl.assign("P");
  double dAmountPaid  = ardist.dAmount();
  double dCurrentRemainder = ardist.dRemainder() - ardist.dAmount();
  ardist.remainder.assignDouble(dCurrentRemainder);
  ardist.amountpaid.assignDouble(dAmountPaid);

  if(!ar_clr.isValid())
  {
	 ErrorClose(ar_clr.GetDbfName());
	 return;
  }

  ar_clr.appendStart();
  ar_clr.account.assign      ((LPSTR)strAccount.c_str());
  ar_clr.dept.assign         ((LPSTR)strDept.c_str());
  ar_clr.period.assign       (ardist.period.str());
  ar_clr.mode.assign         ("1");
  ar_clr.date.assign         (ardist.date.str());
  ar_clr.type.assign         ("CH");
  ar_clr.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
  ar_clr.reference.assign    (ardist.reference.str());
  ar_clr.descript.assign     (ardist.descript.str());
  if(dAmountPaid > 0 )
  {
	  ar_clr.debit.assignDouble  (fabs(dAmountPaid));
	  ar_clr.credit.assignDouble (0.0);
  }
  else
  {
	  ar_clr.debit.assignDouble  (0.0);
	  ar_clr.credit.assignDouble (fabs(dAmountPaid));
  }
  ar_clr.UpdateUserTime(szUser);
  ar_clr.append();

  ar_clr.appendStart();
  ar_clr.account.assign      (ardist.apaccount.str());
  ar_clr.dept.assign         (ardist.apdepart.str());
  ar_clr.period.assign       (ardist.period.str());
  ar_clr.date.assign         (ardist.date.str());
  ar_clr.mode.assign         ("1");
  ar_clr.type.assign         ("CH");
  ar_clr.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
  ar_clr.reference.assign    (ardist.reference.str());
  ar_clr.descript.assign     (ardist.descript.str());
  if(dAmountPaid > 0)
  {
	  ar_clr.debit.assignDouble  (0.0);
	  ar_clr.credit.assignDouble (fabs(dAmountPaid));
  }
  else
  {
	  ar_clr.debit.assignDouble  (fabs(dAmountPaid));
	  ar_clr.credit.assignDouble (0.0);
  }
  ar_clr.UpdateUserTime(szUser);
  ar_clr.append();
}
