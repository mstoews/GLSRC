#include "std_inc.h"

#define ID_BANK 5012
#define ID_INVOICE 1030
#define ID_COMMIT  1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_DRCODES 1098

static COLORREF lpBack, lpFore;

extern HINSTANCE hInst;

void TAPCashEntry::Print(RTMessage)
{
 // if(apcash.seek(dGetField(ID_BATCHNO)) == 0)
 //	 PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}

void TAPCashEntry::GetBankAccountDescription(RTMessage Msg)
{
 string szBankAccount;
 gl_setat.SetIndex("name");
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_CASH,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_CASH,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount.c_str()));
	 if(gl_setat.seek((LPSTR)szBankAccount.c_str()) == 0)
	 SetField(ID_APCONTROL,gl_setat.account.str());
	 SetField(ID_APDEPART,gl_setat.dept.str());
	}
 }

}

void TAPCashEntry::RecoverInvoice()
{
 //char szAccountNo[13];
 long dBatchNo;

 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);

 if(apcash.seek(dBatchNo) == 0)
 {
	  apcash.mode.assign("1");
	  apcash.UpdateUserTime(szUser);
	  apcash.flush();
 }
 gldist.SetIndex("apdist");
 if(gldist.seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(gldist.batchno.str())) == dBatchNo)
	 {
		 gldist.deleteRec();
		 gldist.skip();
	 }
	gldist.pack();
  }

 if(apdist.seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(apdist.batchno.str())) == dBatchNo)
	 {
	  apdist.deleteRec();
	  apdist.skip();
	 }
	apdist.pack();
  }
}

void TAPCashEntry::Cancel (RTMessage )
{
 if(bTransaction)
 {
  switch(MessageBox(HWindow,GetString(ID_COMMIT),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDCANCEL: return;
	 case IDNO    : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDYES   : break;
	}
 }
 if(VendorList != NULL)
  VendorList->List->SavePosition();
 if(AcctList != NULL)
  AcctList->List->SavePosition();
 if(SlipList != NULL)
  SlipList->List->SavePosition();
 D->SavePosition();

 apcash.close();
 apcashd.close();
 apdist.close();
 gldist.close();
 glacct.close();
 apdist.close();
 apsetup.close();
 apvendor.close();
 prods.close();
 gldept.close();
 drcodes.close();
 ShutDownWindow();

 return;
}



void TManilaGL::APCash   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPCashEntry(this,1044));
 }

void TAPCashEntry::Top(RTMessage Msg)
{
 apcash.top();
 CurrentBatch(Msg);
}
void TAPCashEntry::Bottom(RTMessage Msg)
{
 apcash.bottom();
 CurrentBatch(Msg);
}
void TAPCashEntry::Next(RTMessage Msg)
{
 apcash.skip();
 CurrentBatch(Msg);
}

void TAPCashEntry::RollBack(RTMessage)
{
 CHourGlass wait;
 apcash.SetIndex(GetString(8140));
 long lBatchno = lGetField(ID_BATCHNO);
 if(apcash.seek(lBatchno) == 0)
	apcash.deleteRec();

 if(apcashd.seek(lBatchno) == 0)
  while(apcashd.lBatchno() == lBatchno)
  {
	 apcashd.deleteRec();
	 apcashd.skip();
  }

 apcashd.pack();
 apcash.pack();

 return;
}


void TAPCashEntry::Previous(RTMessage Msg)
{
 apcash.skip(-1);
 CurrentBatch(Msg);
}

void TAPCashEntry::Delete(RTMessage Msg)
{

 if(MessageBox(HWindow,GetString(6098),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
  if(apcash.recCount() == 0)
	return;
  int iEntryno;
  long iBatchno;
  IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);
  IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&iBatchno);
  apcashd.SetIndex(GetString(8140));
  if(apcashd.seek(iBatchno) == 0)
	 {
	  CHourGlass wait;
	  while(apcashd.lBatchno() == apcash.lBatchno())
		{
		 if(apcashd.iJournal() == iEntryno)
		  {
			apcashd.deleteRec();
			apcashd.pack();
			break;
		  }
		  apcashd.skip();
		}
		CurrentBatch(Msg);
	 }
	return;
  }
}


