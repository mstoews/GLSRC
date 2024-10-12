/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Close the current month.
*/

#include <tbowl.h>
#include "d4all.hpp"
#include "gl.h"
#include "ar_id.h"
#include "glclass.h"
#include "gl_main.h"
#include "db_distd.h"
#include "db_dist.h"
#include "gl_clsm.h"
#include <direct.h>
#include <lzexpand.h>

#define ID_CURRENT 4009



extern Code4 cb;
extern char szApp[20];
extern HINSTANCE hInst;
char  *GetCurrentPeriod ();
char  *GetCurrentPeriod ()

{
  static char szCurrPeriod[20];
  GetPrivateProfileString("PERIOD","DATE","",szCurrPeriod,sizeof(szCurrPeriod),PROFILE);
  return szCurrPeriod;
}

/*

 1. Check to see if there are any outstanding batch entries.
 2. Change to period to the next month
 3. If next month is 1, current month must be 12 therefore warn the user
 to run the year end closing program.

*/

void TGlWindow::GLCloseMonth (RTMessage )
 {
   GetApplication()->ExecDialog(new TGLCloseMonth(this,1020));
   SendMessage(HStatusBar, SBRM_SETITEMTEXT,3,(LPARAM)GetCurrentPeriod());
 }

char  *TGLCloseMonth::GetPeriod(char *szPeriod)
{
  static char szCurrPeriod[20];
  GetPrivateProfileString(szPeriod,"DESCRIPTION","",szCurrPeriod,sizeof(szCurrPeriod),PROFILE);
  return szCurrPeriod;
}

int TGLCloseMonth::GetPeriodn(char *szPeriod)
{
  UINT rc = GetPrivateProfileInt("GL_DIR",szPeriod,0,PROFILE);
  return rc;
}


void TGLCloseMonth::SetupWindow()
{
 TDialog::SetupWindow();
 glaccts = new Data4(&cb,"glaccts");
 gldept  = new Data4(&cb,"gldept");
 glbtchd = new Data4(&cb,"gl_btchd");
 glbtch  = new Tglbtch(&cb);
 gldist  = new Tgldist(&cb);

 NewPeriod = new char [21];
 Period    = new char [21];
 batchnod = new Field4(*(Data4 *)glbtchd,GetString(hInst,8140));



 SendDlgItemMsg(ID_CURRENT,WM_SETTEXT,NULL,(LONG)(LPSTR)GetCurrentPeriod());

 int iPeriod = GetCurrentPeriodn();
 /*
 switch(iPeriod)
  {
	case  1: NewPeriodn=2;  strcpy(NewPeriod,GetPeriod("PERIOD2"));break;
	case  2: NewPeriodn=3;  strcpy(NewPeriod,GetPeriod("PERIOD3"));break;
	case  3: NewPeriodn=4;  strcpy(NewPeriod,GetPeriod("PERIOD4"));break;
	case  4: NewPeriodn=5;  strcpy(NewPeriod,GetPeriod("PERIOD5"));break;
	case  5: NewPeriodn=6;  strcpy(NewPeriod,GetPeriod("PERIOD6"));break;
	case  6: NewPeriodn=7;  strcpy(NewPeriod,GetPeriod("PERIOD7"));break;
	case  7: NewPeriodn=8;  strcpy(NewPeriod,GetPeriod("PERIOD8"));break;
	case  8: NewPeriodn=9;  strcpy(NewPeriod,GetPeriod("PERIOD9"));break;
	case  9: NewPeriodn=10; strcpy(NewPeriod,GetPeriod("PERIOD10"));break;
	case 10: NewPeriodn=11; strcpy(NewPeriod,GetPeriod("PERIOD11"));break;
	case 11: NewPeriodn=12; strcpy(NewPeriod,GetPeriod("PERIOD12"));break;
	case 12: NewPeriodn=1;  strcpy(NewPeriod,GetPeriod("PERIOD1"));break;
	default: NewPeriodn=1;  strcpy(NewPeriod,GetPeriod("PERIOD1"));break;
  }
 */
 SendDlgItemMsg(ID_NEXT,WM_SETTEXT,NULL,(LONG)(LPSTR)NewPeriod);
}

