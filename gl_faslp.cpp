

#include "std_inc.h"

#define LINENO 10
#define ID_YEARS_LIFE 6033
#define ID_DECLINING  7024
#define ID_ACCUMULATED_DEP 6040
#define ID_NBV	6041


extern HINSTANCE hInst;


/*
Structure for table   D:\GL\EIWA_DBF\GL_FASLP.DBF
Table type            DBASE
Number of records     0
Last update           02/18/97
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  PERIOD                     CHARACTER          2           N
	 2  BATCHNO                    NUMERIC            7           N
	 3  DESC                       CHARACTER         30           N
	 4  USER                       CHARACTER         10           N
	 5  UPDATE                     CHARACTER         10           N
	 6  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                         86
*/

_CLASSDEF(Tgl_faslp)
class Tgl_faslp : public Data4
{
 public:
	Field4  period;
	Field4  batchno;
	Field4  desc;
	Field4  user;
	Field4  update;
	Field4  time;

  inline void init()
  {
	period.init(data,1);
	batchno.init(data,2);
	desc.init(data,3);
	user.init(data,4);
	update.init(data,5);
	time.init(data,6);
  }
  Tgl_faslp(){};
  Tgl_faslp(Code4 *cb,LPSTR name=0): Data4 (cb,"gl_faslp")
  {
	if(!isValid())
	 return;
	else
	 init();
  };

