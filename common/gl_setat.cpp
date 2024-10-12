// GEN.CPP  Generated File - Avoid hand modification.
// Created: Nov 27, 1994

#include "d4all.hpp"
#include "gl_setat.hpp"

int GL_SETAT::open( Code4& cb )
{
   int rc_open_data = 0;
   rc_open_data = Data4::open( cb, "GL_SETAT" ) ;

   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

   name.init( *this, "NAME" ) ;
   account.init( *this, "ACCOUNT" ) ;
   dept.init( *this, "DEPT" ) ;
   type.init( *this, "TYPE" ) ;

   return 0 ;
}

