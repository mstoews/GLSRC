// GEN.CPP  Generated File - Avoid hand modification.
// Created: Nov 17, 1994

#include "d4all.hpp"
#include "gl_pds.HPP"


int GL_PDS::open( Code4& cb )
{
   int rc_open_data = 0 ;
   Data4::open( cb, "GL_PDS" );
    //return FALSE;

   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

   company_nm.init( *this, "COMPANY_NM" ) ;
   period.init( *this, "PERIOD" ) ;
   periods.init( *this, "PERIODS" ) ;
   periode.init( *this, "PERIODE" ) ;
   pd_desc.init( *this, "PD_DESC" ) ;
   status.init( *this, "STATUS" ) ;

   return 0 ;
}

