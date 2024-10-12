/* d4data.hpp (c)Copyright Sequiter Software Inc., 1989-1993.  All rights reserved. */
#ifndef D4DATA_HPP
#define D4DATA_HPP

#define S4CBPP

class S4CLASS Data4 ;
class S4CLASS Index4 ;
class S4CLASS Str4len ;
class S4CLASS Str4ptr ;
class S4CLASS Field4info ;
class S4CLASS Tag4info ;
class S4CLASS List4 ;
class S4CLASS File4 ;
class S4CLASS Tag4 ;
class S4CLASS Date4 ;
class S4CLASS Code4 ;

class S4CLASS Expr4
{
public:
   EXPR4 S4PTR *expr ;

   Expr4() { expr = 0 ; }
   Expr4( EXPR4 S4PTR *ex ) { expr = ex ;}
   Expr4( Data4 d, char *ex ) ;
   #ifdef __TURBOC__
      operator double() { double d; expr4double2( expr, &d ) ; return d; }
   #else
      operator double() { return expr4double( expr ) ; }
   #endif
   Data4 data() ;
   void free() { expr4free( expr ) ; expr = 0 ;}
   char *key( ) ;
   int keyLen() { return expr4key_len( expr ) ;}
   int len() { return expr4len( expr ) ;}
   int parse( Data4 d, char *ex) ;
   int isValid() { return expr != 0  ;  }
   char *source() { return expr4source( expr ) ;}
   int true() { return expr4true( expr ) ;}
   int type() { return expr4type( expr ) ;}
   char *vary( ) ; 
} ;

class S4CLASS Code4 : public CODE4
{
public:
   Code4(int initialize = 1) { if( initialize ) init() ; }
   Code4( Code4& ) ;         /* Illegal operation, force link error */
   operator =(Code4 &) ;      /* Illegal operation, force link error */

   int     calcCreate( Expr4 ex, char *name )
                    { return (expr4calc_create( this, ex.expr, name )) ? 1:0; }
   void    calcReset()        {        expr4calc_reset( this ) ; }
   int     closeAll()         { return d4close_all( this ) ; }
   Data4   data( char *alias ) ;
   int     error( int er, char *p1=0, char *p2 = 0, char *p3 = 0 )
                               { return e4describe( this, er, p1,p2,p3 ) ; }
   void    exit()              {        e4exit( this ) ; }
   void    exitTest()         {        e4exit_test( this ) ; }
   int     flushFiles()       { return d4flush_files( this ) ; }
   void    init()             {
                               d4init( this ) ;
                              _fmemset( string_types, 0, sizeof(string_types) ) ;
                              }
   int     initUndo()         { return d4init_undo( this ) ; }
   int     optAll() ;
   int     optStart()         { return d4opt_start( this ) ; }
   int     optSuspend()       { return d4opt_suspend( this ) ; }
   #ifndef S4DLL   
      void    reset( )            { mem4reset( ) ;}
   #endif
   int     unlock()      { return d4unlock_files( this ) ; }

   MEM4 *string_types[16] ;

   #ifdef S4OLD_CODE
      int     close_all()         { return d4close_all( this ) ; }
      void    exit_test()         {        e4exit_test( this ) ; }
      int     flush_files()       { return d4flush_files( this ) ; }
      int     init_undo()         { return d4init_undo( this ) ; }
      int     opt_start()         { return d4opt_start( this ) ; }
      int     opt_suspend()       { return d4opt_suspend( this ) ; }
		int     unlock_files()      { return d4unlock_files( this ) ; }
   #endif
} ;

class S4CLASS Data4
{
public:
   DATA4 S4PTR *data ;

