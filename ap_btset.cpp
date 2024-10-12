#include "std_inc.h"

#include "ap_vendo.h"

#define ID_INVOICE 1030
#define ID_COMMIT 1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_VIN_NUM 1004
#define PROFILE "GL.INI"
#define  NEWBATCH  10
#define  OLDBATCH  11
#define  MODBATCH  12
#define ID_CENTURYDATE 6115
#define ID_PRIORPERIOD 8192
#define ID_DEPT_DESC 3038
#define ID_PERIOD_USED 1037

extern HINSTANCE hInst;

extern BOOL bNEW;


void TAPInvoiceEntry::SetupWindow()
{
 TMainDialog::SetupWindow();

 CHourGlass wait;
 bModify = FALSE;
 DebitTotal=0;
 CreditTotal=0;
 bTransaction = FALSE;
 Code4* cb = ((TAcctModule *)GetApplication())->GetCodeBase();

 if(apbtchd.open(*cb) < 0)
	{
	 ErrorClose(apbtchd.GetDbfName());
	 return;
	}

 if(apbtch.open(*cb) < 0 )
  {
	ErrorClose(apbtch.GetDbfName());
	return;
  }


 if(apvendor.open(*cb) < 0)
  {
	ErrorClose(apvendor.GetDbfName());
	return;
  }


 if(!glacct.open(*cb) < 0)
  {
	ErrorClose(glacct.GetDbfName());
	return;
  }


 if(gldist.open(*cb) < 0)
  {
	ErrorClose(gldist.GetDbfName());
	return;
  }


 if(apdist.open(*cb) < 0)
  {
	ErrorClose(apdist.GetDbfName());
	return;
  }


 if(prods.open(*cb) < 0)
  {
	ErrorClose(prods.GetDbfName());
	return;
  }


 if(gldept.open(*cb) < 0)
  {
	ErrorClose(gldept.GetDbfName());
	return;
  }


 if(drcodes.open(*cb) < 0)
  {
	ErrorClose(drcodes.GetDbfName());
	return;
  }

 mode = "1";

 LoadDoctors();
 LoadDepartments();
 LoadProducts();

 IntSetDlgItemValue(HWindow,ID_BATCHNO,SetBatchNumber());
 IntSetDlgItemValue(HWindow,ID_LINENO,1);

 ::SetPeriods(HWindow);

 //apbtch.SetIndex(GetString(8140));
 apbtch.SetIndexBatchno();
 apvendor.SetIndexVendor();

 GL_SETAT gl_setat;
 gl_setat.open(*cb);
 gl_setat.SetIndex("gl_accts");


 if(gl_setat.seek("WTXI") == 0)
 {
	strIndTaxAccount = gl_setat.account.str();
	strIndTaxDept    = gl_setat.dept.str();
  //strcpy(szIndTaxAccount,gl_setat.account.str());
  //strcpy(szIndTaxDept,gl_setat.dept.str());
 }
 else
 {
	strIndTaxAccount = gl_setat.account.str();
	strIndTaxDept    = gl_setat.dept.str();

  //strcpy(szIndTaxAccount,"");
  //strcpy(szIndTaxDept,"");
 }

 if(gl_setat.seek("WTXC") == 0)
 {
  strCompanyTaxAccount = gl_setat.account.str();
  strCompanyTaxDept    += gl_setat.dept.str();
  //strcpy(szCompanyTaxAccount,gl_setat.account.str());
  //strcpy(szCompanyTaxDept,gl_setat.dept.str());
 }
 else
 {
	strCompanyTaxAccount = gl_setat.account.str();
	strCompanyTaxDept    = gl_setat.dept.str();
	//strcpy(szCompanyTaxAccount,"");
	//strcpy(szCompanyTaxDept,"");
 }


 if(gl_setat.seek("VATP") == 0)
 {
	strVATAccount = gl_setat.account.str();
	strVATDept    = gl_setat.dept.str();

  //strcpy(szVATAccount,gl_setat.account.str());
  //strcpy(szVATDept,gl_setat.dept.str());
 }
 else
 {
	strVATAccount = "";
	strVATDept    = "";

  //strcpy(szVATAccount,"");
  //strcpy(szVATDept,"");
 }

 D = new TAPDistList(this,GetString(8156),&apbtchd);
 D->Attr.Style |= WS_THICKFRAME;
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);


 VendorList = new TCustomerList(this,"");
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);



 RECT rc;
 RECT parent;
 int pos = 72;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+3,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);

 SetBitmap(IDCANCEL);
 SetBitmap(ID_NEW);
 SetBitmap(IDOK);
 SetBitmap(ID_NEW);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_SEARCHCUST);
 SetBitmap(ID_SLIPLIST);
 SetBitmap(ID_PRINT);
 IntSetDlgItemRange(HWindow,4065,0,100);
 IntSetDlgItemRange(HWindow,4073,0,100);
 IntSetDlgItemRange(HWindow,6086,0,365);


 char szPeriod[3];
 ltoa(GetPeriod(),szPeriod,10);
 SetField(ID_PERIOD_USED,szPeriod);
 Clear();

 bNEW = TRUE;
 D->Show(SW_SHOW);
 return;

}
