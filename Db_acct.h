#ifndef __ACCTS_H
#define __ACCTS_H
#include "gl_dlg.h"
#include "gl_pick.h"
 _CLASSDEF(Tglaccts)
 class Tglaccts:public Data4
 {
  public:
  Field4 accounts;
  Field4 dept;
  Field4 descript;
  Tglaccts(){}
  init()
  {
  accounts.init(data,1);
  dept.init(data,2);
  descript.init(data,3);
  }

  Tglaccts(Code4 *cb,LPSTR name=0):Data4(cb,"glaccts")
  {
  	if(!isValid())
	 return;

	init();
  }

  void SetIndexAccts(){SetIndex("accounts");};
  ~Tglaccts()
  {
  }
  };

#endif