  LPSTR GetDbfName(){ return "gl_faslp";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if( isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
  }
  long iPeriod()   {return long(Str4ptr(period.str()));};
  long lBatchno()  {return long (Str4ptr(batchno.str()));};
};






_CLASSDEF(TGLCreateDeprSlip)
class TGLCreateDeprSlip : public TMainDialog
{
 char *szPd;
 Tglfa* glfa;
 Tfatype* fatype;
 Tglbtch* glbtch;
 Tglbtchd* glbtchd;
 TSpread*  AssetList;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;
 HWND 		SHWindow;
 public:
 TGLCreateDeprSlip(PTWindowsObject AWindow,int ATitle,char *szPeriod):TMainDialog(AWindow,ATitle)
 {
  AssetList = new TSpread(this,ID_DISTLIST);
  szPd = new char [20];
  strcpy(szPd,szPeriod);
 };
 ~TGLCreateDeprSlip()
 {
	delete AssetList;
	delete [] szPd;
 }
 virtual void SetupWindow();
 virtual void Ok      (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel  (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Confirm (RTMessage Msg) = [ID_FIRST + ID_CONFIRM];
 void CreateDetailSlip(unsigned int iBatchno,
							  unsigned int iPeriod,
							  char*  szDesc,
							  char*  szAccount,
							  char*  szDept,
							  double Dr,
							  double Cr);
 void CreateBatchHeader(unsigned int iBatchno,double dTotal,unsigned int iPeriod);
 void UpdateFA();
 double dAccum(void);
};

double TGLCreateDeprSlip::dAccum(void)
{
	double dCurrentPeriodDeprec = 0;
	double dSalvage = atof(glfa->salvage.str());
	double dCost    = atof(glfa->cost.str());
	double dDepr    = atof(glfa->curr_dep.str());
	int dLife  = atoi(glfa->lifetime.str());
	int dLeft  = atoi(glfa->lifeleft.str());
	if(dLife-dLeft > 0)
	 {
		if(dLife > 0)
		 dCurrentPeriodDeprec = (dCost-dSalvage)/dLife;
	 }
	return dCurrentPeriodDeprec;

}



void TGLCreateDeprSlip::CreateBatchHeader(unsigned int iBatchno,double dTotal,unsigned int iPeriod)
{
  Date4 dteDate;
  dteDate.today();
  dteDate.format("MM/DD/CCYY");
  glbtch->appendStart();
  glbtch->batchno.assignLong(iBatchno);
  glbtch->mode.assign("1");
  glbtch->date.assign(dteDate.str());
  glbtch->period.assignDouble(iPeriod);
  glbtch->descript.assign(StrTrim(GetString(8118)));
  glbtch->debit.assignDouble(dTotal);
  glbtch->credit.assignDouble(dTotal);
  glbtch->entryno.assignLong(0);
  glbtch->cheque.assign("");
  glbtch->UpdateUserTime(szUser);
  glbtch->append();

}


void TGLCreateDeprSlip::CreateDetailSlip(unsigned int iBatchno,
													  unsigned int iPeriod,
													  char*  szDesc,
													  char*  szAccount,
													  char*  szDept,
													  double Dr,
													  double Cr)
{
  Date4 dDate;
  dDate.today();
  dDate.format("MM/DD/CCYY");

  glbtchd->appendStart();
  glbtchd->batchno.assignLong(iBatchno);
  glbtchd->journal.assignLong(0);
  glbtchd->sourcecode.assign("");
  glbtchd->date.assign(dDate.str());
  glbtchd->period.assignDouble(iPeriod);
  glbtchd->reference.assign(szDesc);
  glbtchd->descript.assign(szDesc);// Monthly depreciation for : period
  glbtchd->account.assign(szAccount);
  glbtchd->dept.assign(szDept);
  glbtchd->cheque.assign("");
  glbtchd->debit.assignDouble(Dr);
  glbtchd->credit.assignDouble(Cr);
  glbtchd->append();
}

void TGLCreateDeprSlip::UpdateFA()
{
  for(glfa->top();!glfa->eof();glfa->skip())
	{
	 if((glfa->dLife() - glfa->dLeft()) > 0)
	 {
		double dLife = glfa->dLeft();
		glfa->mode.assign("0");
		double dDeprec = dAccum();
		double dAccumDep = glfa->dAccum_Dep();
		double dCost     = glfa->dCost();
		double dSalvage  = glfa->dSalvage();
		double dNBV =    dCost - dAccumDep;
		// New Accumulated Depreciation
		if(dNBV > dSalvage)
		{
		 dAccumDep  += dDeprec;
		 dNBV = dCost - dAccumDep;
		 if(dNBV <= dSalvage)
		  {
			dAccumDep = dCost - dSalvage;
			dLife  = glfa->dLife();
		  }
		 else
			dLife++;
		}
		 glfa->lifeleft.assignDouble(dLife);
		 glfa->accum_dep.assignDouble(dAccumDep);
		 glfa->UpdateUserTime(szUser);

	 }
  }
}


void TGLCreateDeprSlip::Confirm(RTMessage Msg)
{
 glbtch->bottom();
 int iBatchno = (int) glbtch->Batchno();
 iBatchno++;
 unsigned int iPeriod = GetCurrentPeriodn();
 char szAccount[8];
 char szDept[5];
 char szDesc[41];
 char szDr[17];
 char szCr[17];
 double dTotal = 0;
 SS_COORD iCol,iLastRowUsed;
 AssetList->GetLastValidCell(&iCol,&iLastRowUsed);
 int iRow = 1;
 AssetList->GetData(1,1,szAccount);
 if(strlen(StrTrim(szAccount)) == 0)
  {
	MessageBox(HWindow,GetString(8165),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
  }

 if(MessageBox(HWindow,GetString(8110),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
	  CHourGlass wait;
	  UpdateFA();
	  while(iRow < iLastRowUsed)
	  {
		AssetList->GetData(1,iRow,szAccount);
		AssetList->GetData(2,iRow,szDept);
		AssetList->GetData(3,iRow,szDesc);
		AssetList->GetData(4,iRow,szDr);
		AssetList->GetData(5,iRow,szCr);
		double Dr = atof(szDr);
		double Cr = atof(szCr);
		if(strlen(StrTrim(szAccount)) > 0)
		{
		 dTotal += Dr;
		 CreateDetailSlip(iBatchno,iPeriod,StrTrim(szDesc),StrTrim(szAccount),StrTrim(szDept),Dr,Cr);
		}
		iRow++;
	  }
	  CreateBatchHeader(iBatchno,dTotal,iPeriod);
  }
  Cancel(Msg);
}



void TGLCreateDeprSlip::Ok(RTMessage)
{
 CHourGlass wait;
 glbtch->top();
 glfa->top();
 unsigned int iEntry = 0;
 fatype->SetIndex("fatype");

 unsigned long iLastRowUsed = 0;
 unsigned long i;

 if(MessageBox(HWindow,GetString(8163),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
	 CHourGlass wait;
	 while (!glfa->eof())
		{
			if((glfa->dLife() - glfa->dLeft() > 0) && (glfa->dCost() - glfa->dAccum_Dep() > glfa->dSalvage()))
			{
			iEntry++;
			if(!glfa->Journal())
			{
			  i = 1;
			  if(fatype->seek(glfa->fatype.str()) == 0)
			  {
				 //char   szAccount[7];
				 //char   szDept[4];
				 //char   szAmount[12];
				 string   szAccount;
				 string   szDept;
				 string   szAmount;

				 BOOL bDebit = FALSE;
				 BOOL bCredit = FALSE;
					while(i < (iLastRowUsed+1))
					{
						AssetList->GetData(1,i,szAccount);
						AssetList->GetData(2,i,szDept);

						string strAccount((LPSTR)szAccount.c_str());
						strAccount += (LPSTR)szDept.c_str();

						string strExpense(StrTrim(fatype->expen_acct.str()));
						strExpense += StrTrim(fatype->expen_dept.str());
						long row = i;
							if(strExpense == strAccount)
							{
								//AssetList->GetData(4,i,(LPSTR)szAmount.c_str());
								AssetList->GetData(4,i,szAmount);
								double dAmount = atof((LPSTR)szAmount.c_str());
								dAmount += glfa->dCurr_dep();
								gcvt(dAmount,16,(LPSTR)szAccount.c_str());
								AssetList->SetData(4,row,StrTrim((LPSTR)szAccount.c_str()));
								bDebit = TRUE;
								break;
							}

							i++;
					 }
					 i = 1;
					 while(i < (iLastRowUsed+1))
					 {
						 AssetList->GetData(1,i,szAccount);
						 AssetList->GetData(2,i,szDept);

						 string strAccount((LPSTR)szAccount.c_str());
						 strAccount += (LPSTR)szDept.c_str();

						 string strAccum(StrTrim(fatype->accum_acct.str()));
						 strAccum += StrTrim(fatype->accum_dept.str());
						 long row = i;
							 if(strAccum == strAccount)
							 {
								AssetList->GetData(5,i,szAmount);
								double dAmount = atof((LPSTR)szAmount.c_str());
								dAmount += glfa->dCurr_dep();
								gcvt(dAmount,16,(LPSTR)szAmount.c_str());
								AssetList->SetData(5,row,StrTrim((LPSTR)szAmount.c_str()));
								bCredit = TRUE;
								break;
							 }
						  i++;
					 }
				  if(!bDebit || !bCredit)
				  {
					iLastRowUsed++;
						{
						if(!bDebit)
							{
							 AssetList->SetData(1,iLastRowUsed,StrTrim(fatype->expen_acct.str()));
							 AssetList->SetData(2,iLastRowUsed,StrTrim(fatype->expen_dept.str()));
							 AssetList->SetData(3,iLastRowUsed,fatype->expen_desc.str());
							 AssetList->SetData(4,iLastRowUsed,glfa->curr_dep.str());
							 iLastRowUsed++;
							}
						if(!bCredit)
							{
							 AssetList->SetData(1,iLastRowUsed,StrTrim(fatype->accum_acct.str()));
							 AssetList->SetData(2,iLastRowUsed,StrTrim(fatype->accum_dept.str()));
							 AssetList->SetData(3,iLastRowUsed,fatype->accum_desc.str());
							 AssetList->SetData(5,iLastRowUsed,glfa->curr_dep.str());
							}
						}
				  }
				}
			}
		 }
	  glfa->skip();
	 }
  }
		char szColumn[2];
		char szFormula[10];
		strcpy(szFormula,"D1:D");
		itoa((int)iLastRowUsed,szColumn,10);
		strcat(szFormula,StrTrim(szColumn));
		strcpy(szFormula,"E1:E");
		strcat(szFormula,StrTrim(szColumn));
		iLastRowUsed++;
		if(iLastRowUsed > 1)
		{
		 AssetList->SetData(3,iLastRowUsed,"Total");
		 AssetList->SetFormula(4,iLastRowUsed,szFormula,TRUE);
		 AssetList->SetFormula(5,iLastRowUsed,szFormula,TRUE);
		 AssetList->SetColorRange(4,iLastRowUsed,5,iLastRowUsed,RGB(192,220,192),RGBCOLOR_BLUE);
		}
		else
		 MessageBox(HWindow,GetString(8196),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);

		 if(iLastRowUsed < 7)
			AssetList->SetMaxRows(7);
		 else
			AssetList->SetMaxRows(iLastRowUsed++);

}


void TGLCreateDeprSlip::SetupWindow()
{
 SetBitmap(ID_CONFIRM);
 TMainDialog::SetupWindow();
 SetField(4001,szPd);
 glfa    = new Tglfa(cb);
 if(!glfa->isValid())
  {
	ErrorClose(glfa->GetDbfName());
	return;
  }

 fatype  = new Tfatype(cb);
 if(!fatype->isValid())
  {
	ErrorClose(fatype->GetDbfName());
	return;
  }

 glbtch  = new Tglbtch(cb);
 if(!glbtch->isValid())
  {
	ErrorClose(glbtch->GetDbfName());
	return;
  }

 glbtchd = new Tglbtchd(cb);
 if(!glbtchd->isValid())
  {
	ErrorClose(glbtchd->GetDbfName());
	return;
  }

 RECT rc;
 AssetList->GetClientRect(&rc);
 short d1Width,d2Width,d4Width,d5Width;
 AssetList->SetMaxCols(5);
 AssetList->SetColWidth(1,8);
 AssetList->SetColWidth(2,5);
 AssetList->SetColWidth(4,10);
 AssetList->SetColWidth(5,10);


 AssetList->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 AssetList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 AssetList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 AssetList->GetColWidthInPixels(5,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left) - d1Width - d2Width - d4Width - d5Width;

 AssetList->SetColWidthInPixels(3,ColWidth);

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 AssetList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 AssetList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 AssetList->SetBool(SSB_PROTECT,TRUE);
 AssetList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 AssetList->SetBool(SSB_NOBEEP,TRUE);
 AssetList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 AssetList->SetBool(SSB_VERTSCROLLBAR,TRUE);
 AssetList->SetMaxRows(1000);
 AssetList->SetData(1,SS_HEADER, GetString(8159));
 AssetList->SetData(2,SS_HEADER, GetString(6054));
 AssetList->SetData(3,SS_HEADER, GetString(6055));
 AssetList->SetData(4,SS_HEADER, GetString(6069));
 AssetList->SetData(5,SS_HEADER, GetString(6070));
 AssetList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 AssetList->SetCellType(1, SS_ALLROWS, &CellType);
 AssetList->SetCellType(2, SS_ALLROWS, &CellType);
 AssetList->SetCellType(3, SS_ALLROWS, &CellType);
 AssetList->SetTypeFloat(&CellType,FS_SEPARATOR,14,2,0,atof(GetString(6107)));
 AssetList->SetCellType(4, SS_ALLROWS, &CellType);
 AssetList->SetCellType(5, SS_ALLROWS, &CellType);

}

void TGLCreateDeprSlip::Cancel(RTMessage)
{
 cb->closeAll();
 cb->initUndo();
 cb->init();
 ShutDownWindow();
 return;
}


void TManilaGL::GLFixedSlip    (RTMessage){GetApplication()->ExecDialog(new TGLCreateDeprSlip(this,4003,GetCurrentPeriod()));}