   Data4()                          { data = 0 ; }
   Data4( DATA4 *d )                { data = d ; }
   Data4( Code4 &code, char *name ) {
				      data = d4open( &code, name ) ;
				    }
   Data4( Code4 *code, char *name ) {
					data = d4open( code, name ) ;

				    }
   char * alias()                   { return d4alias( data ) ; }
   void   alias( char *ptr )        {        d4alias_set( data, ptr ) ; }
   int    append()                  { return d4append( data ) ; }
   int    appendData()             { return d4append_data( data ) ; }
   int    appendBlank()            { return d4append_blank( data ) ; }
   int    appendStart( int memo = 0 )  { return d4append_start( data, memo ) ; }
   void   blank()                   {        d4blank( data ) ; }
   int    bof()                     { return d4bof( data ) ; }
   int    bottom()                  { return d4bottom( data ) ; }
   int    changed( int flag = -1 )       { return d4changed( data, flag ) ; }
   int    check()                   { return d4check( data ) ; }
   int    close()            { int rc = d4close( data ) ; data = 0; return rc; }
   int    create( Code4 &code, char *name, FIELD4INFO *f, TAG4INFO *t = 0 )
                                    { data = d4create( &code, name, f, t ) ;
                                      return code.errorCode ; }
   void   deleteRec()              {        d4delete( data ) ; }
   int    deleted()                 { return d4deleted( data ) ; }
   int    eof()                     { return d4eof( data ) ; }
   int    fieldNumber( char *name) { return d4field_number( data, name) ; }
   char * fileName( ) { return data->file.name ; }
   int    flush()                   { return d4flush( data ) ; }
   int    flushData()              { return d4flush_data( data ) ; }
	int    freeBlocks()             { return d4free_blocks( data ) ; }
   int    go( long rec )            { return d4go( data, rec ) ; }
   int    goData( long rec )       { return d4go_data( data, rec ) ; }
   int    goEof()                  { return d4go_eof( data ) ; }
   Index4 index( char *name ) ;
   int    isValid( )               { return data != 0 ; }
   int    lock( long rec_num )      { return d4lock( data, rec_num ) ; }
   int    lockAll()                { return d4lock_all( data ) ; }
   int    lockAppend()             { return d4lock_append( data ) ; }
   int    lockFile()               { return d4lock_file( data ) ; }
   int    lockGroup(long *r, int n){ return d4lock_group( data, r, n ) ; }
   int    lockIndex()              { return d4lock_index( data ) ; }
   int    lockTest( long rec )     { return d4lock_test( data, rec ) ; }
   int    lockTestAppend()        { return d4lock_test_append( data ) ; }
   int    lockTestFile()          { return d4lock_test_file( data ) ; }
   int    lockTestIndex()         { return d4lock_test_index( data ) ; }
   int    memoCompress()           { return d4memo_compress( data ) ; }
   int    numFields()              { return d4num_fields( data ) ; }
   int    open( Code4 &code, char *name ) { data =  d4open( &code, name ) ;
					   return code.errorCode ; }
   int    open( Code4 *code, char *name ) { data = d4open( code, name ) ;
    					   return code->errorCode ;}
   int    optimize( int opt_flag )  { return d4optimize( data, opt_flag ) ; }
   int    optimizeWrite( int opt_flag )  { return d4optimize_write( data,  opt_flag ) ; }
   int    pack()                    { return d4pack( data ) ; }
   int    packData()               { return d4pack_data( data ) ; }
   #ifdef __TURBOC__
      double position()            { double d;
                                        d4position2( data, &d ) ;
                                        return d; }
   #else
      double position()            { return d4position( data ) ; }
   #endif
   int    position( double pos )    { return d4position_set( data, pos ) ; }
   int    read( long rec,char *buf) { return d4read( data, rec, buf ) ; }
   int    readOld( long rec )      { return d4read_old( data, rec ) ; }
   void   recall()                  {        d4recall( data ) ; }
   long   recCount()                { return d4reccount( data ) ; }
   long   recNo()                   { return d4recno( data ) ; }
   char * record()                  { return d4record( data ) ; }
	long   recPosition( long r ) { return d4record_position( data, r ) ; }
   long   recWidth()            { return d4record_width( data ) ; }
   int    refresh()                 { return d4refresh( data ) ; }
   int    refreshRecord()          { return d4refresh_record( data ) ; }
   int    reindex()                 { return d4reindex( data ) ; }
   int    seek( char *ptr )         { return d4seek( data, ptr ) ; }
   int    seek( char *ptr,int len ) { return d4seek_n( data, ptr, len ) ; }
   int    seek( double d )          { return d4seek_double( data, d ) ; }
   int    skip( long n = 1 )        { return d4skip( data, n ) ; }
   void   select()                  { d4tag_select( data, 0 ) ; }
   void   select( Tag4 tag ) ;
   void   SetIndex(char *tagName)   { d4tag_select ( data,d4tag(data,tagName) ) ; }
   int    top()                     { return d4top( data ) ; }
   int    unlock()                  { return d4unlock( data ) ; }
   int    validateMemo()       { return d4validate_memo_ids( data ) ; }
   int    write( long rec = -1 )    { if (rec == -1)  return d4write( data, d4recno(data) ); 
                                                   return d4write( data, rec ) ; }
   int    writeData( long rec )    { if (rec == -1)  return d4write_data( data, d4recno(data) ); 
                                                   return d4write_data( data, rec ) ; }
   int    writeKeys( long rec )    { return d4write_keys( data, rec ) ; }
   int    zap( long f = 1, long last = (long)2000000000L)   { return d4zap( data, f, last ) ; }
   int    zapData(long f = 1,long last= (long)2000000000L){ return d4zap_data( data, f, last ) ; }

   #ifdef S4OLD_CODE
   /* CodeBase 4.5
   FIELD4INFO * field_info()        { return d4field_info( data ) ; }
   CodeBase++ 1.0
   Field4 field( char * ) ;
   Field4 field( int ) ;
   */

