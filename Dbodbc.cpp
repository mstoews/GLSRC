#include "owlall.h"
#include "sql.h"
#include "sqlext.h"
#include "dbodbc.h"
#include "resource.h"
#include "d4all.hpp"
#include "dir.h"
#include "direct.h"

LPSTR szlpQuery(LPSTR lpQuery,string strConnect);
HINSTANCE hInst;

char szDir[128];

#pragma argsused
int WINAPI LibMain (HINSTANCE hInstance, WORD wDataSeg, WORD hHeapSize, LPSTR lpszCmdLine)
{
	hInst = hInstance;
	return 1;

}

#pragma argsused
int DLLFUNC WEP (int nParam)
{
 return 1;
}

LPSTR   strTrim (LPSTR   Str);
LPSTR   strLTrim(LPSTR Str);
LPSTR   strRTrim(LPSTR Str);
LPSTR   strTrim (LPSTR   Str)
{
	return strLTrim (strRTrim (Str));
}

/*
 *    Remove Leading blanks
 */
LPSTR   strLTrim(LPSTR Str)
{
	LPSTR   s = Str;

	while (isspace (*Str))
		Str++;

	return strcpy (s, Str);
}

/*
 *              Remove trailing blanks
 */
LPSTR   strRTrim(LPSTR Str)
{
	LPSTR   s = Str,p;
	int     Len;

	Len = strlen(Str);
	p = Str+Len-1;

	while(Len-- && isspace (*p))
		p--;

	p[1] = '\0';

	return s;
}




LPSTR DLLFUNC Query(LPSTR lpQuery)
{
  string m_strConnect("ODBC;DSN=General Ledger Files;");
  return _fstrncpy(lpQuery,szlpQuery(lpQuery,m_strConnect),strlen(lpQuery));
}

LPSTR DLLFUNC CB(LPSTR szAccount,LPSTR szDept)
{
  Code4 cb;
  cb.init();

  int drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,"GL.INI");
  GetPrivateProfileString("GL_DIR","DIR","",szDir,sizeof(szDir),"GL.INI");

  _chdrive(drive);
  chdir(szDir);

  Data4 glaccts;
  if(glaccts.open(cb,"glaccts") < 0)
	{
	  cb.closeAll();
	  cb.initUndo();
	  return "";
	}
  glaccts.SetIndex("accounts");
  Relate4set rQuery(glaccts);

  Field4 period(glaccts,"curr_bal");
  string szQuery("ACCOUNT = '");
  szQuery += szAccount;
  szQuery += "'";


  if (strlen(szDept) > 0)
  {
	 szQuery += " .AND. DEPT = '";
	 szQuery += szDept;
	 szQuery += "'";
  }



  string result;
  rQuery.querySet((LPSTR)szQuery.c_str());
  for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 result = strTrim(period.str());
  rQuery.unlock();
  rQuery.free();

  glaccts.close();
  cb.initUndo();
  return (LPSTR)result.c_str();


  /*
  string strQuery("SELECT CURR_BAL FROM GLACCTS WHERE ACCOUNT = ");
  strQuery += "'";
  strQuery += szAccount;
  strQuery += "'";
  if (strlen(szDept) > 0)
  {
	 strQuery += " AND DEPT = '";
	 strQuery += szDept;
	 strQuery += "'";
  }
  strQuery += "ORDER BY ACCOUNT,DEPT";
  return Query((LPSTR)strQuery.c_str());
  */
}

LPSTR DLLFUNC PD1(LPSTR szAccount,LPSTR szDept)
{
  string strQuery("SELECT PERIOD1 FROM GLACCTS WHERE ACCOUNT = ");
  strQuery += "'";
  strQuery += szAccount;
  strQuery += "'" ;
  if (strlen(szDept) > 0)
  {
	 strQuery += " AND DEPT = '";
	 strQuery += szDept;
	 strQuery += "'";
  }
  return Query((LPSTR)strQuery.c_str());
}

