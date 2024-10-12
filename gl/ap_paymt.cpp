
#include "ap.h"

#include "gl_dlg.h"
#include "db_dist.h"
#include "db_vend.h"
#include "db_reqst.h"
#include "db_apdst.h"
#include "db_accts.h"
//#include "ap_ctrl.h"
#include "utildlg.h"
#include "math.h"

extern Code4 cb;
extern char szApp[20];
#define ID_INVOICE 1030
BOOL bCleared;
static COLORREF lpBack, lpFore;



_CLASSDEF (TAPPayment)
class TAPPayment : public TMainDialog
{
 public:
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;

  BOOL          bHasInvoices;
  PTglaccts     glaccts;
  PTapvendor    apvendor;
  PTapdist      apdist;
  PTListArray   ListArray;
  PTSpread      InvoiceList;
  double dAPControl;
  TAPPayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
  {
  ListArray   = new TListArray(1000, 0, 50);
  InvoiceList = new TSpread(this,ID_DISTLIST);
  };
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_DUEDATE];
  virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual BOOL LoadDistributionList();
  void         PrintPaymentList(void);
  void         APAccountUpdate();
  ~TAPPayment(){delete ListArray;};
};

_CLASSDEF (TAPSetChequeNumber)
class TAPSetChequeNumber: public TMainDialog
{
 public:
 PTapdist      apdist;
 char        *szAmount;
 TAPSetChequeNumber(PTWindowsObject AParent,int ATitle,PTapdist reqst,char *Amount)
 :TMainDialog(AParent,ATitle)
  {
	apdist = reqst;
	szAmount = strcpy(new char[10],Amount);
  }
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb.init();ShutDownWindow();};
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};



BOOL Tapreqst::bCleared(void)
{
 if (strcmp(cleared->str(),"0") == 0)
	return TRUE;
 else
	return FALSE;
}

void TAPSetChequeNumber::SetupWindow()
{
 TDialog::SetupWindow();
 Date4 dDatepaid(apdist->datepaid->str());
 SetField(ID_CHEQUE,apdist->chequeno);
 SetField(ID_INVOICE,szAmount);
 SetField(ID_ACCOUNTS,apdist->account);
 SetField(ID_DEPART,apdist->dept);
 SetField(ID_DATE,dDatepaid.str());
}

void TAPSetChequeNumber::Ok (RTMessage)
{
 apdist->seek(GetField(ID_CHEQUE));
 while( strcmp( StrTrim(apdist->chequeno->str()) , StrTrim(GetField(ID_CHEQUE))) == 0)
  {
	apdist->paymentctl->assign("C");
	bCleared = TRUE;
	apdist->skip();
  }
 apdist->UpdateUserTime(szUser);
 apdist->flush();
 cb.init();
 ShutDownWindow();
}

class TPrintCheques: public TMainDialog
{
 public:
 Tapdist*      apdist;
 Tapreqst*     apreqst;
 TPrintCheques(PTWindowsObject AParent,int ATitle)
 :TMainDialog(AParent,ATitle)
  {
  }
 virtual void SetupWindow();
 virtual void Verify         (RTMessage Msg) = [ID_FIRST + 1031];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 void UpdateCheque(double dAmount,int dChequeNo);
 void AddCheque(int dChequeNo);
};

void TPrintCheques::Cancel (RTMessage)
{
  cb.closeAll();
  cb.initUndo();
  cb.init();
  ShutDownWindow();
}

void TPrintCheques::Verify (RTMessage)
{
  // "Confirm that all cheques have been printed correctly."
  // "Set Cheque Numbers"
 if(MessageBox(HWindow,GetString(3079),GetString(3080),MB_YESNO)==IDYES)
  {
	 for(apdist->top();!apdist->eof();apdist->skip())
	  {
		 if(strcmp(StrTrim(apdist->paymentctl->str()),"P") == 0)
			 apdist->paymentctl->assign("V");
	  }
	 apdist->UpdateUserTime(szUser);
	 apdist->flush();
  }
}