   void   alias_set( char *ptr )    {        alias( ptr ) ; }
   int    append_data()             { return d4append_data( data ) ; }
   int    append_blank()            { return d4append_blank( data ) ; }
   int    append_start( int memo )  { return d4append_start( data, memo ) ; }
   void   delete_rec()              {        d4delete( data ) ; }
   int    field_number( char *name) { return d4field_number( data, name) ; }
   int    flush_data()              { return d4flush_data( data ) ; }
   int    free_blocks()             { return d4free_blocks( data ) ; }
   int    go_data( long rec )       { return d4go_data( data, rec ) ; }
	int    go_eof()                  { return d4go_eof( data ) ; }
   int    lock_all()                { return d4lock_all( data ) ; }
   int    lock_append()             { return d4lock_append( data ) ; }
   int    lock_file()               { return d4lock_file( data ) ; }
   int    lock_group(long *r, int n){ return d4lock_group( data, r, n ) ; }
   int    lock_index()              { return d4lock_index( data ) ; }
   int    lock_test( long rec )     { return d4lock_test( data, rec ) ; }
   int    lock_test_append()        { return d4lock_test_append( data ) ; }
   int    lock_test_file()          { return d4lock_test_file( data ) ; }
   int    lock_test_index()         { return d4lock_test_index( data ) ; }
   int    memo_compress()           { return d4memo_compress( data ) ; }
   int    num_fields()              { return d4num_fields( data ) ; }
   int    optimize_write( int opt_flag )  { return d4optimizeWrite( data,  opt_flag ) ; }
   int    pack_data()               { return d4pack_data( data ) ; }
   int    position_set( double d )  { return d4position_set( data, d ) ; }
   int    read_old( long rec )      { return d4read_old( data, rec ) ; }
   long   record_position( long r ) { return d4record_position( data, r ) ; }
   long   record_width()            { return d4record_width( data ) ; }
   int    refresh_record()          { return d4refresh_record( data ) ; }
   int    seek_double( double d )   { return d4seek_double( data, d ) ; }
   TAG4  *tag_default()             { return d4tag_default( data ) ; }
   TAG4  *tag_next( TAG4 *tag )     { return d4tag_next( data, tag ) ; }
   TAG4  *tag_prev( TAG4 *tag )     { return d4tag_prev( data, tag ) ; }
   void   tag_select( TAG4 *tag )   {        d4tag_select( data, tag ) ; }
   TAG4  *tag_selected()            { return d4tag_selected( data ) ; }
   int    update_header(int t,int c){ return d4update_header( data, t, c ) ; }
   int    validate_memo_ids()       { return d4validate_memo_ids( data ) ; }
   int    write_data( long rec )    { return d4write_data( data, rec ) ; }
   int    write_keys( long rec )    { return d4write_keys( data, rec ) ; }
   int    zap_data(long f,long last){ return d4zap_data( data, f, last ) ; }
   #endif

} ;

class S4CLASS Index4 
{
public:
   INDEX4 S4PTR *index ;

   Index4()  { index = 0 ; }
	Index4( INDEX4 S4PTR *in )  { index = in ; }
   Index4( Data4 d, char *name ) { index = i4open( d.data, name ) ; }

   int    addTag( TAG4INFO *newTag ) { return i4add_tag( this->index, newTag ) ; }
   int    close()        { int rc =  i4close( index ) ; index = 0 ; return rc;}
   int    create( Data4 d, char *name, TAG4INFO *info ) 
                    { index = i4create( d.data, name, info ) ;
                              return d.data->code_base->errorCode ; }
   int    create( Data4 d, TAG4INFO *info ) 
                    { index = i4create( d.data, 0, info ) ; 
                              return d.data->code_base->errorCode ; }
   Data4  data()           { return Data4( index->data ) ; }
   char *fileName()       { return index->file.name ;}
   int    isProduction( ) { return i4is_production( index ) ;}
   int    isValid()       { return index!=0;}
   void   init( Data4 d, char *name )  { index = d4index(d.data, name ); }
   int    lock()           { return i4lock( index ) ; }
   int    open( Data4 d, char *file = NULL )  { index = i4open( d.data, file ) ;
                              return d.data->code_base->errorCode ; }
   int    reindex()        { return i4reindex( index ) ; }
   Tag4   tag( char *name ) ;
   int    unlock()         { return i4unlock( index ) ; }
} ;

class S4CLASS Tag4 
{
public:       
   TAG4 S4PTR *tag ;

   Tag4()                      { tag = NULL ; }
   Tag4( TAG4 S4PTR *tg )            { tag = tg ; } 
   Tag4( Data4 d, char *name ) { tag = d4tag( d.data, name ) ;}  

   char * alias()   { return tag->alias ; }
   int add( void *ptr, long rec ) { return t4add( tag, (unsigned char *)ptr, rec); } 
   int addCalc( long rec) { return t4add_calc( tag, rec) ; } 
   int bottom() { return t4bottom( tag ) ; }
   int descending() { return t4is_descending( tag ) ;}
   int down() { return t4down( tag ) ; }
   // dump() only supported for MDX
	int dump(int out_handle, int display_all)
                    { return t4dump( tag, out_handle, display_all ) ;}
   int eof()       { return t4eof( tag ) ;}
   Expr4  expr()   { return Expr4( tag->expr ) ; }
   Expr4  filter() { return Expr4( tag->filter ) ; }
// int flush()     { return t4flush( tag ); }
   int freeAll()  { return t4free_all( tag ) ;}
   int go( char *key, long rec_num )  { return t4go( tag, key, rec_num); }

   Index4 index() { return Index4(tag->index) ; }
   void init( Data4 d, char *name ) { tag=d4tag(d.data, name ) ; }

