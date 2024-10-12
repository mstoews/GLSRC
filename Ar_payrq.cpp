#include "std_inc.h"
/*
#include "tbowl.h"
#include "d4all.hpp"
#include "glclass.h"
#include "ar.h"
#include "gl_dlg.h"
#include "db_arbtc.h"
#include "db_cust.h"
#include "db_arjsm.h"
#include "math.h"
#include "db_ardst.h"
#include "ar_distl.h"
#include "utildlg.h"
#include "gl_setat.hpp"
#include "ar_clear.h"
#include "db_arclr.h"
#include "ar_clr.h"
#include "ar_paymt.h"
*/

BOOL bReset;


// Creates a dialog box to modify the query for the
// viewing of the AR accounts.

void TARClear::Cancel (RTMessage)
	{
	  bReset = TRUE;
	  ((PTARCreatePayment)Parent)->bCancel = TRUE;
	  ShutDownWindow();
	};

void TARCreatePayment::Open(RTMessage)
{
	SendDlgItemMsg(ID_OPEN,BM_SETCHECK,TRUE,NULL);
	SendDlgItemMsg(ID_PAID,BM_SETCHECK,FALSE,NULL);
	SendDlgItemMsg(ID_ALL,BM_SETCHECK,FALSE,NULL);
}

void TARCreatePayment::Paid(RTMessage)
{
	SendDlgItemMsg(ID_OPEN,BM_SETCHECK,FALSE,NULL);
	SendDlgItemMsg(ID_PAID,BM_SETCHECK,TRUE,NULL);
	SendDlgItemMsg(ID_ALL,BM_SETCHECK,FALSE,NULL);
}

void TARCreatePayment::GlobalClearing (RTMessage)
{
  if(SendDlgItemMsg(ID_GLOBAL,BM_GETCHECK,NULL,NULL))
	SendDlgItemMsg(ID_GLOBAL,BM_SETCHECK,FALSE,NULL);
  else
	SendDlgItemMsg(ID_GLOBAL,BM_SETCHECK,TRUE,NULL);

}



void TARCreatePayment::All(RTMessage)
{
	SendDlgItemMsg(ID_OPEN,BM_SETCHECK,FALSE,NULL);
	SendDlgItemMsg(ID_PAID,BM_SETCHECK,FALSE,NULL);
	SendDlgItemMsg(ID_ALL,BM_SETCHECK,TRUE,NULL);

}

void TARClear::Reset(RTMessage)
{
  if(strcmp(ardist->paymentctl.str(),"R") != 0)
		{
			CHourGlass wait;
			BOOL bNoReset = FALSE;
			string strQuery;
			strQuery =  "BATCHNO = ";
			strQuery += ardist->batchno.str();
			strQuery += " .AND. TYPE = 'CH' .AND. MODE = '2'";
			Tarjsum arjsum(cb);
			if(!arjsum.isValid())
			{
			  ErrorClose(arjsum.GetDbfName());
			  return;
			}
			arjsum.SetIndex("batchno");
			Relate4set Query(arjsum);
			Query.querySet((LPSTR)strQuery.c_str());
			for(int qc=Query.top();qc!=r4eof;qc=Query.skip(1L))
			 {
				bNoReset = TRUE;
			 }
			Query.unlock();
			Query.free();

			string strMsg;
			//strMsg =  "An Accounts Receivable transfer for this accounts has already been completed.\n";
			//strMsg += "If you want to continue you must adjust the General Ledger journal entry directly.\n";
			//strMsg += "Do you want to continue?";

			strMsg = GetString(63);
			strMsg += GetString(64);
			strMsg += GetString(65);

			if(bNoReset)
			{
			  int rc = MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO);
			  if(rc == IDNO)
				{
              arjsum.close();
				  return;
				}
				else
				{
					ardist->remainder.assignDouble(ardist->dAmount());
					ardist->amountpaid.assignDouble(0);
					ardist->paymentctl.assign("O");
					bReset = TRUE;
					strQuery =  "BATCHNO = ";
					strQuery += ardist->batchno.str();
					strQuery += " .AND. TYPE = 'CH'";

					Relate4set QueryDiscount(arjsum);
					QueryDiscount.querySet((LPSTR)strQuery.c_str());
					for(qc=QueryDiscount.top();qc!=r4eof;qc=QueryDiscount.skip(1L))
						{
							double dDebit = atof(arjsum.debit.str());
							double dCredit = atof(arjsum.credit.str());
							arjsum.debit.assignDouble(dCredit);
							arjsum.credit.assignDouble(dDebit);
							arjsum.mode.assign("1");
							arjsum.type.assign("RS");
						}
					QueryDiscount.unlock();
					QueryDiscount.free();
					arjsum.close();
					ShutDownWindow();
					return;
				}
			}

			ardist->remainder.assignDouble(ardist->dAmount());
			ardist->amountpaid.assignDouble(0);
			ardist->paymentctl.assign("O");
			bReset = TRUE;
			strQuery =  "BATCHNO = ";
			strQuery += ardist->batchno.str();
			strQuery += " .AND. TYPE = 'CH'";

			Relate4set QueryDiscount(arjsum);
			QueryDiscount.querySet((LPSTR)strQuery.c_str());
			for(qc=QueryDiscount.top();qc!=r4eof;qc=QueryDiscount.skip(1L))
			{
				arjsum.debit.assignDouble(0);
				arjsum.credit.assignDouble(0);
			}
			QueryDiscount.unlock();
			QueryDiscount.free();
			arjsum.close();
			ShutDownWindow();
			return;
		}

  else
  MessageBox(HWindow,GetString(6132),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
}



void TARClear::SetPartial  (RTMessage)
{
  if(SendDlgItemMsg(ID_PARTIAL,BM_GETCHECK,NULL,NULL))
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
  else
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,TRUE,NULL);
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);
}


void TARClear::SetDiscount (RTMessage)
{
  if(SendDlgItemMsg(ID_DISCOUNT,BM_GETCHECK,NULL,NULL))
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
  else
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,TRUE,NULL);
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);

}

void TARClear::SetWithholding (RTMessage)
{
  if(SendDlgItemMsg(ID_WITHHOLDING,BM_GETCHECK,NULL,NULL))
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);
  else
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,TRUE,NULL);
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
}


void TARClear::SetData(Data4 *dbf,int ID,int szField)
{
  Field4 *field = new Field4(*(Data4 *)dbf,szField);
  SetField(ID,field);
  delete field;
}

void TARClear::GetBankAccount(RTMessage Msg)
{
 GL_SETAT gl_setat;
 gl_setat.open(((TAcctModule *)GetApplication())->cb);
 gl_setat.SetIndex("name");

 char *szBankAccount = new char[gl_setat.name.len()+1];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
	long Indx =  SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_DESCRIPT,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));
	 if(gl_setat.seek(szBankAccount) == 0)
	  {
		SetField(ID_ACCOUNTS,gl_setat.account.str());
		SetField(ID_DEPART,gl_setat.dept.str());
	  }
	  SetFocus(GetDlgItem(HWindow,2017));
	}
 }
 delete [] szBankAccount;
 gl_setat.close();
}

