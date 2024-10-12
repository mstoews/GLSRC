/*
	Post Batch - (C) Copyright 1992 by Tegara Corporation
	gl_postb.cpp
*/

#include "std_inc.h"

// All of the database include files are in gl_postb.h
#include "widgets.h"
#include "gl_postb.h"
#include "ttab.h"
#include "lzexpand.h"



extern char szMsg[256];
extern HINSTANCE hInst;
BOOL bAll;

#define ID_GLBTCH   8169
#define ID_GLBTCHD  8173
#define ID_GLDIST   8177
#define ID_ARBTCH   8170
#define ID_ARBTCHD  8174
#define ID_APBTCH   8171
#define ID_APBTCHD  8175
#define ID_DB_APCASH   8172
#define ID_APCASHD  8176
#define ID_LOCKED_FAILED 1080
#define ID_DATA_ERROR 60
#define ID_DATA_ERROR_MSG 61


int ExclusiveLock(LPSTR szDBF);
int ExclusiveLock(LPSTR szDBF)
{
 OFSTRUCT ofStrSrc;
 char szFile[12];
 strcpy(szFile,szDBF);
 strcat(szFile,".dbf");
 HFILE hFile = LZOpenFile(szFile,&ofStrSrc,OF_SHARE_EXCLUSIVE);
 LZClose(hFile);
 //return hFile;
 return (1);
}


_CLASSDEF (TCloseYear)
class TCloseYear : public TMainDialog
{
  Code4 *cb;
  BOOL bUpdated;
  public:
  TCloseYear(PTWindowsObject AParent, int iTitle):TMainDialog(AParent,iTitle)
  {
	bUpdated = TRUE;
	cb = &((TAcctModule *)GetApplication())->cb;
  };
  virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  BOOL bCheckForIncomeData();
  BOOL VerifyOpenBatch(char *strDbf);
  BOOL DeleteAll(char *szDbf);
};

void TPostBatch::HasChanged(RTMessage)
{
		int iTab = HAGetActive(GetDlgItem(HWindow,1001));
		if(iCurrentTab != iTab)
		{
		 iCurrentTab = iTab;
		 switch(iCurrentTab)
			{
			 case 0 : //GLRedraw(); break;
			 case 1 : //ARRedraw(); break;
			 case 2 : //APRedraw(); break;
			 case 3 : //CashRedraw();
			 break;
			}
		}
}

void TGlWindow::GLPostBatch (RTMessage){
 GetApplication()->ExecDialog(new TPostBatch(this,1003));
}


#pragma argsused
void TPostBatch::Query(RTMessage Msg)
{
		int iTab = HAGetActive(GetDlgItem(HWindow,1001));
		switch(iTab)
			{
			 case 0 : GLRedraw(); break;
			 case 1 : ARRedraw(); break;
			 case 2 : APRedraw(); break;
			 case 3 : CashRedraw(); break;
			}
}



void TPostBatch::ErrorClose(LPSTR szFile)
{
  string strMsg;
  wsprintf(szMsg,GetString(ID_DATA_ERROR),e4text(cb->errorCode),szFile);
  strMsg = szMsg;
  strMsg += string(hInst,ID_DATA_ERROR_MSG);
  MsgBox(strMsg,MB_ICONINFORMATION);
  cb->closeAll();
  //cb->exclusive = FALSE;
  int rc;
  rc  = cb->errorCode;
  if(rc < 0)
	cb->init();
  Parent->ShutDownWindow();
}



