#include "std_inc.h"

#include "gl_medrp.h"
#include "gl_medrl.h"


extern HINSTANCE hInst;



void TManilaGL::GLMedicalReps  (RTMessage){GetApplication()->ExecDialog(new TGLMedReps(this,1006));}

void TGLMedRepList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TGLMedRepList::WMSize (RTMessage Msg)
{
	 // NEW
  TWindow::WMSize(Msg);
  long iRows = glmedrp->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  long cRows = ((rc.bottom - rc.top) / iHeight) + 1;

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

void TGLMedRepList::SetWndDetail()
{
 List->SetFields();
}

void TGLMedRepList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);

 char *szBuffer1 = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(glmedrp->seek(szBuffer1) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_UPDATE,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(bHide)
   Show(SW_HIDE);
 delete [] szBuffer1;
 return;
}





BOOL TGLMedReps::TestDatabase()
 {
  return TRUE;
 }

void TGLMedReps::RepsList(RTMessage)
{
 MEDREPS->List->Seek(ID_CODE);
 MEDREPS->Show(SW_SHOW);
}

void TGLMedReps::SetupWindow()
{
 CHourGlass wait;
 TMainDialog::SetupWindow();

 if(glmedreps.open(*cb) < 0)
  {
	ErrorClose(glmedreps.GetDbfName());
	return;
  }

 glmedreps.SetIndex("REPCODE");
 bMedReps = FALSE;

 MEDREPS = new TGLMedRepList(this,"",&glmedreps);
 GetApplication()->MakeWindow(MEDREPS);


}



BOOL TGLMedReps::szCheckField(int ID)
 {
  if(strlen(StrTrim(GetField(ID))) == 0)
  {

	wsprintf(szMsg,GetString(8101),GetString(ID));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
   return TRUE;
 }

 BOOL TGLMedReps::dCheckField(int ID)
 {
  if(dGetField(ID) == 0)
  {

	wsprintf(szMsg,GetString(8101),GetString(ID));
   MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
   return TRUE;
 }

 void TGLMedReps::Ok       (RTMessage Msg)
 {
  CHourGlass wait;
  glmedreps.SetIndex("REPCODE");
  if(*StrTrim(GetField(ID_CODE)) == 0)
   return;

  if(glmedreps.seek(GetField(ID_CODE)) != 0)
  {
   glmedreps.appendStart();
   Update(Msg);
   glmedreps.append();
  }
  else if(MessageBox(HWindow,GetString(8122),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO) == IDYES)
  {
	 Update(Msg);
	 glmedreps.flush();
  }

  New(Msg);
  MEDREPS->List->Redraw();
}

void TGLMedReps::Update (RTMessage)
{
   glmedreps.repcode.assign	(GetField(ID_CODE));
   glmedreps.rpshort.assign	(GetField(4002));
   glmedreps.rpname.assign	(GetField(4003));
   glmedreps.address1.assign	(GetField(ID_ADDRESS1));
   glmedreps.address2.assign	(GetField(ID_ADDRESS2));
   glmedreps.address3.assign	(GetField(ID_ADDRESS3));
   glmedreps.postal.assign	(GetField(ID_POST));
   glmedreps.phone.assign	(GetField(ID_PHONE));
   glmedreps.fax.assign	(GetField(ID_FAX));
   glmedreps.contact.assign	(GetField(ID_CONTACT));
   glmedreps.terms.assignDouble(dGetField(ID_TERMS));
   glmedreps.balance.assignDouble(dGetField(ID_BALANCE));
   glmedreps.creditlmt.assignDouble(dGetField(ID_CREDITLMT));
	glmedreps.territory.assign(GetField(6038));
	glmedreps.UpdateUserTime(szUser);
}

void TGLMedReps::SetData  (RTMessage)
  {
   SetField(ID_CODE,glmedreps.repcode);
   SetField(4002,glmedreps.rpshort);
	SetField(4003,glmedreps.rpname);
   SetField(ID_ADDRESS1,glmedreps.address1);
   SetField(ID_ADDRESS2,glmedreps.address2);
   SetField(ID_ADDRESS3,glmedreps.address3);
   SetField(ID_POST,glmedreps.postal);
   SetField(ID_PHONE,glmedreps.phone);
   SetField(ID_FAX,glmedreps.fax);
   SetField(ID_CONTACT,glmedreps.contact);
   SetField(ID_TERMS,glmedreps.terms);
   SetField(ID_BALANCE,glmedreps.balance);
   SetField(ID_CREDITLMT,glmedreps.creditlmt);
   SetField(6038,glmedreps.territory);
 }

 

void TGLMedReps::Delete     (RTMessage Msg)
 {
  if(MessageBox(HWindow,GetString(8113),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
    if(glmedreps.seek(GetField(ID_CODE)) == 0)
     {
      glmedreps.deleteRec();
      glmedreps.pack();
      MEDREPS->List->Redraw();
      New(Msg);
     }
   }
 }

 void TGLMedReps::Next     (RTMessage Msg)
 {
  glmedreps.skip();
  SetData(Msg);
 }

 void TGLMedReps::Previous (RTMessage Msg)
 {
  glmedreps.skip(-1);
  SetData(Msg);
 }

 void TGLMedReps::Top      (RTMessage Msg)
 { 
   glmedreps.top();
   SetData(Msg);
 }

 void TGLMedReps::Bottom   (RTMessage Msg)
 {
  glmedreps.bottom();
  SetData(Msg);
 }
 void TGLMedReps::New      (RTMessage)
 {
   SetField(ID_CODE,"");
   SetField(4002,"");
   SetField(4003,"");
   SetField(ID_ADDRESS1,"");
   SetField(ID_ADDRESS2,"");
   SetField(ID_ADDRESS3,"");
   SetField(ID_POST,"");
   SetField(ID_PHONE,"");
   SetField(ID_FAX,"");
   SetField(ID_CONTACT,"");
   SetField(ID_TERMS,"");
   SetField(ID_BALANCE,"");
   SetField(ID_CREDITLMT,"");
   SetField(6038,"");

   return;
}

void TGLMedReps::Cancel   (RTMessage)
{
 MEDREPS->List->SavePosition();
 glmedreps.close();
 ShutDownWindow();
}