void TARClear::SetCashAccounts()
{
 GL_SETAT gl_setat;
 gl_setat.open(((TAcctModule *)GetApplication())->cb);
 gl_setat.SetIndex("cash");
 for(gl_setat.top();!gl_setat.eof();gl_setat.skip())
	 SendDlgItemMsg(ID_DESCRIPT,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name.str());
 gl_setat.close();
}



BOOL TARClear::CheckAccount(char *szAccount, char *szDepart)
{
 BOOL bFound = FALSE;
 Tglaccts glaccts(cb);
  if(!glaccts.isValid())
  {
	ErrorClose(glaccts.alias());
	return FALSE;
  }
 Field4 accounts(glaccts,1);
 Field4 dept(glaccts,2);
 glaccts.SetIndex("accounts");
 char *szSeek = new char [accounts.len() + dept.len() + 1];
 wsprintf(szSeek,"%s%s",szAccount,szDepart);
 if(glaccts.seek(szSeek) == 0)
	bFound = TRUE;
 else
	MessageBox(HWindow,GetString(8131),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
 glaccts.close();
 delete [] szSeek;
 return bFound;
}


void TARClear::SetupWindow()
{
 TMainDialog::SetupWindow();
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
}

void TARClear::Ok(RTMessage)
  {
	 CHourGlass wait;
	 char szPaymentctl[3];
	 BOOL bClear = TRUE;
	 BOOL bDiscount = FALSE;
	 BOOL bPartial  = FALSE;
	 BOOL bWithHold = FALSE;
	 bReset = FALSE;
	 if(SendDlgItemMsg(ID_DISCOUNT,BM_GETCHECK,NULL,NULL))
		bDiscount = TRUE;
	 else
	 if(SendDlgItemMsg(ID_PARTIAL,BM_GETCHECK,NULL,NULL))
		bPartial = TRUE;
	 else
	 if(SendDlgItemMsg(ID_WITHHOLDING,BM_GETCHECK,NULL,NULL))
		bWithHold = TRUE;
	 else
		bClear = TRUE;

	 if(strcmp(ardist->paymentctl.str(),"C") == 0 )
	 {
	  MessageBox(HWindow,GetString(8127),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	  return;
	 }

	 if(bDiscount && strcmp(ardist->paymentctl.str(),"R") == 0 )
	 {
	  MessageBox(HWindow,GetString(8194),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	  return;
	 }

	 Date4 dPayment;
	 dPayment.assign(GetField(ID_DATE),GetString(4024));
	 // "CCCCMMDD"
	 dPayment.format(GetString(4023));

	 double dAmountPaid = dGetField(ID_AMOUNT);
	 double dAmount     = ardist->dAmount();
	 double dRemainder  = ardist->dRemainder();

	 if(!CheckAccount(GetField(ID_ACCOUNTS),""))
		return;

	 if(strlen(StrTrim(GetField(ID_DATE))) == 0 )
		return;

	 if(dAmountPaid == 0)
	  {
		 MessageBox(HWindow,GetString(1037),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		 return;
	  }

	 if(round(dAmountPaid,2) > round(dRemainder,2))
	 {
		 MessageBox(HWindow,GetString(8133),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		 return;
	 }

	 if(round(dAmountPaid,2) < 0 && (bPartial || bDiscount))
	 {
		 MessageBox(HWindow,GetString(8195),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		 return;
	 }

	 if(bClear)
	  strcpy(szPaymentctl,"P");

	 if(bPartial)
	  strcpy(szPaymentctl,"R");

	 if(bDiscount)
	  strcpy(szPaymentctl,"D");



	 if(bPartial && (dAmountPaid == dAmount))
		{
		MessageBox(HWindow,GetString(8134),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		return;
		}

	 if(bDiscount && (dAmountPaid == dAmount))
		{
		MessageBox(HWindow,GetString(8134),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		return;
		}


	 if((dAmountPaid < dAmount) && !bDiscount && !bPartial && !bWithHold)
		{
		switch(MessageBox(HWindow,GetString(8132),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL | MB_ICONQUESTION))
		 {
			case IDYES :strcpy(szPaymentctl,"D"); break;
			case IDNO  :strcpy(szPaymentctl,"R"); break;
			case IDCANCEL :return;
		 }
	  }

	  ardist->paymentctl.assign(szPaymentctl);
	  ardist->account.assign(StrTrim(GetField(ID_ACCOUNTS)));
	  ardist->dept.assign(StrTrim(GetField(ID_DEPART)));
	  ardist->datepaid.assign(dPayment.str());
	  ardist->mode.assign("0");
	  ardist->amountpaid.assignDouble(dAmountPaid);

	  /*
	  if(strcmp(szPaymentctl,"D") == 0)
		 {
			if((dRemainder - dAmountPaid) > 0)
			ardist->remainder.assignDouble(dRemainder - dAmountPaid);
			else
			ardist->remainder.assignDouble(0);
		 }
		*/

		szSales->assign(GetField(2016));
		szSalesDept->assign(GetField(2017));
		szVAT->assign(GetField(2018));
		szVATDept->assign(GetField(2019));
		szAR->assign(GetField(2020));
		szARDept->assign(GetField(2021));
		szWithAcct->assign(GetField(2122));
		szWithDept->assign(GetField(2123));

	  ShutDownWindow();
}



void TARCreatePayment::SelectCell(RTMessage Msg)
{
 CHourGlass wait;
 char   szInvoiceno[21];
 char   szVendorno[20];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,3,Cell->Row,szVendorno);
 SSGetData(SHWindow,4,Cell->Row,szInvoiceno);
 strcat(szVendorno,szInvoiceno);
 ardist.SetIndex("invoiceno");

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

	if(SendDlgItemMsg(ID_GLOBAL,BM_GETCHECK,NULL,NULL))
	{
		if(strcmp(ardist.paymentctl.str(),"G")  != 0)  /*Toggle the global key back and forth*/
		 {
			ardist.paymentctl.assign("G");
			SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(255,255,180), RGBCOLOR_BLUE);
		 }
		else
		 {
			ardist.paymentctl.assign("O");
			SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE, RGBCOLOR_BLACK);
		 }
	  SSSetData(SHWindow,2,Cell->Row,ardist.paymentctl.str());


	  return;
	}

	SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
	bCancel = FALSE;
	arclear = new TARClear(this,1155,szVendorno);
	arclear->ardist = &ardist;
	arclear->arcust = &arcust;
	arclear->szSales = &szSales;
	arclear->szSalesDept = &szSalesDept;
	arclear->szVAT = &szVAT;
	arclear->szVATDept = &szVATDept;
	arclear->szAR = &szAR;
	arclear->szARDept = &szARDept;
	arclear->szWithAcct = &szWithAcct;
	arclear->szWithDept = &szWithDept;
	GetApplication()->ExecDialog(arclear);
	SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE, RGBCOLOR_BLACK);
	//GetApplication()->ExecDialog(new TARClear(this,1155,szInvoiceno));
	//SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGBCOLOR_YELLOW,RGBCOLOR_BLUE);
 }
 else
 {
  MessageBox(HWindow,GetString(6131),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  return;
 }


 if(bCancel)  //If cancelled return to the parent window.
	return;

 Tarclear arclear(cb);
 if(!arclear.isValid())
	 {
		ErrorClose(arclear.GetDbfName());
		return;
	 }
	 string strBatchno(GetString(6161)); //strQuery =  "BATCHNO = ";
	 string strType(GetString(6162)); //strQuery += " .AND. TYPE = 'CH'";
	 string strQuery;
	 strQuery = strBatchno;
	 strQuery += ardist.batchno.str();
	 strQuery += strType;

	 Relate4set QueryDiscount(arclear);
	 QueryDiscount.querySet((LPSTR)strQuery.c_str());
	 for(int qc=QueryDiscount.top();qc!=r4eof;qc=QueryDiscount.skip(1L))
	 {
		arclear.deleteRec();
	 }
	 arclear.pack();
	 QueryDiscount.unlock();
	 QueryDiscount.free();
	 arclear.close();

	 ardist.SetIndex("vendorno");
	 SSSetData(SHWindow,2,Cell->Row,ardist.paymentctl.str());
	 Str4large szAmountPaid,szRemainder;
	 szAmountPaid.assignDouble(ardist.dAmountpaid(),16,2);
	 SSSetData(SHWindow,9,Cell->Row,szAmountPaid.ptr());
	 szRemainder.assignDouble(ardist.dRemainder()-ardist.dAmountpaid(),16,2);
	 SSSetData(SHWindow,10,Cell->Row,szRemainder.ptr());
	 SSClearData(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLROWS);
	  if(ardist.dAmountpaid() < ardist.dAmount() && ardist.Paid())
		SetWithHolding();
	  else
	  if(ardist.Paid() || ardist.Remainder() && !bReset)
		SetDistPaid();
	  else
	  if(ardist.Discount())
		SetDistDiscount();

}




void TARCreatePayment::Print(RTMessage)
{
  if(MessageBox(HWindow,GetString(4076),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION) == IDYES)
  PrintPaymentList();
}

class TClearReceivable : public TWindow
{
 public:
 TARCreatePayment* Form;
 TClearReceivable(TWindowsObject *parent,LPSTR ATitle) : TWindow(parent,""){}
 virtual void SetupWindow();
 virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual BOOL CanClose();
 char szlpAWindow[10];
 BOOL SetWindowPosition();
 BOOL SaveWindowPosition();
 int iLeft;
 int iTop;
 int iRight;
 int iBottom;
};

BOOL TClearReceivable::SetWindowPosition()
{
 RECT rc,rcDesktop;
 GetWindowRect(GetDesktopWindow(),&rcDesktop);
 GetWindowRect(HWindow,&rc);
 if(iRight == 0 || iRight > rcDesktop.right)
	iRight = rc.right;
 if(iBottom == 0 || iBottom > rcDesktop.bottom)
 iBottom = rc.bottom;
 int check = iBottom - iTop;
 if(check < 0)
  {
	 iBottom = 600;
	 iTop = 200;
  }
 //return MoveWindow(Parent->HWindow,iLeft,iTop,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 return MoveWindow(HWindow,0,0,iRight-iLeft,iBottom-iTop,TRUE);

}

BOOL TClearReceivable::SaveWindowPosition()
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 Str4ten szLeft;
 Str4ten szTop;
 Str4ten szRight;
 Str4ten szBottom;
 szLeft.assignLong(4,5);
 szTop.assignLong(rc.top,5);
 szRight.assignLong(rc.right,5);
 szBottom.assignLong(rc.bottom,5);
 WritePrivateProfileString(szlpAWindow,"L",szLeft.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"T",szTop.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"R",szRight.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"B",szBottom.str(),PROFILE);
 return TRUE;

}


BOOL TClearReceivable::CanClose()
{
  return TRUE;
}

void TClearReceivable::WMSize(RTMessage Msg)
{
 int HEADER_HEIGHT = 100;
 TWindow::WMSize(Msg);
 RECT rc;
 GetClientRect(HWindow,&rc);
 MoveWindow(GetDlgItem(Form->HWindow,ID_DISTLIST),rc.left,rc.top+HEADER_HEIGHT,
				 LOWORD(Msg.LParam),rc.bottom-(rc.top+HEADER_HEIGHT),TRUE);
 MoveWindow(Form->HWindow,0,0,LOWORD(Msg.LParam),HIWORD(Msg.LParam),TRUE);
 Form->InvoiceList->GetClientRect(&rc);
 short d2Width,d3Width,d4Width,d5Width,d6Width,d7Width,d8Width,d9Width,d10Width;
 Form->InvoiceList->SetMaxCols(10);
 Form->InvoiceList->SetColWidth(2,5);
 Form->InvoiceList->SetColWidth(3,6);
 Form->InvoiceList->SetColWidth(4,9);
 Form->InvoiceList->SetColWidth(5,9);
 Form->InvoiceList->SetColWidth(6,10);
 Form->InvoiceList->SetColWidth(7,10);
 Form->InvoiceList->SetColWidth(8,10);
 Form->InvoiceList->SetColWidth(9,10);
 Form->InvoiceList->SetColWidth(10,10);

 Form->InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 Form->InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 Form->InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 Form->InvoiceList->GetColWidthInPixels(5,(LPSHORT)&d5Width);
 Form->InvoiceList->GetColWidthInPixels(6,(LPSHORT)&d6Width);
 Form->InvoiceList->GetColWidthInPixels(7,(LPSHORT)&d7Width);
 Form->InvoiceList->GetColWidthInPixels(8,(LPSHORT)&d8Width);
 Form->InvoiceList->GetColWidthInPixels(9,(LPSHORT)&d9Width);
 Form->InvoiceList->GetColWidthInPixels(10,(LPSHORT)&d10Width);


 short ColWidth = (rc.right - rc.left)
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width
		  - d6Width
		  - d7Width
		  - d8Width
		  - d9Width
		  - d10Width;

 Form->InvoiceList->SetColWidthInPixels(1,ColWidth);

}

void TClearReceivable::SetupWindow()
{
 strcpy(szlpAWindow,"AR_CLEAR");
 iLeft = iTop = iRight = iBottom = 0;
 iLeft    = GetPrivateProfileInt(szlpAWindow,"L",0,PROFILE);
 iTop     = GetPrivateProfileInt(szlpAWindow,"T",0,PROFILE);
 iRight   = GetPrivateProfileInt(szlpAWindow,"R",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szlpAWindow,"B",0,PROFILE);

 RECT rcDesktop;
 GetWindowRect(GetDesktopWindow(),&rcDesktop);
 if((iBottom - iTop)  > (rcDesktop.bottom - rcDesktop.top) ||  (iRight -iLeft) > (rcDesktop.right - rcDesktop.left))
	 {
	  iTop = 0;
	  iBottom = 0;
	  iLeft = 0;
	  iRight = 300;
	 }


 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 200;
 }

  Form = new TARCreatePayment(this,1156);
  Form->EnableAutoCreate();
  GetApplication()->MakeWindow(Form);
  Form->Show(SW_SHOW);
  SetCaption("Clear Accounts Receivable Entry");
  SetWindowPosition();
}


void TManilaGL::ARCreateSlip(RTMessage)
{
 //GetApplication()->ExecDialog(new TARCreatePayment(this,1156));
 TClearReceivable *Clear = new TClearReceivable(this,"");
 GetApplication()->MakeWindow(Clear);
}

void TARCreatePayment::SetWithHolding()
{
 CHourGlass wait;
 double dOriginalAR    = ardist.dAmount();
 double dVatPercent = 0;
 if(ardist.dVatamt() > 0)
  dVatPercent = ardist.dVatamt()/(dOriginalAR-ardist.dVatamt());
 else
  dVatPercent = 0;
 double dOriginalSales = dOriginalAR/(1+dVatPercent);
 double dOriginalVat   = dOriginalAR - dOriginalSales;
 double dAmountPaid    = ardist.dAmountpaid();             //CASH
 double dNewSales      = dAmountPaid/(1+dVatPercent);
 double dNewVat        = dAmountPaid - dNewSales;
 double dSalesDiscount;
 double dVatDiscount   = dOriginalVat - dNewVat;
 if(dVatPercent == 0)
  {
	dSalesDiscount = 0;
	dVatDiscount = dOriginalAR - dAmountPaid;
  }
 else
  dSalesDiscount 	= dOriginalSales - dNewSales;



 /*
	 Change the second and the third entries
	 from gross sales to vat
	 and vat to withholding tax.

 */

 HWND hWnd = GetDlgItem(D->HWindow,ID_DISTLIST);
 SSSetBool(hWnd,SSB_REDRAW,FALSE);
 SSClearData(hWnd,SS_ALLCOLS,SS_ALLROWS);
 // Cash Account
 Str4large szCash;
 szCash.assignDouble(fabs(dAmountPaid),16,2);
 char szDescript[51];
 strcpy(szDescript,GetDescription(ardist.account.str(),ardist.dept.str()));

 int line = 1;
 char szLine[3];
 itoa(line,szLine,10);

 SSSetData(hWnd,1,1,szLine);
 SSSetData(hWnd,2,1,szDescript);
 SSSetData(hWnd,3,1,ardist.account.str());
 SSSetData(hWnd,4,1,ardist.dept.str());
 SSSetData(hWnd,5,1,szCash.str());
 // Create an entry for the cash entry in the AR Journal Summary Database.
  Tarclear arclear(cb);
  if(!arclear.isValid())
  {
	 ErrorClose(arclear.GetDbfName());
	 return;
  }
  arclear.appendStart();
  arclear.account.assign      (ardist.account.str());
  arclear.dept.assign         (ardist.dept.str());
  arclear.period.assign       (ardist.period.str());
  arclear.mode.assign         ("1");
  arclear.date.assign         (ardist.date.str());
  arclear.type.assign         ("CH");
  arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
  arclear.reference.assign    (ardist.reference.str());
  arclear.descript.assign     (ardist.descript.str());
  arclear.debit.assignDouble  (fabs(dAmountPaid));
  arclear.credit.assignDouble (0.0);
  arclear.append();

 if(dSalesDiscount > 0)
 {
 // Vat Amount
 line++;
 itoa(line,szLine,10);
 Str4large szSalesDiscount;
 szSalesDiscount.assignDouble(fabs(dSalesDiscount),16,2);
 strcpy(szDescript,GetDescription((LPSTR)szVAT.c_str(),(LPSTR)szVATDept.c_str()));
 SSSetData(hWnd,1,line,szLine);
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,(LPSTR)szVAT.c_str());
 SSSetData(hWnd,4,line,(LPSTR)szVATDept.c_str());
 SSSetData(hWnd,5,line,szSalesDiscount.str());

 arclear.appendStart();
 arclear.account.assign      ((LPSTR)szVAT.c_str());
 arclear.dept.assign         ((LPSTR)szVATDept.c_str());
 arclear.period.assign       (ardist.period.str());
 arclear.date.assign         (ardist.date.str());
 arclear.mode.assign         ("1");
 arclear.type.assign         ("CH");
 arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
 arclear.reference.assign    (ardist.reference.str());
 arclear.descript.assign     (GetDescription((LPSTR)szVAT.c_str(),(LPSTR)szVATDept.c_str()));
 arclear.debit.assignDouble  (fabs(dSalesDiscount));
 arclear.credit.assignDouble (0);
 arclear.append();
 }
 //WithHolding Tax

 line++;
 itoa(line,szLine,10);
 Str4large szVatDiscount;
 strcpy(szDescript,GetDescription((LPSTR)szWithAcct.c_str(),(LPSTR)szWithDept.c_str()));
 szVatDiscount.assignDouble(fabs(dVatDiscount),16,2);
 SSSetData(hWnd,1,line,szLine);
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,(LPSTR)szWithAcct.c_str());
 SSSetData(hWnd,4,line,(LPSTR)szWithDept.c_str());
 SSSetData(hWnd,5,line,szVatDiscount.str());
 arclear.appendStart();
 arclear.account.assign      ((LPSTR)szWithAcct.c_str());
 arclear.dept.assign         ((LPSTR)szWithDept.c_str());
 arclear.period.assign       (ardist.period.str());
 arclear.date.assign         (ardist.date.str());
 arclear.mode.assign         ("1");
 arclear.type.assign         ("CH");
 arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
 arclear.reference.assign    (ardist.reference.str());
 arclear.descript.assign     (GetDescription((LPSTR)szWithAcct.c_str(),(LPSTR)szWithDept.c_str()));
 arclear.debit.assignDouble  (fabs(dVatDiscount));
 arclear.credit.assignDouble (0);
 arclear.append();

 // AR Account
 line++;
 itoa(line,szLine,10);

 Str4large szOriginalAR;
 szOriginalAR.assignDouble(fabs(dOriginalAR),16,2);
 strcpy(szDescript,GetDescription((LPSTR)szAR.c_str(),(LPSTR)szARDept.c_str()));
 SSSetData(hWnd,1,line,szLine);
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,ardist.apaccount.str());
 SSSetData(hWnd,4,line,ardist.apdepart.str());
 SSSetData(hWnd,6,line,szOriginalAR.ptr());
 // AR Amount
 arclear.appendStart();
 arclear.account.assign      (ardist.apaccount.str());
 arclear.dept.assign         (ardist.apdepart.str());
 arclear.period.assign       (ardist.period.str());
 arclear.date.assign         (ardist.date.str());
 arclear.mode.assign         ("1");
 arclear.type.assign         ("CH");
 arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
 arclear.reference.assign    (ardist.reference.str());
 arclear.descript.assign     (ardist.descript.str());
 arclear.debit.assignDouble  (0);
 arclear.credit.assignDouble (fabs(dOriginalAR));
 arclear.append();
 SSSetBool(hWnd,SSB_REDRAW,TRUE);
 arclear.close();
}

