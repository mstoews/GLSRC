#ifndef __DB_ARCLR_H
#define __DB_ARCLR_H

_CLASSDEF(Tarclear)
class Tarclear : public Data4
{
  public:
  Field4 account;
  Field4 dept;
  Field4 mode;
  Field4 type;
  Field4 period;
  Field4 date;
  Field4 batchno;
  Field4 reference;
  Field4 cheque;
  Field4 descript;
  Field4 debit;
  Field4 credit;
  inline void init()
  {
	account.init(data, 1 );
	dept.init(data, 2 );
	period.init(data, 3 );
	mode.init(data, 4 );
	date.init(data, 5 );
	type.init(data, 6 );
	batchno.init(data, 7 );
	reference.init(data, 8 );
	cheque.init(data, 9 );
	descript.init(data, 10 );
	debit.init(data, 11 );
	credit.init(data, 12 );
  }
  Tag4 deftag;
  Tarclear(){};
  Tarclear(Code4 *cb,LPSTR name = 0):Data4(cb,"ar_clear")
  {
	if(!isValid())
	 return;
	init();
	deftag.initDefault(this->data);
	select(deftag);
  }
 ~Tarclear()
 {
	//close();
 }

  LPSTR GetDbfName(){ return "ar_clear";}
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
#endif