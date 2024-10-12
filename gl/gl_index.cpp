/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Index Databases
*/

#include "tbowl.h"
#include <string.h>
#include "d4all.hpp"
#include "gl_strg.h"
#include "glclass.h"
#include "gl.h"
#include "gl_dlg.h"
#include "gl_pick.h"

#include "gl_main.h"
#include "gl_dlg.h"
#include "db_index.h"

#define ID_DESCRIPTION  1349
#define ID_TYPE         1350
#define ID_DBF          1351
#define ID_RECNO        1352
#define ID_FCODE        1353
#define ID_TYPES        1354


extern Code4 cb;


static TAG4INFO  gl_tagTag[] =
{
 {"fdbf","fdbf",0,0,0},
 {0,0,0,0,0},
};


/*** System Reindex ***/
_CLASSDEF(TReindex)
class TReindex : public TMainDialog
{
 public:
 PTIndex  Indx;
 PTTagx   Tagx;
 PTSpread List;

 TReindex(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  List = new TSpread(this,ID_DISTLIST);
 };
 virtual void SetupWindow();
 virtual void Cancel            (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok                (RTMessage Msg) = [ID_FIRST + IDOK];
 //virtual void IndexTagCreate    (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK]{CreateIndex();};
 virtual void ModifyDatabases   (RTMessage Msg) = [WM_FIRST + WM_LBUTTONDBLCLK];
 virtual void ReIndex           (RTMessage Msg) = [CM_FIRST + IDOK];
 char *SetRecNumber (char *Dbf);
 void Index (char *iName,TAG4INFO *tag,int iLine);
 void Index (char *iName,int iRow);
 void CreateIndex();
 void CreateTag(char *szDbf);
 void SetupListWindow(void);
};

_CLASSDEF(TModifyDbf)
class TModifyDbf : public TMainDialog
{
 public:
  PTIndex Indx;
  PTTagx  Tags;
  TModifyDbf(PTWindowsObject AParent, LPSTR AName,PTIndex I,PTTagx T): TMainDialog(AParent,AName)
   {
    Indx = I;
    Tags = T;
   }
  virtual void Add    (RTMessage Msg) = [ID_FIRST + ID_ADD];
  virtual void Modify (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
  virtual void List   (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
  virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL]{ShutDownWindow();};
  virtual void SetupWindow();
};

void TModifyDbf::SetupWindow()
{
 TMainDialog::SetupWindow();
 Indx->top();
 SetField(ID_FCODE,Indx->fDbf);
 SetField(ID_DESCRIPT,Indx->fDescript);
 SetField(ID_TYPES,Indx->fType);
}

void TModifyDbf::Add(RTMessage)
{


}

void TModifyDbf::Modify(RTMessage)
{
 MessageBox(HWindow,"Modify","",MB_OK);
}

void TModifyDbf::List(RTMessage)
{
 MessageBox(HWindow,"List","",MB_OK);
}


void TReindex::ModifyDatabases(RTMessage)
{
 GetApplication()->ExecDialog(new TModifyDbf(this,"MODIFYDBF",Indx,Tagx)); 
}

void TGlWindow::GLReindex     (RTMessage)
  {
	GetApplication()->ExecDialog(new TReindex(this,1065));
  }

void TReindex::ReIndex(RTMessage Msg)
{
 char *szItem = new char[11];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 List->GetData(4,Cell->Row,szItem);
 Index(szItem,(int) Cell->Row);
 delete szItem;
}


void TReindex::Ok (RTMessage)
{
 HourGlassOn(); 
 SetHandleCount(40);
 cb.autoOpen = FALSE;
 int i = 0;
 char *name = new char [Indx->fDbf->len()+1];
 for(Indx->top();!Indx->eof();Indx->skip())
  {
   strcpy(name,Indx->fDbf->str());
   Index(name,++i);
  }
 delete name;
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TReindex::Index(char *szName,int iRow)
{
 char *szQuery = new char[20];
 Tag4info tags(cb);
 strcpy(szQuery,"FDBF = '"); strcat(szQuery,szName); strcat(szQuery,"'");
 Indx->SetIndex("FDBF");
 Relate4set *rQuery = new Relate4set(Tagx);
 rQuery->querySet(szQuery);
 if(Indx->seek(szName) == 0 )
 {
  for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
    {
     char *szTagName = new char[Tagx->fTagname->len()+1];
     char *szTag  = new char[Tagx->fTag->len()+1];
     char *szFilter = new char[Tagx->fFilter->len()+1];
     strcpy(szFilter,Tagx->fFilter->str());
     strcpy(szTagName,Tagx->fTagname->str());     
     strcpy(szTag,Tagx->fTag->str());
     StrTrim(szTagName);
     StrTrim(szTag);
     StrTrim(szFilter);
     tags.add(szTagName,szTag,szFilter,Tagx->iUnique(),Tagx->iDescending());
     delete [] szTagName;
     delete []szTag;
     delete []szFilter;
     }
  }
 rQuery->unlock();
 rQuery->free();
 Index(szName,tags.tags(),iRow);
 delete szQuery;
}


void TReindex::Index(char *iName,TAG4INFO *tag,int iLine)
{
  cb.autoOpen = FALSE;
  cb.safety = FALSE;
  COLORREF lpBack, lpFore;
  List->GetColor(SS_ALLCOLS,iLine,&lpBack,&lpFore);
  List->SetColor(SS_ALLCOLS,iLine, RGBCOLOR_DARKBLUE, RGBCOLOR_YELLOW);
  List->ShowCell(1,iLine,SS_SHOW_NEAREST);
  Data4 Dbf(cb,iName);
  Index4 index;
  index.create(Dbf,NULL,tag);
  Dbf.close();
  List->SetColor(SS_ALLCOLS,iLine,lpBack,lpFore);
  cb.autoOpen = TRUE;
  return;
}


char  *TReindex::SetRecNumber(char *Dbf)
{
 char *szBuf = new char[10];
 cb.autoOpen = FALSE;
 DATA4 *Data;
 Data = d4open(&cb,Dbf);
 long iRecno = d4reccount(Data);
 ltoa(iRecno,szBuf,10);
 d4close(Data);
 cb.autoOpen = TRUE;
 return szBuf;
}



void TReindex::SetupWindow()
{
	 TDialog::SetupWindow();
	 cb.autoOpen = FALSE;
	 Index("gl_sys",gl_tagTag,0);
	 Index("gl_tag",gl_tagTag,0);
	 cb.closeAll();

	 Indx = new TIndex(&cb);
	 Tagx = new TTagx(&cb);
	 //CreateIndex();
	 SetBitmap(IDOK);
	 SetBitmap(IDCANCEL);
    SetupListWindow();
}


void TReindex::CreateIndex()
{
 if(MessageBox(GetFocus(),GetString(1348),"Tags",MB_YESNO | MB_ICONQUESTION) == IDYES)
 {  
  Tagx->zap();
  char *name = new char [Indx->fDbf->len()+1];
  for(Indx->top();!Indx->eof();Indx->skip())
  {
   strcpy(name,Indx->fDbf->str());
   CreateTag(name);
  }
  delete name;
 }
}

void TReindex::CreateTag(char *szDbf)
{ 
  Data4 db(cb,szDbf);
  Tag4 tag;
  for (tag.initFirst(db); tag.isValid(); tag.initNext())
   {  
    Str4large szName(tag.expr().source());
    szName.trim();  

    Tagx->appendStart();
		 Tagx->fDbf->assign(db.alias());
    Tagx->fTagname->assign(tag.alias());
    Tagx->fTag->assign(szName.str());
    if( tag.filter().isValid() )
     {	    
	Tagx->fFilter->assign(tag.filter().source());
     }
    else
       Tagx->fFilter->assign(" ") ;
    switch( tag.uniqueError() )
    {
       case r4unique:
	   Tagx->fUnique->assignLong(20);
           break ;
       case r4uniqueContinue:
	   Tagx->fUnique->assignLong(25) ;
           break ;
       case e4unique:
	   Tagx->fUnique->assignLong(30) ;
           break ;
       case 0:
	   Tagx->fUnique->assignLong(0) ;
           break ;
    }
   if( tag.descending() )
	  Tagx->fDescending->assignLong(10);
   else
	  Tagx->fDescending->assignLong(0);
    Tagx->append();
   }
   db.close();
}



void TReindex::Cancel (RTMessage)
{
 delete List;
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TReindex::SetupListWindow(void)
{
 RECT rc;
 SS_CELLTYPE 	CellType;
 HCURSOR  hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
 ShowCursor (TRUE);

 short d1Width,d2Width,d3Width;

 List->SetColWidth(2,8);
 List->SetColWidth(3,10);
 List->SetColWidth(4,10);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 GetWindowRect(List->HWindow,&rc);
 short ColWidth = (rc.right - rc.left) - d2Width - GetSystemMetrics(SM_CXHSCROLL);
 List->SetColWidthInPixels(1,ColWidth);
 List->SetMaxRows(Indx->recCount());
 List->SetMaxCols(2);
 List->SetData(1,SS_HEADER, "Description");
 List->SetData(2,SS_HEADER, "Records");
 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,14,0,0,999999999);
 List->SetCellType(2, SS_ALLROWS, &CellType);

 Indx->top();
 long i = 1;
 char szRecNo[6];
 List->SetBool(SSB_REDRAW,FALSE);
 while(!Indx->eof())
  {
   List->SetData(1,i,Indx->fDescript->str());
   strcpy(szRecNo,SetRecNumber(Indx->fDbf->str()));
   List->SetData(2,i,szRecNo);
   i++;
	Indx->skip(1);
  }

 if(i < 8)
 i = 8;
 List->SetMaxRows(i-1);
 List->SetBool(SSB_REDRAW,TRUE);
 ShowCursor (FALSE);
 SetCursor (hCursor) ;

}