LPSTR DLLFUNC PD2(LPSTR szAccount,LPSTR szDept)
{
  string strQuery("SELECT PERIOD2 FROM GLACCTS WHERE ACCOUNT = ");
  strQuery += "'";
  strQuery += szAccount;
  if (strlen(szDept) > 0)
  {
	 strQuery += " AND DEPT = '";
	 strQuery += szDept;
	 strQuery += "'";
  }
  return Query((LPSTR)strQuery.c_str());
}

LPSTR DLLFUNC PD3(LPSTR szAccount,LPSTR szDept)
{
  string strQuery("SELECT PERIOD3 FROM GLACCTS WHERE ACCOUNT = ");
  strQuery += "'";
  strQuery += szAccount;
  if (strlen(szDept) > 0)
  {
	 strQuery += " AND DEPT = '";
	 strQuery += szDept;
	 strQuery += "'";
  }
  return Query((LPSTR)strQuery.c_str());
}



LPSTR DLLFUNC PD(LPSTR szAccount,LPSTR szDept,LPSTR szPeriod)
{

  Code4 cb;
  cb.init();

  int drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,"GL.INI");
  GetPrivateProfileString("GL_DIR","DIR","",szDir,sizeof(szDir),"GL.INI");

  _chdrive(drive);
  chdir(szDir);

  Data4 glaccts;
  if(glaccts.open(cb,"glaccts") < 0)
	{
	  cb.closeAll();
	  cb.initUndo();
	  return "";
	}
  glaccts.SetIndex("accounts");
  Relate4set rQuery(glaccts);

  Field4 period;
  string szQuery("ACCOUNT = '");
  szQuery += szAccount;
  szQuery += "'";


  if (strlen(szDept) > 0)
  {
	 szQuery += " .AND. DEPT = '";
	 szQuery += szDept;
	 szQuery += "'";
  }

	int iPeriod = atoi(strTrim(szPeriod));


	switch(iPeriod)
	 {
	  case 1: {
					period.init(glaccts,"period1");
					break;
				 }
	  case 2: {
					period.init(glaccts,"period2");
					break;
				 }
	  case 3: {
					period.init(glaccts,"period3");
					break;
				 }
	  case 4: {
					period.init(glaccts,"period4");
					break;
				 }
	  case 5: {
					period.init(glaccts,"period5");
					break;
				 }
	  case 6: {
					period.init(glaccts,"period6");
					break;
				 }
	  case 7: {
					period.init(glaccts,"period7");
					break;
				 }
	  case 8: {
					period.init(glaccts,"period8");
					break;
				 }
	  case 9: {
					period.init(glaccts,"period9");
					break;
				 }
	  case 10: {
					period.init(glaccts,"period10");
					break;
				 }
	  case 11: {
					period.init(glaccts,"period11");
					break;
				 }
	  case 12:
				 {
					period.init(glaccts,"period12");
					break;
				 }
	  default:
				 {
					period.init(glaccts,"period1");
					break;
				 }
	}

  string result;
  rQuery.querySet((LPSTR)szQuery.c_str());
  for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 result = strTrim(period.str());
  rQuery.unlock();
  rQuery.free();

  glaccts.close();
  cb.initUndo();
  return (LPSTR)result.c_str();
  /*

  string period = "PERIOD";
  switch(iPeriod)
  {
	 case 1: period += "1"; break;
	 case 2: period += "2"; break;
	 case 3: period += "3"; break;
	 case 4: period += "4"; break;
	 case 5: period += "5"; break;
	 case 6: period += "6"; break;
	 case 7: period += "7"; break;
	 case 8: period += "8"; break;
	 case 9: period += "9"; break;
	 case 10: period += "10"; break;
	 case 11: period += "11"; break;
	 case 12: period += "12"; break;
	 default :period += "1";
  }

  string strQuery("SELECT ");
  strQuery += period;
  strQuery += " FROM GLACCTS WHERE ACCOUNT = ";
  strQuery += "'";
  strQuery += szAccount;
  strQuery += "'" ;
  if (strlen(szDept) > 0)
  {
	 strQuery += " AND DEPT = '";
	 strQuery += szDept;
	 strQuery += "'";
  }
  return Query((LPSTR)strQuery.c_str());
  */

}


