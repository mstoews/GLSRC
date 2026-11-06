// GEN.CPP  Generated File - Avoid hand modification.
// Created: Nov 19, 1994

#include "d4all.hpp"
#include "GEN.HPP"

static FIELD4INFO AR_BTCH_field_info[] =
{
   { "BATCHNO", 'N', 6, 0 },
   { "MODE", 'C', 1, 0 },
   { "TYPE", 'C', 2, 0 },
   { "ADJ", 'C', 1, 0 },
   { "PERIOD", 'N', 2, 0 },
   { "VENDORNO", 'C', 6, 0 },
   { "VDNAME", 'C', 40, 0 },
   { "INVOICENO", 'C', 8, 0 },
   { "ORDERNO", 'C', 10, 0 },
   { "ORDERDATE", 'D', 8, 0 },
   { "VIN", 'C', 10, 0 },
   { "UNITS", 'C', 7, 0 },
   { "DUEDATE", 'D', 8, 0 },
   { "INVDATE", 'D', 8, 0 },
   { "REFERENCE", 'C', 40, 0 },
   { "DESCRIPT", 'C', 10, 0 },
   { "DATE", 'D', 8, 0 },
   { "ACCOUNT", 'C', 7, 0 },
   { "DEPART", 'C', 4, 0 },
   { "PAYABLE", 'N', 16, 2 },
   { "REBATEAMT", 'N', 16, 2 },
   { "ENTRYNO", 'N', 3, 0 },
   { "VATPERCENT", 'N', 2, 0 },
   { "REBATE", 'N', 2, 0 },
   { "TERMS", 'N', 3, 0 },
   { "VATAMT", 'N', 16, 2 },
   { "ADJAMT", 'N', 16, 2 },
   { "MEMO", 'M', 10, 0 },
   { 0,0,0,0 }
} ;

static TAG4INFO AR_BTCH_tag_info[] =
{
   { "BATCHNO", "batchno", 0, 0, 0 },
   { "RECNO", "RECNO()", 0, 0, 0 },
   { "INVOICENO", "vendorno+invoiceno", 0, 0, 0 },
   { "ACCOUNT", "account+depart", 0, 0, 0 },
   { 0,0,0,0,0 }
} ;

int AR_BTCH::open( Code4& cb )
{
   int save_autoOpen = cb.autoOpen ;
   int save_safety =  cb.safety ;

   int rc_open_data = 0, rc_open_index = 0, rc_create_index = 0 ;

   cb.autoOpen = 0 ;

   rc_open_data = Data4::open( cb, "AR_BTCH" ) ;
   if( rc_open_data != 0 )
   {
      cb.safety = 1 ;
      cb.errorCode = 0 ;
      create( cb, "AR_BTCH" , AR_BTCH_field_info ) ;
   }

   cb.safety = 0 ;
   Index4 index ;
   if( rc_open_data == 0 )
      rc_open_index = index.open( *this, "AR_BTCH" ) ;
   if( isValid() && rc_create_index == 0 && 
      (rc_open_data != 0 || rc_open_index != 0) )
   {
      cb.errorCode = 0 ;
      rc_create_index = index.create( *this, 0, AR_BTCH_tag_info ) ;
   }

   cb.safety = save_safety ;
   cb.autoOpen = save_autoOpen ;

   if( cb.errorCode != 0 )
   {
      if( isValid() )
         close() ;
      return cb.errorCode ;
   }

   batchno.init( *this, "BATCHNO" ) ;
   mode.init( *this, "MODE" ) ;
   type.init( *this, "TYPE" ) ;
   adj.init( *this, "ADJ" ) ;
   period.init( *this, "PERIOD" ) ;
   vendorno.init( *this, "VENDORNO" ) ;
   vdname.init( *this, "VDNAME" ) ;
   invoiceno.init( *this, "INVOICENO" ) ;
   orderno.init( *this, "ORDERNO" ) ;
   orderdate.init( *this, "ORDERDATE" ) ;
   vin.init( *this, "VIN" ) ;
   units.init( *this, "UNITS" ) ;
   duedate.init( *this, "DUEDATE" ) ;
   invdate.init( *this, "INVDATE" ) ;
   reference.init( *this, "REFERENCE" ) ;
   descript.init( *this, "DESCRIPT" ) ;
   date.init( *this, "DATE" ) ;
   account.init( *this, "ACCOUNT" ) ;
   depart.init( *this, "DEPART" ) ;
   payable.init( *this, "PAYABLE" ) ;
   rebateamt.init( *this, "REBATEAMT" ) ;
   entryno.init( *this, "ENTRYNO" ) ;
   vatpercent.init( *this, "VATPERCENT" ) ;
   rebate.init( *this, "REBATE" ) ;
   terms.init( *this, "TERMS" ) ;
   vatamt.init( *this, "VATAMT" ) ;
   adjamt.init( *this, "ADJAMT" ) ;
   memo.init( *this, "MEMO" ) ;

   tag_batchno.init( *this, "BATCHNO" ) ;
   tag_recno.init( *this, "RECNO" ) ;
   tag_invoiceno.init( *this, "INVOICENO" ) ;
   tag_account.init( *this, "ACCOUNT" ) ;
   return 0 ;
}

