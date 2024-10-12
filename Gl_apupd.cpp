/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	AP Update Procedure.
*/
#include "std_inc.h"

/*
#include "gll.h"
#include "db_accts.h"
#include "db_dist.h"
#include "db_btch.h"
#include "db_btchd.h"
#include "db_jsumh.h"
#include "ap_jsum.h"
#include "gl_sum.h"
#include "gl_dlg.h"
#include "ar_jsum.h"


#include "gl_apupd.h"  // Update from AP dialog box class.
#include "utildlg.h"
#include "math.h"
*/

extern HINSTANCE hInst;


char szMessage[40];

void TGlWindow::GLUpdateAR (RTMessage ){}


void TGlWindow::GLUpdateAP (RTMessage )
 {
	GetApplication()->ExecDialog(new TGLAPUpdate(this,1030));
 }

void TGLAPUpdate::ARUpdate()
{
 //cb->exclusive = TRUE;
 /*
 if(gljsumh.open(*cb) < 0)
  {
	ErrorClose("GL_BTCHD");
	return;
  }
 */

 glaccts.open(cb,"GLACCTS");
 if(!glaccts.isValid())
  {
	ErrorClose("GLACCTS");
	return;
  }

 glbtchd.open(*cb);
  if(!glbtchd.isValid())
  {
	ErrorClose("GL_BTCHD");
	return;
  }
 glbtchd.init();

 glbtch.open(*cb);
  if(!glbtch.isValid())
  {
	ErrorClose(glbtch.GetDbfName());
	return;
  }
 glbtch.init();


  arjsum.open(*cb);
  if(!arjsum.isValid())
  {
  ErrorClose(arjsum.GetDbfName());
  return;
  }
 arjsum.init();

 glsum.open(*cb,"GL_SUM");
  if(!glsum.isValid())
  {
	ErrorClose("gl_sum");
	return;
  }
 glsum.init();
 gl_setat.open(*cb);
 if(!gl_setat.isValid())
  {
	 ErrorClose("GL_SETAT");
	 return;
  }



 period  =  ((TGlWindow *)Parent)->iGetPeriod();

 descript = new Field4(glaccts,"descript");

 SetField(ID_ADD,GetString(1044));
 // "Accounts Receivable GL Update"
 GetPrivateProfileString("GL_UPDATE","AR","",szMessage,sizeof(szMessage),"GL.INI");
 //strcpy(szMessage,GetString(1045));
 glbtch.SetIndex(GetString(8140));
 int iEntryno=0;
 int iSlipNumber = GetNextSlipNumber();
 glsum.zap();
 glsum.pack();
 glsum.SetIndex(GetString(8145));
 arjsum.SetIndex(GetString(8145));
 BOOL bHasEntries = FALSE;
 Relate4set rQuery(arjsum);
 rQuery.querySet(GetString(1046));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
	iEntryno++;
	CreateSlipDetailEntry("AR",&arjsum.account,&arjsum.dept,&arjsum.depart,iSlipNumber,iEntryno,arjsum.Debit(),arjsum.Credit());
	arjsum.mode.assign("2");
	SetField(ID_ADD,arjsum.account.str());
	arjsum.flush();

	arjsum.UpdateUserTime(szUser);
	bHasEntries = TRUE;
  }

 rQuery.unlock();
 rQuery.free();
 //int iPeriod = GetCurrentPeriodn();
 if(bHasEntries)
 {
  UpdateGLBatch(iSlipNumber,"AR");
  UpdateArBatch();
 }
 glbtchd.close();
 glbtch.close();
 arjsum.close();
 glsum.close();
 glaccts.close();
 //gljsumh.close();
 gl_setat.close();
 //cb->exclusive = FALSE;
 return;
}


void TGLAPUpdate::SetupWindow()
{
 TMainDialog::SetupWindow();
}

void TGLAPUpdate::Cancel (RTMessage)
{
 ShutDownWindow();
}