void TARCreatePayment::PostClearedInvoices()
{
  BOOL bDelete = FALSE;;
  Tarclear arclear(cb);
  if(!arclear.isValid())
  {
		ErrorClose(arclear.GetDbfName());
		return;
  }

  if(arclear.recCount() ==  0)
  {
	 arclear.close();
	 return;
  }

  Tarjsum arjsum(cb);
  if(!arjsum.isValid())
  {
		ErrorClose(arjsum.GetDbfName());
		return;
  }
  arjsum.SetIndex("batchno");


  for (arclear.top();!arclear.eof();arclear.skip())
  {
	if(arjsum.seek(double(Str4ptr(arclear.batchno.str()))) == 0)
	{
	  if(strcmp(arjsum.type.str(),"AR") != 0)
	  {
			bDelete = TRUE;
			arjsum.appendStart();
			arjsum.account.assign      (arclear.account.str());
			arjsum.dept.assign         (arclear.dept.str());
			arjsum.period.assign       (arclear.period.str());
			arjsum.mode.assign         (arclear.mode.str());
			arjsum.date.assign         (arclear.date.str());
			arjsum.type.assign         (arclear.type.str());
			arjsum.batchno.assignDouble(double(Str4ptr(arclear.batchno.str())));
			arjsum.reference.assign    (arclear.reference.str());
			arjsum.descript.assign     (arclear.descript.str());
			arjsum.debit.assignDouble  (fabs(double(Str4ptr(arclear.debit.str()))));
			arjsum.credit.assignDouble (fabs(double(Str4ptr(arclear.credit.str()))));
			arjsum.UpdateUserTime(szUser);
			arjsum.append();
		}
	 }
	else
	  {
			bDelete = TRUE;
			arjsum.appendStart();
			arjsum.account.assign      (arclear.account.str());
			arjsum.dept.assign         (arclear.dept.str());
			arjsum.period.assign       (arclear.period.str());
			arjsum.mode.assign         (arclear.mode.str());
			arjsum.date.assign         (arclear.date.str());
			arjsum.type.assign         (arclear.type.str());
			arjsum.batchno.assignDouble(double(Str4ptr(arclear.batchno.str())));
			arjsum.reference.assign    (arclear.reference.str());
			arjsum.descript.assign     (arclear.descript.str());
			arjsum.debit.assignDouble  (fabs(double(Str4ptr(arclear.debit.str()))));
			arjsum.credit.assignDouble (fabs(double(Str4ptr(arclear.credit.str()))));
			arjsum.UpdateUserTime(szUser);
			arjsum.append();
		}
  }
  if(bDelete)
  {
	arclear.zap();
	arclear.pack();
  }
  arclear.close();
  arjsum.close();
}


