#include "std_inc.h"
/*
#include <tbowl.h>
#include <ctl3d.h>
#include <dir.h>
#include <direct.h>
#include <commdlg.h>
#include "d4all.hpp"
#include "glclass.h"
#include "gl.h"
#include "crpe.h"
#include "gl_dlg.h"
#include "gl_main.h"
#include "gl_strg.h"
#include "gl_mains.h"
#include "gl_tooli.h"
#include "gl_tool.h"
#include "gl_manil.h"
#include "gl_man.h"
#include "cstring.h"
#include "gl_user.h"

*/

Tgllock::Tgllock(){}

#pragma argsused
BOOL Tgllock::LockPrg(LPSTR szProgram)
{
	return TRUE;
}
#pragma argsused
BOOL Tgllock::UnLockPrg(LPSTR szProgram)
{
	return TRUE;
}

#pragma argsused
BOOL Tgllock::LockDbf(LPSTR szDbf)
{
  return TRUE;
}
#pragma argsused
BOOL Tgllock::UnLockDbf(LPSTR szDbf)
{
 return TRUE;
}
#pragma argsused
BOOL Tgllock::LockRec(LPSTR szDbf,long lBatchno)
{
 return TRUE;
}
#pragma argsused
BOOL Tgllock::UnLockRec(LPSTR szDbf,long lBatchno)
{
 return TRUE;
}
