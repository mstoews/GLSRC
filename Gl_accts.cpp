#include "std_inc.h"

/*
#include "tbowl.h"
#include "d4all.hpp"
#include "gl_strg.h"
#include "glclass.h"
#include "gl.h"

#include "gl_dlg.h"
#include "gl_pick.h"
#include "db_accts.h"
#include "gl_main.h"
*/


#include "gl_acctl.h"
#include "gl_accts.h"
#include "gl_budgt.h"

#include "db_glsub.h"

extern HINSTANCE hInst;

BOOL  CheckFile(char *szFileName,HWND hWnd);

void Tglaccts::init(void)
  {
  accounts.init(data,1);
  dept.init(data,2);
  descript.init(data,3);
  fdescript.init(data,4);
  type.init(data,5);
  category.init(data,6);
  curr_bal .init(data,7);
  period1.init(data,8);
  period2.init(data,9);
  period3.init(data,10);
  period4.init(data,11);
  period5.init(data,12);
  period6.init(data,13);
  period7.init(data,14);
  period8.init(data,15);
  period9.init(data,16);
  period10.init(data,17);
  period11.init(data,18);
  period12.init(data,19);
  openbal.init(data,20);
  previous1.init(data,21);
  previous2.init(data,22);
  previous3.init(data,23);
  previous4.init(data,24);
  previous5.init(data,25);
  previous6.init(data,26);
  previous7.init(data,27);
  previous8.init(data,28);
  previous9.init(data,29);
  previous10.init(data,30);
  previous11.init(data,31);
  previous12.init(data,32);

  budget1.init(data,33);
  budget2.init(data,34);
  budget3.init(data,35);
  budget4.init(data,36);
  budget5.init(data,37);
  budget6.init(data,38);
  budget7.init(data,39);
  budget8.init(data,40);
  budget9.init(data,41);
  budget10.init(data,42);
  budget11.init(data,43);
  budget12.init(data,44);
  bInitialized = TRUE;
  return;
  }



void TGLAcctsAdd::Seek(RTMessage Msg)
{
 GetSeek(Msg);
}


void TGLAcctsAdd::SetData(RTMessage Msg)
{
 string szBuff;
 szBuff  = GetField(ID_ACCOUNTS);
 szBuff += GetField(ID_DEPART);
 if(strlen(GetField(ID_ACCOUNTS)) == 0)
  glaccts.top();
 else
  glaccts.seek((LPSTR)szBuff.c_str());

 Update(Msg);
}

