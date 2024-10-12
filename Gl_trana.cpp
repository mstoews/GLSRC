#include "std_inc.h"

void TGlWindow::GLTransInquiry (RTMessage)
 {
  GetApplication()->ExecDialog(new TDialog(this,1029));
 }


void TGlWindow::GLTransactions (RTMessage)
 {
  GetApplication()->ExecDialog(new TDialog(this,6070));
 }

