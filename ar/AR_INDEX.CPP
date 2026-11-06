#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "db_dept.h"
#include "db_cash.h"
#include "db_arbtc.h"
#include "ar_class.h"
#include "ar_index.h"

static TAG4INFO btchTag[] =
{
 {"batchno","batchno",0,0,0},
 {"recno","RECNO()",0,0,0},
 {"invoiceno","vendorno+invoiceno",0,0,0},
 {"account","account+depart",0,0,0},
 {0,0,0,0,0},
};

static TAG4INFO btchdTag[] =
{
 {"batchno","batchno",0,0,0},
 {"recno","RECNO()",0,0,0},
 {0,0,0,0,0},
};

static TAG4INFO cashTag[] =
{
 {"batchno","batchno",0,0,0},
 {"recno","RECNO()",0,0,0},
 {"invoiceno","vendorno+invoiceno",0,0,0},
 {"account","account+depart",0,0,0},
 {0,0,0,0,0},
};

static TAG4INFO cashdTag[] =
{
 {"batchno","batchno",0,0,0},
 {"recno","RECNO()",0,0,0},
 {0,0,0,0,0},
};


static TAG4INFO distTag[] =
{
   { "ACCOUNT",    "ACCOUNT"		, 0, 0, 0 },
   { "DEPT",       "DEPT"		, 0, 0, 0 },
   { "INVOICENO",  "vendorno+invoiceno" , 0, 0, 0 },
   { "BATCHNO",    "batchno"		, 0, 0, 0 },
   { "VENDORNO",   "vendorno"		, 0, 0, 0 },
   { "PAYMENTREQ", "PAYMENTREQ"		, 0, 0, 0 },
   { 0,0,0,0,0 }
} ;


TAG4INFO apsysTag[] =
{
 {"fdbf","fdbf",0,0,0},
 {0,0,0,0,0},
};

TAG4INFO vendTag[] =
{
 {"vendorno","vendorno",0,0,0},
 {0,0,0,0,0},
};


TAG4INFO  aptagTag[] =
{
 {"fdbf","fdbf",0,0,0},
 {0,0,0,0,0},
};


TAG4INFO ctrlTag[] =
{
 {"account","account+depart",0,0,0},
 {0,0,0,0,0},
};

TAG4INFO reqstTag[] =
{
 {"paymentreq","paymentreq",0,0,0},
 {0,0,0,0,0},
};

TAG4INFO setupTag[] =
{
 {"cash","account+dept","type = 'CA'",0,0},
 {"name","recno()",0,0,0},
 {0,0,0,0,0},
};

TAG4INFO DistTag[]=
{
 {"account","account+dept",0,0,0},
 {"batchno","batchno",0,0,0},
 {"descript","descript",0,0,0},
 {"apdist"  ,"batchno","type = 'AP '",0,0},
 {"ardist"  ,"batchno","type = 'AR '",0,0},
 {0,0,0,0,0},
};


_CLASSDEF(TReindex)
class TReindex : public TDialog
{
 public:
 PTIndex  Indx;
 PTTagx   Tagx;
 PTSpread List;

 TReindex(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName)
 {
  List = new TSpread(this,ID_DISTLIST);
 };
 virtual void SetupWindow();
 virtual void Cancel            (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok                (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Index             (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 char *SetRecNumber (char *Dbf);
 void Index (char *iName,TAG4INFO *tag,int iLine);
 void SetupListWindow(void);
};

void TGlWindow::APSysReindex     (RTMessage)
 {GetApplication()->ExecDialog(new TReindex(this,"SYSREINDEX"));}

void TReindex::Index(RTMessage Msg)
{
 char szItem[11];
 Tag4info tags(cb);
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 List->GetData(4,Cell->Row,szItem);
 Indx->SetIndex("FDBF");
 if(Indx->seek(szItem) == 0 )
 {
   if(Tagx->seek(szItem) == 0)
   {
    while(strcmp(szItem,Tagx->fDbf->str()) == 0)
     {
      tags.add((char *)Tagx->fTagname->str(),
	       (char *)Tagx->fTag->str(),
	       (char *)Tagx->fFilter->str(),
	       int(Str4ptr(Tagx->fUnique->str())),
	       int(Str4ptr(Tagx->fDescending->str())));
      Tagx->skip();
     }
     Index(szItem,tags.tags(),(unsigned int)Cell->Row);
   } 
 }

}


void TReindex::Ok (RTMessage)
{
 HourGlassOn(); 
 SetHandleCount(40);

 cb.autoOpen = FALSE;
 Index("ar_btch"  ,btchTag ,1);
 Index("ar_btchd" ,btchdTag,2);
 Index("ar_dist"  ,distTag ,3);
 Index("ar_cust"  ,vendTag ,4);
 Index("ar_setup" ,setupTag,5);
 Index("ar_jsum"  ,DistTag ,6);
 Index("gl_dist"  ,DistTag ,7);
 cb.closeAll();
 ShutDownWindow();
}



void TReindex::Index(char *iName,TAG4INFO *tag,int iLine)
{
  cb.autoOpen = FALSE;
  cb.safety = FALSE;
  COLORREF lpBack, lpFore;
  List->GetColor(SS_ALLCOLS,iLine,&lpBack,&lpFore);
  List->SetColor(SS_ALLCOLS,iLine, RGBCOLOR_DARKBLUE, RGBCOLOR_YELLOW);
  List->ShowCell(1,iLine,SS_SHOW_NEAREST);
  DATA4 *Dbf = d4open(&cb,iName); 
  i4create(Dbf,NULL,tag);
  d4close(Dbf);
  List->SetColor(SS_ALLCOLS,iLine,lpBack,lpFore);
  cb.autoOpen = TRUE;
  return;
}


char  *TReindex::SetRecNumber(char *Dbf)
{
 char szBuf[10];
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
    //SetBitmap(IDOK,"ADD");
    //SetBitmap(IDCANCEL,"QUIT");

    cb.autoOpen = FALSE;
    Index("ar_sys",apsysTag,0);
    Index("ar_tag",aptagTag,0);
    cb.closeAll();
    Indx = new TIndex(&cb);
    Tagx = new TTagx(&cb);
    SetupListWindow();
}



void TReindex::Cancel (RTMessage)
{
 cb.closeAll();
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
 List->GetColWidthInPixels(2,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d2Width);
 List->GetColWidthInPixels(4,(LPSHORT)&d3Width);
 GetWindowRect(List->HWindow,&rc);
 short ColWidth = (rc.right - rc.left) - d1Width - d2Width; // - GetSystemMetrics(SM_CXHSCROLL);
 List->SetColWidthInPixels(1,ColWidth);

 List->SetMaxRows(Indx->recCount());
 List->SetMaxCols(4);
 List->SetData(1,SS_HEADER, "Description");
 List->SetData(2,SS_HEADER, "Total Rec.");
 List->SetData(3,SS_HEADER, "Db");

 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);

 Indx->top();
 long i = 1;
 char szRecNo[6];
 List->SetBool(SSB_REDRAW,FALSE);
 while(!Indx->eof())
  {
   List->SetData(1,i,Indx->fDescript->str());
   List->SetData(2,i,"AR");
   strcpy(szRecNo,SetRecNumber(Indx->fDbf->str()));
   List->SetData(3,i,szRecNo);
   List->SetData(4,i,Indx->fDbf->str()); 
   i++;
   Indx->skip(1);
  }

 if(i < 12)
 i = 12;
 List->SetMaxRows(i-1);
 List->SetBool(SSB_REDRAW,TRUE);
 ShowCursor (FALSE);
 SetCursor (hCursor) ;
}
