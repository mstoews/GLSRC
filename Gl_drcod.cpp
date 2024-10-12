#include "std_inc.h"

#define ID_SHORT     4002
#define ID_NAME      4003
#define ID_TERRITORY 6038



extern HINSTANCE hInst;



void TManilaGL::GLDoctor  (RTMessage){GetApplication()->ExecDialog(new TGLDrCodes(this,1040));}
void TGlWindow::GLCloseMonth (RTMessage ) {}

void TGLDrCodeList::SetupWindow()
{ 
 TWindow::SetupWindow();
 SetWndDetail();
}

void TGLDrCodeList::WMSize(RTMessage Msg)
{

	 // NEW
  TWindow::WMSize(Msg);
  long iRows = gldrcodes->recCount();

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

void TGLDrCodeList::SetWndDetail()
{
 List->SetFields();
}

void TGLDrCodeList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);

 char *szBuffer1 = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(gldrcodes->seek(szBuffer1) == 0)
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





BOOL TGLDrCodes::TestDatabase()
 {
  return TRUE;
 }

void TGLDrCodes::RepsList(RTMessage)
{
 DRCODE->List->Seek(ID_CODE);
 DRCODE->Show(SW_SHOW);
}

void TGLDrCodes::SetupWindow()
{
 CHourGlass wait;
 TMainDialog::SetupWindow();

 gldrcodes    = new Tgldrcodes(cb);
 if(!gldrcodes->isValid())
  {
	ErrorClose(gldrcodes->GetDbfName());
	return;
  }

 gldrcodes->SetIndex("DRCODE");
 bDrCodes = FALSE;


 DRCODE = new TGLDrCodeList(this,"",gldrcodes);
 DRCODE->Attr.Style |= WS_THICKFRAME;
 GetApplication()->MakeWindow(DRCODE);


}


BOOL TGLDrCodes::szCheckField(int ID)
 {
  if(strlen(StrTrim(GetField(ID))) == 0)
  {
   char *szMsg = new char [45];
   wsprintf(szMsg,GetString(8101),GetString(ID));
   MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
   return TRUE;
 }

 BOOL TGLDrCodes::dCheckField(int ID)
 {
  if(dGetField(ID) == 0)
  {
   char *szMsg = new char [45];
   wsprintf(szMsg,GetString(8101),GetString(ID));
   MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
   SetFocus(GetDlgItem(HWindow,ID));
   return FALSE;
  }
   return TRUE;
 }

 void TGLDrCodes::Ok       (RTMessage Msg)
 {
  CHourGlass wait;
  gldrcodes->SetIndex("DRCODE");
  if(*StrTrim(GetField(ID_CODE)) == 0)
   return;

  if(gldrcodes->seek(GetField(ID_CODE)) != 0)
  {   
	gldrcodes->appendStart();
	Update(Msg);
	gldrcodes->append();
  }
  else if(MessageBox(HWindow,GetString(8122),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO) == IDYES)
  {
	 Update(Msg);
	 gldrcodes->flush();
  }

  New(Msg);
  DRCODE->List->Redraw();
}

void TGLDrCodes::Update (RTMessage)
{
	gldrcodes->drcode.assign		(GetField(ID_CODE));
	gldrcodes->rpshort.assign		(GetField(ID_SHORT));
	gldrcodes->rpname.assign		(GetField(ID_NAME));
	gldrcodes->address1.assign		(GetField(ID_ADDRESS1));
	gldrcodes->address2.assign		(GetField(ID_ADDRESS2));
	gldrcodes->address3.assign		(GetField(ID_ADDRESS3));
	gldrcodes->postal.assign		(GetField(ID_POST));
	gldrcodes->phone.assign		(GetField(ID_PHONE));
	gldrcodes->fax.assign		(GetField(ID_FAX));
	gldrcodes->contact.assign		(GetField(ID_CONTACT));
	gldrcodes->terms.assignDouble	(dGetField(ID_TERMS));
	gldrcodes->balance.assignDouble	(dGetField(ID_BALANCE));
	gldrcodes->creditlmt.assignDouble	(dGetField(ID_CREDITLMT));
	gldrcodes->territory.assign		(GetField(ID_TERRITORY));
	gldrcodes->UpdateUserTime(szUser);
}

void TGLDrCodes::SetData  (RTMessage)
  {
	SetField(ID_CODE	   ,&gldrcodes->drcode);
	SetField(ID_SHORT	   ,&gldrcodes->rpshort);
	SetField(ID_NAME	   ,&gldrcodes->rpname);
	SetField(ID_ADDRESS1 ,&gldrcodes->address1);
	SetField(ID_ADDRESS2 ,&gldrcodes->address2);
	SetField(ID_ADDRESS3 ,&gldrcodes->address3);
	SetField(ID_POST	   ,&gldrcodes->postal);
	SetField(ID_PHONE	   ,&gldrcodes->phone);
	SetField(ID_FAX	   ,&gldrcodes->fax);
	SetField(ID_CONTACT  ,&gldrcodes->contact);
	SetField(ID_TERMS    ,&gldrcodes->terms);
	SetField(ID_BALANCE  ,&gldrcodes->balance);
	SetField(ID_CREDITLMT,&gldrcodes->creditlmt);
	SetField(ID_TERRITORY,&gldrcodes->territory);
 }

 

void TGLDrCodes::Delete     (RTMessage Msg)
 {
  if(MessageBox(HWindow,GetString(8113),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
    if(gldrcodes->seek(GetField(ID_CODE)) == 0)
     {
      gldrcodes->deleteRec();
      gldrcodes->pack();
      DRCODE->List->Redraw();
      New(Msg);
     }
   }
 }

 void TGLDrCodes::Next     (RTMessage Msg)
 {
  gldrcodes->skip();
  SetData(Msg);
 }

 void TGLDrCodes::Previous (RTMessage Msg)
 {
  gldrcodes->skip(-1);
  SetData(Msg);
 }

 void TGLDrCodes::Top      (RTMessage Msg)
 { 
	gldrcodes->top();
   SetData(Msg);
 }

 void TGLDrCodes::Bottom   (RTMessage Msg)
 {
  gldrcodes->bottom();
  SetData(Msg);
 }
 void TGLDrCodes::New      (RTMessage)
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

void TGLDrCodes::Cancel   (RTMessage)
{
 DRCODE->List->SavePosition();
 gldrcodes->close();
 ShutDownWindow();
}

