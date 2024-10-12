#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "gl_dlg.h"
#include "db_apbtc.h"
#include "db_vend.h"
#include "db_dist.h"

//#include "db_reqst.h"
#include "db_apdst.h"
#include "gl_pick.h"
//#include "db_reqst.h"

#define  CM_CURRENT 1001


static COLORREF lpBack, lpFore;

_CLASSDEF (TAPRequestList)
class TAPRequestList : public TPickList
{
 public:
 PTListArray ListArray;
 PTapreqst apreqst;
 TAPRequestList(PTWindowsObject AParent, LPSTR AName,PTapreqst a):TPickList(AParent,AName)
 {
  ListArray = new TListArray(1000, 0, 50);
  apreqst = a;
 };
 ~TAPRequestList(){delete ListArray;};
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
};



void TAPRequestDetail::Data(unsigned int Col,unsigned int Row, Field4 *field)
{
 SSSetData(List,Col,Row,field->str());
 return;
}

void TAPRequestDetail::Data(unsigned int Col, unsigned int Row, double dTotal)
{
  char *szTotal = new char[16];
  gcvt(dTotal,16,szTotal);
  SSSetData(List,Col,Row,szTotal);
  delete szTotal;
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
  virtual void Ok	      (RTMessage Msg) = [ID_FIRST + IDOK]      {};
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]  {cb.closeAll();cb.init();ShutDownWindow();};
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT]  {};
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEWALL]{};
  virtual void List 	      (RTMessage Msg) = [ID_FIRST + ID_REQUEST];
  virtual void UpdateScreen   (RTMessage Msg) = [CM_FIRST + CM_CURRENT];
  virtual void Top	      (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom	      (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Next	      (RTMessage Msg) = [ID_FIRST + ID_NEXT];
  virtual void Previous	      (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
  void Data(HWND hWnd, unsigned int Col, unsigned int Row, Field4 *field);  
};

void TAPClearRequest::Data(HWND hWnd, unsigned int Col, unsigned int Row, Field4 *field)
{
  SSSetData(hWnd,Col,Row,field->str());
}


void TAPRequestList::HandleListBox(RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRequest=atol(szBuffer);
 MessageBox(HWindow,szBuffer,"Row One",MB_OK);
 apreqst->SetIndex("paymentreq");
 if(apreqst->seek(lRequest)== 0)
 {
  SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENT,NULL);
  Show(SW_HIDE);
 }
}


void TAPRequestDetail::HandleListBox(RTMessage Msg)
{
 char szVendorno[21]; 
 GetDlgItemText(Parent->HWindow,ID_VENDORNO,(LPSTR)szVendorno,sizeof(szVendorno));
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szVendorno); 
 apdist->SetIndex("invoiceno");
 apdist->SetIndexInvoiceno();
 if(apdist->seek(szVendorno)==0)
 {
  SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENT,NULL);
  Show(SW_HIDE);
 }
}






void TAPClearRequest::Top(RTMessage Msg)
{
 apreqst->top();
 UpdateScreen(Msg);
}

void TAPClearRequest::Bottom(RTMessage Msg)
{
 apreqst->bottom();
 UpdateScreen(Msg);
}
void TAPClearRequest::Next(RTMessage Msg)
{ 
 if(apreqst->seek(lGetField(ID_REQUESTNO)) == 0)
 { 
   if(!apreqst->eof())
   {
    apreqst->skip();
    if(apreqst->eof())
    apreqst->bottom();
    UpdateScreen(Msg);
   }
 }
 else
 return;
}
void TAPClearRequest::Previous(RTMessage Msg)
{
 if(apreqst->seek(lGetField(ID_REQUESTNO)) == 0)
 { 
   if(!apreqst->bof())
   {
    apreqst->skip(-1);
    UpdateScreen(Msg);
   }
   else if (apreqst->eof())
   {
    apreqst->skip(-1);
    UpdateScreen(Msg);
   }
 }
 else
 return;
}