void TARCreatePayment::SetDistPaid()
{
  CHourGlass wait;
  HWND hWnd = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSSetBool(hWnd,SSB_REDRAW,FALSE);
  SSClearData(hWnd,SS_ALLCOLS,SS_ALLROWS);
  string szDescript(GetDescription(ardist.account.str(),""));

  double dAmount = atof(ardist.amountpaid.str());
  Str4large strAmount;
  strAmount.assignDouble(fabs(dAmount),16,2);

  if(dAmount > 0)
  {
	  SSSetData(hWnd,1,1,"1");
	  SSSetData(hWnd,2,1,(char *)szDescript.c_str());
	  SSSetData(hWnd,3,1,ardist.account.str());
	  SSSetData(hWnd,4,1,ardist.dept.str());
	  SSSetData(hWnd,5,1,strAmount.str());
	  szDescript = GetDescription(ardist.apaccount.str(),"");
	  SSSetData(hWnd,1,2,"2");
	  SSSetData(hWnd,2,2,(char *) szDescript.c_str());
	  SSSetData(hWnd,3,2,ardist.apaccount.str());
	  SSSetData(hWnd,4,2,ardist.apdepart.str());
	  SSSetData(hWnd,6,2,strAmount.str());
  }
  else
  {

	  SSSetData(hWnd,1,2,"2");
	  SSSetData(hWnd,2,2,(char *) szDescript.c_str());
	  SSSetData(hWnd,3,2,ardist.apaccount.str());
	  SSSetData(hWnd,4,2,ardist.apdepart.str());
	  SSSetData(hWnd,6,2,strAmount.str());

	  szDescript = GetDescription(ardist.apaccount.str(),"");
	  SSSetData(hWnd,1,1,"1");
	  SSSetData(hWnd,2,1,(char *)szDescript.c_str());
	  SSSetData(hWnd,3,1,ardist.account.str());
	  SSSetData(hWnd,4,1,ardist.dept.str());
	  SSSetData(hWnd,5,1,strAmount.str());
  }


  ardist.mode.assign("1");
  double dAmountPaid  = ardist.dAmountpaid();
  double dCurrentRemainder = ardist.dRemainder() - ardist.dAmountpaid();
  ardist.remainder.assignDouble(dCurrentRemainder);
  ardist.amountpaid.assignDouble(0);

  Tarclear arclear(cb);
  if(!arclear.isValid())
  {
	 ErrorClose(arclear.GetDbfName());
	 return;
  }

  arclear.appendStart();
  arclear.account.assign      (ardist.account.str());
  arclear.dept.assign         (ardist.dept.str());
  arclear.period.assign       (ardist.period.str());
  arclear.mode.assign         ("1");
  arclear.date.assign         (ardist.date.str());
  arclear.type.assign         ("CH");
  arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
  arclear.reference.assign    (ardist.reference.str());
  arclear.descript.assign     (ardist.descript.str());
  if(dAmountPaid > 0 )
  {
	  arclear.debit.assignDouble  (fabs(dAmountPaid));
	  arclear.credit.assignDouble (0.0);
  }
  else
  {
	  arclear.debit.assignDouble  (0.0);
	  arclear.credit.assignDouble (fabs(dAmountPaid));
  }
  arclear.append();

  arclear.appendStart();
  arclear.account.assign      (ardist.apaccount.str());
  arclear.dept.assign         (ardist.apdepart.str());
  arclear.period.assign       (ardist.period.str());
  arclear.date.assign         (ardist.date.str());
  arclear.mode.assign         ("1");
  arclear.type.assign         ("CH");
  arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
  arclear.reference.assign    (ardist.reference.str());
  arclear.descript.assign     (ardist.descript.str());
  if(dAmountPaid > 0)
  {
	  arclear.debit.assignDouble  (0.0);
	  arclear.credit.assignDouble (fabs(dAmountPaid));
  }
  else
  {
	  arclear.debit.assignDouble  (fabs(dAmountPaid));
	  arclear.credit.assignDouble (0.0);
  }

  arclear.append();

  SSSetBool(hWnd,SSB_REDRAW,TRUE);
  arclear.close();
}

