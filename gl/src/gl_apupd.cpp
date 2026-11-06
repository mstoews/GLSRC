/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   AP Update Procedure.
*/
#include "gll.h"
#include "db_profi.h"
#include "db_dept.h"
#include "db_accts.h"
#include "db_dist.h"
#include "db_btch.h"
#include "db_btchd.h"
#include "ap_jsum.h"
#include "gl_sum.h"
#include "gl_dlg.h"
#include "ar_jsum.h"

#include "gl_apupd.h"  // Update from AP dialog box class.
#include "utildlg.h"


extern Code4 cb;
char szMessage[40];

void TGlWindow::GLUpdateAR (RTMessage ){}


void TGlWindow::GLUpdateAP (RTMessage )
 {
   GetApplication()->ExecDialog(new TGLAPUpdate(this,1030));   
 }

void TGLAPUpdate::ARUpdate()
{
 SetField(ID_ADD,GetString(1044));
 // "Accounts Receivable GL Update"
 strcpy(szMessage,GetString(1045));
 glbtch->SetIndex(GetString(8140));
 int iEntryno=0;
 int iSlipNumber = GetNextSlipNumber();
 glsum->zap();
 glsum->pack();
 glsum->SetIndex(GetString(8145));
 arjsum->SetIndex(GetString(8145));
 BOOL bHasEntries = FALSE;
 Relate4set *rQuery = new Relate4set(arjsum);
 rQuery->querySet(GetString(1046));
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
   iEntryno++;
   CreateSlipDetailEntry(arjsum->account,arjsum->dept,iSlipNumber,iEntryno,arjsum->Debit(),arjsum->Credit());
   arjsum->mode->assign("2");
   SetField(ID_ADD,arjsum->account->str());
   arjsum->flush();
   arjsum->unlock();
   bHasEntries = TRUE;
  }

 rQuery->unlock();
 rQuery->free();
 int iPeriod = GetCurrentPeriodn();
 if(bHasEntries)
 {
  UpdateGLBatch(iSlipNumber);  
  UpdateArBatch();
 }
 return;
}


void TGLAPUpdate::SetupWindow()
{
 TMainDialog::SetupWindow(); 
 glaccts =  new Data4(&cb,"glaccts"); 
 glbtchd =  new Tglbtchd(&cb); 
 glbtch  =  new Tglbtch(&cb);
 apjsum  =  new Tapjsum(&cb);
 arjsum  =  new Tarjsum(&cb); 
 glsum   =  new Tglsum(&cb); 
 period  =  iGetPeriod();
 descript = new Field4(*(Data4 *)glaccts,"descript");
}

void TGLAPUpdate::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGLAPUpdate::APUpdate ()
{
 SetField(ID_ADD,GetString(1047));
 //"Update GL from AP..."
 strcpy(szMessage,GetString(1049));
 // "Accounts Payable GL Update"
 glbtch->SetIndex(GetString(8140));
 int iEntryno=0;
 int iSlipNumber = GetNextSlipNumber();
 glsum->zap();
 glsum->pack();
 glsum->SetIndex(GetString(8145));
 apjsum->SetIndex(GetString(8145));
 BOOL bHasEntries = FALSE;

 Relate4set *rQuery = new Relate4set(apjsum);
 // "MODE = ' ' .OR. MODE = '1'"
 rQuery->querySet(GetString(1050));
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
   iEntryno++;
   CreateSlipDetailEntry(apjsum->account,apjsum->dept,iSlipNumber,iEntryno,apjsum->Debit(),apjsum->Credit());
   apjsum->mode->assign("2");
   SetField(ID_ADD,apjsum->account->str());
   apjsum->flush();
   apjsum->unlock();
   bHasEntries = TRUE;
  }
 
 rQuery->unlock();
 rQuery->free();
 int iPeriod = GetCurrentPeriodn();
 if(bHasEntries)
 {
  UpdateGLBatch(iSlipNumber);
  UpdateApBatch();
 }
 return;
}

void TGLAPUpdate::UpdateApBatch()
{
 Data4  apbtch;
 apbtch.open(cb,"ap_btch");
 apbtch.SetIndex(GetString(8140));
 Field4 mode(apbtch,"mode");

 Relate4set *rQuery = new Relate4set(apbtch);
 rQuery->querySet("MODE = '0'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {  
	mode.assign("2");
	apbtch.UpdateUserTime(szUser);
	apbtch.flush();
  }
 rQuery->unlock();
 rQuery->free();
 apbtch.close();
}

void TGLAPUpdate::UpdateArBatch()
{
 Data4  apbtch;
 apbtch.open(cb,"ar_btch");
 apbtch.SetIndex(GetString(8140));
 Field4 mode(apbtch,"mode");

 Relate4set *rQuery = new Relate4set(apbtch);
 rQuery->querySet("MODE = '0'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
	apbtch.UpdateUserTime(szUser);
	mode.assign("2");
	apbtch.flush();
  } 
 rQuery->unlock();
 rQuery->free();
 apbtch.close();
}


