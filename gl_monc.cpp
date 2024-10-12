#include "std_inc.h"


class Tglmonthend : public Data4
{
 public:
 Field4 fname;
 Field4 account;
 Field4 depart;
 Field4 type;
 Field4 ID;

 inline void init()
 {
	fname.init(data,1);
	account.init(data,2);
	depart.init(data,3);
	type.init(data,4);
	ID.init(data,5);
 }
 Tglmonthend(){};
 Tglmonthend(Code4 *cb,LPSTR name=0) : Data4 (cb,GetDbfName())
  {
	 if(!isValid())
		return;

  };
  LPSTR GetDbfName(){ return "gl_mnend";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if(! isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
  }

  ~Tglmonthend()
  {
	  }
};



_CLASSDEF(TGLMonthEndInvoice)
class TGLMonthEndInvoice : public TMainDialog
{

 public:
  Tglmonthend glmonthend;
  TGLMonthEndInvoice(PTWindowsObject AParent,int ATitle):TMainDialog(AParent,ATitle){};
  virtual void SetupWindow();
  virtual void Cancel(RTMessage Msg) = [ID_FIRST + IDCANCEL];
};

void TGLMonthEndInvoice::Cancel(RTMessage)
{
  glmonthend.close();
  ShutDownWindow();
}

void TGLMonthEndInvoice::SetupWindow()
{

	if(glmonthend.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glmonthend.GetDbfName());
	return;
  }

  for(glmonthend.top();!glmonthend.eof();glmonthend.skip())
  {
	 int ID = atol(glmonthend.ID.str());
	 /*
	 string account
	 string dept
	 seek glaccount
	 if found
	  based on type return the correct amount and
	  set the amount to the window.
	 */
	  
  }

}


void TGlWindow::GLMonthEndClose (RTMessage)
{
  GetApplication()->ExecDialog(new TGLMonthEndInvoice(this,6070));
}