#pragma argsused
char *TARCreatePayment::GetDescription(char *szAccount, char *szDept)
{
 Data4 glaccts(cb,"glaccts");
 string szDescript;
 glaccts.SetIndex(GetString(8159));
 Field4 account(glaccts,1);
 Field4 depart(glaccts,2);
 Field4 descript(glaccts,3);

 Str4large szSeek;
 szSeek.setLen (account.len()+ depart.len());
 szSeek.set(' ');
 szSeek.replace(Str4ptr(szAccount));
 szSeek.replace(Str4ptr(""),account.len());
 if(glaccts.seek(szSeek.str()) == 0)
  {
	 szDescript = descript.str();
  }
 glaccts.close();
 return (char *) szDescript.c_str();
}

void TARCreatePayment::Seek(RTMessage)
{
 string strQuery;
 strQuery = GetString(6122);

 if(SendDlgItemMsg(ID_OPEN,BM_GETCHECK,NULL,NULL))
 {
  strQuery = GetString(6122);
 }
 else
 if(SendDlgItemMsg(ID_PAID,BM_GETCHECK,NULL,NULL))
 {
  strQuery = GetString(6163);
 }
 else
 if(SendDlgItemMsg(ID_ALL,BM_GETCHECK,NULL,NULL))
 {
  strQuery = GetString(6164);
 }


 string strInvoice(GetApplication()->hInstance,6159);
 string strVendor(GetApplication()->hInstance,6160);

  if(strlen(StrTrim(GetField(ID_INVOICENO))) > 0 &&
	 strlen(StrTrim(GetField(ID_VENDORNO))) > 0)
  {
		//strQuery += ".and. invoiceno = '";
		strQuery += strInvoice;
		strQuery += GetField(ID_INVOICENO);
		strQuery += "'";
		//strQuery += ".and. vendorno = '";
		strQuery += strVendor;
		strQuery += GetField(ID_VENDORNO);
		strQuery += "'";
 }
 else
 if(strlen(StrTrim(GetField(ID_INVOICENO))) > 0)
 {
	//strQuery += ".and. invoiceno = '";
	strQuery += strInvoice;
	strQuery += GetField(ID_INVOICENO);
	strQuery += "'";
 }
 else
 if(strlen(StrTrim(GetField(ID_VENDORNO))) > 0)
 {
	//strQuery += ".and. vendorno = '";
	strQuery += strVendor;
	strQuery += GetField(ID_VENDORNO);
	strQuery += "'";
 }

 SetField(IDD_STATUSBOX,(LPSTR)strQuery.c_str());
 bHasInvoices  =  DistList((LPSTR)strQuery.c_str());
}


