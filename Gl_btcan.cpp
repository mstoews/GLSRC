 /*
	General Ledger  - (C) Copyright 1996 by Tegara Corporation
	Delete General Ledger batch.
*/
#include "std_inc.h"

/*
#include "gll.h"
#include "db_btch.h"
#include "db_btchd.h"
#include "gl_btcan.h"
#include "widgets.h"
*/


#define ID_DELETE_BATCH 5015

void TGlWindow::GLDeleteBatch (RTMessage)
{
  GetApplication()->ExecDialog(new TCancelBatch(this,1043));
}

void TCancelBatch::RedrawAR()
{
 Field4 batchno(arbtch,1);
 Field4 invoiceno(arbtch,7);
 Field4 reference(arbtch,14);
 Field4 payable(arbtch,21);

 Relate4set rQuery(arbtch);
 rQuery.querySet("MODE = '1'");
 int i = 1;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,batchno.str());
		List->SetData(2,i,reference.str());
		List->SetData(3,i,invoiceno.str());
		List->SetData(4,i,payable.str());
	  i++;
  }

 if(i>12)
  List->SetMaxRows(i--);
 else
  List->SetMaxRows(12);
 rQuery.unlock();
 rQuery.free();
}

void TCancelBatch::RedrawGL()
{
 long i=1;
 glbtch.top();
 while(!glbtch.eof())
 {
	  if(StrCmp(glbtch.mode.str(),"1")==0)
	  {
		List->SetData(1,i,glbtch.batchno.str());
		List->SetData(2,i,glbtch.descript.str());
		List->SetData(3,i,glbtch.debit.str());
		List->SetData(4,i,glbtch.credit.str());
	  i++;
	  }
	  glbtch.skip(1);
 }

 if((i-1) > 15)
  List->SetMaxRows(i-1);
 else
  List->SetMaxRows(15);
}

void TCancelBatch::RedrawCash()
{
 Field4 batchno(apcash,  1);
 Field4 descript(apcash, 14);
 Field4 invoiceno(apcash,8);
 Field4 payable(apcash,  18);
 long i=1;
 Relate4set rQuery(apcash);
 // "MODE = '1' .AND. PAYABLE > 0"
 rQuery.querySet(GetString(1041));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,batchno.str());
		List->SetData(2,i,descript.str());
		List->SetData(3,i,invoiceno.str());
		List->SetData(4,i,payable.str());
	  i++;
  }
 rQuery.unlock();
 rQuery.free();

 if((i-1) > 15)
  List->SetMaxRows(i-1);
 else
  List->SetMaxRows(15);


}

void TCancelBatch::RedrawAP()
{
 Field4 batchno(apbtch,  1);
 Field4 descript(apbtch, 13);
 Field4 invoiceno(apbtch,8);
 Field4 payable(apbtch,  17);


 long i=1;
 Relate4set rQuery(apbtch);
 //"MODE = '1' .AND. PAYABLE > 0"
 rQuery.querySet(GetString(1041));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		List->SetData(1,i,batchno.str());
		List->SetData(2,i,descript.str());
		List->SetData(3,i,invoiceno.str());
		List->SetData(4,i,payable.str());
	  i++;
  }
 rQuery.unlock();
 rQuery.free();

}

void TCancelBatch::HasChanged(RTMessage)
{
		int iTab = HAGetActive(GetDlgItem(HWindow,1001));
		if(iCurrentTab != iTab)
		{
		 iCurrentTab = iTab;
		 List->ClearData(SS_ALLCOLS,SS_ALLROWS);
		 switch(iCurrentTab)
			{
			 case 0 : RedrawGL(); break;
			 case 1 : RedrawAR(); break;
			 case 2 : RedrawAP(); break;
			 case 3 : RedrawCash(); break;
			}
		}
}




