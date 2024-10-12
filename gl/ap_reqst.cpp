#include "ap.h"
#include "gl_dlg.h"
#include "db_apbtc.h"
#include "db_vend.h"
#include "db_dist.h"
#include "db_accts.h"
#include "db_reqst.h"
#include "db_apdst.h"
#include "gl_pick.h"
#include "db_reqst.h"

#define CM_CURRENTINVOICE 5111

extern Code4 cb;
extern char szApp[20];
static COLORREF lpBack, lpFore;


_CLASSDEF (TAPRequestList)
class TAPRequestList : public TPickList
{
 public:
 PTapdist  apdist;
 TAPRequestList(PTWindowsObject AParent, LPSTR AName,PTapdist a):TPickList(AParent,AName)
 {
  apdist = a;
 };
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK]; 
 virtual void SetupWindow();

};

_CLASSDEF (TAPRequestDetail)
class TAPRequestDetail : public TPickList
{
 public:
 char szVendorno[10];
 HWND List;
 PTapdist apdist;
 TAPRequestDetail(PTWindowsObject AParent, LPSTR AName,PTapdist a):TPickList(AParent,AName)
 { 
  apdist = a;
 };
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 void ListOn(){SSSetBool(List,SSB_REDRAW,TRUE);};
 void ListOff(){SSSetBool(List,SSB_REDRAW,FALSE);};
 void ListClear(){SSClearData(List,SS_ALLCOLS,SS_ALLROWS);};
 void Data(unsigned int Col,unsigned int Row, Field4 *field);
 void Data(unsigned int Col,unsigned int Row, double dTotal);
 char *GetField(int ID);
};

char *TAPRequestDetail::GetField(int ID)
{
 DWORD len =  SendDlgItemMessage(Parent->HWindow,ID,WM_GETTEXTLENGTH,NULL,NULL);
 char *szTemp = new char[(int)len + 1];
 SendDlgItemMessage(Parent->HWindow,ID,WM_GETTEXT,len + 1,(LPARAM)(LPSTR)szTemp);
 return szTemp;
}


void TAPRequestDetail::Data(unsigned int Col,unsigned int Row, Field4 *field)
{
 SSSetData(GetDlgItem(HWindow,ID_DISTLIST),Col,Row,field->str());
}

void TAPRequestDetail::Data(unsigned int Col, unsigned int Row, double dTotal)
{
  char *szTotal = new char[20];
  gcvt(dTotal,16,szTotal);
  SSSetData(List,Col,Row,szTotal);
  delete [] szTotal;
}



_CLASSDEF (TAPClearRequest)
class TAPClearRequest : public TMainDialog
{
 public:
  PTapdist      apdist;
  PTapvendor    apvendor;
  PTapbtch	apbtch; 
  PTapbtchd     apbtchd;
  PTapreqst     apreqst;
  PTglaccts     glaccts;

  PTAPRequestList RequestList;
  PTAPRequestDetail RequestDetail;
  BOOL bRequestList;