   void initDefault( Data4 d ) { tag = d4tag_default( d.data ) ; }
   void initFirst( Data4 d ) { tag = d4tag_next( d.data, NULL ) ; }
   void initLast( Data4 d ) { tag = d4tag_prev( d.data, NULL ) ; }
   void initNext( ) { if( isValid( ) ) tag = d4tag_next( tag->index->data, tag ) ; }
   void initPrev( ) { if( isValid( ) ) tag = d4tag_prev( tag->index->data, tag ) ; }
   void initSelected( Data4 d ) { tag = d4tag_selected( d.data ) ; }
   int isValid() { return tag!=0; } ;
   char * key() { return t4key( tag ) ;}
   // open() is only supported for N4OTHER
   int open( Data4 d, char *name ) { tag = t4open( d.data, NULL, name ); return isValid( );}           
   int open( Data4 d, Index4 i, char *name ) 
                       { tag = t4open( d.data, i.index, name) ; return isValid( ); }

   #ifdef __TURBOC__
      double position( ) { double d;
                           t4position2( tag, &d ) ;
                           return d; }
   #else
      double position( ) { return t4position( tag ) ; }
   #endif
   int position( double pos ) { return t4position_set( tag, pos ); }
   long recNo() { return t4recno( tag ) ; }
   int remove( void *key, long rec_num ) 
                    { return t4remove( tag, (char *)key, rec_num ); }
   int removeCalc( long rec_num ) { return t4remove_calc( tag, rec_num ) ;}
   int seek( void *key, int key_len ) { return t4seek( tag, key, key_len ) ;}
   long skip( long num_skip = 1L ) { return t4skip( tag, num_skip ) ;}
   int top() {return t4top( tag ) ; }
	void   uniqueError( int unique_code ) { tag->unique_error = unique_code ; }
   int uniqueError( ) { return t4unique( tag ) ; }
   int up() {return t4up( tag ) ;}
   int upToRoot() {return t4up_to_root( tag ) ;}
} ;

class S4CLASS Str4len ;

class S4CLASS Str4
{
public:
   #ifdef __ZTC__
      virtual operator char() ;
      virtual operator double() ;
      virtual operator int() ;
   #else
      operator char() ;
      operator double() ;
      operator int() ;
   #endif
   virtual operator long() ;
   char& operator[]( int index ) ;
   int  operator==( Str4& ) ;  /* Equal, including length */
   int  operator!=( Str4& s ) { return ! operator==(s) ; }
   int  operator< ( Str4& ) ;
   int  operator> ( Str4& ) ;
   int  operator>=( Str4& s ) { return ! operator<(s) ; }
   int  operator<=( Str4& s ) { return ! operator>(s) ; }
   int  add( Str4& ) ;
   int  add( char *ptr ) ;
   int  assign( char * ) ;
   int  assign( char *ptr, unsigned ptr_len ) ;
   int  assign( Str4& from ) ;
   void assignDouble( double, int new_len = -1, int n_dec = -1 ) ;
   void assignLong( long, int new_len = -1, int zeros_in_front = 0 ) ;
   // descend() only supported for S4CLIPPER
   void descend() { c4descend_str( ptr(), ptr(), len() ) ; }
   int  encode( char *, char *, char * ) ;
   char *endPtr() ;
   int  insert( Str4&, unsigned pos= 0 ) ;
	void lower() ;
   unsigned ncpy( char *to_ptr, unsigned to_len ) ;
   int  replace( Str4&, unsigned pos= 0 ) ;
   void set( int chr_value ) ;
   void trim() ;
   void upper() ;

   /* Get information */
   int    at( Str4& ) ;
   Str4len  left( unsigned l ) ;
   Str4len  right( unsigned l ) ;
   Str4len  substr( unsigned pos, unsigned len ) ;
   int    true() ;

   virtual void      changed()          {}
   virtual int       decimals()         { return 0 ; }
   virtual unsigned  len() ;
   virtual unsigned  maximum()          { return len() ; }
   virtual char     *ptr()  = 0 ;
   virtual char     *str() ;
   virtual int       setLen(unsigned)  { return -1 ; }
   virtual int       setMax(unsigned)  { return -1 ; }

   #ifdef S4OLD_CODE
      void   assign_double( double, int new_len = -1, int n_dec = -1 ) ;
      void   assign_long( long, int new_len = -1, int zeros_in_front = 0 ) ;
      virtual int       set_len(unsigned)  { return -1 ; }
      virtual int       set_max(unsigned)  { return -1 ; }
   #endif
} ;

class S4CLASS Str4char : public Str4
{
public:
   Str4char( char ch = 0 ) ; 
   char    *ptr()     { return &c ; }
   unsigned len()     { return 1 ; }

private:
   char c ;
} ;

class S4CLASS Str4ptr : public Str4
{
public:
   Str4ptr( char *ptr )   { p =   ptr ; }

   char *ptr()          { return p ; }
   char *p ;
} ;

class S4CLASS Str4len : public Str4ptr
{
public:
   Str4len( void *ptr, unsigned ptr_len ) : Str4ptr((char *)ptr)
                   { curLen =  ptr_len; }
   unsigned len()  { return curLen ; }
   unsigned curLen ;
} ;

class S4CLASS Str4max : public Str4len
{
public:
   Str4max( void *ptr, unsigned m ) : Str4len(ptr,m)  { maxLen = m; }

