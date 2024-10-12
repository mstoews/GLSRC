#ifndef __DB_APDST_H
#define __DB_APDST_H
_CLASSDEF (Tapdist)
class Tapdist : public Data4
{
 public:
  Field4 *account;
  Field4 *dept;
  Field4 *apaccount;
  Field4 *apdepart;
  Field4 *period;
  Field4 *date;
  Field4 *duedate;
  Field4 *batchno;
  Field4 *invoiceno;
  Field4 *paymentctl;
  Field4 *vendorno;
  Field4 *chequeno;
  Field4 *reference;
  Field4 *descript;
  Field4 *amount;
  Field4 *paymentreq;
  Field4 *datepaid;
  Field4 *rebateamt;
  Field4 *amountpaid;
  Field4 *adjustment;
  Field4 *vatamt;
  Field4 *rebate;

  Tapdist(Code4 *cb,LPSTR name=0):Data4(cb,"ap_dist")
  {
   SetIndex("account");
   top();
   account    = (Field4 *) new Field4(*(Data4 *) this, 1); 
   dept       = (Field4 *) new Field4(*(Data4 *) this, 2); 
   apaccount  = (Field4 *) new Field4(*(Data4 *) this, 3); 
   apdepart   = (Field4 *) new Field4(*(Data4 *) this, 4); 
   period     = (Field4 *) new Field4(*(Data4 *) this, 5); 
   date       = (Field4 *) new Field4(*(Data4 *) this, 6); 
   duedate    = (Field4 *) new Field4(*(Data4 *) this, 7); 
   batchno    = (Field4 *) new Field4(*(Data4 *) this, 8); 
   invoiceno  = (Field4 *) new Field4(*(Data4 *) this, 9); 
   paymentctl = (Field4 *) new Field4(*(Data4 *) this, 10);
   vendorno   = (Field4 *) new Field4(*(Data4 *) this, 11); 
   chequeno   = (Field4 *) new Field4(*(Data4 *) this, 12); 
   reference  = (Field4 *) new Field4(*(Data4 *) this, 13); 
   descript   = (Field4 *) new Field4(*(Data4 *) this, 14); 
   amount     = (Field4 *) new Field4(*(Data4 *) this, 15); 
   rebateamt  = (Field4 *) new Field4(*(Data4 *) this, 16); 
   amountpaid = (Field4 *) new Field4(*(Data4 *) this, 17); 
   paymentreq = (Field4 *) new Field4(*(Data4 *) this, 18); 
   datepaid   = (Field4 *) new Field4(*(Data4 *) this, 19); 
   vatamt     = (Field4 *) new Field4(*(Data4 *) this, 20); 
   adjustment = (Field4 *) new Field4(*(Data4 *) this, 21);
   rebate     = (Field4 *) new Field4(*(Data4 *) this, 22);

  };
  ~Tapdist(){close();};
  double dAmount(){ return double (Str4ptr(amount->str()));};
  double dAmountpaid(){ return double (Str4ptr(amountpaid->str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt->str()));};
  double dAdjustment(){ return double (Str4ptr(adjustment->str()));};
  long   lRequest()   { return long   (Str4ptr(paymentreq->str()));};
  long   lChequeno()   { return long   (Str4ptr(chequeno->str()));};
  BOOL Open()     {if(strcmp(paymentctl->str(),"O") == 0)return TRUE;else return FALSE;};
  BOOL Paid()     {if(strcmp(paymentctl->str(),"P") == 0)return TRUE;else return FALSE;};
  BOOL Requested(){if(strcmp(paymentctl->str(),"R") == 0)return TRUE;else return FALSE;};
  BOOL Held()     {if(strcmp(paymentctl->str(),"H") == 0)return TRUE;else return FALSE;};
  BOOL Forced()   {if(strcmp(paymentctl->str(),"F") == 0)return TRUE;else return FALSE;};
 };
#endif