void TPrintCheques::SetupWindow()
{
  TDialog::SetupWindow();
	if(!CheckFile("AP_DIST",Parent->HWindow) ||
	   !CheckFile("AP_REQST",Parent->HWindow))
	{
	 cb.init();
	 ShutDownWindow();
	 return;
	}

  apdist = new Tapdist(&cb);
  apreqst = new Tapreqst(&cb);
  apreqst->SetIndex("vendorno");
  apdist->SetIndex("cheques");
  //cheques tag is set to filter all "P's" and by vendor number
  apdist->bottom();
  SetField(1087,apdist->chequeno);
  SetBitmap(1031);
  SetBitmap(IDOK);
  SetBitmap(IDCANCEL);
}

void TPrintCheques::Ok (RTMessage)
{
 int dChequeNo = 0;
 dChequeNo = dGetField(1087);
 if(dChequeNo == 0)
  return;

 apreqst->zap();
 apdist->top();
 apdist->SetIndex("cheques");
 --dChequeNo;

 while(!apdist->eof())
  {
	 if(apreqst->seek(apdist->vendorno->str()) != 0)
		{
		 ++dChequeNo;
		 AddCheque(dChequeNo);
		}
	 else
		 UpdateCheque(apdist->dAmountpaid(),dChequeNo);

	 apdist->skip();
  }
 MessageBox(HWindow,GetString(2014),szApp,MB_ICONINFORMATION);
}

void TPrintCheques::UpdateCheque(double dAmount,int dChequeNo)
{
  apdist->UpdateUserTime(szUser);
  apdist->chequeno->assignLong(dChequeNo);
  apdist->flush();
  apreqst->amountpaid->assignDouble(apreqst->dAmountpaid()+dAmount);
  apreqst->flush();
  return;
}

void TPrintCheques::AddCheque(int dChequeNo)
{
  apdist->UpdateUserTime(szUser);
  apdist->chequeno->assignLong(dChequeNo);
  apdist->flush();

  apreqst->appendStart();
  apreqst->vendorno->assign(apdist->vendorno->str());
  apreqst->paymentreq->assign("");
  apreqst->reference->assign("");
  apreqst->amountpaid->assignDouble(apdist->dAmountpaid());
  apreqst->datepaid->assign(GetDate(4075));
  apreqst->cheque->assignLong(dChequeNo);
  apreqst->bankacct->assign(apdist->account->str());
  apreqst->bankdept->assign(apdist->dept->str());
  apreqst->cleared->assign("1");
  apreqst->append();
}


void TManilaGL::APPayment(RTMessage)
{
 GetApplication()->ExecDialog(new TAPPayment(this,1046));
}

void TManilaGL::APChequeAP (RTMessage)
{
 GetApplication()->ExecDialog(new TPrintCheques(this,1157));
}


void TAPPayment::SelectCell (RTMessage Msg)
{
 char szRequest[11];
 char szAmount[16];

 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD) Msg.LParam;
 InvoiceList->GetData(4,Cell->Row,szRequest);
 InvoiceList->GetData(5,Cell->Row,szAmount);
 bCleared = FALSE;
 if(*StrTrim(szRequest) == 0)
	return;
 if(apdist->seek(szRequest) == 0)
  {
	 GetApplication()->ExecDialog(new TAPSetChequeNumber(this,1051,apdist,szAmount));
	 if(bCleared)
	 InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 MessageBox(HWindow,"Invoice Not Found!",szApp,MB_ICONEXCLAMATION);
}

