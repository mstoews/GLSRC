#ifndef __DB_MEDRP_H
#define __DB_MEDRP_H
_CLASSDEF (Tglmedreps)
class Tglmedreps : public Data4
{
 public:
  Field4 repcode;
  Field4 rpshort;
  Field4 rpname;
  Field4 address1;
  Field4 address2;
  Field4 address3;
  Field4 postal;
  Field4 phone;
  Field4 territory;
  Field4 fax;
  Field4 descript;
  Field4 contact;
  Field4 type;
  Field4 ref;
  Field4 status;
  Field4 terms;
  Field4 balance;
  Field4 creditlmt;

  Tglmedreps(){}
  inline void init()
  {
	  SetIndex("repcode");
	  repcode 	.init(data, 1 );
	  rpshort 	.init(data, 2 );
	  rpname 	.init(data, 3 );
	  address1	.init(data, 4 );
	  address2 	.init(data, 5 );
	  address3	.init(data, 6 );
	  postal 	.init(data, 7 );
	  phone		.init(data, 8 );
	  territory .init(data, 9 );
	  fax 		.init(data, 10 );
	  descript 	.init(data, 11 );
	  contact 	.init(data, 12 );
	  type  		.init(data, 13 );
	  ref 		.init(data, 14 );
	  status 	.init(data, 15 );
	  terms 		.init(data, 16 );
	  balance 	.init(data, 17 );
	  creditlmt .init(data, 18 );
  }
  Tglmedreps(Code4 *cb,LPSTR name=0):Data4(cb,"gl_medrp")
	{
	if(!isValid())
	 return;
	 init();
	};

  LPSTR GetDbfName(){ return "gl_medrp";}
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

	~Tglmedreps()
	{
	  //close();
	};
 };
#endif