   unsigned maximum() { return maxLen ; }
   int      setLen( unsigned) ;

   unsigned maxLen ;
   #ifdef S4OLD_CODE
      int      set_len( unsigned m ) { return setLen( m )  ; }
      int      max_len() { return maxLen ; }
   #endif
} ;

/* The following classes  are always guaranteed to be null ended. */
class S4CLASS Str4ten : public Str4
{
public:
   Str4ten()          { curLen = 0; buf[0] = 0; buf[10] = 0; }
	Str4ten( char *p ) { curLen = 0; buf[10] = 0; assign(p); }
   Str4ten( Str4 &s ) { curLen = 0; buf[10]=0; assign(s); }

   unsigned maximum()      { return 10 ; }
   unsigned len()          { return curLen; }
   char    *ptr()          { return buf ; }
   int      setLen(unsigned new_len) ;

   unsigned curLen ;

   #ifdef S4OLD_CODE
      int      set_len( unsigned m ) { return setLen( m )  ; }
   #endif
private:
   char buf[12] ;
} ;

class S4CLASS Str4large: public Str4
{
public:
   Str4large() ;
   Str4large( char *p );
   Str4large( Str4 &s) ;

   unsigned maximum()     { return 255 ; }
   unsigned len()     { return curLen; }
   char    *ptr()     { return buf ; }
   int      setLen(unsigned len) ;

   unsigned curLen ;

   #ifdef S4OLD_CODE
      int      set_len( unsigned m ) { return setLen( m )  ; }
   #endif
private:
   char buf[256] ;
} ;

class S4CLASS Str4flex : public Str4max
{
public:
   Str4flex( Code4 & ) ;
   Str4flex( Str4flex& ) ;
  ~Str4flex() ;
   Str4flex &operator =( Str4flex &s ) { assign( s ) ; return s ;}


   void    free() ;
   int     setMax( unsigned ) ;
   char   *str()        { return ptr() ; }

   Code4 S4PTR *code_base ;
   #ifdef S4OLD_CODE
      int      set_max( unsigned m ) { return setMax( m )  ; }
   #endif
} ;

class S4CLASS Field4 : public Str4
{
public:
   Field4()                      { field =  0 ; }
   Field4( Data4 d, int j )      { field =  d4field_j( d.data, j ) ; }
   Field4( Data4 d, char *name ) { field =  d4field( d.data, name ) ; }

   void          assignField( Field4 f ) { f4assign_field( field, f.field ) ; }
   void          changed()   { field->data->record_changed = 1 ; }
   Data4         data()      { return Data4( field->data ) ; }
   int           decimals()  { return f4decimals(field) ; }
   int           init( Data4 d, char *name )  
                             { field = d4field( d.data, name ) ; 
                               return (field == 0) ? -1 : 0 ; }
   int           init( Data4 d, int j )  
                             { field = d4field_j( d.data, j ) ; 
                               return (field == 0) ? -1 : 0 ; }
   int           isValid() { return field!=0;}
   unsigned      len()       { return f4len(field) ; }
   char         *name()      { return f4name(field) ; }
   int           number()    { return d4field_number( field->data, name() ) ;}
   int           type()      { return f4type(field) ; }
   char         *ptr()       { return f4ptr(field) ; }

   FIELD4    S4PTR *field ;
} ;

class S4CLASS Field4info
{
public:
   Field4info( Code4 &) ;
   Field4info( Data4  ) ;

   ~Field4info() ;
   Field4info( Field4info &);  // Illegal operation, force link error
   operator =(Field4info&) ;   // Illegal operation, force link error
   int add( char *, char , int len = 0, int dec = 0 );
   int add( Field4 ) ;
   int add( Data4 ) ;
   int del( int ) ;
   int del( char *) ;
   void free( ) ;
   FIELD4INFO *fields( ) { return field ;} 
   int numFields() { return size ; } ;
private:
   Code4 S4PTR *code_base ;
   FIELD4INFO *field ;
   int size ;  
   unsigned length ;
} ;

class S4CLASS Tag4info
{
public:
   Tag4info( Code4 &) ;
   Tag4info( Data4 ) ;
   Tag4info( Index4 ) ;
   ~Tag4info() ;
   Tag4info(Tag4info&) ;    // Illegal action, force a link error
   operator =( Tag4info &) ;  // Illegal action, force a link error

   int add( char *, char *,char *filter = NULL, int uniq = 0, int desc = 0 );
   int add( Tag4 ) ;
	int del( int ) ;
   int del( char * ) ;
   TAG4INFO * tags( ) { return tag ; }
   void free( ) ;
   int numTags() { return size ; } ;
private:
   int add_index_tags( INDEX4 * ) ;
   Code4 S4PTR *code_base ;
   TAG4INFO *tag ;
   int size ;  
   unsigned length ;
} ;

class S4CLASS Field4memo : public Field4
{
public:
   Field4memo() {}
   Field4memo( Data4& data, int j ) ;
   Field4memo( Data4& data, char *name ) ;
   Field4memo( Field4 f ) ; 

   void changed() ;
   void free()       { f4memo_free(field) ; }
   unsigned len()    { return f4memo_len(field) ; }
   int  setLen(unsigned new_len) ;

