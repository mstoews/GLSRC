
// Created: Nov 27, 1994
#include <tbowl.h>
#include "d4all.hpp"
#include "gl_setat.hpp"

int GL_SETAT::open( Code4& cb )
{
   Data4::open( cb, "GL_SETAT" ) ;
   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

	type    = new Field4(*(Data4 *)this, 1 ) ;
	name    = new Field4(*(Data4 *)this, 2 ) ;
	account = new Field4(*(Data4 *)this, 3 ) ;
	dept    = new Field4(*(Data4 *)this, 4 ) ;
   return 0 ;
}