void TGLAPUpdate::APUpdate ()
{
 //cb->exclusive = TRUE;
 /*
 if(gljsumh.open(*cb) < 0)
  {
	ErrorClose(gljsumh.GetDbfName());
	return;
  }
 */
 glaccts.open(cb,"GLACCTS");
 if(!glaccts.isValid())
  {
	ErrorClose("GLACCTS");
	return;
  }

 glbtchd.open(*cb);
  if(!glbtchd.isValid())
  {
	ErrorClose("GL_BTCHD");
	return;
  }
 glbtchd.init();

 glbtch.open(*cb);
  if(!glbtch.isValid())
  {
	ErrorClose(glbtch.GetDbfName());
	return;
  }
 glbtch.init();

  apjsum.open(*cb);
  if(!apjsum.isValid())
  {
  ErrorClose(apjsum.GetDbfName());
  return;
  }
 apjsum.init();

 glsum.open(*cb,"GL_SUM");
  if(!glsum.isValid())
  {
	ErrorClose("GL_SUM");
	return;
  }
 glsum.init();

 gl_setat.open(*cb);
 if(!gl_setat.isValid())
  {
	 ErrorClose("GL_SETAT");
	 return;
  }

 period  =  ((TGlWindow *)Parent)->iGetPeriod();

 descript = new Field4(glaccts,"descript");

 SetField(ID_ADD,GetString(1047));
 //"Update GL from AP..."
 //strcpy(szMessage,GetString(1049));
 // "Accounts Payable GL Update"
 GetPrivateProfileString("GL_UPDATE","AP","",szMessage,sizeof(szMessage),"GL.INI");
 glbtch.SetIndex(GetString(8140));
 int iEntryno=0;
 int iSlipNumber = GetNextSlipNumber();
 glsum.zap();
 glsum.pack();
 glsum.SetIndex(GetString(8145));
 apjsum.SetIndex(GetString(8145));
 BOOL bHasEntries = FALSE;

 Relate4set rQuery(apjsum);
 // "MODE = ' ' .OR. MODE = '1'"
 rQuery.querySet(GetString(1050));
 BOOL bFound = TRUE;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
	string strAccount(apjsum.account.str());
	strAccount += apjsum.dept.str();

	if(glaccts.seek((LPSTR)strAccount.c_str()) !=0 )
	 {
		string strMsg(hInst,7731);
		strMsg += strAccount;
		strMsg += "was not found.";
		MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
		bFound = FALSE;
		break;
	 }
 }
 if(bFound == FALSE)
 return;
 string lookup;
 char szBatchno[10];
 for(qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
	iEntryno++;
	CreateSlipDetailEntry("AP",&apjsum.account,&apjsum.dept,&apjsum.depart,iSlipNumber,iEntryno,apjsum.Debit(),apjsum.Credit());
	apjsum.mode.assign("2");
	apjsum.glbatchno.assignDouble(iSlipNumber);
	lookup = ltoa(iSlipNumber,szBatchno,10);
	lookup += apjsum.account.str();
	lookup += apjsum.dept.str();
	apjsum.lookup.assign((LPSTR)lookup.c_str());
	apjsum.UpdateUserTime(szUser);
	SetField(ID_ADD,apjsum.account.str());
	apjsum.flush();

	bHasEntries = TRUE;
  }
 rQuery.unlock();
 rQuery.free();


 /*
 for(glsum.top();!glsum.eof();glsum.skip())
  {
	 double difference = glsum.Debit() - glsum.Credit();
	 if(difference > 0)
		{
		  glsum.debit.assignDouble(difference);
		  glsum.credit.assignDouble(0);
		}
	 else
		{
		  glsum.debit.assignDouble(0);
		  glsum.credit.assignDouble(fabs(difference));
		}
	 if(difference == 0)
		  glsum.deleteRec();
  }
	glsum.pack();
 */
 //int iPeriod = GetCurrentPeriodn();
 if(bHasEntries)
 {
  UpdateGLBatch(iSlipNumber,"AP");
  UpdateApBatch();
 }
 glbtchd.close();
 glbtch.close();
 apjsum.close();
 glsum.close();
 glaccts.close();
 //gljsumh.close();
 gl_setat.close();
 //cb->exclusive = FALSE;
 return;
}

void TGLAPUpdate::UpdateApBatch()
{
 Data4  apbtch;
 apbtch.open(cb,"AP_BTCH");
 apbtch.SetIndex("batchno");
 Field4 mode(apbtch,"mode");

 Relate4set fQuery(apbtch);
 fQuery.querySet("MODE = '0'");
 for(int qc=fQuery.top();qc!=r4eof;qc=fQuery.skip(1L))
  {
	mode.assign("2");
	apbtch.UpdateUserTime(szUser);
	apbtch.flush();
  }
 fQuery.unlock();
 fQuery.free();
 apbtch.close();
}

void TGLAPUpdate::UpdateArBatch()
{
 Data4  apbtch;
 apbtch.open(cb,"AR_BTCH");

 Field4 mode(apbtch,"mode");
 apbtch.SetIndex("batchno");
 Relate4set fQuery(apbtch);
 fQuery.querySet("MODE = '0'");
 for(int qc=fQuery.top();qc!=r4eof;qc=fQuery.skip(1L))
  {
	apbtch.UpdateUserTime(szUser);
	mode.assign("2");
	apbtch.flush();
  }
 fQuery.unlock();
 fQuery.free();
 apbtch.close();
}


