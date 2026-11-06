
#include "windows.h"
#include "gldbf.h"

#include "d4all.hpp"
#include "db_accts.h"




#pragma argsused
int FAR PASCAL LibMain (HINSTANCE hInstance, WORD wDataSeg, WORD hHeapSize, LPSTR lpszCmdLine)
{ 
 if (hHeapSize > 0)
   UnlockData (0);
   return 1;

}

#pragma argsused
int FAR PASCAL _export WEP (int nParam)
{
 return 1; 
}         



double FAR  * PASCAL __export CB(LPSTR lpAccount,LPSTR lpDepart)
{ 
 Code4 cb;
 static double dCurrent;
 Tglaccts glaccts(&cb);
 glaccts.SetIndex("accounts");

 if(glaccts.isValid())
 {
  Str4large szSeek;
  szSeek.setLen(glaccts.accounts->len() + glaccts.dept->len());
  szSeek.set(' ');
  szSeek.replace(Str4ptr(lpAccount));
  szSeek.replace(Str4ptr(lpDepart),glaccts.accounts->len());
  if(glaccts.seek(szSeek.str()) == 0)
   dCurrent = glaccts.Curr_bal();
  else   
   dCurrent = 0; 
 }
 glaccts.close();
 cb.initUndo();
 return &dCurrent;
}

double FAR * PASCAL __export CBR(LPSTR lpAccount,LPSTR lpEnd)
{
 Code4 cb;
 static double dCurrent = 0;
 Tglaccts *glaccts = new Tglaccts(&cb);
 char *szQuery = new char [45];
 Relate4set *rQuery;
 rQuery = new Relate4set(glaccts);

 strcpy(szQuery,"ACCOUNT >= '");
 strcat(szQuery,lpAccount);
 strcat(szQuery,"' .AND. ACCOUNT <= '");
 strcat(szQuery,lpEnd);
 strcat(szQuery,"'");

 rQuery->querySet(szQuery);
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
   {
    dCurrent = dCurrent + glaccts->Curr_bal();
   }
 glaccts->close();
 delete [] szQuery;
 cb.initUndo();
 return &dCurrent;
}

double FAR * PASCAL __export QUERY(LPSTR lpQuery)
{
 Code4 cb;
 static double dCurrent = 0;
 Tglaccts *glaccts = new Tglaccts(&cb);
 Relate4set *rQuery;
 rQuery = new Relate4set(glaccts);

 rQuery->querySet(lpQuery);
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
   {
    dCurrent = dCurrent + glaccts->Curr_bal();
   }
 glaccts->close();
 cb.initUndo();
 return &dCurrent;
}


double FAR  * PASCAL __export OPEN(LPSTR lpAccount,LPSTR lpDepart)
{ 
 Code4 cb;
 static double dCurrent = 0;
 Tglaccts glaccts(&cb);
 if(glaccts.isValid())
 {
  char *szSeek = new char [glaccts.accounts->len() + glaccts.dept->len() + 1];
  lstrcpy(szSeek,lpAccount);
  lstrcat(szSeek,lpDepart);
  if(glaccts.seek(szSeek) == 0)
   dCurrent = glaccts.Open_bal();
  delete [] szSeek;
 }
 glaccts.close();
 cb.initUndo();
 return &dCurrent;
}


char FAR  * PASCAL __export DESC(LPSTR lpAccount,LPSTR lpDepart)
{
 Code4 cb;
 static char szDesc[41];
 Tglaccts glaccts(&cb);
 glaccts.SetIndex("accounts");
 if(glaccts.isValid())
 {
  Str4large szSeek;
  szSeek.setLen(glaccts.accounts->len() + glaccts.dept->len());
  szSeek.set(' ');
  szSeek.replace(Str4ptr(lpAccount));
  szSeek.replace(Str4ptr(lpDepart),glaccts.accounts->len());
  if(glaccts.seek(szSeek.str()) != 0)
    strcpy(szDesc,"");
  else
  strcpy(szDesc,glaccts.descript->str());
 }
 else
 strcpy(szDesc,"");
 glaccts.close();
 cb.initUndo();
 return szDesc;
}


