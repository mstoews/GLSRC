// Access dialog box
// General Accounting Module Password Protection...
#include "std_inc.h"
#define ID_UPDATEWINDOW 1845
#define ID_CLEARING        1846
#define ID_RESET	      1094
#define ID_RECNO		   1173

BOOL   bMenuSetup,bUpdate;
char szAccessLevel[11];
char szUser[11];
char szMenuCode[120];


_CLASSDEF (TGLAccess)
class TGLAccess : public TMainDialog
{
 public :
 BOOL ACCEPT;
 TAccess* access;

 TGLAccess(PTWindowsObject AParent, UINT ATitle):TMainDialog(AParent,ATitle){};
 virtual void SetupWindow();
 virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Add    (RTMessage Msg) = [ID_FIRST + ID_ADD];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};


_CLASSDEF (TGLAdministration)
class TGLAdministration : public TMainDialog
{
 public:
 TAccess* access;
 TAccessl* Accessl;
 TAccessList* AccessList;
 TGLAdministration(PTWindowsObject AParent, UINT ATitle,PTAccess ac) : TMainDialog(AParent,ATitle){access = ac;}
 TGLAdministration(PTWindowsObject AParent, UINT ATitle) : TMainDialog(AParent,ATitle){}
 virtual void Add      (RTMessage Msg) = [ID_FIRST + ID_ADD];
 virtual void Modify   (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void Cancel   (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Next     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Bottom   (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void Previous (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Top      (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Update   (RTMessage Msg) = [CM_FIRST + ID_UPDATEWINDOW];
 virtual void Update();
 virtual void Clear    (RTMessage Msg) = [CM_FIRST + ID_CLEARING];
 virtual void Reset    (RTMessage Msg) = [ID_FIRST + ID_RESET];
 virtual void Menus    (RTMessage Msg) = [ID_FIRST + ID_VIEW];
 virtual void MenuList (RTMessage Msg) = [CM_FIRST + ID_VIEW]{Menus(Msg);};
 virtual void GetCalc  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void Search   (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
 virtual void SetupWindow();
 char* Strip(char *szText,char *t);
};


_CLASSDEF(TGLUserAccess)
class TGLUserAccess : public TGLAdministration
{
  public:
  TAccessLevel* AccessLevel;
  TGLUserAccess(PTWindowsObject AParent, UINT ATitle) : TGLAdministration(AParent,ATitle){}
  virtual void Update   (RTMessage Msg) = [CM_FIRST + ID_UPDATEWINDOW];
  virtual void SetupWindow();
  virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void Next     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
  virtual void Bottom   (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Previous (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
  virtual void Top      (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Search   (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
  virtual void Cancel   (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Add      (RTMessage Msg) = [ID_FIRST + ID_ADD];
  virtual void Update();
  void RedrawList();
};


void TGLUserAccess::Delete(RTMessage)
{
 if(Accessl->seek(GetField(ID_ACCESSLEVEL)) == 0)
  if(IDYES == MessageBox(HWindow,GetString(1036),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION|MB_YESNO))
  {
	// Some sort of validation is necessary so that
	// users don't delete a access level
	// that is still in use.
	Accessl->deleteRec();
	Accessl->pack();
	RedrawList();
  }
 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}


void TGLUserAccess::RedrawList()
{
  if(AccessLevel == NULL)
  {
	AccessLevel = new TAccessLevel(this,"",Accessl);
	AccessLevel->EnableAutoCreate();
	GetApplication()->MakeWindow(AccessLevel);
  }
  AccessLevel->List->SetBool(SSB_REDRAW,FALSE);
  AccessLevel->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
	int row = 1;
	for(Accessl->top();!Accessl->eof();Accessl->skip())
	 {
	  AccessLevel->List->SetData(1,row,Accessl->fLevel.str());
	  AccessLevel->List->SetData(2,row,Accessl->fDescript.str());
	  ++row;
	 }
  AccessLevel->List->SetBool(SSB_REDRAW,TRUE);
}

void TGLUserAccess::Search(RTMessage)
{
  if(AccessLevel == NULL)
  {
	AccessLevel = new TAccessLevel(this,"",Accessl);
	AccessLevel->EnableAutoCreate();
	GetApplication()->MakeWindow(AccessLevel);
  }
  RedrawList();
  AccessLevel->Show(SW_SHOW);
  RedrawList();
}

void TGLAdministration::Search (RTMessage)
{
  if(AccessList == NULL)
  {
	AccessList = new TAccessList(this,"",access);
	AccessList->EnableAutoCreate();
	GetApplication()->MakeWindow(AccessList);
  }
  AccessList->Show(SW_SHOW);
}

void TGLUserAccess::Next(RTMessage)
{
  Accessl->skip();
  if(Accessl->eof())
	Accessl->bottom();
  SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}

void TGLUserAccess::Bottom(RTMessage)
{
  Accessl->bottom();
  SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}

void TGLUserAccess::Top(RTMessage)
{
  Accessl->top();
  SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}

void TGLUserAccess::Previous(RTMessage)
{
  Accessl->skip(-1);
  if(Accessl->bof())
	Accessl->top();
  SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}


void TGLUserAccess::SetupWindow()
{
  TMainDialog::SetupWindow();
  AccessLevel = NULL;
  SetBitmap(ID_MODIFY);
  SetBitmap(ID_SEARCH);
  SetBitmap(ID_ADD);
  SetBitmap(ID_VIEW);
  SetBitmap(ID_RESET);
  Accessl = new TAccessl(cb);
  if(!Accessl->isValid())
  {
	ErrorClose(Accessl->GetDbfName());
	return;
  }


  Accessl->reindex();
}
void TGLUserAccess::Update (RTMessage)
{
 strcpy(szMenuCode,Accessl->fMenuCode.str());
 SetField(ID_ACCESSLEVEL,Accessl->fLevel.str());
 SetField(ID_DESCRIPT,Accessl->fDescript.str());
}

_CLASSDEF (TAccessMenus)
class TAccessMenus : public TMainDialog
{
 public :
 TSpread* List;
 TAccessMenus(PTWindowsObject AParent, UINT ATitle):TMainDialog(AParent,ATitle)
 {
	List = new TSpread(this,ID_DISTLIST);
 };
 virtual void SetupWindow();
 virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Select (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};

void TGLAdministration::Cancel(RTMessage)
{
  if(AccessList != NULL)
	AccessList->List->SavePosition();
  cb->closeAll();
  cb->init();
  ShutDownWindow();
}

void TGLUserAccess::Cancel(RTMessage)
{
  if(AccessLevel != NULL)
	AccessLevel->List->SavePosition();
  cb->closeAll();
  cb->init();
  ShutDownWindow();
}


void TAccessMenus::Select(RTMessage Msg)
{
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD) Msg.LParam;
 char szState[2];
 List->GetData(2,Cell->Row,szState);
 if(strcmp(szState,"0") == 0)
  {
	 List->SetData(2,Cell->Row,"1");
	 List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
  {
	 List->SetData(2,Cell->Row,"0");
	 List->SetColor(SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
  }
 bUpdate = TRUE;
}

void TAccessMenus::Ok (RTMessage)
{
 CHourGlass wait;
 SS_CELLCOORD LastCell;
 List->GetLastValidCell(&LastCell.Col,&LastCell.Row);
 char* szMenus = new char[(int)LastCell.Row];
 char szState[2];
 for(int i = 0;i < LastCell.Row; i++)
 {
	szMenus[i] = '0';
	List->GetData(2,i+1,szState);
	if(strcmp(szState,"1") == 0)
	 szMenus[i] = '1';

 }
 szMenus[(int)LastCell.Row] = NULL;
 strcpy(szMenuCode,szMenus);
 ShutDownWindow();
}


void TAccessMenus::SetupWindow()
{
 TDialog::SetupWindow();

 CHourGlass wait;
 bUpdate = FALSE;

 HFONT   hFont;
 SS_CELLTYPE CellType;

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 List->SetBool(SSB_AUTOSIZE,FALSE);
 List->SetBool(SSB_ALLOWUSERSELBLOCK,TRUE);
 List->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_ALLOWUSERRESIZE,FALSE);
 List->SetBool(SSB_PROTECT,TRUE);
 List->SetBool(SSB_SHOWROWHEADERS,FALSE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);

 RECT rc;
 List->GetClientRect(&rc);
 short d2Width;
 List->SetMaxCols (2);
 List->SetColWidth(2,4);

 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);

 short ColWidth = (rc.right - rc.left)- d2Width;
 List->SetColWidthInPixels(1,ColWidth);


 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(1, SS_ALLROWS, &CellType);

 List->SetTypeButton(
					  &CellType,
					  0,
					  NULL,
					  "PLUS",
					  SUPERBTN_PICT_BITMAP,
					  "MINUS",
					  SUPERBTN_PICT_BITMAP,
					  SUPERBTN_TYPE_2STATE,
					  1,
					  NULL);

 List->SetCellType(2,SS_ALLROWS, &CellType);
 List->SetData(2,SS_ALLROWS,"0");

 SetBitmap(IDOK);
 SetBitmap(IDCANCEL);
 List->SetData(1,SS_HEADER, GetString(8149));
 List->SetData(2,SS_HEADER, " ");

 Data4 Accessm(&((TAcctModule *)GetApplication())->cb,"accessm");
 Accessm.SetIndex("menu");
 List->SetMaxRows(Accessm.recCount());
 Field4 menu(Accessm,3);
 int i = 1;
 for(Accessm.top();!Accessm.eof();Accessm.skip())
  {
	 List->SetData(1,i,menu.str());i++;
  }
 Accessm.close();

 char* t;
 t = szMenuCode;
 i = 1;
 while(*t != '\0')
  {
	if(*t == '1')
	 {
	  List->SetData(2,i,"1");
	  List->SetColor(SS_ALLCOLS,i,RGB(192,220,192), RGBCOLOR_BLUE);
	 }
	i++;
	t++;
  }

}

void TAccessMenus::Cancel(RTMessage)
{
  ShutDownWindow();
}


void TGLAdministration::Menus(RTMessage)
{
  if(strlen(StrTrim(GetField(ID_ACCESSLEVEL))) == 0)
	{
	 MessageBox(HWindow,GetString(6095),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 return;
	}

  GetApplication()->ExecDialog(new TAccessMenus(this,1160));
  //GetApplication()->ExecDialog(new TDialog(this,1160));
  // Seek the menu access level code in the accessl.dbf file
  // if the access level code can't be found add the code
  // if the code is found and the menu code is different
  // ask the user if he wants to change the code
  //
  if(Accessl->seek(GetField(ID_ACCESSLEVEL)) == 0)
  {
  if(bUpdate)
	{
	 if(MessageBox(HWindow,GetString(6096),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	  {
		Accessl->fMenuCode.assign(szMenuCode);
	  }
	}
  }
  else
  {
	Accessl->appendStart();
	Accessl->fLevel.assign(GetField(ID_ACCESSLEVEL));
	Accessl->fMenuCode.assign(szMenuCode);
	Accessl->append();
  }
}


void TManilaGL::GLSysAccess (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLAccess(this,1158));
 }

void TManilaGL::GLUserAccess(RTMessage)
{
 GetApplication()->ExecDialog(new TGLUserAccess(this,1161));
}

void TManilaGL::GLAccessMain (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLAdministration(this,1159));
 }

void TGLAdministration::Update(RTMessage)
{
 Str4ten szRecno;
 szRecno.assignLong(access->recNo(),4);
 SetField(ID_RECNO,StrTrim(szRecno.str()));
 SetField(ID_USER,access->fUsername);
 SetField(ID_DESCRIPT,access->fDescript);
 SetField(ID_PASSWORD,access->fPassword);

 char szEnterDate[11];
 Date4 szDate;
 szDate.assign(access->fDate.str(),"CCYYMMDD");
 szDate.format(szEnterDate,GetString(4024));
 SetField(ID_DATE,szEnterDate);

 DWORD dIndex = SendDlgItemMsg(ID_ACCESSLEVEL,CB_FINDSTRING,0,(LPARAM)(LPSTR)access->fLevel.str());
 if(dIndex > 0)
  SendDlgItemMsg(ID_ACCESSLEVEL,CB_SETCURSEL,(WPARAM)dIndex,NULL);

}



void TGLAdministration::Top(RTMessage)
{
 access->top();
 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}

void TGLAdministration::Bottom(RTMessage)
{
 access->bottom();
 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}


void TGLAdministration::Clear(RTMessage)
{
 SetField(ID_USER,"");
 SetField(ID_DESCRIPT,"");
 SetField(ID_DATE,"");
 SetField(ID_PASSWORD,"");
 SetField(ID_ACCESSLEVEL,"");
}

void TGLAdministration::Next(RTMessage)
{
	if(!access->eof())
	{
	 access->skip();
	 if (access->eof())
	  access->bottom();
	 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
	}
}

void TGLAdministration::Previous(RTMessage)
{
 if(*GetField(ID_USER) !=0)
  if(access->seek(GetField(ID_USER)) == 0 && !access->bof())
	{
	 access->skip(-1L);
	 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
	}
}




void TGLAdministration::Delete(RTMessage)
{
 if(access->seek(GetField(ID_USER)) ==0)
  if(MessageBox(HWindow,GetString(1036),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION|MB_YESNO) == IDYES)
  {
	access->deleteRec();
	access->pack();
  }
 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}

void TGLAdministration::Update()
{
	access->fUsername.assign(GetField(ID_USER));
	access->fDescript.assign(GetField(ID_DESCRIPT));
	access->fDate.assign(GetDate(ID_DATE));
	access->fPassword.assign(GetField(ID_PASSWORD));
	DWORD dIndex = SendDlgItemMsg(ID_ACCESSLEVEL,CB_GETCURSEL,NULL,NULL);
	 string szAccessLevel;
	SendDlgItemMsg(ID_ACCESSLEVEL,CB_GETLBTEXT,(WPARAM)dIndex,(LPARAM)(LPSTR)szAccessLevel.c_str());
	access->fLevel.assign((char *)szAccessLevel.c_str());
}

void TGLUserAccess::Update()
{
	Accessl->fLevel.assign(GetField(ID_ACCESSLEVEL));
	Accessl->fDescript.assign(GetField(ID_DESCRIPT));
	Accessl->fMenuCode.assign(szMenuCode);
}


void TGLAdministration::Add(RTMessage Msg)
{
 if(strlen(StrTrim(GetField(ID_USER))) != 0)
 {
  if(access->seek(GetField(ID_USER)) !=0)
  {
	  access->appendStart();
	  Update();
	  access->append();
	  Clear(Msg);
  }
  else
  {
	Modify(Msg);
	Clear(Msg);
  }
 }
 else
 MessageBox(HWindow,GetString(6105),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
}

void TGLUserAccess::Add(RTMessage Msg)
{

 if(strlen(StrTrim(GetField(ID_ACCESSLEVEL))) == 0)
 {
  MessageBox(HWindow,GetString(6106),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  return;
 }

  if(Accessl->seek(GetField(ID_ACCESSLEVEL)) !=0)
  {
	 if(strlen(StrTrim(szMenuCode)) < 50 )
	  {
		SendMessage(HWindow,WM_COMMAND,ID_VIEW,NULL);
		return;
	  }
	  Accessl->appendStart();
	  Update();
	  Accessl->append();
  }
  else
  Update();

 Clear(Msg);
 RedrawList();
}


void TGLAdministration::Modify(RTMessage)
{
 if(access->seek(GetField(ID_USER))==0)
  //"Are you sure you want to modify this user."
  if(MessageBox(HWindow,GetString(1034),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO |MB_ICONQUESTION) == IDYES)
  {
	Update();
  }
 else
 // "Are you sure you want to modify this user."
 MessageBox(HWindow,GetString(1035),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION | MB_OK);
}


void TGLAdministration::SetupWindow()
{
 TMainDialog::SetupWindow();
 AccessList = NULL;
 SetBitmap(ID_MODIFY);
 SetBitmap(ID_SEARCH);
 SetBitmap(ID_ADD);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_RESET);
 access = new TAccess(cb);
 if(!access->isValid())
  {
	ErrorClose(access->GetDbfName());
	return;
  }


 Accessl = new TAccessl(cb);
 if(!Accessl->isValid())
  {
	ErrorClose(Accessl->GetDbfName());
	return;
  }


 for(Accessl->top();!Accessl->eof();Accessl->skip()){
  SendDlgItemMsg(1126,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)Accessl->fLevel.str());
 }
 SendMessage(HWindow,WM_COMMAND,ID_UPDATEWINDOW,NULL);
}

TAG4INFO  accessTag[] =
{
 {"u","username",0,0,0},
 {0,0,0,0,0},
};



void TGLAccess::SetupWindow()
 {
  TMainDialog::SetupWindow();
  SetBitmap(ID_ADD);
  ACCEPT = FALSE;
  cb->autoOpen = FALSE;
  cb->safety = FALSE;

  Data4 Dbf(cb,"ACCESS.PSW");
  Index4 index;
  index.create(Dbf,NULL,accessTag);
  Dbf.close();
  cb->safety = TRUE;
  cb->autoOpen = TRUE;
  access = new TAccess(cb);
  if(!access->isValid())
  {
	ErrorClose(access->GetDbfName());
	return;
  }


  access->top();
  access->SetIndex("u");
  while (!access->eof())
	{
	 SendDlgItemMsg(ID_USER,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)access->fUsername.str());
	 access->skip(1);
	}
 }



 void TGLAccess::Ok (RTMessage)
 {
  CHourGlass wait;
  char szPassword[11];
  DWORD len =  SendDlgItemMsg(ID_USER,WM_GETTEXTLENGTH,NULL,NULL);
  char *szUserID = new char[(int)len + 1];
  SendDlgItemMsg(ID_USER,WM_GETTEXT,sizeof(szUserID),(LPARAM)(LONG)szUserID);
  len =  SendDlgItemMsg(ID_PASSWORD,WM_GETTEXTLENGTH,NULL,NULL);
  SendDlgItemMsg(ID_PASSWORD,WM_GETTEXT,sizeof(szPassword),(LPARAM)(LONG)szPassword);
  if (access->seek(szUserID) == 0)
  {
	access->fPassword.trim();
	access->fPassword.setLen(sizeof(szPassword));
	if (strcmp(szPassword,access->fPassword.str()) == 0)
		{
		 ACCEPT = TRUE;
		 SendDlgItemMsg(ID_ACCESSLEVEL,WM_SETTEXT,NULL,(LPARAM)(LONG)access->fLevel.str());
		 strcpy(szAccessLevel,access->fLevel.str());
		 strcpy(szUser,access->fUsername.str());
		 TAccessl* Accessl = new TAccessl(cb);
		  if(!Accessl->isValid())
			{
			ErrorClose(Accessl->GetDbfName());
			return;
		  }


		 if(Accessl->seek(szAccessLevel) == 0)
		  {
		  strcpy(szMenuCode,Accessl->fMenuCode.str());
		  }
		 else
		  {
			Accessl->top();
			strcpy(szMenuCode,Accessl->fMenuCode.str());
		  }
		 Accessl->close(); 
		}
		else
	  {
		MessageBox(HWindow,GetString(1032),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
		ACCEPT = FALSE;
	  }
  }
  else
  MessageBox(HWindow,GetString(1033),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  if (!ACCEPT)
  {
	SendMessage(HWindow,WM_COMMAND,ID_CLEARING,NULL);
	return;
  }
  else
  {
	access->close();
	ShutDownWindow();
  }
  SendMessage(GetApplication()->MainWindow->HWindow,WM_COMMAND,WM_INITMENUPOPUP,NULL);
 }

void TManilaGL::WMInitMenuPopup(RTMessage)
 {
  if(!bMenuSetup)
  {
	Data4 accessm(cb,"accessm");
	accessm.SetIndex("menu");
	accessm.top();
	Field4 MenuID(accessm,4);
	Field4 MenuItem(accessm,3);
	UINT iMenuID;
	HMENU hMenu = (HMENU)GetMenu(HWindow);
	char* t;
	t = szMenuCode;
	while(*t != '\0' && !accessm.eof())
		 {
		  iMenuID = (UINT) long (Str4ptr(MenuID.str()));
		  if(*t == '1')
			EnableMenuItem(hMenu,iMenuID,MF_GRAYED);
		  else
			EnableMenuItem(hMenu,iMenuID,MF_ENABLED);
		  t++;
		  accessm.skip();
		 }
	bMenuSetup = TRUE;
	accessm.close();
  }
}

 void TGLAccess::Cancel(RTMessage)
 {
  access->close();
  SendMessage(Parent->HWindow,WM_COMMAND,CM_EXIT,NULL);
 }

void TGLAccess::Add (RTMessage)
{
}

void TGLAdministration::GetCalc(RTMessage)
{
  CHourGlass wait;
  Data4 TAccessm(cb,"accessm");
  TAccessm.SetIndex("menu");
  TAccessm.zap();
  TAccessm.pack();

  Field4 fCount(TAccessm,1);
  Field4 fPopup(TAccessm,2);
  Field4 fText(TAccessm,3);
  Field4 fID(TAccessm,4);

  int i = 0;
  char szPopupString[80];
  char szMenuString[80];
  HMENU hMainMenu = (HMENU) GetMenu(GetApplication()->MainWindow->HWindow);
  WORD wTotalLevelMenuCount = GetMenuItemCount(hMainMenu);
	for(WORD wItem = 0; wItem < wTotalLevelMenuCount; wItem++)
	  {
		 HMENU hSubMenu = GetSubMenu(hMainMenu,wItem);
		 GetMenuString(hMainMenu,wItem,szPopupString,80,MF_BYPOSITION);
		 WORD wSubMenuTotal = GetMenuItemCount(hSubMenu);
		 for(WORD wSubItem = 0; wSubItem < wSubMenuTotal; wSubItem++)
			{

			  int wID = GetMenuItemID(hSubMenu,wSubItem);
			  GetMenuString(hSubMenu,wSubItem,szMenuString,80,MF_BYPOSITION);
			  if(wID > 0)
			  {
				i++;
				TAccessm.appendStart();
				fCount.assignLong(i);
				fPopup.assign(szPopupString);
				fText.assign(szMenuString);
				fID.assignLong(wID);
				TAccessm.append();
			  }
			  else if(wID == -1)
			  {
				HMENU h2SubMenu = GetSubMenu(hSubMenu,wSubItem);
				GetMenuString(hSubMenu,wSubItem,szPopupString,80,MF_BYPOSITION);
				if(h2SubMenu != NULL)
				 {
					  WORD w2SubMenuTotal = GetMenuItemCount(h2SubMenu);
					  for(WORD wSubItem = 0; wSubItem < w2SubMenuTotal; wSubItem++)
					  {
						 UINT wID = GetMenuItemID(h2SubMenu,wSubItem);
						 GetMenuString(h2SubMenu,wSubItem,szMenuString,80,MF_BYPOSITION);
						 if(wID > 0)
						 {
						  i++;
						  TAccessm.appendStart();
						  fPopup.assign(szPopupString);
						  fCount.assignLong(i);
						  fText.assign(szMenuString);
						  fID.assignLong(wID);
						  TAccessm.append();
						 }
					 }
				 }
			  }
		 }
  }

  char szText[81];
  for(TAccessm.top();!TAccessm.eof();TAccessm.skip())
  {
	 Strip(szText,fText.ptr());
	 fText.assign(szText);
  }

  TAccessm.close();

}

void TGLAdministration::Reset(RTMessage)
{
  if(MessageBox(HWindow,GetString(8152),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION|MB_YESNO) == IDYES)
	{
	 if(Accessl->seek(GetField(ID_ACCESSLEVEL)) == 0)
	 {
	  Accessl->fMenuCode.assign(szMenuCode);
	  strcpy(szMenuCode,Accessl->fMenuCode.str());
	 }
	 bMenuSetup = FALSE;
	 return;
	}
}

char* TGLAdministration::Strip(char *t,char *s)
 {
  while(*s != '\0')
	{
	 if(*s != '&')
		*t++ = *s++;
	 else
	  s++;
	}
 return t;
 }