LPSTR DLLFUNC DESC(LPSTR szAccount,LPSTR szDept)
{
  Code4 cb;
  cb.init();

  int drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,"GL.INI");
  GetPrivateProfileString("GL_DIR","DIR","",szDir,sizeof(szDir),"GL.INI");

  _chdrive(drive);
  chdir(szDir);

  Data4 glaccts;
  if(glaccts.open(cb,"glaccts") < 0)
	{
	  cb.closeAll();
	  cb.initUndo();
	  return "";
	}
  glaccts.SetIndex("accounts");
  Relate4set rQuery(glaccts);

  Field4 desc(glaccts,"DESCRIPT");
  string szQuery("ACCOUNT = '");
  szQuery += szAccount;
  szQuery += "'";

  if (strlen(szDept) > 0)
  {
	 szQuery += " .AND. DEPT = '";
	 szQuery += szDept;
	 szQuery += "'";
  }

  string result;
  rQuery.querySet((LPSTR)szQuery.c_str());
  for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 result = strTrim(desc.str());
  rQuery.unlock();
  rQuery.free();

  glaccts.close();
  cb.initUndo();
  return (LPSTR)result.c_str();
}



LPSTR szlpQuery(LPSTR lpQuery,string m_strConnect)
{
 HENV    henv;
 HDBC    hdbc;
 HSTMT   hstmt;
 RETCODE retcode;
 const int NAME_LEN = 512;
 UCHAR szName[512];
 SDWORD cbName;

 string  result;
 retcode = SQLAllocEnv(&henv);              /* Allocate environment handle */
  if (retcode == SQL_SUCCESS) {
	retcode = SQLAllocConnect(henv, &hdbc);  /* Allocate connection handle */
	if (retcode == SQL_SUCCESS) {

		SQLSetConnectOption(hdbc,SQL_ODBC_CURSORS, SQL_CUR_USE_ODBC);
		/* Connect to data source */

		//string m_strConnect;
		char szConnectOutput[512];
		int cbConnStrOut;

		SQLDriverConnect(hdbc,GetFocus(),
			(UCHAR FAR*)(const char *)m_strConnect.c_str(), SQL_NTS,
			(UCHAR FAR*)szConnectOutput, sizeof(szConnectOutput),
			(short FAR*)&cbConnStrOut, SQL_DRIVER_COMPLETE);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

			/* Process data after successful connection */

			retcode = SQLAllocStmt(hdbc, &hstmt); /* Statement handle */

			//string SQL("select descript from glaccts where account = '101000'");
			string SQL(lpQuery);
			int iLength =  SQL.length();
			retcode = SQLExecDirect(hstmt,(UCHAR FAR*)(const char *)SQL.c_str(),iLength);
			if (retcode == SQL_SUCCESS)
			{
			 while(TRUE)
			  {
				 retcode = SQLFetch(hstmt);
				  if (retcode == SQL_SUCCESS)
					{
					 // For the number of columns in the result
					 // create a return value for each item.
					  SQLGetData(hstmt,1,1,szName, NAME_LEN,&cbName);
					  result = (char *)szName;
					  break;
					 }
					else
					break;
			  }
			}
			// must release the statement if the SQL Query fails
			else
			result = (char *) "";
			SQLFreeStmt(hstmt, SQL_DROP);
			SQLDisconnect(hdbc);
		}
		SQLFreeConnect(hdbc);
	}
	SQLFreeEnv(henv);
 }
 return (char *)result.c_str();
}


