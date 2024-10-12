#ifndef __DB_FA_H
#define __DB_FA_H
 _CLASSDEF(Tglfa)
 class Tglfa : public Data4
 {  
  public:
	Field4 facode;
	Field4 dep_meth;
	Field4 reference;
	Field4 descript;
	Field4 date_sold;
	Field4 date_pur;
	Field4 lifeleft;
	Field4 lifetime;
	Field4 percent;
	Field4 cost;
	Field4 accum_dep;
	Field4 curr_dep;
	Field4 salvage;
	Field4 fatype;
	Field4 mode;


 Tglfa(Code4 *cb,LPSTR name=0):Data4(cb,GetDbfName())
	{
    	if(!isValid())
	 return;

	SetIndex("facode");
	facode 		.init(data, 1 );
	dep_meth 	.init(data, 2 );
	reference 	.init(data, 3 );
	descript 	.init(data, 4 );
	date_sold 	.init(data, 5 );
	date_pur 	.init(data, 6);
	lifeleft 	.init(data, 7 );
	lifetime 	.init(data, 8 );
	percent 		.init(data, 9 );
	cost 			.init(data, 10 );
	accum_dep 	.init(data, 11 );
	curr_dep 	.init(data, 12 );
	salvage 		.init(data, 13 );
	fatype 		.init(data, 14 );
	mode 			.init(data, 15 );
	};
	~Tglfa(){
		  //close();
		};
	double dPercent(){return  double(Str4ptr(percent.str()));};

	BOOL   Journal();
	double dCost()    {return double(Str4ptr(cost.str()));};
	double dCurr_dep(){return double(Str4ptr(curr_dep.str()));};
	double dSalvage() {return double(Str4ptr(salvage.str()));};
	double dLife()    {return double(Str4ptr(lifetime.str()));};
	double dLeft()    {return double(Str4ptr(lifeleft.str()));};
	double dAccum_Dep() {return double(Str4ptr(accum_dep.str()));};
	LPSTR GetDbfName() { return "gl_fa";}
 };

 _CLASSDEF(Tfatype)
 class Tfatype : public Data4
 {
  public:
  Field4 	fatype;
  Field4 	fadesc;
  Field4 	fayears;
  Field4 	fapercent;
  Field4 	asset_acct;
  Field4 	asset_dept;
  Field4 	asset_desc;
  Field4 	expen_acct;
  Field4 	expen_dept;
  Field4 	expen_desc;
  Field4 	accum_acct;
  Field4 	accum_dept;
  Field4 	accum_desc;


 Tfatype(Code4 *cb,LPSTR name = NULL):Data4(cb,GetDbfName())
 {
	if(!isValid())
	 return;

	SetIndex("fatype");
	fatype 	 .init(data, 1 );
	fadesc 	 .init(data, 2 );
	fayears 	 .init(data, 3 );
	fapercent .init(data, 4 );
	asset_acct.init(data, 5 );
	asset_dept.init(data, 6 );
	asset_desc.init(data, 7 );
	expen_acct.init(data, 8 );
	expen_dept.init(data, 9 );
	expen_desc.init(data, 10 );
	accum_acct.init(data, 11 );
	accum_dept.init(data, 12 );
	accum_desc.init(data, 13 );

  };
	~Tfatype(){
	  //close();
	 };
	double dPercent(){return (double)Str4ptr(fapercent.str());};
	LPSTR GetDbfName() { return "gl_fatyp";}
 };

_CLASSDEF(TGLFaList)
class TGLFaList : public TDialog
{
 public:
 PTSpread List;
 PTglfa glfa;
 TGLFaList(PTWindowsObject AParent, LPSTR AName,PTglfa fa):
  TDialog (AParent, AName) {glfa = fa;List = new TSpread(this,ID_DISTLIST);};
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};

#endif
