#ifndef __DB_REQST_H
#define __DB_REQST_H
_CLASSDEF (Tapreqst)
class Tapreqst : public Data4
{
 public:
  Field4 vendorno;
  Field4 paymentreq;
  Field4 datepaid;
  Field4 reference;
  Field4 amountpaid;
  Field4 cheque;
  Field4 bankacct;
  Field4 bankdept;
  Field4 cleared;

  Tapreqst(Code4 *cb,LPSTR name=0):Data4(cb,"ap_reqst")
  {
	 if(!isValid())
		return;

	vendorno   .init(data, 1);      // 1
	paymentreq .init(data, 2);      // 2
	reference  .init(data, 3);      // 3
	amountpaid .init(data, 4);      // 4
	datepaid   .init(data, 5);      // 5
	cheque     .init(data, 6);      // 6
	bankacct   .init(data, 7);      // 7
	bankdept   .init(data, 8);      // 8
	cleared    .init(data, 9);      // 9
  };
  double dAmountpaid(){ return double (Str4ptr(amountpaid.str()));};
  BOOL bCleared(void);
  ~Tapreqst()
  {
  }
 };

#endif