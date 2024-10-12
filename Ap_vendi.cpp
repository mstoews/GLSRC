/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
   AP Vendor Inquiry
   ap_vendi.cpp
*/
#include "ap.h"
#include <math.h>
#include "db_vend.h"
#include "gl_dlg.h"
#include "ap_vendl.h"
#include "ap_vendi.h"


static int iVendorList;



void TGlWindow::APVendorInquiry  (RTMessage)
 {
  //GetApplication()->ExecDialog(new TAPVendorInquiry(this,"AP_VENDORACCT"));
  SendMessage(HWindow,WM_COMMAND,ID_OPENACCTS,NULL);
 }

void TAPVendorInquiry::SetupWindow()
{
 TDialog::SetupWindow();
 RECT rc;
 SS_CELLTYPE 	CellType;
 HCURSOR  hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
 ShowCursor (TRUE);
 GetWindowRect(List->HWindow,&rc);
 List->SetMaxCols(3);
 short ColWidth = ((rc.right - rc.left) - GetSystemMetrics(SM_CXHSCROLL))/4;
 List->SetColWidthInPixels(1,ColWidth);
 List->SetColWidthInPixels(2,ColWidth);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetColWidthInPixels(4,ColWidth);
 List->SetData(1,SS_HEADER, "Vendor Number");
 List->SetData(2,SS_HEADER, "Vendor Name");
 List->SetData(4,SS_HEADER, "Amount Due");
 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
 long i = 1;
 List->SetBool(SSB_REDRAW,FALSE);

 if(i < 8)
 i = 8;
 List->SetMaxRows(14);
 List->SetBool(SSB_REDRAW,TRUE);
 ShowCursor (FALSE);
 SetCursor (hCursor);
}
void TAPVendorInquiry::Search        (RTMessage){}
void TAPVendorInquiry::Next	     (RTMessage ){}
void TAPVendorInquiry::Previous	     (RTMessage ){}
void TAPVendorInquiry::SetData	     (RTMessage ){}
void TAPVendorInquiry::Cancel        (RTMessage)
{
	cb.closeAll();
	cb.init();
	ShutDownWindow();
}

