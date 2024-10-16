#include <tbowl.h>
#include <commdlg.h>
#include <dir.h>
#include <direct.h>
#include "gl.h"
#include "glclass.h"
#include "gl_main.h"
#include "d4all.hpp"
#include "gl_manil.h"
#include "gl_dlg.h"
#include "db_itemm.h"
#include "gl_prods.h"
#include "gl_man.h"
#include "utildlg.h"

extern HINSTANCE hInst;
extern Code4 cb;
extern char szApp[20];

void TManilaGL::GLProductCode  (RTMessage){GetApplication()->ExecDialog(new TGLProd(this,1007));}

void TGLProdList::SetupWindow()
 {
  TWindow::SetupWindow();  
  SetDetailWnd();
 }

TGLProdList::~TGLProdList()
{
 //List->SavePosition();
}


void TGLProdList::WMSize(RTMessage Msg)
{
	 // NEW
  TWindow::WMSize(Msg);
  int iRows = glprod->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);

}

void TGLProdList::SetDetailWnd()
 {
 RECT rc;
 short d1Width,d2Width = 0;
 HINSTANCE hInst = GetApplication()->hInstance; 
 SetCaption(GetString(hInst,8104));
 List->SetFields("gl_itemm","fpcode");
 List->SetColWidth(1,6);
 List->SetColWidth(2,10);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(hInst,8105));
 List->SetData(2,SS_HEADER, GetString(hInst,8106));
 List->SetData(3,SS_HEADER, GetString(hInst,8106));
}

void TGLProdList::HandleList (RTMessage Msg)
{
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  char *szCode = new char [glprod->fPcode->len() + 1];
  SSGetData(GetDlgItem(HWindow,ID_DISTLIST),1,Cell->Row,szCode);
  List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
  if(glprod->seek(szCode) == 0)
   {   
    List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
    SendMessage(Parent->HWindow,WM_COMMAND,CM_UPDATE,NULL);
   }
  else
  List->SetColor(SS_ALLCOLS,SS_ALLROWS,List->lpBack,List->lpFore);  
}




BOOL TGLProd::TestDatabase()
 {
  BOOL bDatabase = TRUE;
  if(!CheckFile("GL_ITEMM",Parent->HWindow))
   bDatabase = FALSE;
  if(!bDatabase)
   BringWindowToTop(GetApplication()->MainWindow->HWindow);
  return bDatabase;
 }

void TGLProd::RepsList(RTMessage)
{
  PRODUCTS->List->Seek(ID_CODE);
  PRODUCTS->Show(SW_SHOW);
}

void TGLProd::SetupWindow()
{
 TMainDialog::SetupWindow();
 HourGlassOn();
 if(!TestDatabase())
  return;

 glprod    = new Tprods(&cb);
 glprod->SetIndex("fpcode");

 bProds = FALSE;
 

 PRODUCTS = new TGLProdList(this,"",glprod);
 GetApplication()->MakeWindow(PRODUCTS);
 PRODUCTS->SetCaption(GetString(8128));
 HourGlassOff();
}



BOOL TGLProd::szCheckField(int ID)
 {
  if(strlen(StrTrim(GetField(ID))) == 0)
  {

	wsprintf(szMsg,GetString(8101),GetString(ID));
	MessageBox(HWindow,szMsg,szApp,MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
   return TRUE;
 }

 BOOL TGLProd::dCheckField(int ID)
 {
  if(dGetField(ID) == 0)
  {

	wsprintf(szMsg,GetString(8101),GetString(ID));
   MessageBox(HWindow,szMsg,szApp,MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
   return TRUE;
 }

 void TGLProd::Ok       (RTMessage Msg)
 {

  if(glprod->seek(GetField(ID_CODE)) != 0)
  {
   glprod->appendStart();
   UpdateFields();
   glprod->append();
   glprod->unlock();
   PRODUCTS->List->Redraw();
   New(Msg);
  }
  else
  Update(Msg);
 }

void TGLProd::UpdateFields(void)
{
   glprod->fPcode->assign(GetField(ID_CODE));
   glprod->fPname->assign(GetField(ID_SNAME));
   glprod->fFulln->assign(GetField(ID_FULLN));
   glprod->fPtype->assign(GetField(ID_PTYPE));
   glprod->fUnit->assign(GetField(ID_UNIT));
   glprod->fOnhand->assignDouble(dGetField(ID_STANDARD));
   glprod->fLotsize->assignDouble(dGetField(ID_LOTSIZE));
	glprod->fAvprice->assignDouble(dGetField(ID_AVPRICE));
	glprod->UpdateUserTime(szUser);
}

void TGLProd::Update (RTMessage Msg)
{
 if(MessageBox(HWindow,GetString(8102),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
   UpdateFields();
   glprod->flush();
   glprod->unlock();
   PRODUCTS->List->Redraw();
   New(Msg);
  }
}

void TGLProd::SetData  (RTMessage)
 {
   SetField(ID_CODE,glprod->fPcode);
   SetField(ID_SNAME,glprod->fPname);
   SetField(ID_FULLN,glprod->fFulln);
   SetField(ID_PTYPE,glprod->fPtype);
   SetField(ID_STANDARD,glprod->fOnhand);
   SetField(ID_UNIT,glprod->fUnit);
   SetField(ID_LOTSIZE,glprod->fLotsize);
   SetField(ID_AVPRICE,glprod->fAvprice); 
}


void TGLProd::Delete (RTMessage Msg)
{
 if(szEnterError(ID_CODE))
  return;
  if(MessageBox(HWindow,GetString(8123),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
    if(glprod->seek(GetField(ID_CODE)) == 0)
    {
      glprod->deleteRec();
      glprod->pack();
      PRODUCTS->List->Redraw();
      New(Msg);      
    }
  }
}

void TGLProd::Next     (RTMessage Msg)
 {
  glprod->skip();
  SetData(Msg);
 }

 void TGLProd::Previous (RTMessage Msg)
 {
  glprod->skip(-1);
  SetData(Msg);
 }

 void TGLProd::Top      (RTMessage Msg)
 { 
   glprod->top();
   SetData(Msg);
 }

 void TGLProd::Bottom   (RTMessage Msg)
 {
  glprod->bottom();
  SetData(Msg);
 }
 void TGLProd::New      (RTMessage)
 {
   SetField(ID_CODE,"");
   SetField(ID_SNAME,"");
   SetField(ID_FULLN,"");
   SetField(ID_PTYPE,"");
   SetField(ID_SHOP,"");
   SetField(ID_SEMICODE,"");
   SetField(ID_STANDARD,"");
   SetField(ID_UNIT,"");
   SetField(ID_LOTSIZE,"");
   SetField(ID_AVPRICE,"");
   return;
}

void TGLProd::Cancel   (RTMessage)
{
 PRODUCTS->List->SavePosition();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

