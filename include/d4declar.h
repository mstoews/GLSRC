/* d4declar.h   (c)Copyright Sequiter Software Inc., 1994.  All rights reserved. */

#ifdef __cplusplus
   extern "C" {
#endif

void   S4FUNCTION date4assign( char S4PTR *, long ) ;
long   S4FUNCTION date4long( char S4PTR * ) ;  /* Date Arithmetic */
char S4PTR * S4FUNCTION date4cdow( char S4PTR * ) ;
char S4PTR * S4FUNCTION date4cmonth( char S4PTR * ) ;
int    S4FUNCTION date4day( char S4PTR * ) ;
int    S4FUNCTION date4dow( char S4PTR * ) ;
void   S4FUNCTION date4format( char S4PTR *, char S4PTR *, char S4PTR * ) ;/* 'dt' may be 'result'*/
double S4FUNCTION date4format_mdx( char S4PTR * ) ;
int    S4FUNCTION date4format_mdx2( char S4PTR * , double S4PTR * ) ;
void   S4FUNCTION date4init( char S4PTR *, char S4PTR *, char S4PTR * ) ;
int    S4FUNCTION date4month( char S4PTR * ) ;
void   S4FUNCTION date4today( char S4PTR * ) ;
int    S4FUNCTION date4year( char S4PTR * ) ;
void   S4FUNCTION date4time_now( char S4PTR * ) ;

int    S4FUNCTION b4skip( B4BLOCK S4PTR *, long ) ;
#ifdef S4INDEX_VERIFY
   int S4FUNCTION b4verify( B4BLOCK *b4 ) ;
#endif
#ifdef S4NDX
   int S4FUNCTION b4get_last_key( B4BLOCK S4PTR *,char S4PTR * ) ;
   int S4FUNCTION i4get_last_key( TAG4 S4PTR *,char S4PTR *, long ) ;
#endif

int    S4FUNCTION expr4start( CODE4 S4PTR * ) ;

TAG4  S4PTR *S4FUNCTION t4open( DATA4 S4PTR *, INDEX4 S4PTR *, char S4PTR * ) ;

#ifdef S4FOX
int    S4FUNCTION b4calc_blanks( char S4PTR *, int, char ) ;
int    S4FUNCTION b4calc_dups( char S4PTR *, char S4PTR *, int ) ;
void   S4FUNCTION b4go( B4BLOCK S4PTR *, int ) ;
int    S4FUNCTION b4insert_leaf( B4BLOCK S4PTR *, void S4PTR *, long ) ;
int    S4FUNCTION b4insert_branch( B4BLOCK S4PTR *, void S4PTR *, long, long, char ) ;
void   S4FUNCTION b4leaf_init( B4BLOCK S4PTR * ) ;
int    S4FUNCTION b4leaf_seek( B4BLOCK S4PTR *, char S4PTR *, int ) ;
int    S4FUNCTION b4reindex( B4BLOCK S4PTR * ) ;
void   S4FUNCTION b4remove_leaf( B4BLOCK S4PTR * ) ;
void   S4FUNCTION b4br_replace( B4BLOCK S4PTR *, char S4PTR *, long ) ;
void   S4FUNCTION b4top( B4BLOCK S4PTR * ) ;
int    S4CALL     t4desc_memcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
void   S4FUNCTION t4branch_split( TAG4 S4PTR *, B4BLOCK S4PTR *, B4BLOCK S4PTR * ) ;
void   S4FUNCTION t4leaf_split( TAG4 S4PTR *, B4BLOCK S4PTR *, B4BLOCK S4PTR * ) ;
int    S4FUNCTION t4r_seek( TAG4 S4PTR *, void S4PTR *, int, long ) ;
int    S4FUNCTION b4r_brseek( B4BLOCK S4PTR *, char S4PTR *, int, long ) ;
long   S4FUNCTION x4recno( B4BLOCK S4PTR *, int ) ;
int    S4FUNCTION x4dup_cnt( B4BLOCK S4PTR *, int ) ;
int    S4FUNCTION x4trail_cnt( B4BLOCK S4PTR *, int ) ;
void   S4FUNCTION x4put_info( B4NODE_HEADER S4PTR *, void S4PTR *, long, int, int ) ;
int    S4FUNCTION b4insert( B4BLOCK S4PTR *, void S4PTR *, long, long, char ) ;
int    S4FUNCTION t4init( TAG4 S4PTR *, INDEX4 S4PTR *, long, char S4PTR *) ;
#else
   #ifdef N4OTHER
      void   S4FUNCTION b4append( B4BLOCK S4PTR *, long ) ;
      void   S4FUNCTION b4insert( B4BLOCK S4PTR *, void S4PTR *, long, long ) ;
      #ifdef S4CLIPPER
         int    S4FUNCTION c4clip( char S4PTR *, int ) ;
         void   S4FUNCTION b4append2( B4BLOCK S4PTR *, void S4PTR *, long, long ) ;
         int    S4FUNCTION t4get_replace_entry( TAG4 S4PTR *, B4KEY_DATA S4PTR *, B4BLOCK S4PTR * ) ;
         int    S4FUNCTION t4shrink( TAG4 S4PTR *, long ) ;
      #endif
      int    S4FUNCTION b4room( B4BLOCK S4PTR * ) ;
      int    S4FUNCTION t4close( TAG4 S4PTR * ) ;
      long   S4FUNCTION t4extend( TAG4 S4PTR * ) ;
      int    S4FUNCTION t4lock( TAG4 S4PTR * ) ;
      int    S4FUNCTION t4unlock( TAG4 S4PTR * ) ;
      int    S4FUNCTION t4update_header( TAG4 S4PTR * ) ;
      int    S4FUNCTION t4do_version_check( TAG4 S4PTR *, int, int ) ;
      TAG4  S4PTR *S4FUNCTION t4create( DATA4 S4PTR *, TAG4INFO S4PTR *, INDEX4 S4PTR * ) ;
      int    S4FUNCTION t4reindex( TAG4 S4PTR * ) ;
      void   S4FUNCTION t4remove_branch( TAG4 S4PTR *, B4BLOCK S4PTR * ) ;
   #else
      void   S4FUNCTION b4insert( B4BLOCK S4PTR *, void S4PTR *, long ) ;
      int    S4FUNCTION t4init( TAG4 S4PTR *, INDEX4 S4PTR *, T4DESC S4PTR * ) ;
   #endif
#endif

B4BLOCK S4PTR *S4FUNCTION b4alloc( TAG4 S4PTR *, long ) ;
int    S4FUNCTION b4flush( B4BLOCK S4PTR * ) ;
void   S4FUNCTION b4free( B4BLOCK S4PTR * ) ;
void   S4FUNCTION b4go_eof( B4BLOCK S4PTR * ) ;
B4KEY_DATA S4PTR * S4FUNCTION b4key( B4BLOCK S4PTR *, int ) ;
unsigned char S4PTR * S4FUNCTION b4key_key( B4BLOCK S4PTR *, int ) ;
int    S4FUNCTION b4lastpos( B4BLOCK S4PTR * ) ;
int    S4FUNCTION b4leaf( B4BLOCK S4PTR * ) ;
long   S4FUNCTION b4recno( B4BLOCK S4PTR *, int ) ;
void   S4FUNCTION b4remove( B4BLOCK S4PTR *);
int    S4FUNCTION b4seek( B4BLOCK S4PTR *, char S4PTR *, int ) ;

double S4FUNCTION c4atod( char S4PTR *, int) ;
void   S4FUNCTION c4atod2( char S4PTR *, int, double S4PTR * ) ;
int    S4FUNCTION c4atoi( char S4PTR *, int) ;
long   S4FUNCTION c4atol( char S4PTR *, int) ;

void   S4FUNCTION c4dtoa_clipper( double, char S4PTR *, int, int ) ;
void   S4FUNCTION c4dtoa45( double, char S4PTR *, int, int) ;
void   S4FUNCTION c4encode( char S4PTR *, char S4PTR *, char S4PTR *, char S4PTR *) ;
void   S4FUNCTION c4lower( char S4PTR * ) ;
void   S4FUNCTION c4ltoa45( long, char S4PTR *, int) ;
void   S4FUNCTION c4trim_n( char S4PTR *, int ) ;
void   S4FUNCTION c4upper( char S4PTR * ) ;

CODE4 S4PTR * S4FUNCTION code4alloc( int ) ;

#ifdef S4VBASIC
   int c4parm_check( void S4PTR *, int, char S4PTR * ) ;
#endif

int    S4FUNCTION d4changed( DATA4 S4PTR *, int ) ;
char S4PTR * S4FUNCTION d4alias( DATA4 S4PTR * ) ;
void   S4FUNCTION d4alias_set( DATA4 S4PTR *, char S4PTR * ) ;
int    S4FUNCTION d4append( DATA4 S4PTR * ) ;
int    S4FUNCTION d4append_data( DATA4 S4PTR * ) ;
int    S4FUNCTION d4append_blank( DATA4 S4PTR * ) ;
int    S4FUNCTION d4append_start( DATA4 S4PTR *, int ) ;
void   S4FUNCTION d4blank( DATA4 S4PTR * ) ;
int    S4FUNCTION d4bof( DATA4 S4PTR * ) ;
int    S4FUNCTION d4bottom( DATA4 S4PTR * ) ;
int    S4FUNCTION d4check( DATA4 S4PTR * ) ;
int    S4FUNCTION d4close( DATA4 S4PTR * ) ;
int    S4FUNCTION d4close_all( CODE4 S4PTR * ) ;
DATA4 S4PTR * S4FUNCTION d4create( CODE4 S4PTR *, char S4PTR *, FIELD4INFO S4PTR *, TAG4INFO S4PTR * ) ;
DATA4 S4PTR * S4FUNCTION d4data( CODE4 S4PTR *, char S4PTR * ) ;
void   S4FUNCTION d4delete( DATA4 S4PTR * ) ;
int    S4FUNCTION d4deleted( DATA4 S4PTR * ) ;
int    S4FUNCTION d4eof( DATA4 S4PTR * ) ;
FIELD4 S4PTR * S4FUNCTION d4field( DATA4 S4PTR *, char S4PTR * ) ;
FIELD4INFO S4PTR * S4FUNCTION d4field_info( DATA4 S4PTR * ) ;
FIELD4 S4PTR * S4FUNCTION d4field_j( DATA4 S4PTR *, int ) ;
int    S4FUNCTION d4field_number( DATA4 S4PTR *, char S4PTR * ) ;
int    S4FUNCTION d4flush( DATA4 S4PTR * ) ;
int    S4FUNCTION d4flush_data( DATA4 S4PTR * ) ;
int    S4FUNCTION d4flush_files( CODE4 S4PTR * ) ;
int    S4FUNCTION d4free_blocks( DATA4 S4PTR * ) ;
int    S4FUNCTION d4go( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4go_data( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4go_eof( DATA4 S4PTR * ) ;
INDEX4 S4PTR * S4FUNCTION d4index( DATA4 S4PTR *, char S4PTR * ) ;
void   S4FUNCTION d4init( CODE4 S4PTR * ) ;
int    S4FUNCTION d4init_undo( CODE4 S4PTR * ) ;
int    S4FUNCTION d4lock( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4lock_all( DATA4 S4PTR * ) ;
int    S4FUNCTION d4lock_append( DATA4 S4PTR * ) ;
int    S4FUNCTION d4lock_file( DATA4 S4PTR * ) ;
int    S4FUNCTION d4lock_index( DATA4 S4PTR * ) ;
int    S4FUNCTION d4lock_group( DATA4 S4PTR *, long S4PTR *, int ) ;
int    S4FUNCTION d4lock_test( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4lock_test_file( DATA4 S4PTR * ) ;
int    S4FUNCTION d4lock_test_append( DATA4 S4PTR * ) ;
int    S4FUNCTION d4lock_test_index( DATA4 S4PTR * ) ;
int    S4FUNCTION d4memo_compress( DATA4 S4PTR * ) ;
int    S4FUNCTION d4num_fields( DATA4 S4PTR * ) ;
DATA4 S4PTR *S4FUNCTION d4open( CODE4 S4PTR *, char S4PTR * ) ;
int    S4FUNCTION d4opt_restart( CODE4 S4PTR * ) ;
int    S4FUNCTION d4opt_start( CODE4 S4PTR * ) ;
int    S4FUNCTION d4opt_suspend( CODE4 S4PTR * ) ;
int    S4FUNCTION d4optimize( DATA4 S4PTR *, int ) ;
int    S4FUNCTION d4optimize_write( DATA4 S4PTR *, int ) ;
int    S4FUNCTION d4pack( DATA4 S4PTR * ) ;
int    S4FUNCTION d4pack_data( DATA4 S4PTR * ) ;
int    S4FUNCTION d4position2( DATA4 S4PTR *, double S4PTR * ) ;
double S4FUNCTION d4position( DATA4 S4PTR * ) ;
int    S4FUNCTION d4position_set( DATA4 S4PTR *, double ) ;
int    S4FUNCTION d4read( DATA4 S4PTR *, long, char S4PTR * ) ;
int    S4FUNCTION d4read_old( DATA4 S4PTR *, long ) ;
void   S4FUNCTION d4recall( DATA4 S4PTR * ) ;
long   S4FUNCTION d4reccount( DATA4 S4PTR * ) ;
long   S4FUNCTION d4recno( DATA4 S4PTR * ) ;
char  S4PTR *S4FUNCTION d4record( DATA4 S4PTR * ) ;
long   S4FUNCTION d4record_position( DATA4 S4PTR *, long ) ;
long   S4FUNCTION d4record_width( DATA4 S4PTR * ) ;
int    S4FUNCTION d4refresh( DATA4 S4PTR * ) ;
int    S4FUNCTION d4refresh_record( DATA4 S4PTR * ) ;
int    S4FUNCTION d4reindex( DATA4 S4PTR * ) ;
int    S4FUNCTION d4seek( DATA4 S4PTR *, char S4PTR * ) ;
int    S4FUNCTION d4seek_double( DATA4 S4PTR *, double ) ;
int    S4FUNCTION d4seek_n( DATA4 S4PTR *, char S4PTR *, int ) ;
int    S4FUNCTION d4skip( DATA4 S4PTR *, long ) ;
TAG4 S4PTR *S4FUNCTION d4tag( DATA4 S4PTR *, char S4PTR * ) ;
TAG4 S4PTR *S4FUNCTION d4tag_default( DATA4 S4PTR * ) ;
TAG4 S4PTR *S4FUNCTION d4tag_next( DATA4 S4PTR *, TAG4 S4PTR * ) ;
TAG4 S4PTR *S4FUNCTION d4tag_prev( DATA4 S4PTR *, TAG4 S4PTR * ) ;
void   S4FUNCTION d4tag_select( DATA4 S4PTR *, TAG4 S4PTR * ) ;
TAG4 S4PTR *S4FUNCTION d4tag_selected( DATA4 S4PTR * ) ;
int    S4FUNCTION d4top( DATA4 S4PTR * ) ;
int    S4FUNCTION d4unlock( DATA4 S4PTR * ) ;
int    S4FUNCTION d4unlock_data( DATA4 S4PTR * ) ;
int    S4FUNCTION d4unlock_files( CODE4 S4PTR * ) ;
int    S4FUNCTION d4unlock_append( DATA4 S4PTR * ) ;
int    S4FUNCTION d4unlock_file( DATA4 S4PTR * ) ;
int    S4FUNCTION d4unlock_index( DATA4 S4PTR * ) ;
int    S4FUNCTION d4unlock_records( DATA4 S4PTR * ) ;
int    S4FUNCTION d4update( DATA4 S4PTR * ) ;
int    S4FUNCTION d4update_header( DATA4 S4PTR *, int, int ) ;
int    S4FUNCTION d4update_record( DATA4 S4PTR *, int ) ;
int    S4FUNCTION d4validate_memo_ids( DATA4 S4PTR * ) ;
int    S4FUNCTION d4write( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4write_data( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4write_keys( DATA4 S4PTR *, long ) ;
int    S4FUNCTION d4zap( DATA4 S4PTR *, long, long ) ;
int    S4FUNCTION d4zap_data( DATA4 S4PTR *, long, long ) ;

void   S4FUNCTION e4exit(CODE4 S4PTR *) ;
void   S4FUNCTION e4exit_test( CODE4 S4PTR * ) ;

int    S4FUNCTION e4( CODE4 S4PTR *, int, char S4PTR * ) ;
int    S4FUNCTION e4code( CODE4 S4PTR * ) ;
int    S4FUNCTION e4describe( CODE4 S4PTR * ,int, char S4PTR *, char S4PTR *, char S4PTR * ) ;
void   S4FUNCTION e4hook( CODE4 S4PTR *, int, char S4PTR *, char S4PTR *, char S4PTR * ) ;
int    S4FUNCTION e4set( CODE4 S4PTR *, int ) ;
void   S4FUNCTION e4severe( int, char S4PTR * ) ;
char * S4FUNCTION e4text( int err_code ) ;
#ifdef S4VBASIC
   void   S4FUNCTION  e4severe_vbasic( int, char S4PTR * ) ;
#endif

void   S4FUNCTION f4assign( FIELD4 S4PTR *, char S4PTR * ) ;
void   S4FUNCTION f4assign_char( FIELD4 S4PTR *, int ) ;
void   S4FUNCTION f4assign_double( FIELD4 S4PTR *, double ) ;
void   S4FUNCTION f4assign_field( FIELD4 S4PTR *, FIELD4 S4PTR * ) ;
void   S4FUNCTION f4assign_int( FIELD4 S4PTR *, int ) ;
void   S4FUNCTION f4assign_long( FIELD4 S4PTR *, long ) ;
void   S4FUNCTION f4assign_n( FIELD4 S4PTR *, char S4PTR *, unsigned ) ;
char S4PTR * S4FUNCTION f4assign_ptr( FIELD4 S4PTR * ) ;
void   S4FUNCTION f4blank( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4char( FIELD4 S4PTR * ) ;
DATA4 S4PTR *S4FUNCTION f4data( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4decimals( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4double2( FIELD4 S4PTR *, double S4PTR * ) ;
double S4FUNCTION f4double( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4int( FIELD4 S4PTR * ) ;
unsigned  S4FUNCTION f4len( FIELD4 S4PTR * ) ;
long   S4FUNCTION f4long( FIELD4 S4PTR * ) ;
char S4PTR * S4FUNCTION f4name( FIELD4 S4PTR * ) ;
unsigned S4FUNCTION f4ncpy( FIELD4 S4PTR *, char S4PTR *, unsigned ) ;
char S4PTR * S4FUNCTION f4ptr( FIELD4 S4PTR * ) ;
char S4PTR * S4FUNCTION f4str( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4true( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4type( FIELD4 S4PTR * ) ;

int    S4FUNCTION file4close( FILE4 S4PTR * ) ;
int    S4FUNCTION file4create( FILE4 S4PTR *, CODE4 S4PTR *, char S4PTR *, int ) ;
int    S4FUNCTION file4flush( FILE4 S4PTR * ) ;
int    S4FUNCTION file4low_flush( FILE4 S4PTR *, int ) ;
long   S4FUNCTION file4len( FILE4 S4PTR * ) ;
int    S4FUNCTION file4len_set( FILE4 S4PTR *, long ) ;
int    S4FUNCTION file4lock( FILE4 S4PTR *, long, long ) ;
int    S4FUNCTION file4lock_hook( CODE4 S4PTR *cb, char S4PTR *file_name, long offset, long num_bytes, int num_tries ) ;
int    S4FUNCTION file4open( FILE4 S4PTR *, CODE4 S4PTR *, char S4PTR *, int ) ;
int    S4FUNCTION file4open_test( FILE4 S4PTR * ) ;
int    S4FUNCTION file4optimize( FILE4 S4PTR *, int, int ) ;
int    S4FUNCTION file4optimize_write( FILE4 S4PTR *, int ) ;
unsigned  S4FUNCTION file4read( FILE4 S4PTR *, long, void S4PTR *, unsigned ) ;
int    S4FUNCTION file4read_all( FILE4 S4PTR *, long, void S4PTR *, unsigned ) ;
int    S4FUNCTION file4read_error( FILE4 S4PTR * ) ;
int    S4FUNCTION file4refresh( FILE4 S4PTR * ) ;
int    S4FUNCTION file4replace( FILE4 S4PTR *, FILE4 S4PTR * ) ;
int    S4FUNCTION file4temp( FILE4 S4PTR *, CODE4 S4PTR *, char S4PTR *, int ) ;
int    S4FUNCTION file4unlock( FILE4 S4PTR *, long, long ) ;
int    S4FUNCTION file4write( FILE4 S4PTR *, long, void S4PTR *, unsigned ) ;
int    S4FUNCTION file4change_size( FILE4 *, long ) ;

void   S4FUNCTION file4seq_read_init( FILE4SEQ_READ S4PTR *, FILE4 S4PTR *, long, void S4PTR *, unsigned ) ;
unsigned  S4FUNCTION file4seq_read( FILE4SEQ_READ S4PTR *, void S4PTR *, unsigned ) ;
int    S4FUNCTION file4seq_read_all( FILE4SEQ_READ S4PTR *, void S4PTR *, unsigned ) ;

void   S4FUNCTION file4seq_write_init( FILE4SEQ_WRITE S4PTR *, FILE4 S4PTR *, long, void S4PTR *, unsigned ) ;
int    S4FUNCTION file4seq_write( FILE4SEQ_WRITE S4PTR *, void S4PTR *, unsigned ) ;
int    S4FUNCTION file4seq_write_flush( FILE4SEQ_WRITE S4PTR * ) ;
int    S4FUNCTION file4seq_write_repeat( FILE4SEQ_WRITE S4PTR *, long, char ) ;

int    S4FUNCTION i4add_tag( INDEX4 S4PTR *, TAG4INFO S4PTR * ) ;
int    S4FUNCTION i4check( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4close( INDEX4 S4PTR * ) ;
INDEX4 S4PTR *S4FUNCTION i4create( DATA4 S4PTR *, char S4PTR *, TAG4INFO S4PTR * ) ; /* 0 name -> productn */
long   S4FUNCTION i4extend( INDEX4 S4PTR * ) ;   /* Allocates a block at the end of the file */
int    S4FUNCTION i4flush( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4is_production( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4lock( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4lock_test( INDEX4 S4PTR * ) ;
#ifdef N4OTHER
int    S4FUNCTION t4lock_test( TAG4 S4PTR * ) ;
#endif
INDEX4 S4PTR *S4FUNCTION i4open( DATA4 S4PTR *, char S4PTR * ) ;
int    S4FUNCTION i4read_block( FILE4 S4PTR *, long, B4BLOCK S4PTR *, B4BLOCK S4PTR * ) ;
int    S4FUNCTION i4reindex( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4shrink( INDEX4 S4PTR *, long ) ;  /* Returns a block of disk space */
TAG4 S4PTR *S4FUNCTION i4tag( INDEX4 S4PTR *, char S4PTR * ) ;
TAG4INFO *S4FUNCTION i4tag_info( INDEX4 * ) ;
int    S4FUNCTION i4unlock( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4update( INDEX4 S4PTR * ) ;
int    S4FUNCTION i4update_header( INDEX4 S4PTR * ) ;
int    S4FUNCTION t4version_check( TAG4 S4PTR *, int, int ) ;
int    S4FUNCTION i4version_check( INDEX4 S4PTR *, int, int ) ;

void   S4FUNCTION l4add( LIST4 S4PTR *, void S4PTR * ) ;
void   S4FUNCTION l4add_after(   LIST4 S4PTR *, void S4PTR *, void S4PTR * ) ;
void   S4FUNCTION l4add_before( LIST4 S4PTR *, void S4PTR *, void S4PTR * ) ;
void   l4check( LIST4 S4PTR * ) ;
void S4PTR * S4FUNCTION l4first( LIST4 S4PTR * ) ;  /* Returns 0 if none */
void S4PTR * S4FUNCTION l4last( LIST4 S4PTR * ) ;   /* Returns 0 if none */
void S4PTR * S4FUNCTION l4next( LIST4 S4PTR *, void S4PTR * ) ;  /* Returns 0 if none */
void S4PTR * S4FUNCTION l4prev( LIST4 S4PTR *, void S4PTR * ) ;
void S4PTR * S4FUNCTION l4pop( LIST4 S4PTR * ) ;
void   S4FUNCTION l4remove( LIST4 S4PTR *, void S4PTR * ) ;
int    S4FUNCTION l4seek( LIST4 S4PTR *, void S4PTR * ) ;

int    S4FUNCTION l4lock_check( void ) ;
void   S4FUNCTION l4lock_remove( int,long,long) ;
void   S4FUNCTION l4lock_save( int, long, long ) ;

#ifndef S4MFOX
#ifndef S4MNDX
int    memo4file_chain_flush( MEMO4FILE S4PTR *, MEMO4CHAIN_ENTRY S4PTR * ) ;
int    memo4file_chain_skip( MEMO4FILE S4PTR *, MEMO4CHAIN_ENTRY S4PTR * ) ;
int    S4FUNCTION f4memo_check( MEMO4FILE S4PTR * ) ;
#endif  /*  ifndef S4MFOX  */
#endif  /*  ifndef S4MNDX  */

int    memo4file_check( MEMO4FILE S4PTR * ) ;
int    memo4file_create( MEMO4FILE S4PTR *, CODE4 S4PTR *, DATA4 S4PTR *, char S4PTR * );
          /* if (name == 0), it is a temporary file */
int    memo4file_dump( MEMO4FILE S4PTR *, long, char S4PTR *, unsigned ) ;
int    memo4file_open( MEMO4FILE S4PTR *, DATA4 S4PTR *, char S4PTR * ) ;
int    memo4file_read( MEMO4FILE S4PTR *, long , char S4PTR * S4PTR *, unsigned S4PTR * ) ;
int    memo4file_read_part( MEMO4FILE S4PTR *, long , char S4PTR * S4PTR *, unsigned S4PTR *, unsigned long, unsigned ) ;
int    memo4file_write( MEMO4FILE S4PTR *, long S4PTR *, char S4PTR *, unsigned ) ;
int    memo4file_write_part( MEMO4FILE S4PTR *, long S4PTR *, char S4PTR *, long, long, unsigned ) ;

int    S4FUNCTION f4memo_assign( FIELD4 S4PTR *, char S4PTR * ) ;
int    S4FUNCTION f4memo_assign_n( FIELD4 S4PTR *, char S4PTR *, unsigned ) ;
unsigned S4FUNCTION f4memo_len( FIELD4 S4PTR * ) ;
unsigned S4FUNCTION f4memo_ncpy( FIELD4 S4PTR *, char S4PTR *, unsigned ) ;
int    S4FUNCTION f4memo_set_len( FIELD4 S4PTR *, unsigned ) ;
char   S4PTR * S4FUNCTION f4memo_str( FIELD4 S4PTR * ) ;
char   S4PTR * S4FUNCTION f4memo_ptr( FIELD4 S4PTR * ) ;

int    S4FUNCTION f4memo_flush( FIELD4 S4PTR * ) ;
void   S4FUNCTION f4memo_free( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4memo_read( FIELD4 S4PTR * ) ;       /* Validates memo id's first */
int    S4FUNCTION f4memo_read_low( FIELD4 S4PTR * ) ;   /* Assumes the current memo id is valid */
void   S4FUNCTION f4memo_reset( FIELD4 S4PTR * ) ;      /* Resets to 'Unknown state' */
int    S4FUNCTION f4memo_update( FIELD4 S4PTR * ) ;
int    S4FUNCTION f4memo_write( FIELD4 S4PTR * ) ;
int    S4FUNCTION memo4file_lock( MEMO4FILE S4PTR * ) ;
int    S4FUNCTION memo4file_unlock( MEMO4FILE S4PTR * ) ;
long   S4FUNCTION memo4len_part( MEMO4FILE S4PTR *, long ) ;

#ifdef S4CLIPPER
int    S4FUNCTION t4balance( TAG4 S4PTR *, B4BLOCK S4PTR *, int ) ;
#endif

void   S4FUNCTION t4descending( TAG4 S4PTR *, int ) ;

int    S4FUNCTION t4add( TAG4 S4PTR *, unsigned char S4PTR *, long ) ;  /* Returns r4unique, r4success, r4repeat */
int    S4FUNCTION t4add_calc( TAG4 S4PTR *, long ) ; /* Calculates expression and adds */
char  *S4FUNCTION t4alias( TAG4 S4PTR * ) ;
B4BLOCK S4PTR *S4FUNCTION t4block( TAG4 S4PTR * ) ;
int    S4FUNCTION t4bottom( TAG4 S4PTR * ) ;
int    S4FUNCTION t4check( TAG4 S4PTR * ) ;
int    S4FUNCTION t4is_descending( TAG4 S4PTR * ) ;
int    S4FUNCTION t4down( TAG4 S4PTR * ) ;
int    S4FUNCTION t4dump( TAG4 S4PTR *, int, int ) ;
int    S4FUNCTION t4eof( TAG4 S4PTR * ) ;
int    S4FUNCTION t4expr_key( TAG4 S4PTR *, char S4PTR * S4PTR * ) ;
int    S4FUNCTION t4flush( TAG4 S4PTR * ) ;
int    S4FUNCTION t4free_all( TAG4 S4PTR * ) ;
int    S4FUNCTION t4free_saved( TAG4 S4PTR * ) ;
int    S4FUNCTION t4go( TAG4 S4PTR *, char S4PTR *, long ) ;
int    S4FUNCTION t4go_eof( TAG4 S4PTR * ) ;
int    S4FUNCTION t4go2( TAG4 S4PTR *, char S4PTR *, long ) ;
void   S4FUNCTION t4init_seek_conv( TAG4 S4PTR *, int) ;    /* Initialize 'stok' and 'dtok' */
char S4PTR *S4FUNCTION t4key( TAG4 S4PTR * ) ;
B4KEY_DATA S4PTR *S4FUNCTION t4key_data( TAG4 S4PTR * ) ;              /* The current key */
void   S4FUNCTION t4out_of_date( TAG4 S4PTR * ) ;
int    S4FUNCTION t4position2( TAG4 S4PTR *, double S4PTR * ) ;
double S4FUNCTION t4position( TAG4 S4PTR * ) ;              /* Returns the position as a percent */
double S4FUNCTION t4positionDbl( TAG4 S4PTR * ) ;              /* Returns the position as a percent */
int    S4FUNCTION t4position_set( TAG4 S4PTR *, double ) ;  /* Positions a percentage */
long   S4FUNCTION t4recno( TAG4 S4PTR * ) ;
int    S4FUNCTION t4remove_current( TAG4 S4PTR * ) ;        /* Remove the current key */
int    S4FUNCTION t4remove( TAG4 S4PTR *, char S4PTR *, long ) ;  /* Remove specified key */
int    S4FUNCTION t4remove_calc( TAG4 S4PTR *, long ) ; /* Calculates expression and removes */
int    S4FUNCTION t4rl_bottom( TAG4 S4PTR * ) ;
int    S4FUNCTION t4rl_top( TAG4 S4PTR * ) ;
int    S4FUNCTION t4seek( TAG4 S4PTR *, void S4PTR *, int ) ;    /* r4success, r4found, r4after, r4eof */
#ifdef S4HAS_DESCENDING
long   S4FUNCTION t4dskip( TAG4 *, long ) ;
#endif
long   S4FUNCTION t4skip( TAG4 S4PTR *, long ) ;
#ifdef S4CLIPPER
B4BLOCK S4PTR *S4FUNCTION t4split( TAG4 S4PTR *, B4BLOCK S4PTR *, int ) ;
#else
B4BLOCK S4PTR *S4FUNCTION t4split( TAG4 S4PTR *, B4BLOCK S4PTR * ) ;
#endif
int    S4FUNCTION t4top( TAG4 S4PTR * ) ;
int    S4FUNCTION t4type( TAG4 S4PTR * ) ;
int    S4FUNCTION t4unique( TAG4 S4PTR * ) ;
int    S4FUNCTION t4up( TAG4 S4PTR * ) ;
int    S4FUNCTION t4update( TAG4 S4PTR * ) ;
int    S4FUNCTION t4up_to_root( TAG4 S4PTR * ) ;

/* File name lengths include one extra null character at end */
void   S4FUNCTION u4delay_sec( void ) ;
int    S4FUNCTION u4name_char( unsigned char ) ;
void   S4FUNCTION u4name_ext( char S4PTR *, int, char S4PTR *, int ) ;
int    S4FUNCTION u4name_ret_ext( char S4PTR *, int, char S4PTR * ) ;
void   S4FUNCTION u4name_piece( char S4PTR *, int, char S4PTR *, int, int ) ;
int    S4FUNCTION u4name_path( char S4PTR *, int, char S4PTR * ) ;
long   S4FUNCTION u4switch( void ) ;

unsigned S4FUNCTION u4ncpy( char S4PTR *, char S4PTR *, unsigned ) ;
int    S4FUNCTION u4ptr_equal( void S4PTR *, void S4PTR * ) ;
int    S4FUNCTION u4remove( char S4PTR * ) ;
void   S4FUNCTION u4yymmdd( char S4PTR * ) ;
#ifndef S4NO_RENAME
   int    S4FUNCTION u4rename( char S4PTR *, char S4PTR * ) ;
#endif

int   S4FUNCTION c4descend( FIELD4 S4PTR *, char S4PTR *, int ) ;
char *S4FUNCTION c4descend_str( char S4PTR *, char S4PTR *, int ) ;
char *S4FUNCTION c4descend_date( char S4PTR *, long, int );
char *S4FUNCTION c4descend_num( char S4PTR *, char S4PTR *, int ) ;

void  S4PTR *S4FUNCTION u4alloc( long ) ;
void  S4PTR *S4FUNCTION u4alloc_er( CODE4 S4PTR *, long ) ;
void  S4PTR *S4FUNCTION u4alloc_free( CODE4 S4PTR *, long ) ;
int    S4FUNCTION u4alloc_again( CODE4 S4PTR *, char S4PTR * S4PTR *, unsigned S4PTR *, unsigned ) ;
void   S4FUNCTION u4free( void S4PTR * ) ;
void   S4FUNCTION u4name_make( char S4PTR *, int, char S4PTR *, char S4PTR *, char S4PTR * ) ;

short  S4FUNCTION x4reverse_short( void S4PTR * ) ;
long   S4FUNCTION x4reverse_long( void S4PTR * ) ;

#ifdef S4OS2SEM
#ifdef S4OS2
   int mem4start( CODE4 * ) ;
   void mem4stop( CODE4 * ) ;
#endif
#endif
void  S4PTR *S4FUNCTION mem4alloc( MEM4 S4PTR * ) ;  /* 0 Parm causes 0 return */
void  S4PTR *S4FUNCTION mem4alloc2( MEM4 S4PTR *, CODE4 S4PTR * ) ;  /* 0 Parm causes 0 return */
Y4CHUNK S4PTR *S4FUNCTION mem4alloc_chunk( MEM4 S4PTR * ) ;  /* 0 Parm causes 0 return */
void   S4FUNCTION mem4free( MEM4 S4PTR *, void S4PTR * ) ;
int    S4FUNCTION mem4free_check( int ) ;
void   S4FUNCTION mem4check_memory( void ) ;
void   S4FUNCTION mem4init( void ) ;
#ifdef S4OLD_CODE
MEM4 S4PTR *S4FUNCTION mem4type( int, unsigned, int, int ) ;
#endif
MEM4 S4PTR *S4FUNCTION mem4create( CODE4 S4PTR *, int, unsigned, int, int ) ;
void  S4PTR *S4FUNCTION mem4create_alloc( CODE4 S4PTR *, MEM4 S4PTR * S4PTR *, int, unsigned, int, int ) ;
void   S4FUNCTION mem4release( MEM4 S4PTR * ) ;
void   S4FUNCTION mem4reset( void ) ;

#ifndef S4FOX
#ifndef S4CLIPPER
   int S4CALL c4bcd_cmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
   int S4CALL t4cmp_doub( S4CMP_PARM, S4CMP_PARM, size_t ) ;
#endif
#endif

#ifdef S4DEBUG_DEV
   int S4FUNCTION file4write_part( char S4PTR *, FILE4 S4PTR *, long, unsigned ) ;
   int S4FUNCTION file4cmp_part( CODE4 S4PTR *, char S4PTR *, FILE4 S4PTR *, long, unsigned ) ;
   int S4FUNCTION file4cmp( FILE4 S4PTR * ) ;
#endif

#ifdef __cplusplus
   }
#endif

#ifdef S4LANGUAGE
   #ifdef S4GERMAN
      #ifndef S4FOX
         int S4CALL u4memcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
      #endif
   #else
      int S4CALL u4memcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
   #endif
#endif

#ifdef S4MDX
   void c4bcd_from_a( char S4PTR *, char S4PTR *, int ) ;
   void t4str_to_date_mdx( char S4PTR *, char S4PTR *, int ) ;
   void t4no_change_str( char S4PTR *, char S4PTR *, int ) ;
   void c4bcd_from_d( char S4PTR *, double ) ;
   void t4no_change_double( char S4PTR *, double ) ;
#endif

#ifdef S4FOX
   void t4str_to_fox( char S4PTR *, char S4PTR *, int ) ;
   void t4dtstr_to_fox( char S4PTR *, char S4PTR *, int ) ;
   void t4no_change_str( char S4PTR *, char S4PTR *, int ) ;
   void t4str_to_log( char S4PTR *, char S4PTR *, int ) ;
   void t4dbl_to_fox( char S4PTR *, double ) ;
#endif

#ifdef S4CLIPPER
   void t4str_to_doub( char S4PTR *, char S4PTR *, int ) ;
   void t4no_change_str( char S4PTR *, char S4PTR *, int ) ;
   void t4date_doub_to_str( char S4PTR *, double ) ;
#endif

#ifdef S4NDX
   void t4str_to_doub( char S4PTR *, char S4PTR *, int ) ;
   void t4str_to_date_mdx( char S4PTR *, char S4PTR *, int ) ;
   void t4no_change_str( char S4PTR *, char S4PTR *, int ) ;
   void t4no_change_double( char S4PTR *, double ) ;
#endif