void TAPPayment::Print (RTMessage)
{
 if(MessageBox(HWindow,GetString(8138),szApp,MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
  {
	PrintPaymentList();
  }
 return;
}



void TAPPayment::SetDate (RTMessage)
{
 bHasInvoices = LoadDistributionList();
}


void TAPPayment::SetupWindow()
{
 TDialog::SetupWindow();
 HourGlassOn();
 RECT rc;
 InvoiceList->GetClientRect(&rc);
 short d1Width,d0Width,d2Width,d3Width,d4Width,d5Width;
 apdist  =  new Tapdist(&cb);
 apvendor =  new Tapvendor(&cb);
 apdist   =  new Tapdist(&cb);
 glaccts  =  new Tglaccts(&cb);


 InvoiceList->SetMaxCols(5);
 InvoiceList->SetColWidth(1,apdist->vendorno->len());
 InvoiceList->SetColWidth(2,apdist->account->len());
 InvoiceList->SetColWidth(3,apdist->dept->len());
 InvoiceList->SetColWidth(4,apdist->chequeno->len());

 InvoiceList->GetColWidthInPixels(0,(LPSHORT)&d0Width);
 InvoiceList->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);

 
 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d1Width
		  - d2Width
		  - d3Width
		  - d4Width;


 InvoiceList->SetColWidthInPixels(5,ColWidth);

 //lfFont.lfHeight          = -13;
 lfFont.lfHeight          = -(GetFontSize());
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
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);


 InvoiceList->SetMaxRows(1000);
 InvoiceList->SetData(1,SS_HEADER, "Vendor Code");
 InvoiceList->SetData(2,SS_HEADER, "Bank Account");
 InvoiceList->SetData(3,SS_HEADER, "Dept");
 InvoiceList->SetData(4,SS_HEADER, "Cheque");
 InvoiceList->SetData(5,SS_HEADER, "Amount Due") ;

 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);


 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,999999999.99);
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);

 // Open databases...
 cb.safety = 0;

 apdist->SetIndex("invoiceno");
 SetBitmap(IDCANCEL);
 bHasInvoices  = LoadDistributionList();

 HourGlassOff();
}

void TAPPayment::Cancel  (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

BOOL TAPPayment::LoadDistributionList()
{

 int l = 1;
 char szPaymentDate[9];
 GetDlgItemText(HWindow,ID_DATE,szPaymentDate,sizeof(szPaymentDate));
 Date4 dPayment(szPaymentDate);
 apdist->SetIndex("chequeno");
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 double  dCheque;
 double  dCompare = 0;
 double  dChequeAmount = 0;
 Str4large *dAmountDue = new Str4large;
 InvoiceList->GetColor(SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);
 apdist->top();
 while(!apdist->eof())
  {
	 if(apdist->Paid())
	  {
		InvoiceList->SetData(1,l,apdist->vendorno->str());  // Vendor Code
		InvoiceList->SetData(2,l,apdist->account->str());
		InvoiceList->SetData(3,l,apdist->dept->str());
		InvoiceList->SetData(4,l,apdist->chequeno->str());  // Cheque Number

		dAmountDue->assignDouble(apdist->dAmountpaid()+ double (Str4ptr(dAmountDue->str())),16,6);
		InvoiceList->SetData(5,l,dAmountDue->str()); // Adjustment
		dCheque = double (Str4ptr(apdist->chequeno->str()));
		apdist->skip();
		if(dCheque != double (Str4ptr(apdist->chequeno->str())))
      {
       l++;
		 dAmountDue->assignDouble(0);
      }
     }
    else
    apdist->skip();
   }
 /*
 Relate4set *rQuery = new Relate4set(apdist);
 rQuery->querySet("paymentctl = 'P'");
 Str4large *dAmountDue = new Str4large;
 double dChequeno = 0;
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
      InvoiceList->SetData(1,l,apdist->vendorno->str());  // Vendor Code
		InvoiceList->SetData(2,l,apdist->account->str());
		InvoiceList->SetData(3,l,apdist->dept->str());
      InvoiceList->SetData(4,l,apdist->chequeno->str());  // Cheque Number
      dAmountDue->assignDouble(apdist->dAmountpaid(),16,6);
		InvoiceList->SetData(5,l,dAmountDue->str()); // Adjustment
      if(dChequeno != double (Str4ptr(apdist->chequeno->str()));

     
     }
     else
     dAmountDue->assignDouble(apdist->dAmountpaid() + double (Str4ptr(dAmountDue->str())));
     InvoiceList->SetData(5,l,dAmountDue->str()); // Adjustment
	  dChequeno = double (Str4ptr(apdist->chequeno->str()));
     rQuery->unlock();
     rQuery->free();
  }
 */
 InvoiceList->SetBool(SSB_REDRAW,TRUE);
 if(l>9)
  InvoiceList->SetMaxRows(l);
 else
  InvoiceList->SetMaxRows(9);

 if(l>1)
 return TRUE;
 else
 return FALSE;

}


void TAPPayment::PrintPaymentList(void)
{ 
  PrintReport("ap_paycn",GetApplication()->MainWindow->HWindow);
  return;
}

void TAPPayment::Ok (RTMessage){}