_CLASSDEF(TGLDetail)
class TGLDetail : public TMainDialog
{
 public:
 string account,dept;
 Tglaccts*  glaccts;
 TGLDetail(PTWindowsObject AParent, int AName,Tglaccts* gl): TMainDialog (AParent,AName)
 {
	glaccts = gl;
 };
 virtual void Ok       (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel   (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void SetupWindow();
 void SetAccount(string strAccount,string strDept);
};

void TGLDetail::SetAccount(string strAccount,string strDept)
{
  account = strAccount;
  dept    = strDept;
}
void TGLDetail::Ok(RTMessage)
{
  glaccts->previous1.assignDouble(dGetField(113));
  glaccts->previous2.assignDouble(dGetField(114));
  glaccts->previous3.assignDouble(dGetField(115));
  glaccts->previous4.assignDouble(dGetField(116));
  glaccts->previous5.assignDouble(dGetField(117));
  glaccts->previous6.assignDouble(dGetField(118));
  glaccts->previous7.assignDouble(dGetField(119));
  glaccts->previous8.assignDouble(dGetField(120));
  glaccts->previous9.assignDouble(dGetField(121));
  glaccts->previous10.assignDouble(dGetField(122));
  glaccts->previous11.assignDouble(dGetField(123));
  glaccts->previous12.assignDouble(dGetField(124));
  glaccts->budget1.assignDouble(dGetField(125));
  glaccts->budget2.assignDouble(dGetField(126));
  glaccts->budget3.assignDouble(dGetField(127));
  glaccts->budget4.assignDouble(dGetField(128));
  glaccts->budget5.assignDouble(dGetField(129));
  glaccts->budget6.assignDouble(dGetField(130));
  glaccts->budget7.assignDouble(dGetField(131));
  glaccts->budget8.assignDouble(dGetField(132));
  glaccts->budget9.assignDouble(dGetField(133));
  glaccts->budget10.assignDouble(dGetField(134));
  glaccts->budget11.assignDouble(dGetField(135));
  glaccts->budget12.assignDouble(dGetField(136));
  ShutDownWindow();
}

void TGLDetail::Cancel(RTMessage)
{
  ShutDownWindow();
}


void TGLDetail::SetupWindow()
{
	TMainDialog::SetupWindow();
	SetField(ID_ACCOUNTS,(LPSTR)account.c_str());
	SetField(ID_DEPART,(LPSTR)dept.c_str());
	SetField(137,  glaccts->openbal.str());
	SetField(138,  glaccts->curr_bal.str());
	SetField(101,  glaccts->period1.str());
	SetField(102,  glaccts->period2.str());
	SetField(103,  glaccts->period3.str());
	SetField(104,  glaccts->period4.str());
	SetField(105,  glaccts->period5.str());
	SetField(106,  glaccts->period6.str());
	SetField(107,  glaccts->period7.str());
	SetField(108,  glaccts->period8.str());
	SetField(109,  glaccts->period9.str());
	SetField(110,  glaccts->period10.str());
	SetField(111,  glaccts->period11.str());
	SetField(112,  glaccts->period12.str());
	SetField(113,  glaccts->previous1.str());
	SetField(114,  glaccts->previous2.str());
	SetField(115,  glaccts->previous3.str());
	SetField(116,  glaccts->previous4.str());
	SetField(117,  glaccts->previous5.str());
	SetField(118,  glaccts->previous6.str());
	SetField(119,  glaccts->previous7.str());
	SetField(120,  glaccts->previous8.str());
	SetField(121,  glaccts->previous9.str());
	SetField(122,  glaccts->previous10.str());
	SetField(123,  glaccts->previous11.str());
	SetField(124,  glaccts->previous12.str());
	SetField(125,  glaccts->budget1.str());
	SetField(126,  glaccts->budget2.str());
	SetField(127,  glaccts->budget3.str());
	SetField(128,  glaccts->budget4.str());
	SetField(129,  glaccts->budget5.str());
	SetField(130,  glaccts->budget6.str());
	SetField(131,  glaccts->budget7.str());
	SetField(132,  glaccts->budget8.str());
	SetField(133,  glaccts->budget9.str());
	SetField(134,  glaccts->budget10.str());
	SetField(135,  glaccts->budget11.str());
	SetField(136,  glaccts->budget12.str());


}

void TGLAcctsAdd::Details(RTMessage)
{

  string account(StrTrim(GetField(ID_ACCOUNTS)));
  string depart(StrTrim(GetField(ID_DEPART)));

  if(account.length() == 0)
	return;

  TGLDetail* GLDetail = new TGLDetail(this,1005,&glaccts);
  GLDetail->SetAccount(account,depart);
  GetApplication()->ExecDialog(GLDetail);
  //delete GLDetail;

}

void TGLAcctsAdd::Delete(RTMessage)
{
if(MessageBox(HWindow,GetString(1021),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
{
 //glaccts.SetIndex("accounting");
 glaccts.SetIndexAccts(1);
 if(strlen(GetField(ID_ACCOUNTS)) == 0)
 {
  MessageBox(HWindow,GetString(1025),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  return;
 }

string szSeek = GetField(ID_ACCOUNTS);
szSeek += GetField(ID_DEPART);

if(glaccts.seek((LPSTR)szSeek.c_str()) != 0)
 {
  MessageBox(HWindow,GetString(1026),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  return;
 }
if(
		  glaccts.dPeriod1() != 0
	  || glaccts.dPeriod2() != 0
	  || glaccts.dPeriod3() != 0
	  || glaccts.dPeriod4() != 0
	  || glaccts.dPeriod5() != 0
	  || glaccts.dPeriod6() != 0
	  || glaccts.dPeriod7() != 0
	  || glaccts.dPeriod8() != 0
	  || glaccts.dPeriod9() != 0
	  || glaccts.dPeriod10() != 0
	  || glaccts.dPeriod11() != 0
	  || glaccts.dPeriod12() != 0
	  || glaccts.dPrevious12() != 0
	  || glaccts.dCurrbal() !=0
	  )
 {
  MessageBox(HWindow,GetString(1027),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  return;
 }
 glaccts.deleteRec();
 glaccts.pack();

 SetDlgItemText(HWindow,ID_ACCOUNTS ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_DEPART   ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_DEPTCODE ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_JDESCRIPT ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_CURRBAL  ,(LPSTR) "" );
 SetDlgItemText(HWindow,ID_OPENBAL  ,(LPSTR) "" );

 UpdateReport(&glaccts.accounts,&glaccts.dept);





 SetFocus(GetDlgItem(HWindow,2013));
 GLACCOUNT->List->SetFields();
 }
}

void TGLAcctsAdd::UpdateReport(Field4 *account,Field4 *dept)
{
 Tglsub glsub;
 glsub.open(((TAcctModule *)GetApplication())->cb);

 Data4 gl_cat(((TAcctModule *)GetApplication())->cb,"gl_cat");
 gl_cat.SetIndex("sub");
 glsub.SetIndex("sub");

 // find the balance sheet or income statement category which the new account
 // belongs to.
 // GL_CAT Fields
  Field4 start(gl_cat,5);
  Field4 end(gl_cat,7);
  Field4 report_name(gl_cat,1);
  Field4 report_sub(gl_cat,2);

 gl_cat.top();
 while (!gl_cat.eof())
 {

	 long lStart,lEnd,lAccount;
	 lAccount = atol(account->str());
	 lStart   = atol(start.str());
	 lEnd     = atol(end.str());
	 if(lAccount >= lStart && lAccount <= lEnd)
	 {
		glsub.appendStart();
		glsub.report.assign(report_name.str());
		glsub.sub.assign(report_sub.str());
		glsub.account.assign(account->str());
		glsub.dept.assign(dept->str());
		glsub.depart.assign("");
		glsub.seqno.assignLong(1);
		glsub.field.assign("4");
		glsub.append();
	 }
	gl_cat.skip();
 }

 glsub.top();
 glsub.close();

}


void TGLAcctsAdd::Update(RTMessage)
{
		glaccts.SetIndexAccts(1);
		string strSeek = GetField(ID_ACCOUNTS);
		strSeek += GetField(ID_DEPART);

		if(glaccts.seek((LPSTR)strSeek.c_str()) == 0)
		{
			SetField(ID_ACCOUNTS,glaccts.accounts);
			SetField(ID_DEPART,glaccts.dept);
			SetField(ID_JDESCRIPT,glaccts.descript);
			SetField(ID_DESCRIPT,glaccts.fdescript);
			SetField(ID_OPENBAL,glaccts.openbal);
			SetField(ID_CURRBAL,glaccts.curr_bal);
			SetField(ID_CATEGORY,glaccts.category);

		  if(lstrcmp(glaccts.type.str(),"B")==0)
			{
			SendDlgItemMsg(ID_BS,BM_SETCHECK , 1 ,0L);
			SendDlgItemMsg(ID_RE,BM_SETCHECK , 0 ,0L);
			SendDlgItemMsg(ID_IS,BM_SETCHECK , 0 ,0L);
			}
		  else if(lstrcmp(glaccts.type.str(),"R")==0)
		  {
		  SendDlgItemMsg(ID_BS,BM_SETCHECK , 0 ,0L);
		  SendDlgItemMsg(ID_IS,BM_SETCHECK , 0 ,0L);
		  SendDlgItemMsg(ID_RE,BM_SETCHECK , 1 ,0L);
		  }
		 else if(lstrcmp(glaccts.type.str(),"I")==0)
		 {
		 SendDlgItemMsg(ID_BS,BM_SETCHECK , 0 ,0L);
		 SendDlgItemMsg(ID_IS,BM_SETCHECK , 1 ,0L);
		 SendDlgItemMsg(ID_RE,BM_SETCHECK , 0 ,0L);
		 }
		 SetFocus(GetDlgItem(HWindow,IDOK));
	 }
	 else
	 {
	  string strMsg("Can't find account number :");
	  strMsg += strSeek;
	  MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 }
}

void TGLAcctsAdd::GetSeek (RTMessage)
{

	 GLACCOUNT->List->Seek(ID_ACCOUNTS);
	 GLACCOUNT->Show(SW_SHOW);
}


Tglaccts::Tglaccts()
{
  bInitialized = FALSE;
};

void TGLAcctsAdd::Search  (RTMessage)
{
	 if(GLACCOUNT != NULL)
	 {
	  GLACCOUNT->List->Seek(ID_ACCOUNTS);
	  GLACCOUNT->Show(SW_SHOW);
	 } 
}


void TGLAcctsAdd::SetCategories()
{
	LRESULT rc = 0;
	SendDlgItemMsg(ID_CATEGORY,CB_RESETCONTENT,NULL,NULL);
	string strCategory;
	for(glaccts.top();!glaccts.eof();glaccts.skip())
		{
			strCategory = glaccts.category.str();
			if(strlen(StrTrim((LPSTR)strCategory.c_str())) > 0 )
			{
			  rc = SendDlgItemMsg(ID_CATEGORY,CB_FINDSTRINGEXACT,-1,(LPARAM)(LPCSTR)strCategory.c_str());
			 if(rc >= 0)
				continue;
			 else
				SendDlgItemMsg(ID_CATEGORY,CB_ADDSTRING,NULL,(LPARAM) (LPCSTR)strCategory.c_str());
			}
		}
}


void TGLAcctsAdd::SetupWindow()
{
 TMainDialog::SetupWindow();
 CHourGlass wait;
 //HWND hForeignLanguage = GetDlgItem(HWindow,ID_DESCRIPT);
 //ShowWindow(hForeignLanguage,SW_HIDE);
 SendDlgItemMsg(ID_BS,BM_SETCHECK,TRUE,NULL);
 if(glaccts.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
		ErrorClose(glaccts.GetDbfName());
		return;
  }

 glaccts.SetIndexAccts(1);

 SetCategories();

 GLACCOUNT = new TAccountList(this,"");
 GLACCOUNT->Attr.Style |= WS_THICKFRAME;
 GLACCOUNT->EnableAutoCreate();
 GetApplication()->MakeWindow(GLACCOUNT);

 SetFocus(GetDlgItem(HWindow,2013));
 SendDlgItemMsg(ID_FOREIGN,BM_SETCHECK,1,0L);
 SendDlgItemMsg(ID_ACCOUNTS,EM_LIMITTEXT,glaccts.accounts.len(),NULL);
 SendDlgItemMsg(ID_DEPT,EM_LIMITTEXT,glaccts.dept.len(),NULL);
 SendDlgItemMsg(ID_JDESCRIPT,EM_LIMITTEXT,glaccts.fdescript.len(),NULL);
 SendDlgItemMsg(ID_DESCRIPT,EM_LIMITTEXT,glaccts.descript.len(),NULL);
 SendDlgItemMsg(ID_CATEGORY,EM_LIMITTEXT,glaccts.category.len(),NULL);

 SetBitmap(ID_GLINQ);
 SetBitmap(4076);
 SetBitmap(ID_ACCT);
 SetDlgItemText(HWindow,ID_ACCOUNTS ,(LPSTR) "" );

}



void TGLAcctsAdd::Ok (RTMessage Msg)
{
 CHourGlass wait;
 if(szEnterError(ID_ACCOUNTS))
  return;

 if(szEnterError(ID_JDESCRIPT))
  return;

 //glaccts.SetIndex("accounts");
 if(glaccts.SetIndexAccts(1) < 0)
 {
	ShutDownWindow();
	return;
 }

 string szS;
 szS = GetField(ID_ACCOUNTS);
 szS += GetField(ID_DEPART);

 string type = Type();

 /*
 if(glaccts.seek((LPSTR)szS.c_str()) != 0 && strcmp((LPSTR)type.c_str(),"R") == 0)
	{
	 MessageBox(HWindow,GetString(ID_RETAINEDERROR),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 return;
	}
*/

 if(glaccts.seek((LPSTR)szS.c_str()) == 0 && strcmp((LPSTR)type.c_str(),"R") == 0)
	{
	 for(glaccts.top();!glaccts.eof();glaccts.skip())
	  {
		 if(strcmp(glaccts.type.str(),"R") == 0)
		 {
		  BS(Msg);
		  MessageBox(HWindow,GetString(ID_RETAINEDERROR),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
		  return;
		 }
	  }
	}
  else
 {
 if(glaccts.seek((LPSTR)szS.c_str()) != 0 && strcmp((LPSTR)type.c_str(),"R") == 0)
	{
	 for(glaccts.top();!glaccts.eof();glaccts.skip())
	  {
		 if(strcmp(glaccts.type.str(),"R") == 0)
		 {
		  BS(Msg);
		  MessageBox(HWindow,GetString(ID_RETAINEDERROR),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
		  return;
		 }
	  }
	}
  }


 if(glaccts.seek((LPSTR)szS.c_str())== 0)
 {
  Modify(Msg);
  return;
 }

 string strType;

 if(SendDlgItemMsg(ID_BS,BM_GETCHECK,NULL,NULL) == 1)
  strType = "B";
 else
 if(SendDlgItemMsg(ID_IS,BM_GETCHECK,NULL,NULL) == 1)
  strType = "I";
 else
 if(SendDlgItemMsg(ID_RE,BM_GETCHECK,NULL,NULL) == 1)
  strType = "R";
 else
  strType = "B";

 /*
 string strType(Type());

 if(strcmp((LPSTR)strType.c_str(),"N") == 0)
  {
	MessageBox(HWindow,GetString(4071),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }
 */
 // Before adding to the database make sure that the record hasn't been added
 // previously or that there is anything in the current balance of previous balance for the year.

 glaccts.appendStart();
 /*
 Field4 field;
 for(int fieldNum = 1; fieldNum<=glaccts.numFields(); fieldNum++)
 {
  field.init(glaccts,fieldNum);
  switch(field.type())
  {
	  case r4num: field.assignDouble(0); break;
	  default: break;
  }
 }
 */


 string account(GetField(ID_ACCOUNTS));
 string dept(GetField(ID_DEPART));
 string strDesc(GetField(ID_DESCRIPT));
 string strFDesc(GetField(ID_JDESCRIPT));
 string strCategory (GetField(ID_CATEGORY));


 glaccts.accounts   = account;
 glaccts.dept       = dept;
 glaccts.descript   = strFDesc;
 glaccts.fdescript  = strDesc;
 glaccts.openbal    = dGetField(ID_OPENBAL);
 glaccts.curr_bal   = dGetField(ID_CURRBAL);
 glaccts.type       = strType;
 glaccts.category   = strCategory;
 glaccts.period1.assignDouble(0);
 glaccts.period2.assignDouble(0);
 glaccts.period3.assignDouble(0);
 glaccts.period4.assignDouble(0);
 glaccts.period5.assignDouble(0);
 glaccts.period6.assignDouble(0);
 glaccts.period7.assignDouble(0);
 glaccts.period8.assignDouble(0);
 glaccts.period9.assignDouble(0);
 glaccts.period10.assignDouble(0);
 glaccts.period11.assignDouble(0);
 glaccts.period12.assignDouble(0);

 glaccts.previous1.assignDouble(0);
 glaccts.previous2.assignDouble(0);
 glaccts.previous3.assignDouble(0);
 glaccts.previous4.assignDouble(0);
 glaccts.previous5.assignDouble(0);
 glaccts.previous6.assignDouble(0);
 glaccts.previous7.assignDouble(0);
 glaccts.previous8.assignDouble(0);
 glaccts.previous9.assignDouble(0);
 glaccts.previous10.assignDouble(0);
 glaccts.previous11.assignDouble(0);
 glaccts.previous12.assignDouble(0);

 glaccts.budget1.assignDouble(0);
 glaccts.budget2.assignDouble(0);
 glaccts.budget3.assignDouble(0);
 glaccts.budget4.assignDouble(0);
 glaccts.budget5.assignDouble(0);
 glaccts.budget6.assignDouble(0);
 glaccts.budget7.assignDouble(0);
 glaccts.budget8.assignDouble(0);
 glaccts.budget9.assignDouble(0);
 glaccts.budget10.assignDouble(0);
 glaccts.budget11.assignDouble(0);
 glaccts.budget12.assignDouble(0);



 glaccts.UpdateUserTime(szUser);
 glaccts.append();

 SetField(ID_ACCOUNTS ,"" );
 SetField(ID_DEPART   ,"" );
 SetField(ID_DEPTCODE ,"" );
 SetField(ID_JDESCRIPT,"" );
 SetField(ID_DESCRIPT,"" );
 FloatSetDlgItemValue(HWindow,ID_CURRBAL  ,0);
 FloatSetDlgItemValue(HWindow,ID_OPENBAL  ,0);
 if(strlen(StrTrim((LPSTR)strCategory.c_str())) > 0)
  SetCategories();
 GLACCOUNT->List->SetFields();
 SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));
}

void TGLAcctsAdd::New (RTMessage)
{
 SetField(ID_ACCOUNTS ,"" );
 SetField(ID_DEPART   ,"" );
 SetField(ID_DEPTCODE ,"" );
 SetField(ID_JDESCRIPT,"" );
 SetField(ID_DESCRIPT,"" );
 FloatSetDlgItemValue(HWindow,ID_CURRBAL  ,0);
 FloatSetDlgItemValue(HWindow,ID_OPENBAL  ,0);
}

void TGLAcctsAdd::Modify(RTMessage)
{
	  string strDesc  = GetField(ID_JDESCRIPT);
	  string strFDesc = GetField(ID_DESCRIPT);
	  string strCategory (GetField(ID_CATEGORY));
	  string type (Type());
	  glaccts.descript   = strDesc;
	  glaccts.fdescript  = strFDesc;
	  glaccts.openbal = dGetField(ID_OPENBAL);
	  glaccts.curr_bal   = dGetField(ID_CURRBAL);
	  glaccts.category   = strCategory;
	  glaccts.type       = type;
	  glaccts.UpdateUserTime(szUser);

	  SetCategories();
	  GLACCOUNT->List->Redraw();
	  SendMessage(HWindow,WM_COMMAND,ID_GLINQ,NULL);
	 return;
}



void TGLAcctsAdd::Cancel         (RTMessage)
{
 if(GLACCOUNT != NULL)
	 GLACCOUNT->List->SavePosition();
 glaccts.close();
 ShutDownWindow();
}


LPSTR TGLAcctsAdd::Type()
 {
  static string strType;
  if(SendDlgItemMsg(ID_BS,BM_GETCHECK,0,0))
	 strType = "B";
  else
  if(SendDlgItemMsg(ID_IS,BM_GETCHECK,0,0))
	 strType = "I";
  else
  if (SendDlgItemMsg(ID_RE,BM_GETCHECK,0,0))
	 strType = "R";
  else
	 strType = "B";
  return (LPSTR) strType.c_str();
}

void TGLAcctsAdd::ShowForeign(RTMessage)
{
 DWORD dwIndex = SendDlgItemMsg(ID_FOREIGN,BM_GETCHECK,NULL,NULL);
 if(dwIndex == 1)
 {
  ShowWindow(GetDlgItem(HWindow,ID_DESCRIPT),SW_HIDE);
  SendDlgItemMsg(ID_FOREIGN,BM_SETCHECK,0,0L);
 }
 else
 {
  ShowWindow(GetDlgItem(HWindow,ID_DESCRIPT),SW_SHOW);
  SendDlgItemMsg(ID_FOREIGN,BM_SETCHECK,1,0L);
 }


}

void TGLAcctsAdd::BS (RTMessage )
  {
  SendDlgItemMsg ( ID_BS, BM_SETCHECK , 1 ,0L);
  SendDlgItemMsg ( ID_IS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_RE, BM_SETCHECK , 0 ,0L);
  //glaccts.type.assign("B");
  SetFocus(GetDlgItem(HWindow, IDOK));
  }

void TGLAcctsAdd::IS (RTMessage )
  {
  SendDlgItemMsg ( ID_BS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_IS, BM_SETCHECK , 1 ,0L);
  SendDlgItemMsg ( ID_RE, BM_SETCHECK , 0 ,0L);
  //glaccts.type.assign("I");
  SetFocus(GetDlgItem(HWindow, IDOK));
  }

void TGLAcctsAdd::RE (RTMessage )
  {
  SendDlgItemMsg ( ID_BS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_IS, BM_SETCHECK , 0 ,0L);
  SendDlgItemMsg ( ID_RE, BM_SETCHECK , 1 ,0L);
  //glaccts.type.assign("R");
  SetFocus(GetDlgItem(HWindow, IDOK));
  }




// Function called from the main  - Add accounts from the chart of accts.
void TGlWindow::GLAcctAdd (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLAcctsAdd(this,1010));
 }


 
// Function called from the main  - Delete accounts from the chart of accts.
void TGlWindow::GLAcctDel (RTMessage)
 {
 }

void TGlWindow::GLAcctBudget  (RTMessage)
{
  GetApplication()->ExecDialog(new TGLBudget(this,1017));
}

void TGLBudget::SetupWindow()
{
 TGLAcctsAdd::SetupWindow();
 SetCaption(GetString(8097));
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 RECT rc;
 SSGetClientRect(SHWindow,&rc);
 short d1Width;

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 SSSetBool(SHWindow,SSB_AUTOSIZE,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetMaxCols(SHWindow, 12);
 SSSetMaxRows(SHWindow, 24);

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetColWidth(SHWindow,1,6);

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);


 short ColWidth = ((rc.right - rc.left) - d1Width)/3;

 SSSetColWidthInPixels(SHWindow,2,ColWidth);
 SSSetColWidthInPixels(SHWindow,3,ColWidth);
 SSSetColWidthInPixels(SHWindow,4,ColWidth);

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, GetString(6133));
 SSSetData(SHWindow,2,SS_HEADER, GetString(6134));
 SSSetData(SHWindow,3,SS_HEADER, GetString(6135));
 SSSetData(SHWindow,4,SS_HEADER, GetString(6136));


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,SS_ALLCOLS, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,4,0,0,99);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);

}

void TGLBudget::Update(RTMessage Msg)
{
 TGLAcctsAdd::Update(Msg);

 
 Field4 *budget;
 Field4 *period;
 Field4 *previous;
 char *szPeriod = new char [3];
 SSSetBool(SHWindow,SSB_REDRAW,FALSE);
 for(int i = 1;i<13;i++)
 {
  period   = (Field4 *) new Field4(glaccts,(i+7));
  previous = (Field4 *) new Field4(glaccts,(i+20));
  budget   = (Field4 *) new Field4(glaccts,(i+32));
  SSSetData(SHWindow,2,i,budget->str());
  SSSetData(SHWindow,3,i,period->str());
  SSSetData(SHWindow,4,i,previous->str());
  itoa(i,szPeriod,10);
  SSSetData(SHWindow,1,i,szPeriod);
 }
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
}

void TGLBudget::Ok(RTMessage)
{
 if(MessageBox(HWindow,GetString(8098),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
 {
   CHourGlass wait;
	char *szBudget = new char [glaccts.budget1.len() + 1];
    for(int i=1;i<13;i++)
     {
      	SSGetData(SHWindow,2,i,szBudget);
	double dValue = atof(szBudget);
	  switch (i)
	  {
		 case  1:  glaccts.budget1.assignDouble(dValue); break;
		 case  2:  glaccts.budget2.assignDouble(dValue); break;
		 case  3:  glaccts.budget3.assignDouble(dValue); break;
		 case  4:  glaccts.budget4.assignDouble(dValue); break;
		 case  5:  glaccts.budget5.assignDouble(dValue); break;
		 case  6:  glaccts.budget6.assignDouble(dValue); break;
		 case  7:  glaccts.budget7.assignDouble(dValue); break;
		 case  8:  glaccts.budget8.assignDouble(dValue); break;
		 case  9:  glaccts.budget9.assignDouble(dValue); break;
		 case 10:  glaccts.budget10.assignDouble(dValue); break;
		 case 11:  glaccts.budget11.assignDouble(dValue); break;
		 case 12:  glaccts.budget12.assignDouble(dValue); break;
	  }
	 }
 }
}


/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Modify, add and delete accounts functions.
*/
/*
Field  Field Name                 Type          Length  Dec  Index
	 1  ACCOUNT                    CHARACTER         15           N
	 2  DEPT                       CHARACTER          4           Y
	 3  DESCRIPT                   CHARACTER         45           N
    4  FDESCRIPT                  CHARACTER         45           N
    5  TYPE                       CHARACTER          1           Y
    6  CATEGORY                   CHARACTER          5           N
	 7  CURR_BAL                   NUMERIC           16    2      N
	 8  PERIOD1                    NUMERIC           16    2      N
    9  PERIOD2                    NUMERIC           16    2      N
   10  PERIOD3                    NUMERIC           16    2      N
	11  PERIOD4                    NUMERIC           16    2      N
   12  PERIOD5                    NUMERIC           16    2      N
	13  PERIOD6                    NUMERIC           16    2      N
	14  PERIOD7                    NUMERIC           16    2      N
	15  PERIOD8                    NUMERIC           16    2      N
   16  PERIOD9                    NUMERIC           16    2      N
	17  PERIOD10                   NUMERIC           16    2      N
   18  PERIOD11                   NUMERIC           16    2      N
   19  PERIOD12                   NUMERIC           16    2      N
   20  OPENBAL                    NUMERIC           16    2      N
	21  PREVIOUS1                  NUMERIC           16    2      N
	22  PREVIOUS2                  NUMERIC           16    2      N
	23  PREVIOUS3                  NUMERIC           16    2      N
	24  PREVIOUS4                  NUMERIC           16    2      N
	25  PREVIOUS5                  NUMERIC           16    2      N
	26  PREVIOUS6                  NUMERIC           16    2      N
	27  PREVIOUS7                  NUMERIC           16    2      N
	28  PREVIOUS8                  NUMERIC           16    2      N
	29  PREVIOUS9                  NUMERIC           16    2      N
	30  PREVIOUS10                 NUMERIC           16    2      N
	31  PREVIOUS11                 NUMERIC           16    2      N
	32  PREVIOUS12                 NUMERIC           16    2      N
	33  BUDGET1                    NUMERIC           16    2      N
	34  BUDGET2                    NUMERIC           16    2      N
	35  BUDGET3                    NUMERIC           16    2      N
	36  BUDGET4                    NUMERIC           16    2      N
	37  BUDGET5                    NUMERIC           16    2      N
	38  BUDGET6                    NUMERIC           16    2      N
	39  BUDGET7                    NUMERIC           16    2      N
	40  BUDGET8                    NUMERIC           16    2      N
	41  BUDGET9                    NUMERIC           16    2      N
	42  BUDGET10                   NUMERIC           16    2      N
	43  BUDGET11                   NUMERIC           16    2      N
	44  BUDGET12                   NUMERIC           16    2      N
	45  UPDATE                     DATE               8           N
	46  USER                       CHARACTER         10           N
	47  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        768

*/