double FAR  * PASCAL __export PD(LPSTR lpAccount,LPSTR lpDepart,int iPeriod)
{ 
 Code4 cb;
 static double dCurrent;
 Tglaccts *glaccts = new Tglaccts(&cb);
 char *szSeek = new char [glaccts->accounts->len() + glaccts->dept->len() + 1];
 lstrcpy(szSeek,lpAccount);
 lstrcat(szSeek,lpDepart);
 if(glaccts->seek(szSeek) == 0)
  switch((int)iPeriod)
  {
    case 1 : dCurrent = glaccts->dPeriod1(); break;
    case 2 : dCurrent = glaccts->dPeriod2(); break;
    case 3 : dCurrent = glaccts->dPeriod3(); break;
    case 4 : dCurrent = glaccts->dPeriod4(); break;
    case 5 : dCurrent = glaccts->dPeriod5(); break;
    case 6 : dCurrent = glaccts->dPeriod6(); break;
    case 7 : dCurrent = glaccts->dPeriod7(); break;
    case 8 : dCurrent = glaccts->dPeriod8(); break;
    case 9 : dCurrent = glaccts->dPeriod9(); break;
    case 10: dCurrent = glaccts->dPeriod10();break;
    case 11: dCurrent = glaccts->dPeriod11();break;
    case 12: dCurrent = glaccts->dPeriod12();break;
    default: dCurrent = 0; break;
  }
 glaccts->close();
 delete [] szSeek;
 cb.initUndo();
 return &dCurrent;
}

double FAR  * PASCAL __export PR(LPSTR lpAccount,LPSTR lpDepart,int iPeriod)
{ 
 Code4 cb;
 static double dCurrent;
 Tglaccts *glaccts = new Tglaccts(&cb);
 char *szSeek = new char [glaccts->accounts->len() + glaccts->dept->len() + 1];
 lstrcpy(szSeek,lpAccount);
 lstrcat(szSeek,lpDepart);
 if(glaccts->seek(szSeek) == 0)
  switch((int)iPeriod)
  {
    case 1 : dCurrent = glaccts->dPrevious1(); break;
    case 2 : dCurrent = glaccts->dPrevious2(); break;
    case 3 : dCurrent = glaccts->dPrevious3(); break;
    case 4 : dCurrent = glaccts->dPrevious4(); break;
    case 5 : dCurrent = glaccts->dPrevious5(); break;
    case 6 : dCurrent = glaccts->dPrevious6(); break;
    case 7 : dCurrent = glaccts->dPrevious7(); break;
    case 8 : dCurrent = glaccts->dPrevious8(); break;
    case 9 : dCurrent = glaccts->dPrevious9(); break;
    case 10: dCurrent = glaccts->dPrevious10();break;
    case 11: dCurrent = glaccts->dPrevious11();break;
    case 12: dCurrent = glaccts->dPrevious12();break;
    default: dCurrent = 0; break;
  }
 glaccts->close();
 delete [] szSeek;
 cb.initUndo();
 return &dCurrent;
}

double FAR  * PASCAL __export BG(LPSTR lpAccount,LPSTR lpDepart,int iPeriod)
{ 
 Code4 cb;
 static double dCurrent;
 Tglaccts *glaccts = new Tglaccts(&cb);
 char *szSeek = new char [glaccts->accounts->len() + glaccts->dept->len() + 1];
 lstrcpy(szSeek,lpAccount);
 lstrcat(szSeek,lpDepart);
 if(glaccts->seek(szSeek) == 0)
  switch((int)iPeriod)
  {
    case 1 : dCurrent = glaccts->dBudget1(); break;
    case 2 : dCurrent = glaccts->dBudget2(); break;
    case 3 : dCurrent = glaccts->dBudget3(); break;
    case 4 : dCurrent = glaccts->dBudget4(); break;
    case 5 : dCurrent = glaccts->dBudget5(); break;
    case 6 : dCurrent = glaccts->dBudget6(); break;
    case 7 : dCurrent = glaccts->dBudget7(); break;
    case 8 : dCurrent = glaccts->dBudget8(); break;
    case 9 : dCurrent = glaccts->dBudget9(); break;
    case 10: dCurrent = glaccts->dBudget10();break;
    case 11: dCurrent = glaccts->dBudget11();break;
    case 12: dCurrent = glaccts->dBudget12();break;
    default: dCurrent = 0; break;
  }
 glaccts->close();
 delete [] szSeek;
 cb.initUndo();
 return &dCurrent;
}
