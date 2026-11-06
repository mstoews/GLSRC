#ifndef __GL_BTCHE_H
#define __GL_BTCHE_H

#ifndef __GL_G_H
#include "glb.h"
#endif


_CLASSDEF (TGLModifyBatch)
class TGLModifyBatch : public TDialog
{
 public:
  PTglbtchd glbtchd;
  TGLModifyBatch(PTWindowsObject AParent, LPSTR ATitle,PTglbtchd bdp):TDialog(AParent,ATitle)
  {
   glbtchd = bdp;
  };
  virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
};

extern Code4 cb;
// Database Declaration classes.

_CLASSDEF(Tarbtch)
class Tarbtch : public Data4
{
 public:
  Field4 *batchno;
  Field4 *mode;
  Field4 *descript;
  Field4 *invoiceno;
  Field4 *custno;
  Field4 *period;
  Field4 *date;
  Field4 *debit;
  Field4 *credit;
  Field4 *entryno;

  Tarbtch(Code4 *cb,LPSTR name=0) : Data4 (cb,"ar_btch")
  {
  batchno    = (Field4 *) new Field4(*(Data4 *) this ,"batchno");
  mode       = (Field4 *) new Field4(*(Data4 *) this ,"mode");
  descript   = (Field4 *) new Field4(*(Data4 *) this ,"descript");
  invoiceno  = (Field4 *) new Field4(*(Data4 *) this ,"invoiceno");
  custno     = (Field4 *) new Field4(*(Data4 *) this ,"custno");
  period     = (Field4 *) new Field4(*(Data4 *) this ,"period");
  date       = (Field4 *) new Field4(*(Data4 *) this ,"invdate");
  credit     = (Field4 *) new Field4(*(Data4 *) this ,"credit");
  debit      = (Field4 *) new Field4(*(Data4 *) this ,"debit");
  entryno    = (Field4 *) new Field4(*(Data4 *) this ,"entryno"); 
  };
 ~Tarbtch(){close();};
};



_CLASSDEF(Tarbtchd)
class Tarbtchd :public Data4
{
 public:

  Field4 *batchno;
  Field4 *custno;
  Field4 *billdate;
  Field4 *duedate;
  Field4 *invoiceno;
  Field4 *reference;
  Field4 *descript;
  Field4 *account;
  Field4 *dept;
  Field4 *debit;
  Field4 *credit;

  Tarbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_btchd")
  {
  batchno     = (Field4 *) new Field4(*(Data4 *)this,"batchno");
  custno      = (Field4 *) new Field4(*(Data4 *)this,"custno");
  billdate    = (Field4 *) new Field4(*(Data4 *)this,"billdate");
  invoiceno   = (Field4 *) new Field4(*(Data4 *)this,"invoiceno");
  duedate     = (Field4 *) new Field4(*(Data4 *)this,"duedate");
  reference   = (Field4 *) new Field4(*(Data4 *)this,"reference");
  descript    = (Field4 *) new Field4(*(Data4 *)this,"descript");
  account     = (Field4 *) new Field4(*(Data4 *)this,"account");
  dept        = (Field4 *) new Field4(*(Data4 *)this,"dept");
  debit       = (Field4 *) new Field4(*(Data4 *)this,"debit");
  credit      = (Field4 *) new Field4(*(Data4 *)this,"credit");
  };
 ~Tarbtchd(){close();};
};




#endif