void TAPClearRequest::UpdateScreen(RTMessage)
{
 
 char *szAccounts = new char[apreqst->bankacct->len() + apreqst->bankdept->len() + 1];
 SetField(ID_REQUESTNO,apreqst->paymentreq);

  /*
 SetField(ID_TOTAL,apreqst->amountpaid);
 SetField(ID_VENDORNO,apreqst->vendorno);
 apvendor->seek(GetField(ID_VENDORNO));
 SetField(ID_VENDORNAME,apvendor->vdname);
 SetField(ID_ACCOUNTS,apreqst->bankacct);
 SetField(ID_DEPART,apreqst->bankdept);
 strcpy(szAccounts,GetField(ID_ACCOUNTS));
 strcat(szAccounts,GetField(ID_DEPART));
 glaccts->seek(szAccounts);
 SetField(ID_DESCRIPT,glaccts->descript);
  */
 delete szAccounts;

 RequestDetail->ListOff();
 RequestDetail->ListClear();
 unsigned int i = 1;
 apdist->SetIndex("paymentreq");
 unsigned int iRequestno = (unsigned int) lGetField(ID_REQUESTNO);
 if(apdist->seek(iRequestno) == 0)
  {
    while(iRequestno == apdist->lRequest())
    {
     RequestDetail->Data(1,i,apdist->invoiceno);
     RequestDetail->Data(2,i,apdist->duedate);
     RequestDetail->Data(3,i,apdist->amount);
     RequestDetail->Data(4,i,apdist->adjustment);
     RequestDetail->Data(5,i,apdist->rebateamt);
     double dTotal = apdist->dAmount();
     RequestDetail->Data(6,i,dTotal);
     apdist->skip();
     i++;
    }   
  }
 RequestDetail->ListOn();
 return;
}





void TAPClearRequest::List(RTMessage)
{
 if(bRequestList)
 {
    HWND SHWindow = GetDlgItem(RequestList->HWindow,ID_DISTLIST);
    SSSetBool(SHWindow,SSB_REDRAW,FALSE);
    SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);

    Relate4set rQuery(apreqst);
    rQuery.querySet("PAYMENTREQ <> 0");
    int i = 1;
    for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
    {
     Data(SHWindow,1,i,apreqst->paymentreq);
     Data(SHWindow,2,i,apreqst->reference);
     i++;
    } 
    rQuery.unlock();
    rQuery.free();
    SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 }
 if(IsWindowVisible(RequestList->HWindow))
 RequestList->Show(SW_HIDE);
 else
 RequestList->Show(SW_SHOW);


}


void TGlWindow::APModClearReq(RTMessage)
{
 GetApplication()->ExecDialog(new TAPClearRequest(this,"AR_REQUEST"));
}

void TAPClearRequest::SetupWindow()
{
 apdist   = new Tapdist(&cb);
  if(!apdist->isValid())
  {
	ErrorClose(apdist->GetDbfName());
	return;
  }

 apvendor = new Tapvendor(&cb);
  if(!apvendor->isValid())
  {
	ErrorClose(apvendor->GetDbfName());
	return;
  }

 apbtch   = new Tapbtch(&cb);
  if(!apbtch->isValid())
  {
	ErrorClose(apbtch->GetDbfName());
	return;
  }

 apbtchd  = new Tapbtchd(&cb);
  if(!apbtchd->isValid())
  {
	ErrorClose(apbtchd->GetDbfName());
	return;
  }

 apreqst  = new Tapreqst(&cb);
  if(!apreqst->isValid())
  {
	ErrorClose(apreqst->GetDbfName());
	return;
  }

 glaccts  = new Tglaccts(&cb);
  if(!glaccts->isValid())
  {
	ErrorClose(glaccts->GetDbfName());
	return;
  }



 RequestList = new TAPRequestList(this,"AR_LIST",apreqst);
 RequestList->EnableAutoCreate();
 GetApplication()->MakeWindow(RequestList);
 RequestList->SetCaption("Accounts Payable Request List"); 

 RequestDetail = new TAPRequestDetail(this,"AR_LIST",apdist);
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
 CHourGlass wait;
 TPickList::SetupWindow();
 List = GetDlgItem(HWindow,ID_DISTLIST);
 RECT rc;
 SS_CELLTYPE 	CellType;


 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 SSSetMaxCols(SHWindow, 6);
 SSGetClientRect(SHWindow,&rc);
 short d1Width,d2Width,d3Width,d4Width,d5Width,d6Width;

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
 


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);


}


void TAPRequestList::SetupWindow()
{
 CHourGlass wait; 
 TPickList::SetupWindow();
 RECT rc;
 SS_CELLTYPE 	CellType;


 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 SSSetMaxCols(SHWindow, 2);
 SSGetClientRect(SHWindow,&rc);
 short d1Width,d2Width;

 SSSetColWidth(SHWindow,1,apreqst->paymentreq->len());
 SSSetColWidth(SHWindow,2,apreqst->reference->len());


 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width);

 short ColWidth = d1Width +
		  d2Width +
		  (2*GetSystemMetrics(SM_CXDLGFRAME)+
		  GetSystemMetrics(SM_CXHSCROLL));

 GetWindowRect(HWindow,&rc);
 RECT parent;
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+20,parent.bottom+20,ColWidth,rc.bottom-rc.top,TRUE);
 SetListSize(8);
 SSSetData(SHWindow,1,SS_HEADER, "Request");
 SSSetData(SHWindow,2,SS_HEADER, "Reference");

 SSSetTypeEdit(SHWindow,&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);


}
