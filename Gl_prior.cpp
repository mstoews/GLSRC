/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Prior Period Adjustment Entry
*/
#include "std_inc.h"
/*
#include "gll.h"

#include "gl_dlg.h"
#include "gl_pick.h"
#include "db_accts.h"
//#include "db_dept.h"
#include "db_dist.h"
#include "db_itemm.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_medrp.h"
#include "gl_manil.h"
#include "gl_prodl.h"
#include "gl_medrl.h"
#include "gl_medrp.h"
#include "gl_btche.h"
#include "gl_distl.h"
#include "utildlg.h"
*/


_CLASSDEF (TGLPrior)
class TGLPrior : public TGLBatchEntry
 {
   public:
   TGLPrior(PTWindowsObject AParent,int ATitle):TGLBatchEntry(AParent,ATitle){};
   virtual void SetupWindow();   
   virtual void CurrentBatch     (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
 };

 /* Prior period adjustment should adjust the previous years ending balance and then
    adjust the current balance bassed upon the the current periods amount plus the
	 previous years balance.

	 The current balance should always be the previous years ending balance plus additions
    for each period so far this year.

    The glaccts database should be adjusted to show an addition field for the previous
    periods ending balance. This field will consist of only balance sheet items as prior
    period adjustments should not be made to the income statement.

    Only one modifications from the normal batch entry must be made:
     1. When entering the detail of the batch entry specify that it is a prior period entry
	for the batch header database.

    Modify the posting program to adjust the entries for a prior period adjustment.
    Don't forget to create a program that closes the current year and wraps up everything to
    retained earnings.

 */

void TGlWindow::GLPrior(RTMessage)
 {
  GetApplication()->ExecDialog(new TGLPrior(this,1013)); // GL_BATCH Resource Dialog
 }


void TGLPrior::SetupWindow()
{
 TGLBatchEntry::SetupWindow();
 SetMode("3");
 SetCaption(GetString(3082));
}


void TGLPrior::CurrentBatch(RTMessage Msg)
{

 long CurrentRecord = glbtch.recNo();
 int rc = glbtch.lock(CurrentRecord);
 if(rc != 0)
  {
	 MessageBox(HWindow,GetString(1052),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	 return;
  }

 if(strcmp(glbtch.mode.str(),"3") != 0)
	 {
	  MessageBox(HWindow,GetString(3083),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	  return;
	 }
 TGLBatchEntry::CurrentBatch(Msg);
}

