#ifndef __DB_REQST_H
#define __DB_REQST_H
_CLASSDEF (Tapreqst)
class Tapreqst : public Data4
{
 public:
  Field4 *vendorno;
  Field4 *paymentreq;  
  Field4 *datepaid;
  Field4 *reference;
  Field4 *amountpaid; 
  Field4 *cheque;
  Field4 *bankacct;
  Field4 *bankdept;
  Field4 *cleared;

  Tapreqst(Code4 *cb,LPSTR name=0):Data4(cb,"ap_reqst")
  {   
   top();
   vendorno   = (Field4 *) new Field4(*(Data4 *) this, "vendorno");    // 1
   paymentreq = (Field4 *) new Field4(*(Data4 *) this, "paymentreq");    // 1
   reference  = (Field4 *) new Field4(*(Data4 *) this, "reference");     // 3
   amountpaid = (Field4 *) new Field4(*(Data4 *) this, "amountpaid");    // 4   
   datepaid   = (Field4 *) new Field4(*(Data4 *) this, "datepaid");      // 5
   cheque     = (Field4 *) new Field4(*(Data4 *) this, "cheque");        // 6
   bankacct   = (Field4 *) new Field4(*(Data4 *) this, "bankacct");      // 7
   bankdept   = (Field4 *) new Field4(*(Data4 *) this, "bankdept");      // 8
   cleared    = (Field4 *) new Field4(*(Data4 *) this, "cleared");      // 8

  };
  ~Tapreqst(){close();};
  double dAmountpaid(){ return double (Str4ptr(amountpaid->str()));};
  BOOL bCleared(void);
 };

#endif