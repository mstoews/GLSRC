
// Created: Nov 27, 1994
#include "std_inc.h"

int GL_SETAT::open( Code4& cb )
{
   Data4::open( cb, "GL_SETAT" ) ;
   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

	shortn.init(data,1 ) ;
	name.init(data,2 ) ;
	account.init(data,3 ) ;
	dept.init(data,4 ) ;
	type.init(data, 5 ) ;
   return 0 ;
}