void TPostBatch::GL_PostAll()
{
 if(MessageBox(HWindow,GetString(6048),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 CHourGlass wait;
 if(ValidateGLAccounts())
  {
	bAll = TRUE;
	Relate4set rQuery(glbtch);
	string strQuery = GetQuery(TRUE);
	rQuery.querySet((LPSTR)strQuery.c_str());
	for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	{
	  if(glbtch.Open())
		PostGLSlip(glbtch.Batchno());
	  else if(glbtch.Adjustment())
		PostAdjustment(glbtch.Batchno());
	}
	rQuery.unlock();
	rQuery.free();
	//GLRedraw();
  }
}

void TPostBatch::Ok (RTMessage)
{
  switch (iCurrentTab)
	{
	 case 0 : GL_PostAll(); break;
	 case 1 : AR_PostAll(); break;
	 case 2 : AP_PostAll(); break;
	 case 3 : CASH_PostAll(); break;
	}
}

char *TPostBatch::GetValidateQuery()
{
  string strQuery,strPeriod,strStart,strEnd;
  //strQuery = ".NOT. DELETED()";
  strQuery = GetString(1009);
  if(strlen(StrTrim(GetField(1003))) > 0)
	{
	  if(strlen(StrTrim(GetField(1004))) > 0)
		{
		 //strQuery += " .AND. (BATCHNO >= ";
		 strQuery += GetString(1013);
		 strQuery += GetField(1003);
		 //strQuery += " .AND. BATCHNO <= ";
		 strQuery += GetString(1014);
		 strQuery += GetField(1004);
		 strQuery += ")";
		}
	  else
		{
		 //strQuery += " .AND. (BATCHNO = ";
		 strQuery += GetString(1053);
		 strQuery += GetField(1003);
		 strQuery += ")";
		}
  }


  return (LPSTR)strQuery.c_str();

}


BOOL TPostBatch::ValidateGLAccounts()
{
  BOOL bValidate = TRUE;
  Relate4set vQuery(glbtchd);
  vQuery.querySet("MODE = '1'");  // ".NOT. DELETED()"
  for(int rc = vQuery.top();rc!=r4eof;rc=vQuery.skip())
	{
		string szSeek = glbtchd.account.str();
		szSeek += glbtchd.dept.str();
		if(glaccts.seek((LPSTR)szSeek.c_str()) != 0)
		 {
		  string Message,Batch;
		  Batch = glbtchd.batchno.str();
		  Message =  GetString(5124);
		  Message += GetString(1082);
		  Message += Batch;
		  Message += GetString(1083); // "\nAccount Code : ";
		  Message += szSeek;
		  MsgBox(Message,MB_ICONINFORMATION);
		  bValidate = FALSE;
		  break;
		}
	}
  vQuery.unlock();
  vQuery.free();
  return bValidate;
}

BOOL TPostBatch::ValidateGLAccount(long lBatchno)
{
  BOOL bValidate = FALSE;
  glbtchd.SetIndex(GetString(8140));
  if(glbtchd.seek(lBatchno) == 0)
	{
	 while(glbtchd.Batchno() == lBatchno)
	 {
	  string szSeek = glbtchd.account.str();
	  szSeek += glbtchd.dept.str();
	  if(glaccts.seek((char *)szSeek.c_str()) != 0)
			{
			  string Message;
			  Message  =  GetString(5124);
			  Message +=  szSeek;
			  MsgBox(Message,MB_ICONINFORMATION);
			  bValidate = FALSE;
			  return bValidate;
			}
	 }
	 glbtchd.skip();
	 bValidate = TRUE;
  }
 return bValidate;
}


char *TPostBatch::GetQuery(BOOL bPost)
{
  string strQuery,strPeriod,strStart,strEnd;
  if (!bPost)
	strQuery = GetString(1054);  //strQuery = "(.NOT. DELETED())";
  else
	  strQuery = GetString(1055); //strQuery = "(.NOT. DELETED())";
  if(strlen(StrTrim(GetField(ID_PERIOD))) > 0)
	{
	  strQuery += GetString(1059);  //strQuery += " .AND. PERIOD = ";
	  strQuery += StrTrim(GetField(ID_PERIOD));
	}
	if(strlen(StrTrim(GetField(1003))) > 0)
	{
	  if(strlen(StrTrim(GetField(1004))) > 0)
		{
		 strQuery += GetString(1063); //strQuery += " .AND. (BATCHNO >= ";
		 strQuery += StrTrim(GetField(1003));
		 strQuery += GetString(1053); //strQuery += " .AND. BATCHNO <= ";
		 strQuery += StrTrim(GetField(1004));
		 strQuery += ")";
		}
	  else
		{
		 strQuery += GetString(1070);  //strQuery += " .AND. (BATCHNO = ";
		 strQuery += GetField(1003);
		 strQuery += ")";
		}
  }
  return (LPSTR)strQuery.c_str();
}



void TPostBatch::GLRedraw()
 {
  CHourGlass wait;
  TMainDialog test(this,1016);
  GetApplication()->MakeWindow(&test);
  test.SetField(ID_TEXT,GetString(334));
  test.Show(SW_SHOW);
  string strMsg1,strMsg;
  strMsg1 = GetString(335);

  List->SetBool(SSB_REDRAW,FALSE);
  List->SetMaxRows(glbtch.recCount());
  List->ClearData(SS_ALLCOLS,SS_ALLROWS);
  Field4 user(glbtch,"user");
  Field4 update(glbtch,"update");
  long i=1;
  char szDate[11];
  Relate4set rQuery(glbtch);
  string strQuery = GetQuery();
  rQuery.querySet((LPSTR)strQuery.c_str());

  //MessageBox(HWindow,(LPSTR)strQuery.c_str(),"Query",MB_OK);
  //rQuery.unlock();
  //rQuery.free();
  //return;


  for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		strMsg = (LPSTR)strMsg1.c_str();
		List->SetData(1,i,glbtch.batchno.str());
		List->SetData(2,i,glbtch.descript.str());
		Date4 UpDate(update.str());
		UpDate.format(szDate,GetString(4024));
		List->SetData(3,i,szDate);   // Due Date
		List->SetData(4,i,user.str());
		List->SetData(6,i,glbtch.debit.str());
		strMsg += glbtch.batchno.str();
		test.SetField(ID_NUMBER,(LPSTR)strMsg.c_str());
		SetFocus(test.HWindow);
		i++;
  }
  rQuery.unlock();
  rQuery.free();
  RECT rc;
  short iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(i < cRows)
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(i-1);
	List->SetBool(SSB_REDRAW,FALSE);
	return;
}



void TPostBatch::Cancel (RTMessage)
{
	glbtch.close();
	glbtchd.close();
	glaccts.close();
	gldist.close();

	arbtch.close();
	arbtchd.close();
	ardist.close();
	arjsum.close();

	apbtch.close();
	apbtchd.close();
	apdist.close();
	apjsum.close();

	apcash.close();
	apcashd.close();
	ShutDownWindow();
}


BOOL TPostBatch::UpdateGL(Field4 *account, Field4 *dept)
{
	BOOL bValid = FALSE;
	//glaccts.SetIndexAccts();
	if(glaccts.SetIndexAccts(1) < 0)
	{
	 ShutDownWindow();
	 return bValid;
	}

	string strSeek(account->str());
	strSeek += dept->str();
	if(glaccts.seek((LPSTR)strSeek.c_str()) ==0)
		{
		 int iPeriod = glbtchd.iPeriod();
		 double credit = glbtchd.Credit();
		 double debit  = glbtchd.Debit();
		 switch(iPeriod)
		 {
		  case 1  : glaccts.period1.assignDouble(glaccts.dPeriod1()+debit-credit);break;
		  case 2  : glaccts.period2.assignDouble(glaccts.dPeriod2()+debit-credit);break;
		  case 3  : glaccts.period3.assignDouble(glaccts.dPeriod3()+debit-credit);break;
		  case 4  : glaccts.period4.assignDouble(glaccts.dPeriod4()+debit-credit);break;
		  case 5  : glaccts.period5.assignDouble(glaccts.dPeriod5()+debit-credit);break;
		  case 6  : glaccts.period6.assignDouble(glaccts.dPeriod6()+debit-credit);break;
		  case 7  : glaccts.period7.assignDouble(glaccts.dPeriod7()+debit-credit);break;
		  case 8  : glaccts.period8.assignDouble(glaccts.dPeriod8()+debit-credit);break;
		  case 9  : glaccts.period9.assignDouble(glaccts.dPeriod9()+debit-credit);break;
		  case 10 : glaccts.period10.assignDouble(glaccts.dPeriod10()+debit-credit);break;
		  case 11 : glaccts.period11.assignDouble(glaccts.dPeriod11()+debit-credit);break;
		  case 12 : glaccts.period12.assignDouble(glaccts.dPeriod12()+debit-credit);break;
		  default : break;
		}
		glaccts.UpdateUserTime(szUser);
		glaccts.flush();
		bValid = TRUE;
	 }
	 else
	 {
	  string strMsg(GetString(3081));
	  strMsg =+ "\nAccount No. : ";
	  strMsg =+ (LPSTR)strSeek.c_str();
	  MsgBox(strSeek,MB_ICONINFORMATION);
	 }
  return bValid;
}


void TPostBatch::PostGLSlip(long lBatchno)
{
     glbtch.seek(lBatchno);
	  glbtchd.SetIndex(GetString(8140));
	  if(double(Str4ptr(glbtch.debit.str())) == double(Str4ptr(glbtch.credit.str())))
	  {
		  double dCurrBal = 0;
		  if(glbtchd.seek(glbtch.batchno.str())==0)
		  {
			while ((long)glbtchd.Batchno() == (long)glbtch.Batchno())
			 {
			  if(UpdateGL(&glbtchd.account,&glbtchd.dept))
			  {
				 SetField(ID_BATCHNO,glbtchd.batchno);
				 dCurrBal  = double(Str4ptr(glaccts.curr_bal.str()))
				  + double(Str4ptr(glbtchd.debit.str())) -
					double(Str4ptr(glbtchd.credit.str()));

				  glaccts.curr_bal.assignDouble(dCurrBal);
				  glbtch.mode.assign("0");
				  glbtch.UpdateUserTime(szUser);
				  glbtch.flush();

				  glbtchd.mode.assign("0");
				  glbtchd.flush();
				  gldist.appendStart();
				  gldist.account.assign(glbtchd.account.str());
				  gldist.dept.assign(glbtchd.dept.str());
				  gldist.period.assign(glbtchd.period.str());
				  gldist.date.assign(glbtchd.date.str());
				  gldist.batchno.assignDouble(double(Str4ptr(glbtchd.batchno.str())));
				  gldist.reference.assign(glbtchd.reference.str());
				  gldist.cheque.assign(glbtchd.cheque.str());
				  gldist.type.assign(glbtch.type.str());
				  gldist.medrep.assign(glbtchd.depart.str());
				  gldist.prod.assign(glbtchd.product.str());
				  gldist.drcode.assign(glbtchd.drcode.str());
				  gldist.descript.assign(glbtchd.descript.str());
				  gldist.debit.assignDouble(double(Str4ptr(glbtchd.debit.str())));
				  gldist.credit.assignDouble(double(Str4ptr(glbtchd.credit.str())));
				  gldist.UpdateUserTime(szUser);
				  gldist.append();
				  glbtchd.skip(1);
			  }
			  else
			  break;
			}
	  }
	}
	else
	  {
		wsprintf(szMsg,GetString(8116),glbtch.batchno.str());
		MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
		bAll = FALSE;
		return;
	  }
}

double TPostBatch::dCurrentYearTotal()
{
	return glaccts.dPeriod1() +
	glaccts.dPeriod2() +
	glaccts.dPeriod3() +
	glaccts.dPeriod4() +
	glaccts.dPeriod5() +
	glaccts.dPeriod6() +
	glaccts.dPeriod7() +
	glaccts.dPeriod8() +
	glaccts.dPeriod9() +
	glaccts.dPeriod10() +
	glaccts.dPeriod11() +
	glaccts.dPeriod12();
}

void TPostBatch::PostAdjustment(long lBatchno)
{
  if(glbtch.seek(lBatchno) == 0)
	{
	  if(double(Str4ptr(glbtch.debit.str())) == double(Str4ptr(glbtch.credit.str())))
	  {
		double dNewOpenBal = 0.0;
		double dCurrBal = 0.0;
		double  dOpenbal = 0.0;
		//glbtchd.SetIndex("batchno");
		glbtchd.SetIndexBatchno();
		 if(glbtchd.seek(glbtch.batchno.str())==0)
		  {
			while (double(Str4ptr(glbtchd.batchno.str())) == double(Str4ptr(glbtch.batchno.str())))
			{
			  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)glbtchd.batchno.str());
			  string szSeek;
			  dNewOpenBal = 0.0;
			  dCurrBal = 0.0;
			  dOpenbal = 0.0;
			  // Post a prior year entry to the GL_DIST
			  gldist.appendStart();
			  gldist.account.assign(glbtchd.account.str());
			  gldist.dept.assign(glbtchd.dept.str());
			  gldist.period.assign(glbtchd.period.str());
			  gldist.type.assign("PG");
			  gldist.date.assign(glbtchd.date.str());
			  gldist.batchno.assignDouble(double(Str4ptr(glbtchd.batchno.str())));
			  gldist.reference.assign(glbtchd.reference.str());
			  gldist.cheque.assign(glbtchd.cheque.str());
			  gldist.medrep.assign(glbtchd.depart.str());
			  gldist.prod.assign(glbtchd.product.str());
			  gldist.drcode.assign(glbtchd.drcode.str());
			  gldist.descript.assign(glbtchd.descript.str());
			  gldist.debit.assignDouble(double(Str4ptr(glbtchd.debit.str())));
			  gldist.credit.assignDouble(double(Str4ptr(glbtchd.credit.str())));
			  gldist.UpdateUserTime(szUser);
			  gldist.append();

			  szSeek  = glbtchd.account.str();
			  szSeek += glbtchd.dept.str();
			  if(glaccts.seek((char *)szSeek.c_str()) == 0)
				 {
						if(strcmp(glaccts.type.str(), "I") == 0)
						{
							/*
							  If the entry is a income statement entry. Record the income amount
							  as previous 12 in the income statment portion then record the amount
							  in the retained earnings portion.
							*/
							dOpenbal = glaccts.dOpenbal();
							dOpenbal += glbtchd.Debit() - glbtchd.Credit();
							glaccts.previous12.assignDouble(dOpenbal);
							glaccts.UpdateUserTime(szUser);
							glbtch.UpdateUserTime(szUser);
							glbtch.mode.assign("2");

							for(glaccts.top();!glaccts.eof();glaccts.skip())
							  {
								 if(strcmp(glaccts.type.str(),"R") == 0)
								  break;
								 else
								  continue;
							  }
							dNewOpenBal  = (glaccts.dCurrbal() - dCurrentYearTotal()) + glbtchd.Debit() - glbtchd.Credit();
							dCurrBal =  dNewOpenBal + dCurrentYearTotal();
							glaccts.curr_bal.assignDouble(dCurrBal);
							dOpenbal = glaccts.dOpenbal();
							dOpenbal += glbtchd.Debit() - glbtchd.Credit();
							glaccts.openbal.assignDouble(dOpenbal);
							glaccts.UpdateUserTime(szUser);
						  }
						  else
						  {
							dNewOpenBal  = (glaccts.dCurrbal() - dCurrentYearTotal()) + glbtchd.Debit() - glbtchd.Credit();
							dCurrBal =  dNewOpenBal + dCurrentYearTotal();
							glaccts.curr_bal.assignDouble(dCurrBal);
							dOpenbal = glaccts.dOpenbal();
							dOpenbal += glbtchd.Debit() - glbtchd.Credit();
							glaccts.openbal.assignDouble(dOpenbal);
							glaccts.UpdateUserTime(szUser);
							glbtch.UpdateUserTime(szUser);
							glbtch.mode.assign("2");
						}
				 }
			 glbtchd.skip(1);
		  }
		}
	 }
  }
}