void TCancelBatch::CancelGL(long lBatchno,char* szBatchno)
{
 CHourGlass wait;
 string szMsg;
 szMsg = GetString(ID_DELETE_BATCH);
 szMsg += szBatchno;
 glbtch.SetIndex("batchno");
 if(glbtch.seek(lBatchno)==0)
 {
  if( MessageBox(HWindow,(LPSTR)szMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 glbtch.mode.assign("2");
	 glbtchd.SetIndex("batchno");
	 if(glbtchd.seek(lBatchno) == 0)
		{
			while(long (Str4ptr(glbtchd.batchno.str())) == lBatchno)
				{
					glbtchd.debit.assignDouble(0);
					glbtchd.credit.assignDouble(0);
					glbtchd.mode.assign("2");
					glbtchd.descript.assign((LPSTR)strCancelled.c_str());
					glbtchd.skip();
				}
		}
	}
 }
}

void TCancelBatch::CancelAR(long lBatchno,char* szBatchno)
{
 Field4 mode(arbtch,2);
 Field4 reference(arbtch,14);
 Field4 payable(arbtch,21);

 Field4 batchno(arbtchd,1);
 Field4 descript(arbtchd,3);
 Field4 debit(arbtchd,11);
 Field4 credit(arbtchd,12);

 string szMsg;
 szMsg = GetString(ID_DELETE_BATCH);
 szMsg += szBatchno;

 if(arbtch.seek(lBatchno)==0)
 {
  if( MessageBox(HWindow,(LPSTR)szMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign((LPSTR)strCancelled.c_str());
	 mode.assign("2");
	 payable.assignDouble(0);
	 arbtch.UpdateUserTime(szUser);
	 arbtch.flush();
	 if(arbtchd.seek(lBatchno) == 0)
		{

		 while(long (Str4ptr(batchno.str())) == lBatchno)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign((LPSTR)strCancelled.c_str());
			arbtchd.flush();
			arbtchd.skip();
		 }
	  }
	}
 }
 return;


}

#pragma argsused
void TCancelBatch::CancelAP(long lBatchno,char* szBatchno)
{
 Field4 mode (apbtch,2);
 Field4 reference(apbtch,12);
 Field4 payable(apbtch,17);
 Field4 vatamt(apbtch,23);


 Field4 batchno(apbtchd,1);
 Field4 descript(apbtchd,3);
 Field4 debit(apbtchd,9);
 Field4 credit(apbtchd,10);

 if(apbtch.seek(lBatchno)==0)
 {
  if( MessageBox(HWindow,GetString(ID_DELETE_BATCH),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign(GetString(1042));
	 payable.assignDouble(0);
	 vatamt.assignDouble(0);
	 mode.assign("2");
	 apbtch.UpdateUserTime(szUser);
	 apbtch.flush();
	 if(apbtchd.seek(lBatchno) == 0)
		{

		 while(long (Str4ptr(batchno.str())) == lBatchno)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign(GetString(1042));
			apbtchd.flush();
			apbtchd.skip();
		 }
	  }
	 apbtchd.unlock();
	 apbtch.unlock();
	}
 }
 return;
}

#pragma argsused
void TCancelBatch::CancelCash(long lBatchno,char* szBatchno)
{
 Field4 mode (apcash,2);
 Field4 reference(apcash,13);
 Field4 payable(apcash,18);
 Field4 vatamt(apcash,24);
 Field4 batchno(apcashd,1);
 Field4 descript(apcashd,3);
 Field4 debit(apcashd,9);
 Field4 credit(apcashd,10);


 if(apcash.seek(lBatchno)==0)
 {
  if( MessageBox(HWindow,GetString(ID_DELETE_BATCH),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign(GetString(1042));
	 payable.assignDouble(0);
	 vatamt.assignDouble(0);
	 mode.assign("2");
	 apcash.UpdateUserTime(szUser);
	 apcash.flush();
	 if(apcashd.seek(lBatchno) == 0)
		{
		 while(long (Str4ptr(batchno.str())) == lBatchno)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign(GetString(1042));
			apcashd.flush();
			apcashd.skip();
		 }
	  }
	}
 }
 return;

}

void TCancelBatch::GLSelectSlip (RTMessage Msg)
{
 char szBatch[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 List->GetData(1,Cell->Row,szBatch);
 long lRecord=atol(szBatch);
 List->DelRow(Cell->Row);
 switch (iCurrentTab)
  {
	 case 0:CancelGL   (lRecord,szBatch); break;
	 case 1:CancelAR   (lRecord,szBatch); break;
	 case 2:CancelAP   (lRecord,szBatch); break;
	 case 3:CancelCash (lRecord,szBatch); break;
  }
 return;
}



void TCancelBatch::Cancel (RTMessage)
{
 glbtch.close();
 glbtchd.close();
 apbtch.close();
 apbtchd.close();
 apcash.close();
 apcashd.close();
 arbtch.close();
 arbtchd.close();
 ShutDownWindow();
}

void TCancelBatch::SetupWindow()
{
 TMainDialog::SetupWindow();
 CHourGlass wait;
 SS_CELLTYPE 	CellType;
 HFONT   	hFont;

 SetCaption("Cancel General Ledger Voucher");

 strCancelled = "Cancelled";

 if(glbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glbtch.GetDbfName());
	return;
  }


 if(glbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glbtchd.GetDbfName());
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

 if(arbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(arbtchd.GetDbfName());
	return;
  }
 if(arbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(arbtch.GetDbfName());
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



 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 List->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 List->SetBool(SSB_PROTECT,TRUE);
 List->SetBool(SSB_HORZSCROLLBAR,FALSE);
 List->SetBool(SSB_SHOWROWHEADERS,FALSE);

 List->SetMaxCols( 4);
 List->SetMaxRows(glbtch.recCount());

 List->SetData(1,SS_HEADER, GetString(8141));
 List->SetData(2,SS_HEADER, GetString(8106));
 List->SetData(3,SS_HEADER, GetString(6069));
 List->SetData(5,SS_HEADER, GetString(6128));

 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 RECT rc;
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
		  - d4Width
		  - GetSystemMetrics(SM_CXHSCROLL);

 List->SetColWidthInPixels(2,ColWidth);

 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(1, SS_ALLROWS, &CellType);
 List->SetCellType(2, SS_ALLROWS, &CellType);

 List->SetTypeFloat(&CellType,FS_SEPARATOR,13,2,0,atof(GetString(6107)));
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetCellType(4, SS_ALLROWS, &CellType);

 RedrawGL();
 iCurrentTab = 0;
}



