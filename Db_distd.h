#ifndef __GL_DISTB_H
#define __GL_DISTB_H
_CLASSDEF (Tgldistb)
class Tgldistb : public Data4
{
 public:
  Field4 account;
  Field4 dept;
  Field4 type;
  Field4 period;
  Field4 date;
  Field4 batchno;
  Field4 reference;
  Field4 cheque;
  Field4 descript;
  Field4 debit;
  Field4 credit;
  Tgldistb(Code4 *cb,LPSTR name=0):Data4(cb,"gl_distb")
  {
  	if(!isValid())
	 return;

	SetIndex("account");
	top();
	account   .init(data, "account");
	dept      .init(data, "dept");
	type      .init(data, "type");
	period    .init(data, "period");
	date      .init(data, "date");
	batchno   .init(data, "batchno");
	reference .init(data, "reference");
	cheque    .init(data,"cheque");
	descript  .init(data, "descript");
	debit     .init(data, "debit");
	credit    .init(data, "credit");
  };
  ~Tgldistb(){
    //close();
  };
 };




#endif