   char *ptr()       { return f4memo_ptr(field) ; }
   char *str()       { return f4memo_str(field) ; }
   #ifdef S4OLD_CODE
      int      set_len( unsigned m ) { return setLen( m )  ; }
   #endif
} ;

class S4CLASS  Date4 : public Str4
{
public:
   Date4() ; 
   Date4( long l ) ;
   Date4( char *p) ; 
   Date4( char *p, char *pict ) ;

   operator long()           { return date4long(ptr()) ; }
   #ifdef __TURBOC__
      operator double()         { double d; 
                                  date4format_mdx2( ptr(), &d );
                                  return d; }
   #else
      operator double()         { return date4format_mdx( ptr() ); }
   #endif
   
   long operator +( long l ) { return (date4long(ptr())+l) ; }
   long operator -( long l ) { return (date4long(ptr())-l) ; }
   void operator +=( long l) { date4assign( ptr(), date4long(ptr())+l ) ; }
   void operator -=(long l ) { date4assign( ptr(), date4long(ptr())-l ) ; }
   long operator ++( )       { *this+=1L ; return long(*this) ; }
   long operator ++(int i)   { *this+=1L ; return long(*this)-1L ; }
   long operator --( )       { *this-=1L ; return long(*this) ; }
   long operator --(int i)   { *this-=1L ; return long(*this)+1L ; }
   void   assign(long l) { date4assign(ptr(),l) ; }
   void   assign( char *p ) { u4ncpy( ptr(), p, sizeof(dt) ) ; }
   void   assign( char *p, char *pict ) { date4init( ptr(), p, pict) ; }
   void   assign( Str4 &s) { Str4::assign(s) ; }
   char  *cdow()         { return date4cdow( ptr() ) ; }
   char  *cmonth()       { return date4cmonth(ptr()) ; }
   int    day()          { return date4day(ptr()) ; }    /* Day of month  (1-31) */
   // descend() only supported for S4CLIPPER
   void descend()        { c4descend_date( ptr(), date4long(ptr()), 8 ) ; }
   int    dow()          { return date4dow(ptr()) ; }    /* Day of week   (1-7) */
   void   format( char *result, char *pict ) { date4format( ptr(), result, pict ) ; }
   char  *format( char *pict ) ;
   int    isLeap() { long y = date4year(dt) ; 
                      return (y%4 == 0 && y%100 != 0 || y%400 == 0) ?  1 : 0 ; }
   int    month()   { return date4month(ptr()) ; }/* Month of year (1-12) */
   void   today()   { date4today(ptr()) ; }
   int    year()    { return date4year(ptr()) ; } ;

   unsigned  len()  { return 8 ; }
   char *ptr()      { return  dt ; }
   #ifdef S4OLD_CODE
      void   init( char *value, char *picture ) 
							  { date4init( ptr(), value, picture ) ; }
      long   julianDay()    { return date4long(ptr()) ; }
   #ifdef __TURBOC__
      double format_mdx()       { double d; 
                                  date4format_mdx2( ptr(), &d );
                                  return d; }
   #else
      double format_mdx()         { return date4format_mdx( ptr() ); }
   #endif
      long   julian_day()   { return date4long(ptr()) ; }
   #endif

private:
   char dt[9] ;
} ;


class S4CLASS File4 : public FILE4 
{
public:
   File4() { hand = -1 ; }
   File4( Code4 &code, char *name, int do_alloc = 0 ) { open( code, name, do_alloc ) ;}
   File4( File4& ) ;         /* Illegal operation, force link error */

   int close() { return file4close( this ) ; }
   int create( Code4 &code, char *name, int do_alloc = 0 ) 
                    { return file4create( this, &code, name, do_alloc ) ; }
   int createTemp( Code4 &code, char *buf = NULL, int auto_remove = 1 ) 
                    { return file4temp( this, &code, buf, auto_remove );}
   char *fileName() { return name ;}
   int flush() { return file4flush( this ) ;}
   int isValid( ) { return ( hand >= 0 ) ? 1 : 0 ;}
   long len() { return file4len( this ) ; }
   int setLen( long l) { return file4len_set( this, l ) ; }
   int lock( long start, long bytes ) { return file4lock( this, start, bytes); }

   int open( Code4 &code, char *name, int do_alloc = 0 )
                    { return file4open( this, &code, name, do_alloc ) ; }
   int optimize( int opt_flag, int file_type ) 
                    { return file4optimize( this, opt_flag, file_type ) ;}
	int optimizeWrite( int opt_flag )
                    { return file4optimize_write( this, opt_flag ) ; }
   unsigned read( long pos, void *ptr, unsigned len )
                    { return file4read( this, pos, ptr, len ) ; }
   unsigned read( long pos, Str4 &string ) 
                    { return read( pos, string.ptr(), string.len());}
   unsigned readAll( long pos, void *ptr, unsigned len )
                    { return file4read_all( this, pos, ptr, len) ;  }
   unsigned readAll( long pos, Str4 &string ) 
                    { return readAll( pos, string.ptr(), string.len() ) ;}
   int refresh() { return file4refresh( this ) ; }
   int replace( File4 &new_file ) { return file4replace( this, &new_file ) ;}
   int unlock( long pos_start, long num_bytes ) 
                    { return file4unlock (this, pos_start, num_bytes) ; }
   int write( long pos, void *ptr, unsigned len ) 
                    { return file4write( this, pos, ptr, len ) ;  }
   int write( long pos, char *nullended ) 
                    { return write( pos, nullended, strlen(nullended) ) ; }
   int write( long pos, Str4 &s ) { return write( pos, s.ptr( ), s.len( ) ) ;}
} ;
   
