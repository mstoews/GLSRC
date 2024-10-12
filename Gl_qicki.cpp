#include "glh.h"
#include "glclass.h"
#include "db_accts.h"
//#include "db_dept.h"
#include "db_profi.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_dist.h"
#include "db_alloc.h"
#include "db_srce.h"
#include "db_xacct.h"

#include "gl_main.h"
#include "gl_dlg.h"



_CLASSDEF (TGLXaccts)
class TGLXaccts : public TMainDialog
{
 public:
 PTgl_xacct gl_xacct;
  TGLXaccts(PTWindowsObject AParent,LPSTR ATitle,PTgl_xacct glx):
   TMainDialog(AParent,ATitle)
   {
     gl_xacct = glx;
   };
  virtual void Ok (RTMessage Msg ) = [ID_FIRST + IDOK];
  virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};

void TGLXaccts::Ok (RTMessage)
{
  CHourGlass wait;
  gl_xacct->appendStart(0);
  gl_xacct->oldacct->assign(GetField(ID_ACCOUNT));
  gl_xacct->dept->assign(GetField(ID_JDESCRIPT));
  gl_xacct->olddesc->assign(GetField(ID_DESCRIPT));
  gl_xacct->newacct->assign(GetField(ID_ACCOUNTS));
  gl_xacct->newdept->assign(GetField(ID_DEPART));
  gl_xacct->append();
  ShutDownWindow();
  return;
}

void TGLXaccts::Cancel(RTMessage)
{
 ShutDownWindow();
}

_CLASSDEF(TQuickEntry)
class TQuickEntry : public TMainDialog
{
 public:
  char szMsg[150];
  char szAccount[8];
  char szDept[5];

  PTgl_xacct  gl_xacct;
  PTglaccts   glaccts;
  PTglbtch glbtch;
  PTglbtchd glbtchd;
  PTGLBatchDetailList BATCH;
  BOOL bBatch;

