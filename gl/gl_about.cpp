#include <bwcc.h>
#include <commdlg.h>   
#include <tbowl.h>
#include "gl.h"
#include "d4all.hpp"
#include "db_profi.h"
#include "gl_dlg.h"
#include "ap_glslp.h"
#include "crpe.h"
#include "gl_main.h"

void TGlWindow::CMNewYear (RTMessage)
{
    GetApplication()->ExecDialog(new TDialog(this,"NEWYEAR"));
}
