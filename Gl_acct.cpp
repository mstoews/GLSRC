// GEN.CPP  Generated File - Avoid hand modification.
// Created: Nov 18, 1994

#include "d4all.hpp"
#include "gl_acct.HPP"


static TAG4INFO GLACCTS_tag_info[] =
{
   { "ACCOUNT", "account+dept", 0, 0, 0 },
   { "ACCOUNTS", "account+dept", 0, 0, 0 },
   { "DEPT", "dept", 0, 0, 0 },
   { "TYPE", "type", 0, 0, 0 },
   { 0,0,0,0,0 }
} ;

int GLACCTS::open( Code4& cb )
{
   int save_autoOpen = cb.autoOpen ;
   int save_safety =  cb.safety ;

   int rc_open_data = 0, rc_open_index = 0, rc_create_index = 0 ;


   rc_open_data = Data4::open( cb, "GLACCTS" ) ;
   /*
   cb.safety = 0 ;
   Index4 index ;
   if( rc_open_data == 0 )
      rc_open_index = index.open( *this, "GLACCTS" ) ;
   if( isValid() && rc_create_index == 0 && 
      (rc_open_data != 0 || rc_open_index != 0) )
   {
      cb.errorCode = 0 ;
      rc_create_index = index.create( *this, 0, GLACCTS_tag_info ) ;
   }
   */
   cb.safety = save_safety ;
   cb.autoOpen = save_autoOpen ;

   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

   account.init( *this, "ACCOUNT" ) ;
   dept.init( *this, "DEPT" ) ;
   descript.init( *this, "DESCRIPT" ) ;
   fdescript.init( *this, "FDESCRIPT" ) ;
   type.init( *this, "TYPE" ) ;
   category.init( *this, "CATEGORY" ) ;
   curr_bal.init( *this, "CURR_BAL" ) ;
   return 0 ;
}