BOOL TPostBatch::CheckGLBalance(long lBatchno)
{
  // glbtchd.SetIndex("batchno");
  glbtch.SetIndexOpen();
  glbtchd.SetIndexBatchno();
  double dDebit  = 0 ;
  double dCredit = 0 ;
  if(glbtch.seek(lBatchno) == 0) {
	 if(glbtch.seek(lBatchno) == 0){
		while((long)glbtchd.Batchno() == (long)lBatchno){
		 dCredit  += glbtchd.Credit();
		 dDebit   += glbtchd.Debit();
		 glbtchd.skip();
		}
	 }
  }
  else
	return TRUE;

  if(round(dCredit,2) == round(dDebit,2))
	return TRUE;
  else
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	wsprintf(szMsg,GetString(1007),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return FALSE;
  }
}

void TPostBatch::HandleListBox(RTMessage Msg)
{
 //switch (((TPostEntries *)Parent)->szTransaction)
 /*
 switch(iCurrentTab)
 {

	 case 3 : // Cash
	 {
	  dAPControl = 0;
		if(MessageBox(HWindow,GetString(6048),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
			 break;
		CHourGlass wait;
		char szBuffer[10];
		LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
		HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
		SSGetData(SHWindow,1,Cell->Row,szBuffer);
		long lRecord=atol(szBuffer);
		if(CASH_CheckBalance(lRecord))
		  {
			apcash.SetIndex(GetString(8140));
				if(apcash.seek(lRecord)==0)
					{
					  long lBatchno = apcash.lBatchno();
						 CASH_AppendSummary(lBatchno);
					}

					SetField(ID_BATCHNO,"");
					SSClearData(SHWindow,SS_ALLCOLS,Cell->Row);
			}
	  break;
	 }
	 case 1 : // Accounts Receivable
	 {
		string strMsg;
		dAPControl = 0;
		char szBuffer[10];
		LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
		HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
		SSGetData(SHWindow,1,Cell->Row,szBuffer);
		long lRecord=atol(szBuffer);
		 if(arbtch.seek(lRecord)==0)
			 {
			  if(strcmp(arbtch.mode.str(),"3") == 0)
					{
					 strMsg = GetString(3093);
					 MsgBox(strMsg,MB_ICONINFORMATION);
					 break;
					}
				wsprintf(szMsg,GetString(8129),StrTrim(arbtch.batchno.str()));
				if(MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
					 string szARAccount(arbtch.account.str());
					 szARAccount += arbtch.depart.str();
					 if(AR_CheckBalance(lRecord,arbtch.dPayable(),(LPSTR)szARAccount.c_str()))
						  {
							char *szInvoice = strcpy(new char[arbtch.vendorno.len()+arbtch.invoiceno.len()+1],arbtch.vendorno.str());
							strcat(szInvoice,arbtch.invoiceno.str());
							AR_AppendDist(szInvoice);
							AR_AppendSummary(lRecord);
							delete [] szInvoice;

							 SetField(ID_BATCHNO,"");
							 SSClearData(SHWindow,SS_ALLCOLS,Cell->Row);
						  }
					 }
				}
	  break;
	 }
	 case 2 : // Accounts Payable
	 {
		string strMsg;
		dAPControl = 0;
		char szBuffer[10];
		LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
		HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
		SSGetData(SHWindow,1,Cell->Row,szBuffer);
		long lRecord=atol(szBuffer);
		 if(apbtch.seek(lRecord)==0)
			 {
			  if(strcmp(apbtch.mode.str(),"3") == 0)
					{
					 strMsg = GetString(3093);
					 MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
					 break;
					}
				wsprintf(szMsg,GetString(8129),StrTrim(apbtch.batchno.str()));
				if(MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
					 if(AP_CheckAPBalance(lRecord))
						  {
							char *szInvoice = strcpy(new char[apbtch.vendorno.len()+apbtch.invoiceno.len()+1],apbtch.vendorno.str());
							strcat(szInvoice,apbtch.invoiceno.str());
							AP_AppendAPDist(szInvoice);
							delete [] szInvoice;
							long lBatchno = apbtch.lBatchno();
								if(strcmp(apbtch.mode.str(),"3") != 0)
									{
									 AP_AppendAPSummary(lBatchno);
									 apbtch.mode.assign("0");
									 apbtch.UpdateUserTime(szUser);
									}
								else
									AP_AppendPriorPeriod(lBatchno);
						  }
							 SetField(ID_BATCHNO,"");
							 SSClearData(SHWindow,SS_ALLCOLS,Cell->Row);
					 }
				}
			 break;
	 }
	 case 0 : // General Ledger
	 {

		LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
		 double colWidth;
		 List->GetColWidth(1,&colWidth);
		 char *szBuffer = new char [(int)colWidth + 1];
		 List->GetData(1,Cell->Row,szBuffer);
		 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
		 glbtch.SetIndex("post_gl");
		 if(glbtch.seek(szBuffer) == 0)
		 {
		  List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
		  wsprintf(szMsg,GetString(8129),StrTrim(glbtch.batchno.str()));
			  if(MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
				  {
					if(CheckGLBalance(glbtch.Batchno()))
						{
							 if(glbtch.Open() && (glbtch.Debit() == glbtch.Credit()) )
								{
								  if(ValidateGLAccounts())
										{
										 PostGLSlip(glbtch.Batchno());
										 List->DelRow(Cell->Row);
										}
								}
							  else if(glbtch.Adjustment() && (glbtch.Debit() == glbtch.Credit()) )
						{
					 if(ValidateGLAccounts())
				  {
					PostAdjustment(glbtch.Batchno());
					List->DelRow(Cell->Row);
		  }
		}
		else
		  {
			 wsprintf(szMsg,GetString(8116),glbtch.batchno.str());
			 MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
		  }
		}
	  }
	  else
		List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
		}
	 delete [] szBuffer;
  }
 }
*/
}