class S4CLASS File4seqRead : public FILE4SEQ_READ 
{
public:
  
   File4seqRead( File4 &f, long start_pos, void *buffer, unsigned buf_len )
              { file4seq_read_init( this, &f, start_pos, buffer, buf_len ) ;} 
   File4seqRead() {}

   File4seqRead& operator>>( Str4 &) ;

   void init( File4 &f, long start_pos, void *buffer, unsigned buf_len ) 
         { file4seq_read_init( this, &f, start_pos, buffer, buf_len ) ;}
   unsigned read( void *ptr, unsigned len ) 
                    { return file4seq_read( this, ptr, len ) ; }         
   unsigned read( Str4 &s ) 
                    { return file4seq_read( this, s.ptr(), s.len());}
   int readAll( void *ptr, unsigned len ) 
                    { return file4seq_read_all( this, ptr, len);}
   int readAll( Str4 &s ) 
						  { return file4seq_read_all( this, s.ptr(), s.len());}
} ;

class S4CLASS File4seqWrite : public FILE4SEQ_WRITE
{
public:
   File4seqWrite( File4 &f, long start_pos, void *buffer, unsigned buf_len )
       { file4seq_write_init( this, &f, start_pos, buffer, buf_len ) ; }
   File4seqWrite() {}
   File4seqWrite &operator<<( Str4 &buffer) { write( buffer ) ; return *this ;}
   File4seqWrite &operator<<( long ) ;
   File4seqWrite &operator<<( char *buffer ) { write( buffer, strlen(buffer)); return *this ;}


   void init( File4 &f, long start_pos, void *buffer, unsigned buf_len )
         { file4seq_write_init( this, &f, start_pos, buffer, buf_len ) ; }
   void flush() { file4seq_write_flush( this ) ;}
   int repeat( long num_repeat, char ch )
                    { return file4seq_write_repeat( this, num_repeat, ch);}
   int write( void *info, unsigned info_len )
                    { return file4seq_write( this, info, info_len ) ;}
   int write( char *info )
                    { return file4seq_write( this, info, strlen(info));}
   int write( Str4 &s )
                    { return file4seq_write( this, s.ptr(), s.len()) ;}
} ;

class S4CLASS List4 : public LIST4  
{
public:
   List4() { reset() ; }
   List4( List4 &) ; // illegal operation, force link error
   operator =( List4 &); // illegal operation, force link error

   void *add( void *item ) { l4add( this, item ) ;return item ;}
   void *addAfter( void *anchor, void *item ) 
                    { l4add_after( this, anchor, item ) ; return item; }
   void *addBefore( void *anchor, void *item ) 
                    { l4add_before( this, anchor, item ) ; return item;}
   void *first() { return l4first( this ) ;}
	void *last() { return last_node ;}
   void *next( void *item = NULL ) { return l4next( this, item ) ;}
   int   numNodes() { return n_link ; }
   void *pop() { return l4pop( this ) ;}
   void *prev( void *item) { return l4prev( this, item ) ; }
   void remove( void *item ) { l4remove( this, item ) ; } 
   void reset() { last_node = 0 ; n_link = 0 ; }
} ;
 
class S4CLASS Sort4 : public SORT4
{
public:
   Sort4() {} ;
   Sort4( Code4 &code, int sort_len, int other_len = 0 ) 
         { sort4init( this, &code, sort_len, other_len ) ;}
   Sort4( Sort4 &) ;      // Illegal operation, force link error
   operator =( Sort4&) ;  // Illegal operation, force link error
   void *result, *resultOther ;
   long  resultRec ;

   #ifdef S4WINDOWS
      #ifndef _MSC_VER
       //  void assignCmp(S4CMP_FUNCTION *f) { sort4assignCmp( this, f ) ;}
      #endif
   #else
      void assignCmp(S4CMP_FUNCTION *f) { sort4assignCmp( this, f ) ;}
   #endif
   int free() { return sort4free( this ) ;}
   int get() { return sort4get( this, &resultRec, &result, &resultOther ) ;}
   int getInit() { return sort4get_init( this ) ;}
   int init( Code4 &code, int sort_len, int other_len = 0 ) 
         { return sort4init( this, &code, sort_len, other_len ) ;}
   int put( void *sort_info, void *other_info = NULL, long rec = 0L )
         { return sort4put( this, rec, sort_info, other_info ) ;}

};