BOOL TAPCashEntry::Closed(void)
{
 if(strcmp(GetField(IDOK),GetString(ID_RECOVER)) == 0)
  {
	if(MessageBox(HWindow,GetString(4111),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	  {
		RecoverInvoice();
      SS_COORD Row,Col;
		SSGetActiveCell(GetDlgItem(SlipList->HWindow,ID_DISTLIST),&Col,&Row);
		SSSetColor(GetDlgItem(SlipList->HWindow,ID_DISTLIST),SS_ALLCOLS,Row,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
     }
   return TRUE;
  }
 return FALSE;
}

BOOL TAPCashEntry::PreviousInvoice(void)
 {
  char *szInvoiceno = strcpy(new char[apcash.invoiceno.len()+ apcash.vendorno.len() + 1],GetField(ID_VENDORNO));
  apcash.SetIndex("invoiceno");
  strcat(szInvoiceno,GetField(ID_INVOICENO));
  if(apcash.seek(szInvoiceno) == 0)
	 {
       SetField(IDD_STATUSBOX,GetString(ID_INVOICECREATED));
       return TRUE;
    }
  else
  return FALSE;
 }




void TAPCashEntry::ModifyCurrentSlip()
{
	apcash.mode.assign("1");
	apcash.date.assign(InvoiceDate.str());
	apcash.period.assignDouble(GetPeriod());
	/*
	 apbtch->orderno.assign(GetField(ID_CURRBAL));
	 apbtch->vendorno.assign(GetField(ID_VENDORNO));
	 apbtch->vdname.assign(GetField(ID_VENDORNAME));
	 apbtch->invoiceno.assign(GetField(ID_INVOICENO));

	*/
	apcash.orderno.assign(GetField(ID_CURRBAL));
	apcash.vendorno.assign(GetField(ID_VENDORNO));
	apcash.vdname.assign(GetField(ID_VENDORNAME));
	apcash.invdate.assign(InvoiceDate.str());
	apcash.reference.assign(GetField(ID_REFERENCE));
	apcash.payable.assignDouble(lInvoiceAmount);
	apcash.chequeno.assign(GetField(ID_CHEQUE));
	apcash.invoiceno.assign(GetField(ID_INVOICENO));
	apcash.vatpercent.assignDouble(dGetField(ID_VATPERCENTAGE));
	apcash.UpdateUserTime(szUser);

	apcashd.go(apcashd.iCurrentRecord);
	//apcashd.batchno.assignLong(iBatchno);
	apcashd.descript.assign(GetField(ID_JDESCRIPT));
	apcashd.account.assign(GetField(ID_ACCOUNTS));  // AP account is always set to the default.
	apcashd.dept.assign(GetField(ID_DEPART));        // AP department code is set to the default dept code.
	apcashd.product.assign(GetProduct());
	apcashd.medreps.assign(GetDept());
	apcashd.drcodes.assign(GetDoctor());
	apcashd.debit.assignDouble(dGetField(ID_DEBIT));
	apcashd.credit.assignDouble(dGetField(ID_CREDIT));
	apcashd.flush();

	bSlipList = TRUE;
	SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	return;
}


void TAPCashEntry::Sliplist() //slip list search
{
 if(SlipList == NULL)
 {
  CHourGlass wait;
  SlipList  = new TCashList(this,"",&apcash);
  SlipList->EnableAutoCreate();
  GetApplication()->MakeWindow(SlipList);
 }

 SlipList->List->Seek(ID_BATCHNO);
 SlipList->Show(SW_SHOW);
}



void TAPCashEntry::New(RTMessage)
{
 if(bTransaction)
 {
  int rc = MessageBox(HWindow,GetString(ID_COMMIT),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL |MB_ICONQUESTION);
  if(rc == IDYES)
	Clear();
  else if (rc == IDNO)
  {
	SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL);
	Clear();
  }
  else if (rc == IDCANCEL)
  return;
 }
 else
 Clear();
 return;

}


void TAPCashEntry::SetLineItem(RTMessage)
{
  bModify = TRUE;

  SetFocus(GetDlgItem(HWindow,IDOK));
  SetField(ID_JDESCRIPT,apcashd.descript);
  SetField(ID_ACCOUNTS ,apcashd.account);
  SetField(ID_DEPART   ,apcashd.dept);
  SetField(ID_DEBIT    ,apcashd.debit);
  SetField(ID_CREDIT   ,apcashd.credit);
  SetField(ID_LINENO   ,apcashd.entryno);
  //ChangeSize(TRUE);
}




void TAPCashList::SetupWindow()
{
 TWindow::SetupWindow();
 RECT rc;
 CHourGlass wait;

 HFONT   hFont;

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 List->SetBool(SSB_AUTOSIZE,FALSE);
 List->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 List->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_ALLOWUSERRESIZE,TRUE);
 List->SetBool(SSB_PROTECT,TRUE);
 List->SetBool(SSB_SHOWROWHEADERS,FALSE);
 List->SetBool(SSB_NOBEEP,TRUE);



 List->GetColor(SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 List->SetMaxCols(6);
 List->GetClientRect(&rc);
 iLeft = iTop = iRight = iBottom = 0;
 string szClassName(GetClassName());
 iLeft    = GetPrivateProfileInt(szClassName.c_str(),GetString(hInst,5112),0,PROFILE);
 iTop     = GetPrivateProfileInt(szClassName.c_str(),GetString(hInst,5115),0,PROFILE);
 iRight   = GetPrivateProfileInt(szClassName.c_str(),GetString(hInst,5113),0,PROFILE);
 iBottom  = GetPrivateProfileInt(szClassName.c_str(),GetString(hInst,5114),0,PROFILE);
 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 100;
 }
 SetPosition();

 List->SetColWidth(1,3);
 List->SetColWidth(3,6);
 List->SetColWidth(4,5);
 List->SetColWidth(5,14);
 List->SetColWidth(6,14);

 List->GetColWidthInPixels(1,(LPSHORT)&d0Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d2Width);
 List->GetColWidthInPixels(4,(LPSHORT)&d3Width);
 List->GetColWidthInPixels(5,(LPSHORT)&d4Width);
 List->GetColWidthInPixels(6,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;


 List->SetColWidthInPixels(2,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6067));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6044));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6068));
 List->SetData(4,SS_HEADER, GetString(GetApplication()->hInstance,6054));
 List->SetData(5,SS_HEADER, GetString(GetApplication()->hInstance,6069));
 List->SetData(6,SS_HEADER, GetString(GetApplication()->hInstance,6070));

 List->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 List->SetCellType(1, SS_ALLROWS, &CellType);
 List->SetCellType(2, SS_ALLROWS, &CellType);
 List->SetTypeEdit(&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetCellType(4, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);
 List->SetCellType(5, SS_ALLROWS, &CellType);
 List->SetCellType(6, SS_ALLROWS, &CellType);


}
void TAPCashList::SelectCell (RTMessage Msg)
{
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  unsigned int nIndx = (unsigned int) Cell->Row;
  apcashd->SetIndex(GetString(GetApplication()->hInstance,8140));
  int iBatchno;
  IntGetValue(GetDlgItem(Parent->HWindow,ID_BATCHNO),(LPLONG)&iBatchno);
  //HWND hWin = GetDlgItem(HWindow,ID_DISTLIST);
  apcashd->iCurrentRecord = 0;
  if(iBatchno > 0){
	if(apcashd->seek(iBatchno) == 0){
	  while(iBatchno == apcashd->lBatchno()){
		 if(nIndx == apcashd->iJournal()){
		  apcashd->iCurrentRecord = (long) apcashd->recNo();
		  break;
		 }
		  apcashd->skip();
	  }
	 }
  }
  if(apcashd->iCurrentRecord == 0)
	{
	 //List->SetColor(hWin,SS_ALLCOLS,SS_ALLCOLS,RGB(255,255,255),RGB(0,0,0));
	 SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	 return;
	}
  List->SetColor(SS_ALLCOLS,SS_ALLCOLS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
  List->SetColor(SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
  SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
  return;

}



void TAPCashEntry::AddDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 Str4large szReference;
 szReference.setLen(51);
 if(strcmp(StrTrim(szDescript),"") == 0)
   {
    char *szSeek = new char[12];
    strcpy(szSeek,szAccount);
    strcat(szSeek,szDepart);
	 glacct.SetIndex("accounts");
    if(glacct.seek(szSeek) == 0)
	  szReference.assign(glacct.descript.str());
    delete  [] szSeek;
    }
  else
  {
    szReference.assign(szDescript);
  }

 apcashd.appendStart();
 apcashd.batchno.assignLong(iBatchno);
 apcashd.descript.assign(szReference.str());
 apcashd.account.assign(szAccount);
 apcashd.dept.assign(szDepart);
 apcashd.product.assign(GetProduct());
 apcashd.medreps.assign(GetDept());
 apcashd.drcodes.assign(GetDoctor());

 if(dEntry < 0)
   {
	 apcashd.debit.assignDouble(0.00);
    apcashd.credit.assignDouble(fabs(dEntry));
	}
 else if (dEntry > 0)
   {
    apcashd.debit.assignDouble(fabs(dEntry));
	 apcashd.credit.assignDouble(0.00);
   }
 apcashd.append();

 return;
}



	 /*
	 if(dVatPercentage > 0 && dRebatePercentage == 0)
	  {
		CreateVatSlip(apcash,apcashd,D);
	  }
	 else if(dVatPercentage > 0 && dRebatePercentage > 0)
	  CreateVat_RebateSlip(apcash,apcashd,D);
	 else if (dVatPercentage == 0 && dRebatePercentage > 0)
	  CreateRebateSlip(apcash,apcashd,D);
	 else
	  CreateNewApSlip(apcash,apcashd,D);
	  SetField(IDD_STATUSBOX,"");
	  bTransaction = TRUE;
	  return;
	}
  else if (!bModify)
	{
	  if(dGetField(ID_INVOICE) == 0)
		{
		  SetFocus(GetDlgItem(HWindow,ID_INVOICE));
		  return;
		}
	  AddToCurrentSlip(iBatchno);
	  SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	  return;
	}
  */
