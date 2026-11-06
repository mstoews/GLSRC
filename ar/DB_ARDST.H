_CLASSDEF (Tapdist)
class Tapdist : public Data4
{
 public:
  Field4 *account;
  Field4 *dept;
  Field4 *mode;
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
  Field4 *vatamt;
  Field4 *adjustment;
  Field4 *rebate;
  Field4 *remainder;

  Tapdist(Code4 *cb,LPSTR name=0):Data4(cb,"ar_dist")
  {
   SetIndex("account");
   top();
   account    = (Field4 *) new Field4(*(Data4 *) this, "account");       // 1
   dept       = (Field4 *) new Field4(*(Data4 *) this, "dept");          // 2
   mode       = (Field4 *) new Field4(*(Data4 *) this, "mode");          // 2
   apaccount  = (Field4 *) new Field4(*(Data4 *) this, "apaccount");     // 3
   apdepart   = (Field4 *) new Field4(*(Data4 *) this, "apdepart");      // 4
   period     = (Field4 *) new Field4(*(Data4 *) this, "period");        // 5
   date       = (Field4 *) new Field4(*(Data4 *) this, "date");          // 6
   duedate    = (Field4 *) new Field4(*(Data4 *) this, "duedate");       // 7
   batchno    = (Field4 *) new Field4(*(Data4 *) this, "batchno");       // 8
   invoiceno  = (Field4 *) new Field4(*(Data4 *) this, "invoiceno");     // 9
   paymentctl = (Field4 *) new Field4(*(Data4 *) this, "paymentctl");    // 10
   vendorno   = (Field4 *) new Field4(*(Data4 *) this, "vendorno");      // 11
   chequeno   = (Field4 *) new Field4(*(Data4 *) this, "chequeno");      // 12
   reference  = (Field4 *) new Field4(*(Data4 *) this, "reference");     // 13
   descript   = (Field4 *) new Field4(*(Data4 *) this, "descript");      // 14
   amount     = (Field4 *) new Field4(*(Data4 *) this, "amount");        // 15
   paymentreq = (Field4 *) new Field4(*(Data4 *) this, "paymentreq");    // 16
   datepaid   = (Field4 *) new Field4(*(Data4 *) this, "datepaid");      // 17
   amountpaid = (Field4 *) new Field4(*(Data4 *) this, "amountpaid");    // 18
   rebateamt  = (Field4 *) new Field4(*(Data4 *) this, "rebateamt");     // 19
   vatamt     = (Field4 *) new Field4(*(Data4 *) this, "vatamt");     // 19
   adjustment = (Field4 *) new Field4(*(Data4 *) this, "adjustment");    // 20
   rebate     = (Field4 *) new Field4(*(Data4 *) this, "rebate");        // 21
   remainder  = new Field4 (*(Data4 *)this,24);

  };
  ~Tapdist(){close();};
  double dAmount(){ return double (Str4ptr(amount->str()));};
  double dRemainder(){ return double (Str4ptr(remainder->str()));};
  double dAmountpaid(){ return double (Str4ptr(amountpaid->str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt->str()));};
  double dAdjustment(){ return double (Str4ptr(adjustment->str()));};
  double dVatamt    (){ return double (Str4ptr(vatamt->str()));};
  long   lRequest()   { return long   (Str4ptr(paymentreq->str()));};
  BOOL Open()     {if(strcmp(paymentctl->str(),"O") == 0)return TRUE;else return FALSE;};
  BOOL Paid()     {if(strcmp(paymentctl->str(),"P") == 0)return TRUE;else return FALSE;};
  BOOL Remainder(){if(strcmp(paymentctl->str(),"R") == 0)return TRUE;else return FALSE;};
  BOOL Held()     {if(strcmp(paymentctl->str(),"H") == 0)return TRUE;else return FALSE;};
  BOOL Forced()   {if(strcmp(paymentctl->str(),"F") == 0)return TRUE;else return FALSE;};
  BOOL Discount() {if(strcmp(paymentctl->str(),"D") == 0)return TRUE;else return FALSE;};

 };
