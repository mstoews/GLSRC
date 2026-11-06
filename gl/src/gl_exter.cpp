#include "gll.h"
#include "gl_man.h"
void TManilaGL::GLExternal(RTMessage)
{
  GetApplication()->ExecDialog(new TDialog(this,1058));
}