void TARCreatePayment::SetDistDiscount()
{
 CHourGlass wait;
 double dOriginalAR    = ardist.dAmount(); //  - ardist.dRemainder();
 double dVatPercent    = ardist.dVatamt()/(ardist.dAmount()-ardist.dVatamt());
 double dOriginalSales = dOriginalAR/(1+dVatPercent);
 double dOriginalVat   = dOriginalAR - dOriginalSales;
 double dAmountPaid    = ardist.dAmountpaid();             //CASH
 double dNewSales      = dAmountPaid/(1+dVatPercent);
 double dNewVat        = dAmountPaid - dNewSales;
 double dSalesDiscount = dOriginalSales - dNewSales;
 double dVatDiscount   = dOriginalVat - dNewVat;
 int line =1 ;
 HWND hWnd = GetDlgItem(D->HWindow,ID_DISTLIST);
 SSSetBool(hWnd,SSB_REDRAW,FALSE);
 SSClearData(hWnd,SS_ALLCOLS,SS_ALLROWS);
 // Cash Account
 Str4large szCash;
 szCash.assignDouble(fabs(dAmountPaid),16,2);
 char szDescript[51];
 strcpy(szDescript,GetDescription(ardist.account.str(),ardist.dept.str()));
 SSSetData(hWnd,1,line,"1");
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,ardist.account.str());
 SSSetData(hWnd,4,line,ardist.dept.str());
 SSSetData(hWnd,5,line,szCash.str());
 // Create an entry for the cash entry in the AR Journal Summary Database.
	Tarclear arclear(cb);
  if(!arclear.isValid())
  {
	 ErrorClose(arclear.GetDbfName());
	 return;
  }

  arclear.appendStart();
  arclear.account.assign      (ardist.account.str());
  arclear.dept.assign         (ardist.dept.str());
  arclear.period.assign       (ardist.period.str());
  arclear.mode.assign         ("1");
  arclear.date.assign         (ardist.date.str());
  arclear.type.assign         ("CH");
  arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
  arclear.reference.assign    (ardist.reference.str());
  arclear.descript.assign     (ardist.descript.str());
  arclear.debit.assignDouble  (fabs(dAmountPaid));
  arclear.credit.assignDouble (0.0);
  arclear.UpdateUserTime(szUser);
  arclear.append();

// Sales Account
 line++;
 Str4large szSalesDiscount;
 szSalesDiscount.assignDouble(fabs(dSalesDiscount),16,2);
 strcpy(szDescript,GetDescription((LPSTR)szSales.c_str(),(LPSTR)szSalesDept.c_str()));
 SSSetData(hWnd,1,line,"2");
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,(LPSTR)szSales.c_str());
 SSSetData(hWnd,4,line,(LPSTR)szSalesDept.c_str());
 SSSetData(hWnd,5,line,szSalesDiscount.str());

 arclear.appendStart();
 arclear.account.assign      ((LPSTR)szSales.c_str());
 arclear.dept.assign         ((LPSTR)szSalesDept.c_str());
 arclear.period.assign       (ardist.period.str());
 arclear.date.assign         (ardist.date.str());
 arclear.mode.assign         ("1");
 arclear.type.assign         ("CH");
 arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
 arclear.reference.assign    (ardist.reference.str());
 arclear.descript.assign     (GetDescription((LPSTR)szSales.c_str(),(LPSTR)szSalesDept.c_str()));
 arclear.debit.assignDouble  (fabs(dSalesDiscount));
 arclear.credit.assignDouble (0);
 arclear.append();

 //VAT Account
 if(dVatDiscount > 0)
 {
 line++;
 Str4large szVatDiscount;
 strcpy(szDescript,GetDescription((LPSTR)szVAT.c_str(),(LPSTR)szVATDept.c_str()));
 szVatDiscount.assignDouble(fabs(dVatDiscount),16,2);
 SSSetData(hWnd,1,line,"3");
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,(LPSTR)szVAT.c_str());
 SSSetData(hWnd,4,line,(LPSTR)szVATDept.c_str());
 SSSetData(hWnd,5,line,szVatDiscount.str());
 arclear.appendStart();
 arclear.account.assign      ((LPSTR)szVAT.c_str());
 arclear.dept.assign         ((LPSTR)szVATDept.c_str());
 arclear.period.assign       (ardist.period.str());
 arclear.date.assign         (ardist.date.str());
 arclear.mode.assign         ("1");
 arclear.type.assign         ("CH");
 arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
 arclear.reference.assign    (ardist.reference.str());
 arclear.descript.assign     (GetDescription((LPSTR)szVAT.c_str(),(LPSTR)szVATDept.c_str()));
 arclear.debit.assignDouble  (fabs(dVatDiscount));
 arclear.credit.assignDouble (0);
 arclear.append();
 }
 // AR Account
 line++;
 Str4large szOriginalAR;
 szOriginalAR.assignDouble(fabs(dOriginalAR),16,2);
 strcpy(szDescript,GetDescription((LPSTR)szAR.c_str(),(LPSTR)szARDept.c_str()));
 string strLine;
 ltoa(line,(LPSTR)strLine.c_str(),10);
 SSSetData(hWnd,1,line,(LPSTR)strLine.c_str());
 SSSetData(hWnd,2,line,szDescript);
 SSSetData(hWnd,3,line,ardist.apaccount.str());
 SSSetData(hWnd,4,line,ardist.apdepart.str());
 SSSetData(hWnd,6,line,szOriginalAR.ptr());
 // AR Amount
 arclear.appendStart();
 arclear.account.assign      (ardist.apaccount.str());
 arclear.dept.assign         (ardist.apdepart.str());
 arclear.period.assign       (ardist.period.str());
 arclear.date.assign         (ardist.date.str());
 arclear.mode.assign         ("1");
 arclear.type.assign         ("CH");
 arclear.batchno.assignDouble(double(Str4ptr(ardist.batchno.str())));
 arclear.reference.assign    (ardist.reference.str());
 arclear.descript.assign     (ardist.descript.str());
 arclear.debit.assignDouble  (0);
 arclear.credit.assignDouble (fabs(dOriginalAR));
 arclear.append();
 SSSetBool(hWnd,SSB_REDRAW,TRUE);
 arclear.close();
}