#pragma argsused
void TGLAPUpdate::CreateSlipDetailEntry(char *strType,Field4 *account,Field4 *dept,Field4 *depart,int iSlipNumber,int Entryno,double Debit,double Credit)
{

 string strAccount(account->str());
 strAccount += dept->str();

 glaccts.SetIndex(GetString(8145));
 if(glaccts.seek((LPSTR)strAccount.c_str()) !=0 )
  {
	string strMsg(hInst,7731);
	strMsg += strAccount;
	strMsg += "was not found.";
	MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	return ;
  }
 Field4 descript(glaccts,3);
 strAccount += depart->str();

 Date4 Today;
 Today.today();

 if(glsum.seek((LPSTR)strAccount.c_str()) == 0)
  {
		glsum.debit.assignDouble(glsum.Debit() + Debit);
		glsum.credit.assignDouble(glsum.Credit() + Credit);
  }
  else
 {
		glsum.appendStart();
		glsum.account.assign(account->str());
		glsum.dept.assign(dept->str());
		glsum.depart.assign(depart->str());
		glsum.reference.assign(descript.str());
		glsum.period.assignLong(period);
		glsum.mode.assign("1");
		glsum.date.assign(Today.str());
		glsum.type.assign(strType);
		glsum.debit.assignDouble(Debit);
		glsum.credit.assignDouble(Credit);
		glsum.append();
 }
}

char *TGLAPUpdate::UpdateGLBatch(long iSlipNumber,char* szType)
{
  Date4 Today;
  Today.today();
  Today.format(GetString(4023));

  glsum.top();
  int lineno = 1;
  double DebitTotal = 0;
  double CreditTotal = 0;
  char *szAccount = new char [ glsum.account.len() + glsum.dept.len() + 1];
  string lookup;
  char batchno[10];
  while(!glsum.eof())
  {
	strcpy(szAccount,glsum.account.str());
	strcat(szAccount,glsum.dept.str());
	glaccts.seek(szAccount);
	SetField(ID_ADD,szAccount);
	glbtchd.appendStart();
	glbtchd.account.assign(glsum.account.str());
	glbtchd.dept.assign(glsum.dept.str());
	glbtchd.mode.assign("1");
	glbtchd.batchno.assignLong(iSlipNumber);
	glbtchd.journal.assignLong(lineno);
	glbtchd.date.assign(glsum.date.str());
	glbtchd.period.assignLong(((TGlWindow *)Parent)->iGetPeriod());
	glbtchd.reference.assign(descript->str());
	glbtchd.descript.assign(descript->str());
	glbtchd.debit.assignDouble(glsum.Debit());
	glbtchd.credit.assignDouble(glsum.Credit());
	glbtchd.depart.assign(glsum.depart.str());
	glbtchd.append();

	/*
	gljsumh.appendStart();
	gljsumh.glbatch.assign(iSlipNumber);
	gljsumh.type.assign(szType);
	lookup = ltoa(iSlipNumber,batchno,10);
	lookup += glsum.account.str();
	lookup += glsum.dept.str();
	gljsumh.lookup.assign((LPSTR)lookup.c_str());
	gljsumh.append();
	*/

	DebitTotal = DebitTotal + glsum.Debit();
	CreditTotal = CreditTotal + glsum.Credit();
	lineno++;
	glsum.skip();
  }

  glbtch.appendStart();
  glbtch.batchno.assignLong(iSlipNumber);
  glbtch.mode.assign("1");
  glbtch.date.assign(Today.str());
  glbtch.type.assign("AP");
  glbtch.period.assignLong(((TGlWindow *)Parent)->iGetPeriod());
  glbtch.descript.assign(szMessage);
  glbtch.debit.assignDouble(DebitTotal);
  glbtch.credit.assignDouble(CreditTotal);
  glbtch.entryno.assignLong(lineno);
  glbtch.UpdateUserTime(szUser);
  glbtch.append();

  /*
  gljsumh.appendStart();
  gljsumh.glbatch.assignLong(iSlipNumber);
  gljsumh.type.assign("AP");
  gljsumh.UpdateUserTime(szUser);
  gljsumh.append();
  */

  delete szAccount;
  return (LPSTR)lookup.c_str();
}

int TGLAPUpdate::GetNextSlipNumber()
{
 glbtch.SetIndex(GetString(8140));
 glbtch.bottom();
 int batchno = (int)glbtch.Batchno();
 batchno++;
 if(batchno == 0)
  batchno = 1;
 return batchno;
}


void TGLAPUpdate::Ok (RTMessage)
{
  CHourGlass wait;
  if(SendMessage(GetDlgItem(HWindow,1001),BM_GETCHECK,NULL,NULL))
	APUpdate();
  if(SendMessage(GetDlgItem(HWindow,1002),BM_GETCHECK,NULL,NULL))
	ARUpdate();
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