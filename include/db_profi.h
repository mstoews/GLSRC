_CLASSDEF (Tprofile)
class Tprofile : public Data4
{
 public:
 Field4 *company;
 Field4 *address1;
 Field4 *address2;
 Field4 *address3;
 Field4 *postal;
 Field4 *telephone;
 Field4 *fax;
 Field4 *companyn;
 Field4 *curr_pd;    //Current accounting period.
 Field4 *curr_pdn;
 Field4 *batchno;
 Field4 *previousyr;
 Field4 *department;
 Field4 *batch;
 Field4 *retained;
 Field4 *period1;
 Field4 *period1e;
 Field4 *pd1_desc;

 Field4 *period2;
 Field4 *period2e;
 Field4 *pd2_desc;

 Field4 *period3;
 Field4 *period3e;
 Field4 *pd3_desc;

 Field4 *period4;
 Field4 *period4e;
 Field4 *pd4_desc;

 Field4 *period5;
 Field4 *period5e;
 Field4 *pd5_desc;

 Field4 *period6;
 Field4 *period6e;
 Field4 *pd6_desc;

 Field4 *period7;
 Field4 *period7e;
 Field4 *pd7_desc;

 Field4 *period8;
 Field4 *period8e;
 Field4 *pd8_desc;

 Field4 *period9;
 Field4 *period9e;
 Field4 *pd9_desc;

 Field4 *period10;
 Field4 *period10e;
 Field4 *pd10_desc;

 Field4 *period11;
 Field4 *period11e;
 Field4 *pd11_desc;

 Field4 *period12;
 Field4 *period12e;
 Field4 *pd12_desc;

Tprofile(Code4 *cb,LPSTR name=0):Data4(cb,"gl_adm")
 {
  top();
  
  company   = (Field4 *) new Field4(*(Data4 *) this,"company");
  address1  = (Field4 *) new Field4(*(Data4 *) this,"address1");
  address2  = (Field4 *) new Field4(*(Data4 *) this,"address2");
  address3  = (Field4 *) new Field4(*(Data4 *) this,"address3");
  postal    = (Field4 *) new Field4(*(Data4 *) this,"postal");
  telephone = (Field4 *) new Field4(*(Data4 *) this,"telephone");
  fax       = (Field4 *) new Field4(*(Data4 *) this,"fax");
  companyn  = (Field4 *) new Field4(*(Data4 *) this,"companyn");
  curr_pd   = (Field4 *) new Field4(*(Data4 *) this,"curr_pd");
  curr_pdn  = (Field4 *) new Field4(*(Data4 *) this,"curr_pdn");
  batchno   = (Field4 *) new Field4(*(Data4 *) this,"batchno");

  previousyr= (Field4 *) new Field4(*(Data4 *) this,"previousyr");
  department= (Field4 *) new Field4(*(Data4 *) this,"department");
  batch     = (Field4 *) new Field4(*(Data4 *) this,"batch");
  retained  = (Field4 *) new Field4(*(Data4 *) this,"retained");

  period1   = (Field4 *) new Field4(*(Data4 *) this,"period1");
  period1e  = (Field4 *) new Field4(*(Data4 *) this,"period1e");
  pd1_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd1_desc");

  period2   = (Field4 *) new Field4(*(Data4 *) this,"period2");
  period2e  = (Field4 *) new Field4(*(Data4 *) this,"period2e");
  pd2_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd2_desc");

  period3   = (Field4 *) new Field4(*(Data4 *) this,"period3");
  period3e  = (Field4 *) new Field4(*(Data4 *) this,"period3e");
  pd3_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd3_desc");

  period4   = (Field4 *) new Field4(*(Data4 *) this,"period4");
  period4e  = (Field4 *) new Field4(*(Data4 *) this,"period4e");
  pd4_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd4_desc");

  period5   = (Field4 *) new Field4(*(Data4 *) this,"period5");
  period5e  = (Field4 *) new Field4(*(Data4 *) this,"period5e");
  pd5_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd5_desc");

  period6   = (Field4 *) new Field4(*(Data4 *) this,"period6");
  period6e  = (Field4 *) new Field4(*(Data4 *) this,"period6e");
  pd6_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd6_desc");

  period7   = (Field4 *) new Field4(*(Data4 *) this,"period7");
  period7e  = (Field4 *) new Field4(*(Data4 *) this,"period7e");
  pd7_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd7_desc");

  period8   = (Field4 *) new Field4(*(Data4 *) this,"period8");
  period8e  = (Field4 *) new Field4(*(Data4 *) this,"period8e");
  pd8_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd8_desc");

  period9   = (Field4 *) new Field4(*(Data4 *) this,"period9");
  period9e  = (Field4 *) new Field4(*(Data4 *) this,"period9e");
  pd9_desc  = (Field4 *) new Field4(*(Data4 *) this,"pd9_desc");

  period10  = (Field4 *) new Field4(*(Data4 *) this,"period10");
  period10e = (Field4 *) new Field4(*(Data4 *) this,"period10e");
  pd10_desc = (Field4 *) new Field4(*(Data4 *) this,"pd10_desc");

  period11  = (Field4 *) new Field4(*(Data4 *) this,"period11");
  period11e = (Field4 *) new Field4(*(Data4 *) this,"period11e");
  pd11_desc = (Field4 *) new Field4(*(Data4 *) this,"pd11_desc");

  period12  = (Field4 *) new Field4(*(Data4 *) this,"period12");
  period12e = (Field4 *) new Field4(*(Data4 *) this,"period12e");
  pd12_desc = (Field4 *) new Field4(*(Data4 *) this,"pd12_desc");
 }

};