BOOL TCloseYear::VerifyOpenBatch(char *strDbf)
{
 BOOL bClose = TRUE;
 string dbf = strDbf;
 Data4 Database(cb,strDbf);
 Field4 fMode(Database,"mode");
 Relate4set rQuery(Database);
 string strQuery = GetString(8167);
 rQuery.querySet((LPSTR)strQuery.c_str());
 for(int qc = rQuery.top();qc!=r4eof;qc=rQuery.skip(1L) )
 {
  if(strcmp(fMode.str(),"1") == 0)
	{
	 bClose = FALSE;
	 break;
	}
 }
 rQuery.unlock();
 rQuery.free();
 Database.close();
 if(!bClose)
  {

	string strMsg = GetString(8166);
	strMsg += dbf;
	MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	delete szMsg;
  }
 return bClose;
}

BOOL TCloseYear::bCheckForIncomeData()
{
  CHourGlass wait;
  PTglaccts gl = new Tglaccts(cb);
  if(!gl->isValid())
  {
	ErrorClose(gl->GetDbfName());
	return FALSE;
  }

	//gl->SetIndexAccts();
	if(gl->SetIndexAccts(1) < 0)
	{
	  ShutDownWindow();
	  return FALSE;
	}

  double dCurrentBalance = 0;
  for(gl->top();!gl->eof();gl->skip())
	{
		if(strcmp(gl->type.str(),"I") == 0)
		 dCurrentBalance += gl->dCurrbal();
	}
  gl->close();

  if(dCurrentBalance == 0)
	  return FALSE;
  else
	  return TRUE;
}

