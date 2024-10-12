#ifndef _DB_BTCH_H
#define _DB_BTCH_H
 _CLASSDEF(Tglbtch)
 class Tglbtch : public Data4
 {
  public:
  Field4 batchno;
  Field4 mode;
  Field4 descript;
  Field4 type;
  Field4 period;
  //Field4 fiscalyr;
  Field4 date;
  Field4 debit;
  Field4 credit;
  Field4 entryno;
  Field4 cheque;
  //Field4 fiscalyr;

 Tglbtch(){};
 Tglbtch(Code4 *cb,LPSTR name=0):Data4(cb,"gl_btch")
	{
		if(!isValid())
	 return;
	 init();
/*
Structure for table   C:\GL\SAMPLE\GL_BTCH.DBF
Table type            DBASE
Number of records     2759
Last update           10/12/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  BATCHNO                    NUMERIC            6           Y
	 2  MODE                       CHARACTER          1           N
	 3  REFERENCE                  CHARACTER         40           Y
	 4  TYPE                       CHARACTER          3           N
	 5  PERIOD                     NUMERIC            2           N
	 6  DATE                       DATE               8           N
	 7  DEBIT                      NUMERIC           14    2      N
	 8  CREDIT                     NUMERIC           14    2      N
	 9  ENTRYNO                    NUMERIC            3           N
	10  CHEQUE                     CHARACTER          7           N
	11  UPDATE                     DATE               8           N
	12  USER                       CHARACTER         10           N
	13  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        143
*/
	//fiscalyr  .init(data,11);
	};

	void init()
	{
	 batchno   .init(data,1);
	 mode      .init(data,2);
	 descript  .init(data,3);
	 type      .init(data,4);
	 period    .init(data,5);
	 date      .init(data,6);
	 debit     .init(data,7);
	 credit    .init(data,8);
	 entryno   .init(data,9);
	 cheque    .init(data,10);
	}

	~Tglbtch()
	{
	  //close();
	}

  LPSTR GetDbfName(){ return "GL_BTCH";}
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

	BOOL Open();
	BOOL Closed();
	BOOL Adjustment();
	int   Batchno() { return int   (Str4ptr( batchno.str ()));};
	double Debit()  { return double(Str4ptr( debit.str   ()));};
	double Credit() { return double(Str4ptr( credit.str  ()));};
	long Entryno()  { return long  (Str4ptr( entryno.str ()));};
	int iPeriod()   { return int   (Str4ptr( period.str  ()));};
	inline void   SetIndexOpen(){SetIndex("open");}
	inline void   SetIndexBatchno(){SetIndex("batchno");}
 };

_CLASSDEF(TGLBatchList)
class TGLBatchList : public TDialog
{
 public:
 PTSpread List;
 PTListArray ListArray;
 PTglbtch  glbtch;
 TGLBatchList(PTWindowsObject AParent, LPSTR AName,PTglbtch glb):
  TDialog (AParent, AName) {glbtch=glb; ListArray = new TListArray(1000, 0, 50);List = new TSpread(this,ID_DISTLIST);};
 ~TGLBatchList();
 virtual void WMInitDialog  (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG]{};
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();

};
#endif
