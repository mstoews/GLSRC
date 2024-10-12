// Created: Nov 17, 1994
#include <tbowl.h>
#include "d4all.hpp"
#include "gl_prof.HPP"


int GL_PROF::open( Code4& cb )
{
	Data4::open( cb, "GL_PROF" ) ;
   cb.errorCode = 0 ;

   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

	company.init ( *this, 1 ) ;
	address1.init( *this, 2 ) ;
	address2.init( *this, 3 ) ;
	address3.init( *this, 4 ) ;
	postal.init  ( *this, 5 ) ;
	telephone.init( *this,6 ) ;
	fax.init     ( *this, 7 ) ;
	contact.init ( *this, 8 ) ;
   return 0 ;
}