void TARCreatePayment::Ok (RTMessage)
{
	if(MessageBox(HWindow,GetString(6165),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
	 return;

	CHourGlass wait;
	ardist.SetIndex(GetString(8140));
	if(ardist.dRemainder() == 0)
	 ardist.paymentctl.assign("C");
	ardist.UpdateUserTime(szUser);
	Relate4set rQuery(ardist);
	//"PAYMENTCTL = 'P'"
	string szQuery(GetString(6109));
	rQuery.querySet((char *)szQuery.c_str());
	for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 {
	 // If the total amount of the invoice has been paid
	 // make the invoice a cleared invoice.

		ardist.paymentctl.assign("C");
		ardist.remainder.assignDouble(0);
		ardist.UpdateUserTime(szUser);
	 }
	 rQuery.unlock();
	 rQuery.free();

	Relate4set pQuery(ardist);
	// "PAYMENTCTL = 'R' .AND. AMOUNTPAID > 0"
	szQuery = GetString(4031);
	pQuery.querySet((char *)szQuery.c_str());
	 for(qc=pQuery.top();qc!=r4eof;qc=pQuery.skip(1L))
	  {
		// If the total amount of the invoice has been paid
		// make the invoice a cleared invoice.
		  ardist.mode.assign("1");
		  double dCurrentRemainder = ardist.dRemainder() - ardist.dAmountpaid();
		  ardist.remainder.assignDouble(dCurrentRemainder);
		  ardist.amountpaid.assignDouble(0);
		  if(ardist.dRemainder() == 0)
			ardist.paymentctl.assign("C");
		  ardist.UpdateUserTime(szUser);
	  }
	pQuery.unlock();
	pQuery.free();

	Relate4set rDiscount(ardist);
	szQuery = GetString(8160);
	//"PAYMENTCTL = 'D'"
	rDiscount.querySet((char *)szQuery.c_str());
	for(qc=rDiscount.top();qc!=r4eof;qc=rDiscount.skip(1L))
	  {
		  ardist.remainder.assignDouble(0);
		  ardist.paymentctl.assign("C");
		  ardist.UpdateUserTime(szUser);
	  }
	rDiscount.unlock();
	rDiscount.free();
 //szQuery = GetString(8161);
 //DistList((char *)szQuery.c_str());
 return;
}
#pragma argsused
void TARCreatePayment::SetRequestNumber(char *szVendorno,int iRequestNumber){}

void TARCreatePayment::SetDate (RTMessage)
{
 // "PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O'"
 string szSearch(GetString(6021));
 bHasInvoices = DistList((char *)szSearch.c_str());
 if(!bHasInvoices)
  MessageBox(HWindow,GetString(4074),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
}

void TARCreatePayment::ViewAll(RTMessage)
{
 string  NotDeleted = GetString(8162);
 DistList((char *)NotDeleted.c_str());
}

void TARCreatePayment::SetupWindow()
{
 TMainDialog::SetupWindow();
 D = new TARDistribution(this,6067);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);

 CHourGlass wait;
 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetBool(SSB_VERTSCROLLBAR,TRUE);

 InvoiceList->SetMaxRows(1000);
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 InvoiceList->SetData(1,SS_HEADER, GetString(6117));
 InvoiceList->SetData(2,SS_HEADER, GetString(ID_CONTROL));
 InvoiceList->SetData(3,SS_HEADER, GetString(ID_VENDORCODE));
 InvoiceList->SetData(4,SS_HEADER, GetString(ID_INVOICENO));
 InvoiceList->SetData(5,SS_HEADER, GetString(ID_DUEDATE));
 InvoiceList->SetData(6,SS_HEADER, GetString(ID_AMOUNT));
 InvoiceList->SetData(7,SS_HEADER, GetString(6118));
 InvoiceList->SetData(8,SS_HEADER, GetString(6119));
 InvoiceList->SetData(9,SS_HEADER, GetString(6120));
 InvoiceList->SetData(10,SS_HEADER,GetString(6121)  );


 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_RIGHT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,atof(GetString(6107)));
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(7, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(8, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(9, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(10, SS_ALLROWS, &CellType);


  // Open databases...
  if(ardist.open(*cb) < 0)
  {
	ErrorClose("AR_DIST");
	return;
  }

  int rc = ardist.lockAll();
  if(rc != 0)
	 ShutDownWindow();
  

  if(!arcust.open(*cb) < 0)
  {
	ErrorClose(arcust.GetDbfName());
	return;
  }


  if(arbtch.open(*cb) < 0)
  {
	ErrorClose(arbtch.GetDbfName());
	return;
  }


 bHasInvoices = FALSE;

 D->Show(SW_SHOW);
 SetBitmap(ID_VIEW);
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
 SetBitmap(ID_SETDATA);
 SetBitmap(ID_SEARCH);
 SetBitmap(ID_VENDORNO);
 SetBitmap(ID_SET);
 SetBitmap(ID_GLOBAL_CLEARING);
 SendDlgItemMsg(ID_OPEN,BM_SETCHECK,TRUE,NULL);
}

void TARCreatePayment::Cancel  (RTMessage)
{
 CHourGlass Wait;
 PostClearedInvoices();
 ((TClearReceivable *) Parent)->SaveWindowPosition();
 ShutDownWindow();
 Parent->ShutDownWindow();
}


void TARCreatePayment::Redraw(RTMessage)
{
  if(!SendMessage(GetDlgItem(HWindow,ID_REDRAW),BM_GETCHECK,NULL,NULL))
	  SendMessage(GetDlgItem(HWindow,ID_REDRAW),BM_SETCHECK,TRUE,NULL);
  else
	  SendMessage(GetDlgItem(HWindow,ID_REDRAW),BM_SETCHECK,FALSE,NULL);
}

BOOL TARCreatePayment::DistList(char *szQuery)
{
 CHourGlass wait;
 int l = 1;
 //char szControl[2];

 Date4 dPayment;
 dPayment.assign(GetField(ID_DATE),GetString(4024));
 dPayment.format(GetString(4023));
 BOOL bRedraw = TRUE;
 if(!SendMessage(GetDlgItem(HWindow,ID_REDRAW),BM_GETCHECK,NULL,NULL))
	{
	 InvoiceList->SetBool(SSB_REDRAW,FALSE);
	 bRedraw = FALSE;
	}


 if(ardist.recCount() > 16)
  InvoiceList->SetMaxRows(ardist.recCount());
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 ardist.SetIndex("invoiceno");
 Relate4set rQuery(ardist);
 rQuery.querySet(szQuery);
 InvoiceList->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 char szDate[11];
	 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 {
		 Date4 dDueDate(ardist.duedate.str());
		 arcust.seek(ardist.vendorno.str());
		 //if(bRedraw)
		 //	 InvoiceList->SetBool(SSB_REDRAW,FALSE);
		 InvoiceList->SetData(1,l,arcust.vdshort.str());  // Vendor Name
		 InvoiceList->SetData(2,l,ardist.paymentctl.str()); // Payment Control
		 InvoiceList->SetData(3,l,ardist.vendorno.str());  // Vendor Code
		 InvoiceList->SetData(4,l,ardist.invoiceno.str()); // Invoice Number
		 Date4 dDate(ardist.duedate.str());
		 dDate.format(szDate,GetString(4024));
		 InvoiceList->SetData(5,l,szDate);   // Due Date
		 Str4large szSales;
		 double dSalesAmount = ardist.dAmount()-ardist.dVatamt();
				szSales.assignDouble(dSalesAmount,16,2);
		 InvoiceList->SetData(6,l,szSales.str());   // Invoice Amount
		 InvoiceList->SetData(7,l,ardist.vatamt.str()); // VAT
		 Str4large szRemainder;
		 szRemainder.assignDouble(ardist.dRemainder(),16,2);

		 Str4large szAmount;
		 szAmount.assignDouble(ardist.dAmount(),16,2);

		 InvoiceList->SetData(8,l,szAmount.str());
		 Str4large szAmountPaid;
		 szAmountPaid.assignDouble(ardist.dAmountpaid(),16,2);
		 InvoiceList->SetData(9,l,szAmountPaid.ptr());
		 InvoiceList->SetData(10,l,szRemainder.ptr());
		 if(strcmp(ardist.paymentctl.str(),"C") == 0)
		  InvoiceList->SetColor(SS_ALLCOLS,l,RGB(255,255,201),RGBCOLOR_BLUE);
		 if(strcmp(ardist.paymentctl.str(),"G") == 0)
		  InvoiceList->SetColor(SS_ALLCOLS,l,RGB(255,255,180),RGBCOLOR_BLUE);

		 //InvoiceList->SetColor(SS_ALLCOLS,l,RGBCOLOR_GREEN,RGBCOLOR_BLACK);

		 if(bRedraw)
			{
			InvoiceList->ShowCell(SS_ALLCOLS,l,SS_SHOW_NEAREST);
			//InvoiceList->SetBool(SSB_REDRAW,TRUE);
			}
		 l++;
		 //  }
	 }
	 rQuery.unlock();
	 rQuery.free();
 //InvoiceList->ShowCell(SS_ALLCOLS,l,1);
 //InvoiceList->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE, RGBCOLOR_BLACK);
 InvoiceList->SetBool(SSB_REDRAW,TRUE);
 if(l>24)
 InvoiceList->SetMaxRows(l--);
 else
 InvoiceList->SetMaxRows(24);
 SetField(IDD_STATUSBOX,GetString(6020));

 if(l>1)
 return TRUE;
 else
 return FALSE;
}


void TARCreatePayment::PrintPaymentList(void)
{
  //PrintCR(GetApplication()->MainWindow->HWindow,"ap_reqst.rpt"))
  return;
}


void TARDistribution::SetupWindow()
{
 TDialog::SetupWindow();
 CHourGlass wait;
 int Rows = 8;
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 short iHeight;
 HFONT   hFont;
 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 SSSetBool(SHWindow,SSB_AUTOSIZE,FALSE);
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetAutoSizeVisible(SHWindow,6,Rows);

 SSGetRowHeightInPixels(SHWindow,1,&iHeight);
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ 2*GetSystemMetrics(SM_CXDLGFRAME) + (Rows*iHeight) + 1;
 GetWindowRect(HWindow,&wd);
 MoveWindow(HWindow,wd.left,wd.top,wd.right-wd.left,Height,TRUE);
 GetClientRect(HWindow,&rc);
 GetWindowRect(GetDlgItem(HWindow,IDD_STATUSBOX),&wd);
 MoveWindow(SHWindow,rc.left+1, rc.top, rc.right - (wd.right-wd.left) , rc.bottom - rc.top, TRUE);
 GetWindowRect(SHWindow,&rc);
 MoveWindow(GetDlgItem(HWindow,IDD_STATUSBOX),rc.right,rc.top,rc.right-rc.left -(rc.right - (wd.right-wd.left)),rc.bottom-rc.top,TRUE);
 SSSetMaxRows(SHWindow,Rows+1);


 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);


 SSSetMaxCols(SHWindow, 6);
 SSGetClientRect(SHWindow,&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width;

 SSSetColWidth(SHWindow,1,3);
 SSSetColWidth(SHWindow,3,8);
 SSSetColWidth(SHWindow,4,5);
 SSSetColWidth(SHWindow,5,10);
 SSSetColWidth(SHWindow,6,10);

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d0Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d2Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d3Width);
 SSGetColWidthInPixels(SHWindow,5,(LPSHORT)&d4Width);
 SSGetColWidthInPixels(SHWindow,6,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;

 HINSTANCE hInstance = GetApplication()->hInstance;
 SSSetColWidthInPixels(SHWindow,2,ColWidth);
 SSSetData(SHWindow,1,SS_HEADER, GetString(hInstance,6123));
 SSSetData(SHWindow,2,SS_HEADER, GetString(hInstance,6124));
 SSSetData(SHWindow,3,SS_HEADER, GetString(hInstance,6125));
 SSSetData(SHWindow,4,SS_HEADER, GetString(hInstance,6126));
 SSSetData(SHWindow,5,SS_HEADER, GetString(hInstance,6127));
 SSSetData(SHWindow,6,SS_HEADER, GetString(hInstance,6128));

 SSSetTypeEdit(SHWindow,&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetTypeEdit(SHWindow,&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(GetApplication()->hInstance,6107)));
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);


}
/*
void TARDistribution::SetListSize (int Rows)
{
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 short iHeight;
 LOGFONT lfFont;
 HFONT   hFont;
 lfFont.lfHeight          = -13;
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = 1; //default character set.
 lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);

 SSSetBool(SHWindow,SSB_AUTOSIZE,FALSE);
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetAutoSizeVisible(SHWindow,6,Rows);

 SSGetRowHeightInPixels(SHWindow,1,&iHeight);
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ 2*GetSystemMetrics(SM_CXDLGFRAME) + (Rows*iHeight) + 1;
 GetWindowRect(HWindow,&wd);
 MoveWindow(HWindow,wd.left,wd.top,wd.right-wd.left,Height,TRUE);
 GetClientRect(HWindow,&rc);
 GetWindowRect(GetDlgItem(HWindow,IDD_STATUSBOX),&wd);
 MoveWindow(SHWindow,rc.left+1, rc.top, rc.right - (wd.right-wd.left) , rc.bottom - rc.top, TRUE);
 GetWindowRect(SHWindow,&rc);
 MoveWindow(GetDlgItem(HWindow,IDD_STATUSBOX),rc.right,rc.top,rc.right-rc.left -(rc.right - (wd.right-wd.left)),rc.bottom-rc.top,TRUE);
 SSSetMaxRows(SHWindow,Rows+1);
}
*/
