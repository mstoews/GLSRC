// GEN.CPP  Generated File - Avoid hand modification.
// Created: Nov 17, 1994
#include <tbowl.h>
#include "d4all.hpp"
#include "gl_pds.HPP"


int GL_PDS::open( Code4& cb )
{
	Data4::open( cb, "GL_PDS" ) ;

	if( cb.errorCode != 0 )
	{
		if( isValid() )
			close() ;
		return cb.errorCode ;
	}

	company_nm.init( *this, 1 ) ;
	period.init    ( *this, 2 ) ;
	periods.init   ( *this, 3 ) ;
	periode.init   ( *this, 4 ) ;
	pd_desc.init   ( *this, 5 ) ;
	status.init    ( *this, 6 ) ;

	return 0 ;
}

