/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Maintain Specifications
*/
#include <dir.h>
#include "tbowl.h"
#ifndef D4ALL_INC 
#include "d4all.h"
#endif

#ifndef D4DATA_H
#include "d4data.hpp"
#endif

#ifndef   __GLCLASS_H
#include "glclass.h"
#endif

#include "gl.h"         
#include "gl_mains.h"

extern char   CurDir[64];


void TGLMainSpecs::WMInitDialog ( RTMessage ){}

void TGLMainSpecs::Cancel (RTMessage )
{
}

void TGLMainSpecs::Ok     (RTMessage )
{
}



