
#include "owl.h"
#include "utildlg.h"
#include "toolbox.h"

#include "d4all.hpp"
#include "gl_acct.hpp"
#include "gl_pds.hpp"
#include "gl_setat.hpp"
#include "gl_prof.hpp"
#include "crpe.h"
#include "dir.h"
#include "direct.h"
#include "math.h"



#define ID_PERIOD 1091
#define ID_PERIOD_DESC 1074
#define INI "GL.INI"


BOOL SetField(int ID, Field4 *field);
BOOL SetField(int ID, char *field);
char *GetString(UINT iError);
Code4 cb;
HINSTANCE hInst;

#pragma argsused
int FAR PASCAL LibMain (HINSTANCE hInstance, WORD wDataSeg, WORD hHeapSize, LPSTR lpszCmdLine)
{
 hInst = hInstance;
 if (hHeapSize > 0)
   UnlockData (0);
   return 1;

}

#pragma argsused
int FAR PASCAL _export WEP (int nParam)
{
 return 1; 
}         

BOOL SetField(int ID, Field4 *field)
{
  SendDlgItemMessage(GetFocus(),ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  return TRUE;
}      

BOOL SetField(int ID, char *field)
{
  SendDlgItemMessage(GetFocus(),ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return TRUE;
}


char FAR * PASCAL __export GetAccount(int ID)
{

 Code4 cb;
 char szAccount[8];
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.top();
 if(gl_setat.isValid())
  {
	switch(ID)
	{
	 case VATACCT:   strcpy(szAccount,gl_setat.account.str());  break;
	 case SALESACCT: strcpy(szAccount,gl_setat.account.str()); break;
	 case RETAINED:  strcpy(szAccount,gl_setat.account.str()); break;
	 case AP:        strcpy(szAccount,gl_setat.account.str()); break;
	 case AR:        strcpy(szAccount,gl_setat.account.str()); break;
	 case COMPANY:   strcpy(szAccount,gl_setat.account.str()); break;
	 case INDIV:     strcpy(szAccount,gl_setat.account.str()); break;
	 default: strcpy(szAccount,"");break;
	}
 }
 gl_setat.close();
 cb.initUndo();
 return szAccount;

}



BOOL FAR   PASCAL __export ValidateGL(LPSTR lpAccount,LPSTR lpDepart)
{
 Code4 cb;
 BOOL bValid = FALSE;
 GLACCTS glaccts;
 glaccts.open(cb);
 glaccts.SetIndex("ACCOUNTS");
 if(glaccts.isValid())
 {
  char *szSeek = new char [glaccts.account.len() + glaccts.dept.len() + 1];
  lstrcpy(szSeek,lpAccount);
  lstrcat(szSeek,lpDepart);
  if(glaccts.seek(szSeek) == 0)
   bValid = TRUE;
  delete [] szSeek;
 } 
 glaccts.close();
 cb.initUndo();
 return bValid;
}


void FAR PASCAL __export SetPeriods(HWND hWnd)
{                             
 DWORD dwIndex;
 GL_PDS gl_pds;
 gl_pds.open(cb);
 gl_pds.SetIndex("PERIOD");
 gl_pds.bottom();
 while(!gl_pds.bof())
  {
   if(strcmp(gl_pds.status.str(),"0") == 0)
     dwIndex = SendDlgItemMessage(hWnd,ID_PERIOD,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_pds.period.str());
   gl_pds.skip(-1);
  }

 char szPeriod[3];
 SendDlgItemMessage(hWnd,ID_PERIOD,CB_SETCURSEL,(WPARAM)dwIndex,NULL);
 SendDlgItemMessage(hWnd,ID_PERIOD,WM_GETTEXT,sizeof(szPeriod),(LPARAM)(LPSTR)szPeriod);
 if(gl_pds.seek(szPeriod) == 0)
   SendDlgItemMessage(hWnd,ID_PERIOD_DESC,WM_SETTEXT,NULL, (LPARAM)(LPSTR)gl_pds.pd_desc.str());
 gl_pds.close();
}

int FAR PASCAL __export GetPeriod(HWND hWnd)
{
 int rc = 0;
 char szPeriod[3];
 DWORD Indx = SendDlgItemMessage(hWnd,ID_PERIOD,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
   {      
    SendDlgItemMessage(hWnd,ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
    rc = atoi(szPeriod);
   }
 return rc;
}

char FAR * PASCAL __export szGetPeriod()
{
 Code4 cb;
 GL_PDS gl_pds;
 if(gl_pds.open(cb) < 0)
 {
	 cb.closeAll();
	 cb.init();
	 ShutDownWindow();
 }
 gl_pds.SetIndex("Period");
 int rc = gl_pds.bottom();
 char szPeriod[11];
 while(!gl_pds.bof())
  {
	if(strcmp(gl_pds.status.str(),"0") == 0)
	 strcpy(szPeriod,gl_pds.pd_desc.str());
	gl_pds.skip(-1);
  }
 gl_pds.close();
 cb.initUndo();
 return szPeriod;
}


int FAR  PASCAL __export iGetPeriod()
{
 Code4 cb;
 GL_PDS gl_pds;
 gl_pds.open(cb);
 gl_pds.SetIndex("Period");
 gl_pds.bottom();
 int iPeriod;
 while(!gl_pds.bof())
  {
   if(strcmp(gl_pds.status.str(),"0") == 0)
    iPeriod = atoi(gl_pds.period.str());
   gl_pds.skip(-1);
  }
 gl_pds.close();
 cb.initUndo();
 return iPeriod;
}


char FAR * PASCAL __export GetCompany()
{
 Code4 cb;
 GL_PROF gl_prof;
 gl_prof.open(cb);
 char szCompany[41];
 gl_prof.top();
 if(gl_prof.isValid())
    strcpy(szCompany,gl_prof.company.str());
 gl_prof.close();
 cb.initUndo();
 return szCompany;
}


void FAR PASCAL __export PrintReport(char *szReportName,HWND HWindow)
 {
  PEOpenEngine();
  HANDLE ReportHandle,SelectionFormula;

  short JobNumber = PEOpenPrintJob(szReportName);
  if(JobNumber != 0)
  {

   char   *szReportCaption;
   short  iReportTitleLen;

   char   *szReportSelection;
   short  iSelectionLen;
    
   PEGetReportTitle(JobNumber,&ReportHandle,&iReportTitleLen);
   szReportCaption = (char *)malloc(iReportTitleLen);

   PEGetSelectionFormula(JobNumber,&SelectionFormula,&iSelectionLen);
   szReportSelection = (char *)malloc(iSelectionLen);

   char *szCompanyName = new char[41];
   char *szPeriod = new char[4];

   wsprintf(szCompanyName,"'%s'",GetCompany());
   wsprintf(szPeriod,"'%d'",((TGlWindow *)Parent)->iGetPeriod());

   PESetFormula (JobNumber,"Company Name",szCompanyName);
   PESetFormula (JobNumber,"Current Period",szPeriod);

   PEGetHandleString(ReportHandle,szReportCaption,iReportTitleLen);
   PEGetHandleString(SelectionFormula,szReportSelection,iSelectionLen);
   if(strlen(szReportSelection) > 0)
   { 
    //MessageBox(HWindow,szReportSelection,"",MB_OK);
    //PTDialog TheDialog;
    //TheDialog = new TDialog(HWindow,"Test",DLLModule);

    PESetSelectionFormula(JobNumber,szReportSelection);
   }
    
   PEOutputToWindow(JobNumber,szReportCaption,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,HWindow);
   PEStartPrintJob(JobNumber,TRUE);
   //PEClosePrintJob(JobNumber);
   delete [] szCompanyName;
   delete [] szPeriod;
   free(szReportCaption);
   return;
  }
  else
  {
  HANDLE textHandle;
  char   *textBuffer;
  short   textLength;
  PEGetErrorText(JobNumber,&textHandle,&textLength);
  textBuffer = (char *)malloc(textLength+20);
  PEGetHandleString(textHandle,textBuffer,textLength);
  strcat(textBuffer,szReportName);
  MessageBox(HWindow,textBuffer,"Report Error",MB_OK);
  free(textBuffer);
  return;
  }
}

#define ID_DATA_ERROR 331
#define ID_INDEX_ERROR 332

static TAG4INFO  gl_Tag[] =
{
 {"fdbf","fdbf",0,0,0},
 {0,0,0,0,0},
};



BOOL FAR PASCAL __export InitFiles()
{
 Code4 cb;
 BOOL bError = FALSE;
 Data4 glSys,glTag,db;
 Index4 glInx,tgInx;
 Field4 fDbf,fTag;
 SetHandleCount(40);
 // Check to see if all the databases are there.
 glSys.open(cb,"GL_SYS");
 if(glSys.isValid())
 {
  glSys.SetIndex("FDBF");
  fDbf.init(glSys,"FDBF");
  for(glSys.top();!glSys.eof();glSys.skip())
   {
     db.open(cb,fDbf.str());
     if(!db.isValid())
      bError = TRUE;
   }
 }
 else
 MessageBox(GetFocus(),"unable to open GL_SYS","",MB_OK);
 // Check to see if the tags for the database are correct.
 //

BOOL bTest = TRUE;
if(glTag.open(cb,"GL_TAG"))
  {
   if(glTag.isValid())
    MessageBox(GetFocus(),"Valid GL_TAG","",MB_OK);
   glTag.SetIndex("FDBF");
   Field4 *fDatabase = new Field4;
   fDatabase->init(glTag,"FDBF");
   MessageBox(GetFocus(),fDatabase->str(),"Database",MB_OK);

   Field4 fTagName;
   fTagName.init(glTag,"TAGNAME");  
   MessageBox(GetFocus(),"Tag name","",MB_OK);
   for(glTag.top();!glTag.eof();glTag.skip())
    {
      MessageBox(GetFocus(),glTag.GetDbfName(),fTagName.str(),MB_OK);
      Tag4 tag;
      for(tag.initFirst(db);tag.isValid();tag.initNext())
	{
	 if(strcmp(StrTrim(tag.GetDbfName()),StrTrim(fTagName.str())) == 0)
	   bTest = FALSE;
	   // loop through the tags if the tag is not found the error return will remain FALSE
	   // if is found the loop quits and we go on to the next tag.
	  char szMsg[200];
			 wsprintf(szMsg,"File name = %s, Tag Name = %s, Tag Alias = %s",glTag.GetDbfName(),tag.GetDbfName(),fTag.str());
          MessageBox(GetFocus(),szMsg,"Tag Alias",MB_OK);
	}    
    }
  }
 if(bTest == TRUE)
  bError = TRUE;
 cb.closeAll();
 cb.initUndo();
 return bError;
}

BOOL FAR PASCAL __export CheckFile(LPSTR szFileName,HWND hWnd)
{
 Code4 cb;
 cb.autoOpen = FALSE;
 BOOL bFile = TRUE;
 char *szMsg = new char [255];
 
 Data4 db,gl_tag;
 Index4 idx;
 
 int rc = db.open(cb,szFileName);
 if(rc != 0)
  {   
   wsprintf(szMsg,GetString(ID_DATA_ERROR),e4text(rc),szFileName);
   MessageBox(hWnd,szMsg,"Database Error",MB_ICONINFORMATION);
   bFile = FALSE;
  }

 
 if(bFile)
 {
  rc = idx.open(db,szFileName);
  if(rc != 0)
  {   
   wsprintf(szMsg,GetString(ID_DATA_ERROR),e4text(rc),szFileName);
   MessageBox(hWnd,szMsg,"Database Error",MB_ICONINFORMATION);
   bFile = FALSE;
  }
 }

 cb.closeAll();
 cb.initUndo();
 delete [] szMsg;
 return bFile;
}


char *GetString(UINT iErrorCode)
{
 static char szMsg[255]; 
 LoadString(hInst,fabs(iErrorCode),szMsg,sizeof(szMsg));
 return szMsg;
}


/*
short	tableN;	// The table number in the report
struct PETableLocation location;
tableN = 0;	// The first table in the report

// Initialize values in location structure
location.StructSize = PE_SIZEOF_TABLE_LOCATION;
location.Location = "C:\REPORTS\ORDERS.RPT";

if ( !PESetNthTableLocation(Job, tableN, &location) ) {
	// Handle error
}

   short iNumberOfTables =  PEGetNTables(JobNumber);
   struct PETableLocation location;
   for(int i=0;i<iNumberOfTables;i++)
   {
     location.StructSize = PE_SIZEOF_TABLE_LOCATION;
     //wsprintf(location.Location,"%s",szCurDir);//,szReportName);
     wsprintf(location.Location,"%s","C:\GL\DBF\GL_CHART.DBF");//,szReportName);
     //location.Location = "C:\GL\DBF\GL_CHART.RPT";
     if(!PESetNthTableLocation(JobNumber,i,&location))
      {
	  HANDLE textHandle;
	  char   *textBuffer;
	  short   textLength;
	  PEGetErrorText(JobNumber,&textHandle,&textLength);
	  textBuffer = (char *)malloc(textLength);
	  PEGetHandleString(textHandle,textBuffer,textLength);
	  MessageBox(HWindow,textBuffer,"Report Error",MB_OK);
	  free(textBuffer);
	  return;

      }
   }


  char szGLDR[MAXPATH];
  char szCurDir[MAXPATH];
  getcwd(szCurDir,MAXPATH);
  strcpy(szGLDR,szCurDir);
  wsprintf(szGLDR,"%s\\%s",szCurDir,"GLDR.INI");
  int drive = GetPrivateProfileInt("GL_DIR","DRIVE",0,szGLDR);
  int rc = GetPrivateProfileString("GL_DIR","DIR","",szCurDir,sizeof(szCurDir),"GLDR.INI");
  if(rc <= 0)
   {   
     exit(1);
   }
   _chdrive(drive);

  


*/