#pragma argsused
void TGLAPUpdate::CreateSlipDetailEntry(Field4 *account,Field4 *dept,int iSlipNumber,int Entryno,double Debit,double Credit)
{
 char *szAccount = new char[account->len() + dept->len() +1];
 strcpy(szAccount,account->str());
 strcat(szAccount,dept->str());

 glaccts->SetIndex(GetString(8145));
 if(glaccts->seek(szAccount) !=0 )
  MessageBox(HWindow,"Can't find ",szAccount,MB_OK);
 Field4 descript(*(Data4 *)glaccts,"Descript");

 Date4 Today;
 Today.today();

 if(glsum->seek(szAccount) == 0)
  {
	glsum->debit->assignDouble(glsum->Debit() + Debit);
	glsum->credit->assignDouble(glsum->Credit() + Credit);
   glsum->flush();
   glsum->unlock();

  }
 else
  {
   glsum->appendStart();
   glsum->account->assign(account->str());
   glsum->dept->assign(dept->str());
   glsum->reference->assign(descript.str());
   glsum->period->assignLong(period);
   glsum->mode->assign("1");
   glsum->date->assign(Today.str());
   glsum->type->assign("AP");
   glsum->debit->assignDouble(Debit);
   glsum->credit->assignDouble(Credit);
   glsum->append();
   glsum->unlock();
  }
 delete szAccount;
}

void TGLAPUpdate::UpdateGLBatch(iSlipNumber)
{
  Date4 Today;
  Today.today();

  glsum->top();
  int lineno = 0;
  double DebitTotal = 0;
  double CreditTotal = 0;
  char *szAccount = new char [ glsum->account->len() + glsum->dept->len() + 1];

  while(!glsum->eof())
  {  
   strcpy(szAccount,glsum->account->str());
	strcat(szAccount,glsum->dept->str());
   glaccts->seek(szAccount);
   SetField(ID_ADD,szAccount);
   glbtchd->appendStart();
   glbtchd->account->assign(glsum->account->str());
   glbtchd->dept->assign(glsum->dept->str());    
   glbtchd->mode->assign("1");
   glbtchd->batchno->assignLong(iSlipNumber);
   glbtchd->date->assign(glsum->date->str());
   glbtchd->period->assignLong(iGetPeriod());
   glbtchd->reference->assign(descript->str());
   glbtchd->descript->assign(descript->str());
   glbtchd->debit->assignDouble(glsum->Debit());
   glbtchd->credit->assignDouble(glsum->Credit());
   glbtchd->append();
   glbtchd->unlock();
   DebitTotal = DebitTotal + glsum->Debit();
   CreditTotal = CreditTotal + glsum->Credit();
   lineno++;
   glsum->skip();
  }
   
  glbtch->appendStart();
  glbtch->batchno->assignLong(iSlipNumber);
  glbtch->mode->assign("1");
  glbtch->date->assign(Today.str());
  glbtch->period->assignLong(iGetPeriod());
  glbtch->descript->assign(szMessage);
  glbtch->debit->assignDouble(DebitTotal);
  glbtch->credit->assignDouble(CreditTotal);
  glbtch->entryno->assignLong(lineno);
  glbtch->UpdateUserTime(szUser);
  glbtch->append();
  glbtch->unlock();
  delete szAccount;

}

int TGLAPUpdate::GetNextSlipNumber()
{
 glbtch->SetIndex(GetString(8140));
 glbtch->bottom();
 int batchno = (int)glbtch->Batchno();
 batchno++;
 if(batchno == 0)
  batchno = 1;
 return batchno;
}


void TGLAPUpdate::Ok (RTMessage)
{
  if(SendMessage(GetDlgItem(HWindow,1001),BM_GETCHECK,NULL,NULL))
   APUpdate();
  if(SendMessage(GetDlgItem(HWindow,1002),BM_GETCHECK,NULL,NULL))
   ARUpdate();
  cb.closeAll();
  cb.init();  
  ShutDownWindow();
  return;
}


void TGLAPUpdate::APCheck (RTMessage)
{
 if(SendMessage(GetDlgItem(HWindow,1001),BM_GETCHECK,NULL,NULL))
  SendMessage(GetDlgItem(HWindow,1001),BM_SETCHECK,FALSE,NULL);
 else
  SendMessage(GetDlgItem(HWindow,1001),BM_SETCHECK,TRUE,NULL);
}

void TGLAPUpdate::ARCheck (RTMessage)
{
 if(SendMessage(GetDlgItem(HWindow,1002),BM_GETCHECK,NULL,NULL))
  SendMessage(GetDlgItem(HWindow,1002),BM_SETCHECK,FALSE,NULL);
 else
  SendMessage(GetDlgItem(HWindow,1002),BM_SETCHECK,TRUE,NULL);
}