void TCloseYear::Cancel (RTMessage)
{
  //cb->exclusive = FALSE;
  ShutDownWindow();
}




void TCloseYear::Ok (RTMessage Msg)
{
  // Verify all transactions are closed.
  if(!VerifyOpenBatch(GetString(8169)) ||
	 !VerifyOpenBatch(GetString(8170)) ||
	 !VerifyOpenBatch(GetString(8171)) ||
	 !VerifyOpenBatch(GetString(8172)) )
  {
	return;
  }

  if(bCheckForIncomeData())
  {
	if(MessageBox(HWindow,GetString(8178),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	 {
		/*
			Delete the batch files for :
			gl_btch
			gl_btchd
			gl_dist
			ar_btch
			ar_btchd
			ap_btch
			ap_btchd
			ap_cash
			ap_cashd
		*/
		CHourGlass wait;
		if(!DeleteAll(GetString(ID_GLBTCH)))
		 return;
		if(!DeleteAll(GetString(ID_GLBTCHD)))
		 return;
		if(!DeleteAll(GetString(ID_GLDIST)))
		 return;
		if(!DeleteAll(GetString(ID_ARBTCH)))
		 return;
		if(!DeleteAll(GetString(ID_ARBTCHD)))
		 return;
		if(!DeleteAll(GetString(ID_APBTCH)))
		 return;
		if(!DeleteAll(GetString(ID_APBTCHD)))
		 return;
		if(!DeleteAll(GetString(ID_DB_APCASH)))
		 return;
		if(!DeleteAll(GetString(ID_APCASHD)))
		 return;


		PTglaccts gl = new Tglaccts(cb);
		if(!gl->isValid())
		  {
			ErrorClose(gl->GetDbfName());
			return;
		  }

		if(gl->SetIndexAccts(1) < 0)
		{
		 ShutDownWindow();
		 return;
		}

		gl->top();
		double dRetainedEarnings = 0;
		while(!gl->eof())
		{
		 if(strcmp(gl->type.str(),"B") != 0)
		 {
		  // If an income or retained earnings account change to zero.
		  dRetainedEarnings += gl->dCurrbal();
		  gl->curr_bal.assignDouble(0);
		  gl->openbal.assignDouble(0);
		 }
		 else
		 {
		  // If a balance sheet account change to opening balance.
		  gl->openbal.assignDouble(gl->dCurrbal());
		 }
		  gl->previous1.assignDouble(gl->dPeriod1());
		  gl->previous2.assignDouble(gl->dPeriod2());
		  gl->previous3.assignDouble(gl->dPeriod3());
		  gl->previous4.assignDouble(gl->dPeriod4());
		  gl->previous5.assignDouble(gl->dPeriod5());
		  gl->previous6.assignDouble(gl->dPeriod6());
		  gl->previous7.assignDouble(gl->dPeriod7());
		  gl->previous8.assignDouble(gl->dPeriod8());
		  gl->previous9.assignDouble(gl->dPeriod9());
		  gl->previous10.assignDouble(gl->dPeriod10());
		  gl->previous11.assignDouble(gl->dPeriod11());
		  gl->previous12.assignDouble(gl->dPeriod12());
		  gl->period1.assignDouble(0);
		  gl->period2.assignDouble(0);
		  gl->period3.assignDouble(0);
		  gl->period4.assignDouble(0);
		  gl->period5.assignDouble(0);
		  gl->period6.assignDouble(0);
		  gl->period7.assignDouble(0);
		  gl->period8.assignDouble(0);
		  gl->period9.assignDouble(0);
		  gl->period10.assignDouble(0);
		  gl->period11.assignDouble(0);
		  gl->period12.assignDouble(0);
		  gl->skip();
	 }
	 for(gl->top();!gl->eof();gl->skip())
	 {
	 if(strcmp(gl->type.str(),"R") == 0)
	  {
			double dCurrentRetained = gl->dCurrbal();
			gl->curr_bal.assignDouble(dCurrentRetained + dRetainedEarnings);
			gl->openbal.assignDouble(dCurrentRetained + dRetainedEarnings);
	  }

	}
	 gl->close();
	 bUpdated=FALSE;
	}
	}
	else
	MessageBox(HWindow,GetString(8168),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	string strMsg = GetString(8180);
	SendMessage(GetDlgItem(HWindow,4095),WM_SETTEXT,NULL,(LPARAM)(LPSTR)strMsg.c_str());
	Cancel(Msg);
 }

void TGlWindow::GLYearEnd   (RTMessage)
 {
  //if(MessageBox(HWindow,GetString(GetApplication()->hInstance,8111),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
  //{
	GetApplication()->ExecDialog(new TCloseYear(this,4005));
  //}
 }


BOOL TCloseYear::DeleteAll(char *szDbf)
{
  string strDbf = szDbf;
  string strMsg = GetString(8179);
  strMsg += strDbf;
  SendMessage(GetDlgItem(HWindow,4095),WM_SETTEXT,NULL,(LPARAM)(LPSTR)strMsg.c_str());
  Data4 dbf(cb,(LPSTR)strDbf.c_str());
  if(!dbf.isValid())
	return FALSE;
  dbf.zap();
  dbf.close();
  return TRUE;
}


/*
void TTabControl::WMDrawItem(RTMessage Msg)
{
				LPDRAWITEMSTRUCT lpdis;
				lpdis = (LPDRAWITEMSTRUCT) Msg.LParam;
				switch(lpdis->CtlID)
					{
					case  AP_TAB :
					 {
							if (lpdis->itemState & ODS_SELECTED || lpdis->itemState & ODS_FOCUS) ///* if selected
							  {
								 ((TPostEntries *)Parent)->szTransaction = 'P';
								 Selected(lpdis,strAP);
							  }
							if (((TPostEntries *)Parent)->szTransaction != 'P')
								 UnSelected(lpdis,strAP);
							break;
					 }
					case AR_TAB :
					 {

							if (lpdis->itemState & ODS_SELECTED || lpdis->itemState & ODS_FOCUS) ///* if selected
							 {
								((TPostEntries *)Parent)->szTransaction = 'R';
								Selected(lpdis,strAR);
							 }
							if (((TPostEntries *)Parent)->szTransaction != 'R')
								UnSelected(lpdis,strAR);
							break;
					 }
					case CASH_TAB :
					 {

							if (lpdis->itemState & ODS_SELECTED || lpdis->itemState & ODS_FOCUS) ///* if selected
							{
								((TPostEntries *)Parent)->szTransaction = 'C';
								Selected(lpdis,strCash);
							}
							if (((TPostEntries *)Parent)->szTransaction != 'C')
								UnSelected(lpdis,strCash);
							break;
					 }
					case GL_TAB :
					 {
							 if (lpdis->itemState & ODS_SELECTED || lpdis->itemState & ODS_FOCUS) ///* if selected
							 {
								((TPostEntries *)Parent)->szTransaction = 'G';
								Selected(lpdis,strGL);
							 }
							 if (((TPostEntries *)Parent)->szTransaction != 'G')
								UnSelected(lpdis,strGL);
							 break;
					 }

				}
}


void TTabControl::SetupWindow()
{
		tab_r = LoadBitmap((HINSTANCE)GetApplication()->hInstance, MAKEINTRESOURCE(4098)); // "TAB_R");
		tab_c = LoadBitmap((HINSTANCE)GetApplication()->hInstance, MAKEINTRESOURCE(4096)); //"TAB_C");
		tab_l = LoadBitmap((HINSTANCE)GetApplication()->hInstance, MAKEINTRESOURCE(4097)); //"TAB_L");
		untab_r = LoadBitmap((HINSTANCE)GetApplication()->hInstance, MAKEINTRESOURCE(4101)); //"UNTAB_R");
		untab_c = LoadBitmap((HINSTANCE)GetApplication()->hInstance, MAKEINTRESOURCE(4099)); //"UNTAB_C");
		untab_l = LoadBitmap((HINSTANCE)GetApplication()->hInstance, MAKEINTRESOURCE(4100)); //"UNTAB_L");
		nPtSize = -5;
		pszFace = "MS Sans Serif";

		strAR = GetString(3096);
		strAP = GetString(3097);
		strCash = GetString(3098);
		strGL   = GetString(3099);

}

void TTabControl::Selected(LPDRAWITEMSTRUCT lpdis,string strText)
{
		HDC hdcMem = CreateCompatibleDC(lpdis->hDC);
		SelectObject(hdcMem, tab_c);

		HFONT hfont, hfontOld;

		PLOGFONT plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT));
		plf->lfHeight = -MulDiv(nPtSize, GetDeviceCaps(lpdis->hDC, LOGPIXELSY), 72);
		strcpy(plf->lfFaceName, pszFace);
		plf->lfWeight = FW_SEMIBOLD;
		hfont = CreateFontIndirect(plf);
		hfontOld = (HFONT) SelectObject(lpdis->hDC, hfont);

		StretchBlt(lpdis->hDC,lpdis->rcItem.left,lpdis->rcItem.top,lpdis->rcItem.right - lpdis->rcItem.left,lpdis->rcItem.bottom - lpdis->rcItem.top,
							hdcMem,    // handle of source device context
								0,         // x-coordinate upper-left source
								0,         // y-coordinate upper-left source
								4,        // source bitmap width
								29,        // source bitmap height
								SRCCOPY);  // raster operation
		BITMAP bm;
		SelectObject(hdcMem, tab_r);
		GetObject(tab_r,sizeof(BITMAP),&bm);
		//BitBlt(lpdis->hDC, 0, 0,bm.bmWidth,bm.bmHeight,hdcMem, 0, 0, SRCCOPY);
		StretchBlt(lpdis->hDC,lpdis->rcItem.left,lpdis->rcItem.top,7,lpdis->rcItem.bottom - lpdis->rcItem.top,
								hdcMem,    // handle of source device context
								0,         // x-coordinate upper-left source
								0,         // y-coordinate upper-left source
								7,        // source bitmap width
								29,        // source bitmap height
								SRCCOPY);  // raster operation

		GetObject(tab_l,sizeof(BITMAP),&bm);
		SelectObject(hdcMem, tab_l);
		//BitBlt(lpdis->hDC, lpdis->rcItem.right - bm.bmWidth, 0, bm.bmWidth,bm.bmHeight,hdcMem, 0, 0, SRCCOPY);
		StretchBlt(lpdis->hDC,lpdis->rcItem.right - bm.bmWidth,lpdis->rcItem.top,7,lpdis->rcItem.bottom - lpdis->rcItem.top,
								hdcMem,    // handle of source device context
								0,         // x-coordinate upper-left source
								0,         // y-coordinate upper-left source
								7,        // source bitmap width
								29,        // source bitmap height
								SRCCOPY);  // raster operation

		DrawText(lpdis->hDC,(LPSTR)strText.c_str(), -1,&lpdis->rcItem,DT_CENTER | DT_SINGLELINE |	DT_VCENTER);
		LocalFree((HLOCAL) plf);
		SelectObject(lpdis->hDC, hfontOld);
		DeleteObject(hfont);
		DeleteObject(hfontOld);
		DeleteDC(hdcMem);

}

void TTabControl::UnSelected(LPDRAWITEMSTRUCT lpdis,string strText)
{
		HDC hdcMem = CreateCompatibleDC(lpdis->hDC);
		SelectObject(hdcMem, untab_c);

		HFONT hfont, hfontOld;

		PLOGFONT plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT));
		plf->lfHeight = -MulDiv(nPtSize, GetDeviceCaps(lpdis->hDC, LOGPIXELSY), 72);
		strcpy(plf->lfFaceName, pszFace);
		plf->lfWeight = FW_LIGHT;
		hfont = CreateFontIndirect(plf);
		hfontOld = (HFONT) SelectObject(lpdis->hDC, hfont);

		StretchBlt(lpdis->hDC,lpdis->rcItem.left,lpdis->rcItem.top,lpdis->rcItem.right - lpdis->rcItem.left,lpdis->rcItem.bottom - lpdis->rcItem.top,
							hdcMem,    // handle of source device context
								0,         // x-coordinate upper-left source
								0,         // y-coordinate upper-left source
								4,        // source bitmap width
								29,        // source bitmap height
								SRCCOPY);  // raster operation

		BITMAP bm;
		SelectObject(hdcMem, untab_r);
		GetObject(tab_r,sizeof(BITMAP),&bm);
		//BitBlt(lpdis->hDC, 0, 0,bm.bmWidth,bm.bmHeight,hdcMem, 0, 0, SRCCOPY);
		StretchBlt(lpdis->hDC,lpdis->rcItem.left,lpdis->rcItem.top,7,lpdis->rcItem.bottom - lpdis->rcItem.top,
								hdcMem,    // handle of source device context
								0,         // x-coordinate upper-left source
								0,         // y-coordinate upper-left source
								7,        // source bitmap width
								29,        // source bitmap height
								SRCCOPY);  // raster operation

		GetObject(tab_l,sizeof(BITMAP),&bm);
		SelectObject(hdcMem, untab_l);
		//BitBlt(lpdis->hDC, lpdis->rcItem.right - bm.bmWidth, 0, bm.bmWidth,bm.bmHeight,hdcMem, 0, 0, SRCCOPY);
		StretchBlt(lpdis->hDC,lpdis->rcItem.right - bm.bmWidth,lpdis->rcItem.top,7,lpdis->rcItem.bottom - lpdis->rcItem.top,
								hdcMem,    // handle of source device context
								0,         // x-coordinate upper-left source
								0,         // y-coordinate upper-left source
								7,        // source bitmap width
								29,        // source bitmap height
								SRCCOPY);  // raster operation

		DrawText(lpdis->hDC,(LPSTR)strText.c_str(), -1,&lpdis->rcItem,DT_CENTER | DT_SINGLELINE |	DT_VCENTER);
		
		SelectObject(hdcMem, untab_r);
		BitBlt(lpdis->hDC, 0, 0,  7,  29, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, untab_l);
		BitBlt(lpdis->hDC, lpdis->rcItem.right - 7, 0,7,29,hdcMem, 0, 0, SRCCOPY);


		MoveToEx(lpdis->hDC,2,lpdis->rcItem.top - 2 , NULL);
		POINT ptStart,ptEnd;
		ptStart.x = 2;
		ptStart.y = lpdis->rcItem.top - 2;
		ptEnd.x   = 2;
		ptEnd.y   = lpdis->rcItem.right - 2;
		LineTo(lpdis->hDC,ptEnd.x,ptEnd.y);


		LocalFree((HLOCAL) plf);
		SelectObject(lpdis->hDC, hfontOld);
		DeleteObject(hfont);
		DeleteObject(hfontOld);
		DeleteDC(hdcMem);

}


*/



/*
void TPostEntries::RedrawGL(RTMessage)
{
  CHourGlass wait;
  //GLRedraw();
}

void TPostEntries::RedrawAP(RTMessage)
{
 CHourGlass wait;
 //APRedraw();
}

void TPostEntries::RedrawAR(RTMessage)
{
  CHourGlass wait;
  //ARRedraw();
}

void TPostEntries::RedrawCash(RTMessage)
{
  CHourGlass wait;
  //CashRedraw();
}


BOOL TPostEntries::CanClose()
{
  return TRUE;
}
*/


/*
void TPostEntries::SetupWindow()
{
  TMainDialog::SetupWindow();
  Tab  = new TTabControl (this,"TAB");
  Tab->EnableAutoCreate();
  GetApplication()->MakeWindow(Tab);
  //Tab->Show(SW_SHOW);
  Tab->Show(SW_SHOW);

  GLForm = new TPostBatch(this,1061);
  GLForm->EnableAutoCreate();
  GetApplication()->MakeWindow(GLForm);

  SetCaption(GetString(3095));
  int TAB_HEIGHT = 40;
  int HEADER_HEIGHT = 40;
  RECT rc,tabRect;
  GetClientRect(HWindow,&rc);
  GetWindowRect(Tab->HWindow,&tabRect);
  MoveWindow(Tab->HWindow,0,0,rc.right-rc.left,TAB_HEIGHT,FALSE);
  MoveWindow(GLForm->HWindow,0,TAB_HEIGHT,rc.right-rc.left,rc.bottom-rc.top-TAB_HEIGHT,FALSE);
  MoveWindow(GetDlgItem(GLForm->HWindow,ID_DISTLIST),rc.left,rc.top,rc.right-rc.left,rc.bottom-
																	 (rc.top+TAB_HEIGHT+HEADER_HEIGHT),TRUE);
  SetFocus(GetDlgItem(Tab->HWindow,GL_TAB));
  short d1Width,d3Width;
  GLForm->List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  GLForm->List->GetColWidthInPixels(3,(LPSHORT)&d3Width);

  short ColWidth = (rc.right - rc.left)
		  - d1Width
		  - (4*d3Width)
		  - GetSystemMetrics(SM_CXHSCROLL);
  GLForm->Show(SW_SHOW);
  GLForm->List->SetColWidthInPixels(2,ColWidth);


}
*/

