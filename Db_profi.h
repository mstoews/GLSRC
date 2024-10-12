_CLASSDEF (Tprofile)
class Tprofile : public Data4
{
 public:
 Field4  company;
 Field4  address1;
 Field4  address2;
 Field4  address3;
 Field4  postal;
 Field4  telephone;
 Field4  fax;
 Field4  companyn;
 Field4  curr_pd;    //Current accounting period.
 Field4  curr_pdn;
 Field4  batchno;
 Field4  previousyr;
 Field4  department;
 Field4  batch;
 Field4  retained;
 Field4  period1;
 Field4  period1e;
 Field4  pd1_desc;

 Field4  period2;
 Field4  period2e;
 Field4  pd2_desc;

 Field4  period3;
 Field4  period3e;
 Field4  pd3_desc;

 Field4  period4;
 Field4  period4e;
 Field4  pd4_desc;

 Field4  period5;
 Field4  period5e;
 Field4  pd5_desc;

 Field4  period6;
 Field4  period6e;
 Field4  pd6_desc;

 Field4  period7;
 Field4  period7e;
 Field4  pd7_desc;

 Field4  period8;
 Field4  period8e;
 Field4  pd8_desc;

 Field4  period9;
 Field4  period9e;
 Field4  pd9_desc;

 Field4  period10;
 Field4  period10e;
 Field4  pd10_desc;

 Field4  period11;
 Field4  period11e;
 Field4  pd11_desc;

 Field4  period12;
 Field4  period12e;
 Field4  pd12_desc;

Tprofile(Code4  cb,LPSTR name=0):Data4(cb,"gl_adm")
 {
 	 if(!isValid())
		return;

  top();
  
  company   .init(data,1);// "company");
  address1  .init(data,2);//"address1");
  address2  .init(data,3);//"address2");
  address3  .init(data,4);//"address3");
  postal    .init(data,5);//"postal");
  telephone .init(data,6);//"telephone");
  fax       .init(data,7);//"fax");
  companyn  .init(data,8);//"companyn");
  curr_pd   .init(data,9);//"curr_pd");
  curr_pdn  .init(data,10);//"curr_pdn");
  batchno   .init(data,11);//"batchno");

  previousyr.init(data,12);//"previousyr");
  department.init(data,13);//"department");
  batch     .init(data,14);//"batch");
  retained  .init(data,15);//"retained");

  period1   .init(data,16);//"period1");
  period1e  .init(data,17);//"period1e");
  pd1_desc  .init(data,18);//"pd1_desc");

  period2   .init(data,19);//"period2");
  period2e  .init(data,20);//"period2e");
  pd2_desc  .init(data,21);//"pd2_desc");

  period3   .init(data,22);//"period3");
  period3e  .init(data,23);//"period3e");
  pd3_desc  .init(data,24);//"pd3_desc");

  period4   .init(data,25);//"period4");
  period4e  .init(data,26);//"period4e");
  pd4_desc  .init(data,27);//"pd4_desc");

  period5   .init(data,28);//"period5");
  period5e  .init(data,29);//"period5e");
  pd5_desc  .init(data,30);//"pd5_desc");

  period6   .init(data,31);//"period6");
  period6e  .init(data,32);//"period6e");
  pd6_desc  .init(data,33);//"pd6_desc");

  period7   .init(data,34);//"period7");
  period7e  .init(data,35);//"period7e");
  pd7_desc  .init(data,36);//"pd7_desc");

  period8   .init(data,37);//"period8");
  period8e  .init(data,38);//"period8e");
  pd8_desc  .init(data,39);//"pd8_desc");

  period9   .init(data,40);//"period9");
  period9e  .init(data,41);//"period9e");
  pd9_desc  .init(data,42);//"pd9_desc");

  period10  .init(data,43);//"period10");
  period10e .init(data,44);//"period10e");
  pd10_desc .init(data,45);//"pd10_desc");

  period11  .init(data,46);//"period11");
  period11e .init(data,47);//"period11e");
  pd11_desc .init(data,48);//"pd11_desc");

  period12  .init(data,49);//"period12");
  period12e .init(data,50);//"period12e");
  pd12_desc .init(data,51);//"pd12_desc");
 }
 ~Tprofile()
 {
 }
};
