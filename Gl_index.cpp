/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Index Databases
*/
#include "std_inc.h"

#include "db_index.h"
#include "lzexpand.h"

#define ID_DESCRIPTION  1349
#define ID_DBF          1351
#define ID_RECNO        1352
#define ID_FCODE        1353
//#define ID_TYPES        1354




static TAG4INFO  gl_tagTag[] =
{
 {"fdbf","UPPER(FDBF)+FSEQNO",0,0,0},
 {0,0,0,0,0},
};


/*** System Reindex ***/
_CLASSDEF(TReindex)
class TReindex : public TMainDialog
{
 public:
 TGlsys*  Indx;
 TTagx*   Tagx;
 TSpread* List;

 TReindex(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  List = new TSpread(this,ID_DISTLIST);
 };
 virtual void SetupWindow();
 virtual void Cancel            (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok                (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void OnTagView         (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void ModifyDatabases   (RTMessage Msg) = [WM_FIRST + WM_LBUTTONDBLCLK];
 virtual void ReIndex           (RTMessage Msg) = [CM_FIRST + IDOK];
 char *SetRecNumber (char *Dbf);
 void Index (char *iName,TAG4INFO *tag,int iLine);
 void Index (char *iName,int iRow);
 void CreateIndex();
 void CreateTag(char *szDbf);
 void SetupListWindow(void);
 void TagView(string strDbf);
};

_CLASSDEF(TCreateReindex)
class TCreateReindex : public TReindex
{
 public:
 TCreateReindex(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TReindex(AParent,AName)
 {
	List = new TSpread(this,ID_DISTLIST);
	SetCaption("Index Maintenance - Create Index File");
 };
  virtual void Ok  (RTMessage Msg) = [ID_FIRST + IDOK]{CreateIndex();}
};

_CLASSDEF(TModifyDbf)
class TModifyDbf : public TMainDialog
{
 public:
	string strDbf;
	TGlsys Indx;
	TTagx  Tags;

	TModifyDbf(PTWindowsObject AParent,int AName): TMainDialog(AParent,AName){}

	inline void SetInfo(string strDbfName)
	{
	  strDbf = strDbfName;
	}
	virtual void Add    (RTMessage Msg) = [ID_FIRST + ID_ADD];
	virtual void Modify (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
	virtual void List   (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
	virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL]{ShutDownWindow();};
	virtual void SetupWindow();
};

void TModifyDbf::SetupWindow()
{
 /*
 TMainDialog::SetupWindow();
 SetField(ID_FCODE,Indx.fDbf);
 SetField(ID_DESCRIPT,Indx.fDescript);
 SetField(ID_TYPES,Indx.fType);
 */
}

void TModifyDbf::Add(RTMessage)
{
}

void TModifyDbf::Modify(RTMessage)
{
 MessageBox(HWindow,"","",MB_OK);
}

void TModifyDbf::List(RTMessage)
{
 MessageBox(HWindow,"","",MB_OK);
}

void TReindex::TagView(string strDbf)
{

	 TModifyDbf* Modify = new TModifyDbf(this,6069);
	 Modify->SetInfo(strDbf);
	 GetApplication()->MakeWindow(Modify);
	 Modify->Show(SW_SHOW);
}



void TReindex::OnTagView(RTMessage)
{
  string dbf = "AP_BTCH";
  TagView(dbf);
}

void TReindex::ModifyDatabases(RTMessage)
{
 //GetApplication()->ExecDialog(new TModifyDbf(this,"MODIFYDBF",Indx,Tagx));
}

void TGlWindow::GLReindex     (RTMessage)
  {
	GetApplication()->ExecDialog(new TReindex(this,1065));
  }

void TGlWindow::GLCreateIndex     (RTMessage)
  {
	GetApplication()->ExecDialog(new TCreateReindex(this,1065));
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
 //OFSTRUCT ofStrSrc;
 //char szFile[12];

 CHourGlass wait;
 cb->autoOpen = FALSE;
 int i = 0;
 char *name = new char [Indx->fDbf.len()+1];
 for(Indx->top();!Indx->eof();Indx->skip())
  {
	strcpy(name,StrTrim(Indx->fDbf.str()));
	//strcpy(szFile,name);
	//strcat(szFile,".dbf");
	//HFILE hFile = LZOpenFile(szFile,&ofStrSrc,OF_SHARE_EXCLUSIVE);
	//LZClose(hFile);
	//if(hFile < 0)
	// {
	//	ErrorClose(name);
	//	return;
	// }
	Index(name,++i);
  }
 delete name;
 // cb->closeAll();
 //cb->exclusive = FALSE;
 ShutDownWindow();
}

void TReindex::Index(char *szName,int iRow)
{

  Tag4info tags(*cb);
  string strName = StrTrim(szName);
  string strTagname,strTag,strFilter;
  int    iUnique;
  string strDbf;

  if(Tagx->seek((LPSTR)strName.c_str()) == 0)
  {
		strDbf = StrTrim(Tagx->fDbf.str());
		while(strDbf == strName)
		{
		  strTagname = StrTrim(Tagx->fTagname.str());
		  strTag     = StrTrim(Tagx->fTag.str());
		  strFilter  = StrTrim(Tagx->fFilter.str());
		  iUnique    = int(Str4ptr(Tagx->fUnique.str()));
		  if(iUnique == 1)
			iUnique = r4unique;
		  else
			iUnique = 0;


		 if(strFilter.length() > 0 || iUnique != 0)
		  tags.add((LPSTR)strTagname.c_str(),(LPSTR)strTag.c_str(),(LPSTR)strFilter.c_str(),iUnique,0);
		 else
		  tags.add((LPSTR)strTagname.c_str(),(LPSTR)strTag.c_str());

		 Tagx->skip();
		 strDbf = StrTrim(Tagx->fDbf.str());
	  }
 }
 Index(StrTrim(szName),tags.tags(),iRow);
}


void TReindex::Index(char *iName,TAG4INFO *tag,int iLine)
{
  cb->autoOpen = FALSE;
  cb->safety = FALSE;
  COLORREF lpBack, lpFore;
  List->GetColor(SS_ALLCOLS,iLine,&lpBack,&lpFore);
  List->SetColor(SS_ALLCOLS,iLine, RGBCOLOR_DARKBLUE, RGBCOLOR_YELLOW);
  List->ShowCell(1,iLine,SS_SHOW_NEAREST);
  Data4 Dbf(cb,iName);
  if(Dbf.isValid())
  {
	Index4 index;
	index.create(Dbf,NULL,tag);
	if(!index.isValid())
	 {
	 string strMsg = GetString(2150);
	 strMsg += iName;
	 MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	 }

  }
  else
  {
	 string strMsg = GetString(2150);
	 strMsg += iName;
	 MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  }
  Dbf.close();
  List->SetColor(SS_ALLCOLS,iLine,lpBack,lpFore);
  cb->autoOpen = TRUE;
  return;
}


char  *TReindex::SetRecNumber(char *Dbf)
{
 long lRecno;
 static char szBuf[10];
 cb->autoOpen = FALSE;
 Data4 dbf(cb,Dbf);
 if(dbf.isValid())
	lRecno = dbf.recCount();
 else
	{
	 ErrorClose(Dbf);
	 return "" ;
	}
 dbf.close();
 ltoa(lRecno,szBuf,10);
 cb->autoOpen = TRUE;
 return szBuf;
}




void TReindex::SetupWindow()
{
	 cb = &((TAcctModule *)GetApplication())->cb;
	 //cb->exclusive = TRUE;
	 TDialog::SetupWindow();
	 List->SetupWindow();
	 cb->autoOpen = FALSE;
	 Index("gl_sys",gl_tagTag,0);
	 Index("gl_tag",gl_tagTag,0);
	 cb->closeAll();
	 cb->autoOpen = TRUE;
	 HFONT   	hFont;
	 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
	 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
	 DeleteObject(hFont);


	 Indx = new TGlsys(cb);
	 if(!Indx->isValid())
		{
		ErrorClose("gl_sys");
		return;
		}


	 Indx->SetIndex("FDBF");
	 Tagx = new TTagx(cb);
	if(!Tagx->isValid())
		{
		ErrorClose("gl_tag");
		return;
		}


	 Tagx->SetIndex("FDBF");

	 SetBitmap(IDOK);
	 SetBitmap(IDCANCEL);
	 SetBitmap(ID_VIEW);
	 SetupListWindow();
}




void TReindex::CreateIndex()
{
 if(MessageBox(GetFocus(),GetString(5125),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
 {
  Tagx->zap();
  Tagx->pack();
  char *name = new char [Indx->fDbf.len()+1];
  Indx->SetIndex("FDBF");
  for(Indx->top();!Indx->eof();Indx->skip())
  {
	strcpy(name,Indx->fDbf.str());
	CreateTag(name);
  }
  delete name;
  SendMessage(HWindow,WM_COMMAND,IDCANCEL,NULL);
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
	 Tagx->fDbf.assign(db.alias());
	 Tagx->fTagname.assign(tag.alias());
	 Tagx->fTag.assign(szName.str());
	 if( tag.filter().isValid() )
		Tagx->fFilter.assign(tag.filter().source());
	 else
		 Tagx->fFilter.assign(" ") ;
	 switch( tag.uniqueError() )
	 {
		 case r4unique: 			Tagx->fUnique.assignLong(r4unique); break ;
		 case r4uniqueContinue: Tagx->fUnique.assignLong(r4uniqueContinue); break ;
		 case e4unique:			Tagx->fUnique.assignLong(e4unique) ; break ;
		 case 0:						Tagx->fUnique.assignLong(0) ;  break ;
	 }
	if( tag.descending() )
	  Tagx->fDescending.assignLong(10);
	else
	  Tagx->fDescending.assignLong(0);
	Tagx->append();
  }
 db.close();
}



void TReindex::Cancel (RTMessage)
{
 delete List;
 cb->closeAll();
 cb->init();
 //cb->exclusive = FALSE;
 ShutDownWindow();
}

void TReindex::SetupListWindow(void)
{
 RECT rc;
 SS_CELLTYPE 	CellType;
 HCURSOR  hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
 ShowCursor (TRUE);
 short d1Width,d3Width;
 GetWindowRect(List->HWindow,&rc);
 List->SetMaxCols(3);
 List->SetColWidth(1,10);
 List->SetColWidth(3,8);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 short ColWidth = (rc.right - rc.left) - d1Width - d3Width- GetSystemMetrics(SM_CXHSCROLL);
 List->SetColWidthInPixels(2,ColWidth);
 List->SetMaxRows(Indx->recCount());

 List->SetData(1,SS_HEADER, GetString(5127));
 List->SetData(2,SS_HEADER, GetString(5119));
 List->SetData(3,SS_HEADER, GetString(5126));

 List->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 List->SetTypeStaticText(&CellType,SS_TEXT_LEFT);
 List->SetCellType(SS_ALLCOLS, SS_ALLROWS, &CellType);

 Indx->top();
 long i = 1;
 string strRecNum,strRecords;
 List->SetBool(SSB_REDRAW,FALSE);
 while(!Indx->eof())
  {

	strRecNum = SetRecNumber(Indx->fDbf.str());
	if(strRecNum.length() == 0)
	{
		MessageBox(HWindow,GetString(6150),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
		cb->closeAll();
		cb->init();
		//cb->exclusive = FALSE;
		ShutDownWindow();
		break;
	}
	List->SetData(1,i,Indx->fDbf.str());
	List->SetData(2,i,Indx->fDescript.str());
	strRecords = "   ";
	strRecords += strRecNum;
	List->SetData(3,i,(LPSTR)strRecords.c_str());
	i++;
	Indx->skip();
  }

 if(i < 13)
	 i = 13;
 List->SetMaxRows(i-1);
 List->SetBool(SSB_REDRAW,TRUE);
 
 ShowCursor (FALSE);
 SetCursor (hCursor) ;

}
