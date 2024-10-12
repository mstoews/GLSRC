#ifndef _DB_BTCHD_H
#define _DB_BTCHD_H
_CLASSDEF(Tglbtchd)
class Tglbtchd :public Data4
{
 public:

  Field4 batchno;
  Field4 mode;
  Field4 journal;
  Field4 period;
  Field4 sourcecode;
  Field4 date;
  Field4 reference;
  Field4 descript;
  Field4 cheque;
  Field4 account;
  Field4 dept;
  Field4 depart;
  Field4 product;
  Field4 drcode;
  Field4 debit;
  Field4 credit;

  /*
  ------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  BATCHNO                    NUMERIC            6           Y
	 2  MODE                       CHARACTER          1           N
	 3  JOURNAL                    NUMERIC            6           Y
	 4  PERIOD                     CHARACTER          2           N
	 5  SOURCECODE                 CHARACTER          6           N
	 6  DATE                       DATE               8           N
	 7  REFERENCE                  CHARACTER         40           N
	 8  DESCRIPT                   CHARACTER         40           N
	 9  CHEQUE                     CHARACTER          7           N
	10  ACCOUNT                    CHARACTER          7           Y
	11  DEPT                       CHARACTER          4           Y
	12  MEDREP                     CHARACTER          5           N
	13  DRCODE                     CHARACTER          5           N
	14  PROD                       CHARACTER          7           N
	15  DEBIT                      NUMERIC           16    2      N
	16  CREDIT                     NUMERIC           16    2      N
------------------------------------------------------------------
** Total **                                        177

  */
  void init()
  {
	  batchno    .init(data,1);
	  mode       .init(data,2);
	  journal    .init(data,3);
	  period     .init(data,4);
	  sourcecode .init(data,5);
	  date       .init(data,6);
	  reference  .init(data,7);
	  descript   .init(data,8);
	  cheque     .init(data,9);
	  account    .init(data,10);
	  dept       .init(data,11);
	  depart     .init(data,12);
	  drcode     .init(data,13);
	  product    .init(data,14);
	  debit      .init(data,15);
	  credit     .init(data,16);
  }

  Tglbtchd(){};
  Tglbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"gl_btchd")
  {
	if(!isValid())
	 return;
	init();
  };
 ~Tglbtchd(){
	//close();
 };
  void   SetIndexBatchno(){SetIndex("batchno");}
  long   Batchno() { return long(Str4ptr(batchno.str()));};
  double Debit()   { return double(Str4ptr(debit.str()));};
  double Credit()  { return double(Str4ptr(credit.str ()));};
  int    iJournal(){ return int   (Str4ptr(journal.str()));};
  int    iPeriod (){ return int   (Str4ptr(period.str()));};
  LPSTR GetDbfName(){ return "gl_btchd";}
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

};

_CLASSDEF(TGLBatchDetailList)
class TGLBatchDetailList : public TDialog
{
 public:
 char szQuery[255];
 PTSpread List;
 PTglbtchd  glbtchd;
 TGLBatchDetailList(PTWindowsObject AParent, LPSTR AName,LPSTR AQuery,PTglbtchd t):
  TDialog (AParent, AName)
  {
  strcpy(szQuery,AQuery);
  glbtchd = t;
  List = new TSpread(this,ID_DISTLIST);
  };
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void VirtualData (RTMessage Msg) = [WM_FIRST + SSM_VQUERYDATA];
 virtual void SetupWindow();
 virtual void SetField(int ID, Field4 *field);
};

#endif