void TGLCloseMonth::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TGLCloseMonth::Ok (RTMessage)
{
 HourGlassOn();


 glbtch->SetIndex(GetString(hInst,8140));
 glbtch->top();
 Field4 mode(*(Data4 *)glbtch,"mode");
 while (!glbtch->eof())
 {
  
  if(strcmp(mode.str(),"1") == 0)
  {
	 MessageBox(HWindow,GetString(GetApplication()->hInstance,8114),szApp,MB_ICONINFORMATION);
	 cb.closeAll();
	 cb.init();
    ShutDownWindow();
    return;
  }
  else
  glbtch->skip(1);
 }
 glbtchd->bottom();
 double dBatchNo = double (Str4ptr(batchnod->str()));
 char *szBatchno= new char [6];
 char *szPeriod = new char[3];
 gcvt(dBatchNo,14,szBatchno);
 if(dBatchNo > 0)
 WritePrivateProfileString("PERIOD","BATCHNO",szBatchno,PROFILE);
 SendDlgItemMsg(ID_ADD,WM_SETTEXT,NULL,(LPARAM)(LPSTR)"Updating profile file...");
 
 itoa((int)NewPeriodn,szPeriod,10);
 WritePrivateProfileString("GL_DIR","PERIOD",szPeriod,PROFILE);
 WritePrivateProfileString("PERIOD","DATE",NewPeriod,PROFILE);

 CreateDistBackup();
 
 CopyINI();

 glbtch->top();
 glbtch->zap();
 glbtchd->zap();
 gldist->zap();
 gldist->pack();
 glbtch->pack();
 glbtchd->pack();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
 HourGlassOff();
 delete [] szBatchno;
 delete [] szPeriod;
 return;
}


void TGLCloseMonth::CreateDistBackup(void)
{
 Tgldistb* gldistb = new Tgldistb(&cb);
 Relate4set* rQuery = new Relate4set(gldist);
 rQuery->querySet(".NOT. DELETED()");
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
 {
  gldistb->appendStart();
  gldistb->account->assign(gldist->account->str());
  gldistb->dept->assign(gldist->dept->str());
  gldistb->type->assign(gldist->type->str());
  gldistb->period->assign(gldist->period->str());
  gldistb->date->assign(gldist->date->str());
  gldistb->batchno->assign(gldist->batchno->str());
  gldistb->reference->assign(gldist->reference->str());
  gldistb->cheque->assign(gldist->cheque->str());
  gldistb->descript->assign(gldist->descript->str());
  gldistb->debit->assignDouble(double (Str4ptr(gldist->debit->str())));
  gldistb->credit->assignDouble(double (Str4ptr(gldist->credit->str())));
  gldistb->append();
 }

}

void TGLCloseMonth::CopyINI(void)
{
  /*
  OFSTRUCT ofStrSrc;
  OFSTRUCT ofStrDest;
  HFILE hfSrcFile, hfDstFile;
  hfSrcFile = LZOpenFile("C:\\windows\\gl.ini", &ofStrSrc, OF_READ);
  hfDstFile = LZOpenFile("d:\\windows\\gl.ini", &ofStrDest, OF_CREATE);
  LZCopy(hfSrcFile, hfDstFile);
  LZClose(hfSrcFile);
  LZClose(hfDstFile);
 */
}

/*
BOOL TTopicsCopy::Copy(char *szSrc)
{
  SetField(104,szSrc);
  char *szDrive   = new char[5];
  char *szSource  = new char[MAXPATH];
  char *szTarget  = new char[MAXPATH];
  DWORD dwIndex = SendDlgItemMsg(101,CB_GETCURSEL,NULL,NULL);
  SendDlgItemMsg(101,CB_GETLBTEXT,(WPARAM)dwIndex,(LPARAM)(LPSTR)szDrive);
  strcat(szDrive,"\\");
  if(chdir(szDrive))
   {
    chdir(CurDir);
    char *szMsg = new char[50];
    strcpy(szMsg,"Make sure that a diskette is place in drive - ");
    strcat(szMsg,szDrive);
	 strcat(szMsg,". ");
    MessageBox(HWindow,szMsg,"Transfer",MB_ICONSTOP);
    delete szMsg;
	 return FALSE;
   }

  strcpy(szSource,szDrive);
  strcat(szSource,szSrc);
  strcpy(szTarget,GetField(102));
  strcat(szTarget,"\\");
  strcat(szTarget,szSrc);
  OFSTRUCT ofStrSrc;
  OFSTRUCT ofStrDest;
  HFILE hfSrcFile, hfDstFile;
  hfSrcFile = LZOpenFile(szSource, &ofStrSrc, OF_READ);
  hfDstFile = LZOpenFile(szTarget, &ofStrDest, OF_CREATE);
  LZCopy(hfSrcFile, hfDstFile);
  LZClose(hfSrcFile);
  LZClose(hfDstFile);
  delete [] szSource;
  delete [] szTarget;
  delete [] szDrive;
  return TRUE;
}

*/