class S4CLASS Mem4
{
public:
	Mem4() { mem = 0; }
   Mem4( Code4 &code, int start, int size, int expand, int temp = 1 )
          { mem = mem4create( &code, start, size, expand, temp ) ;}
   Mem4( MEM4 S4PTR *m ) { this->mem = m ; }
   MEM4 S4PTR *mem ;
   void *alloc()  { return mem4alloc( mem ) ;}
   int  create( Code4 &code, int units_start, int unit_size, int units_expand, int make_temp = 1 )
          { mem = mem4create( &code, units_start, unit_size, units_expand, make_temp ) ;
            return (mem==0) ? -1 : 0; }
   void free( void *ptr ) { mem4free( mem, ptr ) ;}
   int isValid( ) { return mem!=0 ;}
   void release() { mem4release(mem) ;}
} ;

class S4CLASS Relate4
{
public:
   Relate4() { relate = 0 ; }
   Relate4( Relate4 master, Data4 slave, char *m_expr, Tag4 t )
		    { relate = relate4create_slave( master.relate, slave.data, m_expr, t.tag ) ; }
   Relate4( Relate4 *master, Data4 *slave, char *m_expr, TAG4 *t)
			 { relate = relate4create_slave( master->relate, slave->data, m_expr, t ) ; }

   Relate4( RELATE4 S4PTR *r ) { relate = r ; } 
   Data4 data( ) { return (relate) ? Data4((DATA4 *)relate->data) : Data4((DATA4 *)NULL);}
   Tag4 dataTag( ) { return (relate) ? Tag4((TAG4 *)relate->data_tag) : Tag4((TAG4 *)NULL);}
   int doOne( )         { return relate4do_one( relate ) ; }
   int errorAction( int a ) { return relate4error_action( relate, a ) ; }
   int init( Relate4 master, Data4 slave, char *m_expr, Tag4 t )
                    { relate = relate4create_slave( master.relate, slave.data, m_expr, t.tag ) ;
                      return slave.data->code_base->errorCode ; }

   int isValid( ) { return relate != NULL ; } 
   
   Relate4 master() { return (relate) ? Relate4((RELATE4 *)relate->master):Relate4((RELATE4 *)NULL) ;}
   Expr4 masterExpr() { return (relate)?Expr4((EXPR4 *)relate->master_expr):Expr4((EXPR4 *)NULL);}
   int matchLen( int p ) { return relate4match_len( relate, p ) ;}
   int type( int p )    { return relate4type( relate, p ) ; }
   
   RELATE4 S4PTR *relate ;
} ;

class S4CLASS Relate4set : public Relate4
{
public:
   Relate4set( Data4 data) { relate = relate4init( data.data ) ; }
   Relate4set( Data4 *data) { relate = relate4init( data->data ) ; }
   Relate4set()          {relate = 0;}
   Relate4set( RELATE4 S4PTR *r ) { relate = r ; }

   int bottom()              { return relate4bottom( relate ) ; }
   void changed()            { relate4changed( relate ) ; }
   int doAll()              { return relate4do( relate ) ; }
   int free( int p = 0 )     { int rc = relate4free( relate, p ) ; relate = 0; return rc; }
   int init( Data4 data )   { relate = relate4init( data.data ) ; return data.data->code_base->errorCode ; }
   int lock()                { return relate4lock( relate ) ; }
   int querySet( char *p = NULL )  { return relate4query_set( relate, p ) ; }
   int skip( long l = 1)     { return relate4skip( relate, l ) ; }
   int skipEnable( int do_enable = 1 ) { return relate4skip_enable( relate, do_enable ) ; }
   int sortSet( char *p = NULL )   { return relate4sort_set( relate, p ) ; }
   int top()                 { return relate4top( relate ) ; }
   int unlock()              { return relate4unlock( relate ) ; }
} ;

class Relate4iterator : public Relate4
{
public:
   Relate4iterator( ) { relate = 0 ; }
   Relate4iterator( Relate4 r ) { relate = r.relate ; }
   Relate4 operator =( Relate4 r ) { relate = r.relate; return r; } 
   int next( )  { return relate4next( &relate ) != 2 ; }
   int nextPosition( )  { return relate4next( &relate ) ; }
} ;


inline Expr4::Expr4( Data4 d, char *ex )
{ 
   expr = expr4parse( d.data, ex ) ; 
}

inline Data4 Code4::data( char *alias )
{
   return Data4( d4data( this, alias ) ) ;
}

inline char * Expr4::key( )
{
   char *r ;
   expr4key( expr, &r); 
   return r ;
}

inline char * Expr4::vary( )
{
   char *r ;
   expr4vary( expr, &r); 
   return  r ;
}

inline int Expr4::parse( Data4 d, char *ex )
{ 
   #ifdef S4DEBUG
      if( expr )
         e4( d.data->code_base, e4info, "Expr4::parse called before Expr4::free" ) ;
   #endif
   expr = expr4parse( d.data, ex ) ;
   return d.data->code_base->errorCode ;
}

inline Index4 Data4::index( char *name ) 
{ 
   return Index4( d4index(data, name)) ; 
}

inline void Data4::select( Tag4 tag ) 
{
   d4tag_select( data, tag.tag ) ; 
}

inline int Field4memo::setLen( unsigned new_len )
{
   #ifdef S4OFF_MEMO
      return e4not_memo ;
   #else
      return f4memo_set_len(field,new_len) ; 
   #endif
}

#endif