  TQuickEntry (PTWindowsObject AParent, LPSTR ATitle):TMainDialog(AParent,ATitle){};
  virtual void SetupWindow();
  virtual void Add         (RTMessage Msg) = [ID_FIRST + ID_ADD];   // Modify is the same
  virtual void Modify      (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
  virtual void Top         (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom      (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Forward     (RTMessage Msg) = [ID_FIRST + ID_FORWARD];
  virtual void Back        (RTMessage Msg) = [ID_FIRST + ID_BACK];
  virtual void Delete      (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void Search      (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
  virtual void UpdateWindow(void);
  virtual void ClearScreen (void);
};



void TQuickEntry::Search(RTMessage)
{
 if(bBatch)
 {
  BATCH = new TGLBatchDetailList(this,"PICKLIST","FCUSTNO <> ' '",glbtchd);
  BATCH->EnableAutoCreate();
  GetApplication()->MakeWindow(BATCH);
  bBatch = FALSE;
 }
 /*
 if(!IsWindowVisible(BATCH->HWindow))
   BATCH->Show(SW_SHOW);
 else
   BATCH->Show(SW_HIDE);
 */
}

void TQuickEntry::SetupWindow()
 {
  TMainDialog::SetupWindow();
  glaccts = new Tglaccts(&cb);
	if(!->isValid())
  {
	ErrorClose();
	return;
  }


  //glaccts->SetIndex("accounts");
  glaccts->SetIndexAccts();
  gl_xacct = new Tgl_xacct(&cb);
  gl_xacct->SetIndex("oldacct");
  glbtchd = new Tglbtchd(&cb); // open the database "ar_cust.dbf"
  glbtch  = new Tglbtch(&cb);
  glbtchd->top();
  bBatch = TRUE;

  BATCH = new TGLBatchDetailList(this,"GL_QUICKLIST","BATCHNO > 0",glbtchd);
  BATCH->EnableAutoCreate();
  GetApplication()->MakeWindow(BATCH);
  bBatch = FALSE;
  BATCH->Show(SW_SHOW);
 }

void TGlWindow::GLQuickEntry (RTMessage){
  GetApplication()->ExecDialog(new TQuickEntry(this,"GL_QUICKENTRY"));
}

void TQuickEntry:: Add         (RTMessage Msg)
{
 if(dGetField(ID_BATCHNO)	== 0){EnterError(ID_BATCHNO)   ; return;}
 if(*GetField(ID_ACCOUNTS)      == 0){EnterError(ID_ACCOUNTS)   ; return;}
 if(*GetField(ID_DESCRIPT)      == 0){EnterError(ID_DESCRIPT); return;}
 if(*GetField(ID_SOURCEJ)        == 0){EnterError(ID_SOURCEJ) ; return;}
 if(dGetField(ID_DEBIT)         == 0 && dGetField(ID_CREDIT) == 0){EnterError(ID_DEBIT) ; return;}

 long lBatchno = dGetField(ID_BATCHNO);
 long lJournalno = dGetField(ID_SOURCEJ);

 char szSeek[12];
 strcpy(szSeek,GetField(ID_ACCOUNTS));
 strcat(szSeek,GetField(ID_DEPART));

 if(glaccts->seek(szSeek) != 0)
  {
    if(gl_xacct->seek(szSeek) != 0)
      {
	GetApplication()->ExecDialog(new TGLXaccts(this,"GL_XACCT",gl_xacct));
	strcpy(szAccount,gl_xacct->newacct->str());
	strcpy(szDept,gl_xacct->newdept->str());
	strcpy(szSeek,szAccount);
	strcat(szSeek,szDept);
	if(gl_xacct->seek(szSeek) != 0)
         {
          BWCCMessageBox(HWindow,"Entry not completed.","Add Slip Entry",MB_ICONINFORMATION);
          return;
         }

      }
    else
      {
	strcpy(szAccount,gl_xacct->newacct->str());
	strcpy(szDept,gl_xacct->newdept->str());
        MessageBox(HWindow,szAccount,szDept,MB_OK);
      }
  }
 else
  {
   strcpy(szAccount,glaccts->accounts->str());
   strcpy(szDept,glaccts->dept->str());
  }  




 if(glbtchd->seek(dGetField(ID_BATCHNO)) == 0 )
 {
    if(lBatchno != long (Str4ptr(glbtchd->batchno->str())) )
      {
       while(lBatchno == long (Str4ptr(glbtchd->batchno->str())) && !glbtchd->eof())
        {
	    if(lJournalno == long (Str4ptr(glbtchd->journal->str())))
		 {
		   Modify(Msg);
		   return;
		 }
            else
          glbtchd->skip();
        }
     }
   else
   {
    Modify(Msg);
    return;
    }
 }    
    glbtchd->appendStart();
    glbtchd->batchno->assignDouble(dGetField(ID_BATCHNO));
    glbtchd->mode->assign("1");
    glbtchd->journal->assignDouble(dGetField(ID_LINENO));
    glbtchd->period->assign(GetField(ID_PERIODS));
    glbtchd->sourcecode->assign("");
    glbtchd->date->assign(GetField(ID_DATE));
    glbtchd->reference->assign(GetField(ID_SOURCEJ));
    glbtchd->descript->assign(GetField(ID_DESCRIPT));
    glbtchd->cheque->assign(GetField(ID_CHEQUE));
    glbtchd->account->assign(szAccount);
    glbtchd->dept->assign(szDept);
    glbtchd->debit->assignDouble(dGetField(ID_DEBIT));
    glbtchd->credit->assignDouble(dGetField(ID_CREDIT));
    glbtchd->append();
    ClearScreen();
    bBatch = TRUE;
 
};

void TQuickEntry:: Modify      (RTMessage )
{
  if(DlgMsg(ID_QUICKMODIFY,ID_MODIFY,MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
   glbtchd->batchno->assignDouble(dGetField(ID_BATCHNO));
   glbtchd->mode->assign("1");
   glbtchd->journal->assignDouble(dGetField(ID_LINENO));
   glbtchd->period->assign(GetField(ID_PERIODS));
   glbtchd->sourcecode->assign("");
   glbtchd->date->assign(GetField(ID_DATE));
   glbtchd->reference->assign(GetField(ID_SOURCEJ));
   glbtchd->descript->assign(GetField(ID_DESCRIPT));
   glbtchd->cheque->assign(GetField(ID_CHEQUE));
   glbtchd->account->assign(szAccount);
   glbtchd->dept->assign(szDept);
   glbtchd->debit->assignDouble(dGetField(ID_DEBIT));
   glbtchd->credit->assignDouble(dGetField(ID_CREDIT));
   glbtchd->flush();
   bBatch = TRUE;
   }
};

void TQuickEntry:: Top         (RTMessage )
{
 glbtchd->top();
 UpdateWindow();
};

void TQuickEntry:: Bottom      (RTMessage )
{
 glbtchd->bottom();
 UpdateWindow();
};

void TQuickEntry:: Forward     (RTMessage )
{
 glbtchd->skip(1);
 UpdateWindow();
};

void TQuickEntry:: Back        (RTMessage )
{
 glbtchd->skip(-1);
 UpdateWindow();
};

void TQuickEntry:: Delete      (RTMessage )
{
 if(DlgMsg(ID_DELETE,ID_DELETE,MB_ICONQUESTION | MB_YESNO) == IDYES)
 {
  glbtchd->deleteRec();
  glbtchd->pack();
  UpdateWindow();
  bBatch = TRUE;
 }
};

void TQuickEntry:: UpdateWindow(void)
{
   SetField(ID_BATCHNO,	glbtchd->batchno);
   SetField(ID_LINENO,	glbtchd->journal);
   SetField(ID_PERIODS,	glbtchd->period);
   SetField(ID_DATE,	glbtchd->date);
   SetField(ID_SOURCEJ,	glbtchd->reference);
   SetField(ID_DESCRIPT,glbtchd->descript);
   SetField(ID_CHEQUE,	glbtchd->cheque);
   SetField(ID_ACCOUNTS,glbtchd->account);
   SetField(ID_DEPART,	glbtchd->dept);
   SetField(ID_DEBIT,	glbtchd->debit);
   SetField(ID_CREDIT,	glbtchd->credit);
};

void TQuickEntry:: ClearScreen(void)
{
   SetField(ID_BATCHNO,	"");
   SetField(ID_LINENO,	"");
   SetField(ID_PERIODS,	"");
   SetField(ID_DATE,	"");
   SetField(ID_SOURCEJ,	"");
   SetField(ID_DESCRIPT,"");
   SetField(ID_CHEQUE,	"");
   SetField(ID_ACCOUNTS,"");
   SetField(ID_DEPART,	"");
   SetField(ID_DEBIT,	"");
   SetField(ID_CREDIT,	"");

};


void TGLBatchDetailList::VirtualData(RTMessage )
{
/*
 LPSS_VQUERYDATA lpVQuery = (LPSS_VQUERYDATA) Msg.LParam;
 short i;
 lpVQuery->RowsLoaded = lpVQuery->RowsNeeded;
 for(i = 1; i<lpVQuery->RowsLoaded; i++)
 {
   List->SetData(1,i,glbtchd->batchno->str());
   List->SetData(2,i,glbtchd->journal->str());
   List->SetData(3,i,glbtchd->period->str());
   List->SetData(4,i,glbtchd->date->str());
   List->SetData(5,i,glbtchd->reference->str());
   List->SetData(6,i,glbtchd->descript->str());
   List->SetData(7,i,glbtchd->cheque->str());
   List->SetData(8,i,glbtchd->account->str());
   List->SetData(9,i,glbtchd->dept->str());
   List->SetData(10,i,glbtchd->debit->str());
   List->SetData(11,i,glbtchd->credit->str());
   glbtchd->skip(1);
 }
*/
}


void TGLBatchDetailList::SetupWindow ()
{
 TDialog::SetupWindow();
 RECT rc;
 SS_CELLTYPE 	CellType;

 HFONT   	hFont;
 HCURSOR  hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
 ShowCursor (TRUE);
 
 GetClientRect(HWindow,&rc);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
 MoveWindow(List->HWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 short d1Width
      ,d2Width
      ,d3Width
      ,d4Width
      ,d5Width
      ,d7Width
      ,d8Width
      ,d9Width
      ,d10Width
      ,d11Width;

 //List->SetBool(SSB_VIRTUALMODE,TRUE);
 //List->VScrollSetSpecial(TRUE,0);
 //List->VSetStyle(SSV_NOROWNUMBERS);
 //List->VSetMax(1500);
 List->SetMaxCols(11);
 List->SetColWidth(1,4);
 List->SetColWidth(2,3);
 List->SetColWidth(3,2);
 List->SetColWidth(4,9);
 List->SetColWidth(5,15);
 // Col 6 is adjusted
 List->SetColWidth(7,7);
 List->SetColWidth(8,7);
 List->SetColWidth(9,4);
 List->SetColWidth(10,12);
 List->SetColWidth(11,12);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 List->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 List->GetColWidthInPixels(5,(LPSHORT)&d5Width);
 List->GetColWidthInPixels(7,(LPSHORT)&d7Width);
 List->GetColWidthInPixels(8,(LPSHORT)&d8Width);
 List->GetColWidthInPixels(9,(LPSHORT)&d9Width);
 List->GetColWidthInPixels(10,(LPSHORT)&d10Width);
 List->GetColWidthInPixels(11,(LPSHORT)&d11Width);


 short ColWidth = (rc.right - rc.left)
		- d1Width
		- d2Width
		- d3Width
		- d4Width
		- d5Width
		- d7Width
		- d8Width
		- d9Width
		- d10Width
		- d11Width
		- GetSystemMetrics(SM_CXHSCROLL);

 List->SetColWidthInPixels(6,ColWidth); 

 List->SetBool(SSB_SHOWROWHEADERS,FALSE);
 List->SetBool(SSB_ALLOWUSERSELBLOCK,TRUE);
 List->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_ALLOWUSERRESIZE,FALSE);
 List->SetBool(SSB_AUTOSIZE,TRUE);
 List->SetBool(SSB_HORZSCROLLBAR,FALSE);


  hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 List->SetMaxRows(glbtchd->recCount());
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 List->SetData(1,SS_HEADER, "Bt");
 List->SetData(2,SS_HEADER, "Jn");
 List->SetData(3,SS_HEADER, "Pd");
 List->SetData(4,SS_HEADER, "Date");
 List->SetData(5,SS_HEADER, "Source");
 List->SetData(6,SS_HEADER, "Description");
 List->SetData(7,SS_HEADER, "Cheque");
 List->SetData(8,SS_HEADER, "Account");
 List->SetData(9,SS_HEADER, "Dept");
 List->SetData(10,SS_HEADER, "Debit");
 List->SetData(11,SS_HEADER, "Credit");

 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
 List->SetTypeInteger(&CellType,0,9999);
 List->SetCellType(1, SS_ALLROWS, &CellType);
 List->SetTypeInteger(&CellType,0,99);
 List->SetCellType(2, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,999999999.99);
 List->SetCellType(10, SS_ALLROWS, &CellType);
 List->SetCellType(11, SS_ALLROWS, &CellType);
 glbtchd->SetIndex(GetString(8140));
 glbtchd->top();
 long i = 1;
 Relate4set rQuery(glbtchd);
 rQuery.querySet(szQuery);
 List->SetBool(SSB_REDRAW,FALSE);
 for(int qc = rQuery.top();qc != r4eof;qc = rQuery.skip(1L))
  {
   List->SetData(1,i,glbtchd->batchno->str());
   List->SetData(2,i,glbtchd->journal->str());
   List->SetData(3,i,glbtchd->period->str());
   List->SetData(4,i,glbtchd->date->str());
   List->SetData(5,i,glbtchd->reference->str());
   List->SetData(6,i,glbtchd->descript->str());
   List->SetData(7,i,glbtchd->cheque->str());
   List->SetData(8,i,glbtchd->account->str());
   List->SetData(9,i,glbtchd->dept->str());
   List->SetData(10,i,glbtchd->debit->str());
   List->SetData(11,i,glbtchd->credit->str());
   i++;
  }
 List->SetBool(SSB_REDRAW,TRUE);
 rQuery.unlock();
 rQuery.free(0); 
 List->SetMaxRows(i);
 SetCaption("Standard Cost Allocation List");
 ShowCursor (FALSE);
 SetCursor (hCursor) ;
}

void TGLBatchDetailList::HandleListBox (RTMessage Msg)
{
 char szBatchno[7];
 char szJournalno[7];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 List->GetData(1,Cell->Row,szBatchno);
 List->GetData(2,Cell->Row,szJournalno);

 long lBatchno = atol(szBatchno);
 long lJournalno = atol(szJournalno);

 if(glbtchd->seek(lBatchno) == 0)
  {
   while(lJournalno != long (Str4ptr(glbtchd->journal->str())) && !glbtchd->eof())
     {
      glbtchd->skip(1);
     }
	SetField(ID_BATCHNO,	glbtchd->batchno);
   	SetField(ID_LINENO,	glbtchd->journal);
	SetField(ID_PERIODS,	glbtchd->period);
   	SetField(ID_DATE,	glbtchd->date);
   	SetField(ID_SOURCEJ,	glbtchd->reference);
   	SetField(ID_DESCRIPT,glbtchd->descript);
   	SetField(ID_CHEQUE,	glbtchd->cheque);
   	SetField(ID_ACCOUNTS,glbtchd->account);
   	SetField(ID_DEPART,	glbtchd->dept);
   	SetField(ID_DEBIT,	glbtchd->debit);
	SetField(ID_CREDIT,	glbtchd->credit);     
  }
  else
  BWCCMessageBox(HWindow,"Item number could not be found.","E R R O R",MB_ICONEXCLAMATION);
 //Show(SW_HIDE);
 SetFocus(Parent->HWindow);
}

void TGLBatchDetailList::SetField(int ID, Field4 *field)
{
  SendMessage(GetDlgItem(Parent->HWindow,ID),WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  return;
}     

