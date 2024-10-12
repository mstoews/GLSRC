#ifndef __AR_DIST_H
#define __AR_DIST_H

_CLASSDEF (Tardist)
class Tardist : public Data4
{
 public:
  Field4  account;
  Field4  dept;
  Field4  mode;
  Field4  apaccount;
  Field4  apdepart;
  Field4  period;
  Field4  date;
  Field4  duedate;
  Field4  batchno;
  Field4  invoiceno;
  Field4  paymentctl;
  Field4  vendorno;
  Field4  chequeno;
  Field4  reference;
  Field4  descript;
  Field4  amount;
  Field4  rebateamt;
  Field4  amountpaid;
  Field4  paymentreq;
  Field4  datepaid;
  Field4  vatamt;
  Field4  adjustment;
  Field4  rebate;
  Field4  remainder;

  void init()
  {
	  account 	 .init(data, 1 );
	  dept 		 .init(data, 2 );
	  mode 		 .init(data, 3 );
	  apaccount  .init(data, 4 );
	  apdepart   .init(data, 5 );
	  period     .init(data, 6 );
	  date 		 .init(data, 7 );
	  duedate    .init(data, 8 );
	  batchno    .init(data, 9 );
	  invoiceno  .init(data, 10 );
	  paymentctl .init(data, 11 );
	  vendorno   .init(data, 12 );
	  chequeno   .init(data, 13 );
	  reference  .init(data, 14 );
	  descript   .init(data, 15 );
	  amount     .init(data, 16 );
	  rebateamt  .init(data, 17 );
	  amountpaid .init(data, 18 );
	  paymentreq .init(data, 19 );
	  datepaid   .init(data, 20 );
	  vatamt     .init(data, 21 );
	  adjustment .init(data, 22 );
	  rebate     .init(data, 23 );
	  remainder  .init(data, 24 );

  }
  Tardist(){};
  Tardist(Code4 *cb,LPSTR name=0):Data4(cb,"ar_dist")
  {
	if(!isValid())
	 return;
    init();
  };
  ~Tardist()
  {
	 //close();
  }
  LPSTR GetDbfName(){ return "AR_DIST";}
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

  double dAmount(){ return double (Str4ptr(amount.str()));};
  double dRemainder(){ return double (Str4ptr(remainder.str()));};
  double dAmountpaid(){ return double (Str4ptr(amountpaid.str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt.str()));};
  double dAdjustment(){ return double (Str4ptr(adjustment.str()));};
  double dVatamt    (){ return double (Str4ptr(vatamt.str()));};
  long   lRequest()   { return long   (Str4ptr(paymentreq.str()));};
  inline BOOL Open()        {if(strcmp(paymentctl.str(),"O") == 0)return TRUE;else return FALSE;};
  inline BOOL Paid()        {if(strcmp(paymentctl.str(),"P") == 0)return TRUE;else return FALSE;};
  inline BOOL Remainder()   {if(strcmp(paymentctl.str(),"R") == 0)return TRUE;else return FALSE;};
  inline BOOL Held()        {if(strcmp(paymentctl.str(),"H") == 0)return TRUE;else return FALSE;};
  inline BOOL Forced()      {if(strcmp(paymentctl.str(),"F") == 0)return TRUE;else return FALSE;};
  inline BOOL Discount()    {if(strcmp(paymentctl.str(),"D") == 0)return TRUE;else return FALSE;};
  inline BOOL Withholding() {if(strcmp(paymentctl.str(),"W") == 0)return TRUE;else return FALSE;};
  inline void SetIndexVendorno(){SetIndex("vendorno");}

 };

#endif 
