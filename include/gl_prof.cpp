// Created: Nov 17, 1994

#include "d4all.hpp"
#include "gl_prof.HPP"


int GL_PROF::open( Code4& cb )
{
   int rc_open_data = 0, rc_open_index = 0, rc_create_index = 0 ;

   rc_open_data = Data4::open( cb, "GL_PROF" ) ;
   cb.errorCode = 0 ;

   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

   company.init( *this, "COMPANY" ) ;
   address1.init( *this, "ADDRESS1" ) ;
   address2.init( *this, "ADDRESS2" ) ;
   address3.init( *this, "ADDRESS3" ) ;
   postal.init( *this, "POSTAL" ) ;
   telephone.init( *this, "TELEPHONE" ) ;
   fax.init( *this, "FAX" ) ;
   contact.init( *this, "CONTACT" ) ;
   return 0 ;
}