  TAPClearRequest(PTWindowsObject AParent, LPSTR ATitle):TMainDialog(AParent,ATitle)
  {
   bRequestList = TRUE;
  };
  virtual void SetupWindow();
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]  {cb.closeAll();cb.init();ShutDownWindow();};
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT]  {};
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEWALL]{};
  virtual void List 	      (RTMessage Msg) = [ID_FIRST + ID_REQUEST];
  virtual void UpdateScreen   (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
  virtual void Top	      (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom	      (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Next	      (RTMessage Msg) = [ID_FIRST + ID_NEXT];
  virtual void Previous	      (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
  virtual void Invoice        (RTMessage Msg) = [CM_FIRST + CM_CURRENTINVOICE];
  virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  void Data(HWND hWnd, unsigned int Col, unsigned int Row, Field4 *field);  
};


void TAPClearRequest::Delete(RTMessage Msg)
{
apdist->SetIndex("chequeno");
if (apdist->seek(lGetField(ID_REQUESTNO)) == 0)
 {
  if(MessageBox(HWindow,"Do you wish to delete this request?",szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
   apdist->seek(lGetField(ID_REQUESTNO));
   while(lGetField(ID_REQUESTNO) == apdist->lRequest())
    {
     apdist->paymentctl->assign("H");
     apdist->chequeno->assignDouble(0);
     apdist->datepaid->assign("");
     apdist->skip();
    }
   UpdateScreen(Msg);
   RequestList->Show(SW_HIDE);
   bRequestList = TRUE;
  }  
 }
}


void TAPClearRequest::Data(HWND hWnd, unsigned int Col, unsigned int Row, Field4 *field)
{
  SSSetData(hWnd,Col,Row,field->str());
}


void TAPClearRequest::Invoice (RTMessage)
{
 SetField(ID_INVOICENO, apdist->invoiceno);
 SetField(ID_JDESCRIPT, apdist->descript);
 SetField(ID_REFERENCE, apdist->reference);
 SetField(ID_DUEDATE,   apdist->duedate);
 SetField(ID_INVDATE,   apdist->date);
 SetField(ID_AMOUNT,    apdist->amount);

}

void TAPRequestList::HandleListBox(RTMessage Msg)
{
 apdist->SetIndex("chequeno");
 char szBuffer[12];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRequest=atol(szBuffer);
 
 if(apdist->seek(lRequest) == 0)
 {
  SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
  Show(SW_HIDE);
 }
 SetFocus(Parent->HWindow);
}


void TAPRequestDetail::HandleListBox(RTMessage Msg)
{
 
 
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST); 
 char *szInvoiceno = strcpy(new char[apdist->invoiceno->len()+ apdist->vendorno->len() + 1],GetField(ID_VENDORNO));


 char *szInvoice = new char[apdist->invoiceno->len() + 1];
 SSGetData(SHWindow,1,Cell->Row,szInvoice);
 strcat(szInvoiceno,szInvoice);
 

 apdist->SetIndex("invoiceno");

 if(apdist->seek(szInvoiceno) == 0)
 {
  SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTINVOICE,NULL); 
 }
 else
 MessageBox(HWindow,"Invoice Not Found.","Requests",MB_OK);
 delete [] szInvoiceno;
 delete [] szInvoice;
}







void TAPClearRequest::Top(RTMessage Msg)
{ 
 apdist->SetIndex("chequeno");
 apdist->top();
 UpdateScreen(Msg);
}

void TAPClearRequest::Bottom(RTMessage Msg)
{
 apdist->SetIndex("chequeno");
 apdist->bottom();
 UpdateScreen(Msg);
}

void TAPClearRequest::Next(RTMessage Msg)
{
 long lRequest = lGetField(ID_REQUESTNO);


 apdist->SetIndex("chequeno");
 if(apdist->seek(lGetField(ID_REQUESTNO)) == 0)
 {
    if(!apdist->eof())    
     apdist->skip();
    if(apreqst->eof())
     apreqst->bottom();
    UpdateScreen(Msg);
 }
}

void TAPClearRequest::Previous(RTMessage Msg)
{
 apdist->SetIndex("chequeno");
 if(apdist->seek(lGetField(ID_REQUESTNO)) == 0)
 { 
   if(!apdist->bof())
   {
    apdist->skip(-1);
    UpdateScreen(Msg);
   }
   else if (apdist->eof())
   {
    apdist->skip(-1);
    UpdateScreen(Msg);
   }
 }
}


void TAPClearRequest::UpdateScreen(RTMessage)
{
 SetField(ID_INVOICENO, "");
 SetField(ID_JDESCRIPT, "");
 SetField(ID_REFERENCE, "");
 SetField(ID_DUEDATE,   "");
 SetField(ID_INVDATE,   "");
 SetField(ID_AMOUNT,    "");

 
 char *szAccounts = new char[11 + 1];
 SetField(ID_REQUESTNO,apdist->chequeno);
 SetField(ID_VENDORNO,apdist->vendorno);
 if(apvendor->seek(apdist->vendorno->str()) == 0)
  SetField(ID_VENDORNAME,apvendor->vdname);

 SetField(ID_ACCOUNTS,apdist->account);
 SetField(ID_DEPART,apdist->dept);
 strcpy(szAccounts,GetField(ID_ACCOUNTS));
 strcat(szAccounts,GetField(ID_DEPART));
 glaccts->seek(szAccounts);
  SetField(ID_DESCRIPT,glaccts->descript);
   
 HWND hRequest = GetDlgItem(RequestDetail->HWindow,ID_DISTLIST);
  
 SSSetBool(hRequest,SSB_REDRAW,FALSE);
 SSClearData(hRequest,SS_ALLCOLS,SS_ALLROWS);
 unsigned int i = 1;
 apdist->SetIndex("chequeno");
 unsigned int iRequestno = lGetField(ID_REQUESTNO);
 double dSum = 0.00;
 char szTotal[16];
 if(apdist->seek(iRequestno) == 0)
  {
    while(iRequestno == apdist->lChequeno())
    {
     SSSetData(hRequest,1,i,apdist->invoiceno->str());
     SSSetData(hRequest,2,i,apdist->duedate->str());
     SSSetData(hRequest,3,i,apdist->amount->str());
     SSSetData(hRequest,4,i,apdist->adjustment->str());
     SSSetData(hRequest,5,i,apdist->rebate->str());
     double dTotal = apdist->dAmount();
     dSum += dTotal;
     Str4large szTotal;
     szTotal.assignDouble(dTotal,16,2);
     SSSetData(hRequest,6,i,szTotal.str());
     apdist->skip();
     i++;
    }   
  }
 Str4large szSumOfRequests;
 szSumOfRequests.assignDouble(dSum,16,2);
 SetField(ID_TOTAL,szSumOfRequests.str());
 SSSetBool(hRequest,SSB_REDRAW,TRUE);
}





void TAPClearRequest::List(RTMessage)
{
 if(bRequestList)
 {
    HWND SHWindow = GetDlgItem(RequestList->HWindow,ID_DISTLIST);
    SSSetBool(SHWindow,SSB_REDRAW,FALSE);
    SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
    //apdist->SetIndex("payment_u");
    Relate4set *rQuery = new Relate4set(apdist);
    rQuery->querySet("PAYMENTCTL = 'P'");
    Str4large szRequest;
    int i = 1;
    double compare = 0;
    double request = 0;

    for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
    {     
     request = double (Str4ptr(apdist->chequeno->str()));
     if(request != compare)
     { 
      SSSetData(SHWindow,1,i,apdist->chequeno->str());
      SSSetData(SHWindow,2,i,apdist->vendorno->str());
      SSSetData(SHWindow,3,i,apdist->reference->str());
      compare = double (Str4ptr(apdist->chequeno->str()));
      i++;
     }      

    } 
    if(i>10)
    SSSetMaxRows(SHWindow,i);
    else
    SSSetMaxRows(SHWindow,10);
    rQuery->unlock();
    rQuery->free();
    apdist->SetIndex("chequeno");
    SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 }
 
 if(IsWindowVisible(RequestList->HWindow))
 RequestList->Show(SW_HIDE);
 else
 RequestList->Show(SW_SHOW);


}


void TGlWindow::APModClearReq(RTMessage)
{
 GetApplication()->ExecDialog(new TAPClearRequest(this,"AP_REQUEST"));
}

void TAPClearRequest::SetupWindow()
{
 apdist   = new Tapdist(&cb);
 apdist->SetIndex("chequeno");
 apvendor = new Tapvendor(&cb);
 apbtch   = new Tapbtch(&cb);
 apbtchd  = new Tapbtchd(&cb);
 apreqst  = new Tapreqst(&cb);
 glaccts  = new Tglaccts(&cb);

 apreqst->SetIndex("unpaid");


 RequestList = new TAPRequestList(this,"GL_ACCTLIST",apdist);
 RequestList->EnableAutoCreate();
 GetApplication()->MakeWindow(RequestList);
 RequestList->SetCaption("Accounts Payable Request List"); 

 RequestDetail = new TAPRequestDetail(this,"GL_ACCTLIST",apdist);
 RequestDetail->EnableAutoCreate();
 GetApplication()->MakeWindow(RequestDetail);
 RequestDetail->SetCaption("Accounts Payable Request Detail");
 RequestDetail->Show(SW_SHOW);


 
 RECT rc;
 RECT child;
 RECT parent;
 int pos = 30;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+pos,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 GetWindowRect(HWindow,&rc);
 GetWindowRect(RequestDetail->HWindow,&child);
 MoveWindow(RequestDetail->HWindow,rc.left+pos,rc.bottom+pos,child.right-child.left,child.bottom-child.top,TRUE);
 GetWindowRect(RequestList->HWindow,&child);
 MoveWindow(RequestList->HWindow,rc.right+pos,rc.top+pos,child.right-child.left,child.bottom-child.top,TRUE);
}

void TAPRequestDetail::SetupWindow()
{
 HourGlassOn(); 
 TPickList::SetupWindow();
 List = GetDlgItem(HWindow,ID_DISTLIST);
 RECT rc;
 SS_CELLTYPE 	CellType;


 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 SSSetMaxCols(SHWindow, 6);
 SSGetClientRect(SHWindow,&rc);
 short d1Width,d2Width,d3Width,d4Width,d5Width,d6Width,d7Width;

 SSSetColWidth(SHWindow,1,apdist->invoiceno->len());
 SSSetColWidth(SHWindow,2,apdist->duedate->len());
 SSSetColWidth(SHWindow,3,apdist->amount->len());
 SSSetColWidth(SHWindow,4,apdist->adjustment->len());
 SSSetColWidth(SHWindow,5,apdist->rebateamt->len());
 SSSetColWidth(SHWindow,6,apdist->rebateamt->len());


 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d3Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d4Width);
 SSGetColWidthInPixels(SHWindow,5,(LPSHORT)&d5Width);
 SSGetColWidthInPixels(SHWindow,6,(LPSHORT)&d6Width);


 short ColWidth = d1Width +
		  d2Width +
		  d3Width +
		  d4Width +
		  d5Width +
		  d6Width +
		  (2*GetSystemMetrics(SM_CXDLGFRAME)+
		  GetSystemMetrics(SM_CXHSCROLL));

 GetWindowRect(HWindow,&rc);
 RECT parent;
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+20,parent.bottom+20,ColWidth,rc.bottom-rc.top,TRUE);
 SetListSize(8);
 SSSetData(SHWindow,1,SS_HEADER, "Invoice");
 SSSetData(SHWindow,2,SS_HEADER, "Order No.");
 SSSetData(SHWindow,3,SS_HEADER, "Amount");
 SSSetData(SHWindow,4,SS_HEADER, "Adjust");
 SSSetData(SHWindow,5,SS_HEADER, "Rebate");
 SSSetData(SHWindow,6,SS_HEADER, "Total");

 SSSetTypeEdit(SHWindow,&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999999.99);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);

 HourGlassOff();
}


void TAPRequestList::SetupWindow()
{
 HourGlassOn(); 
 TPickList::SetupWindow();
 RECT rc;
 SS_CELLTYPE 	CellType;


 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 SSSetMaxCols(SHWindow, 3);
 SSGetClientRect(SHWindow,&rc);
 short d1Width,d2Width,d3Width;

 SSSetColWidth(SHWindow,1,apdist->paymentreq->len());
 SSSetColWidth(SHWindow,2,apdist->vendorno->len());
 SSSetColWidth(SHWindow,3,apdist->reference->len());

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d3Width);

 short ColWidth = d1Width +
		  d2Width +
		  d3Width +
		  (2*GetSystemMetrics(SM_CXDLGFRAME)+
		  GetSystemMetrics(SM_CXHSCROLL));

 GetWindowRect(HWindow,&rc);
 RECT parent;
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+20,parent.bottom+20,ColWidth,rc.bottom-rc.top,TRUE);
 SetListSize(8);
 SSSetData(SHWindow,1,SS_HEADER, "Request");
 SSSetData(SHWindow,2,SS_HEADER, "Vendor No.");
 SSSetData(SHWindow,3,SS_HEADER, "Reference");

 SSSetTypeEdit(SHWindow,&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 HourGlassOff();

}
