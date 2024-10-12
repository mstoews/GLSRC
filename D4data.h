/* d4data.h   (c)Copyright Sequiter Software Inc., 1990-1994.  All rights reserved. */

extern char v4buffer[257] ;

typedef int S4CALL  S4CMP_FUNCTION( S4CMP_PARM, S4CMP_PARM, size_t) ;

#ifdef __SC__
   typedef void _cdecl C4DTOK( char S4PTR *, double ) ;
   typedef void _cdecl C4STOK( char S4PTR *, char S4PTR *, int ) ;
#else
   typedef void C4DTOK( char S4PTR *, double ) ;
   typedef void C4STOK( char S4PTR *, char S4PTR *, int ) ;
#endif

struct DATA4_st ;
struct F4MEMO_st ;
struct INDEX4_st ;
struct TAG4_st ;
#ifdef __cplusplus
   class S4CLASS FILE4 ;
#else
   struct FILE4_st ;
#endif
#ifdef __cplusplus
   class S4CLASS CODE4 ;
#else
   struct CODE4_st ;
#endif

#ifdef __cplusplus
   #ifdef S4WINDOWS
      #ifdef S4WIN32
         typedef struct _declspec(dllexport) l4link_st
      #else
         typedef struct _export l4link_st
      #endif
   #else
      typedef struct l4link_st
   #endif
#else
typedef struct l4link_st
#endif
{
   struct l4link_st S4PTR *n, S4PTR *p ;
} LINK4 ;

#ifdef __cplusplus
class S4CLASS LIST4
{
public:
#else
typedef struct
{
#endif
   LINK4 S4PTR *last_node ;        /* The last Link */
   void   S4PTR *selected ;
   int     n_link ;            /* The number of links in the list */
#ifdef __cplusplus
} ;
#else
} LIST4 ;
#endif

typedef struct
{
   LINK4 link ;
   double  data ;  /* Make sure it is on a boundry good for at least a double  */
} Y4CHUNK ;

typedef struct
{
   LINK4      link ;

   LIST4      chunks ;      /* Chunks of pieces */
   LIST4      pieces ;      /* A list of available memory pieces */

   #ifdef __cplusplus
      CODE4 S4PTR    *code_base ;
   #else
      struct  CODE4_st *code_base ;
   #endif
   int        unitStart;   /* The starting # of entries for the Memory Type */
   unsigned   unit_size ;   /* The size of each allocated piece */
   int        unit_expand ; /* The expansion # of entries for the Memory Type */
   int        n_repeat ;    /* The number of times entry returned for 'new' */
                            /* If n_repeat is '-1', it is a temporary entry. */
   int        n_used ;      /* The number of entries used */
}  MEM4 ;

typedef struct OPT4BLOCK_st
{
   LINK4             link ;
   LINK4             lru_link ;
   char              changed ;
   unsigned          len ;

   /* these next 2 elements must match the OPT4CMP structure above */
   #ifdef __cplusplus
      FILE4          S4PTR *file ;
   #else
      struct FILE4_st *file ;
   #endif
   long              pos ;

   void              S4PTR *data ;
} OPT4BLOCK ;

#ifdef __cplusplus
   typedef struct S4CLASS
#else
   typedef struct
#endif
{
   int              num_buffers ;
   unsigned long    block_size ;
   unsigned long    buffer_size ;
   unsigned long    hash_trail ;       /* where last optimized file ended */
   unsigned long    num_blocks ;

   LIST4            avail ;
   LIST4            dbf_lru ;
   LIST4            index_lru ;
   LIST4            other_lru ;
   LIST4 S4PTR     *prio[3] ;

   unsigned char    old_mode ;
   unsigned char    do_update ;
   unsigned char    check_count ;
   unsigned char    dummy_char ;
   int              min_link ;
   void  S4PTR* S4PTR* buffers ;
   OPT4BLOCK    S4PTR *blocks ;
   char             block_power ;
   char             num_shift ;
   unsigned long    num_lists ;
   unsigned long    mask ;
   LIST4 S4PTR      *lists ;
   unsigned         max_blocks ;

   LIST4            opt_files ;

   char             is_skip ;
   char             force_current ;    /* switch forces a read of current contents */

   char S4PTR       *read_buffer ;
   unsigned long     read_start_pos ;
   int               in_read_buf ;

   char S4PTR       *write_buffer ;
   unsigned long     write_start_pos ;
   unsigned long     write_cur_pos ;
   unsigned          write_block_count ; /* is the buffer full? */
   #ifdef __cplusplus
      FILE4 S4PTR    *write_file ;
   #else
      struct FILE4_st S4PTR *write_file ;        /* which file has the write buffer? */
   #endif
} OPT4 ;

#ifdef __cplusplus
class S4CLASS CODE4
{
public:
#else
typedef struct CODE4_st
{
#endif
   HWND      hWnd ;                /* For use under Microsoft Windows */
   HINSTANCE hInst ;

   LIST4   data_list ;           /* A list of open data files. */
   int defaultUniqueError ;     /* e4unique, r4unique, r4uniqueContinue */
   char dateFormat[20];          /* Longest is September 15, 1990 */

   unsigned memSizeBlock ;      /* Block size (bytes) for memo and index files */
   unsigned memSizeSortPool ;  /* The default pool size for sorting */
   unsigned memSizeSortBuffer ;/* The default file buffer size when sorting */
   unsigned memSizeBuffer ;     /* Pack, Zap */
   unsigned memSizeMemo ;
   unsigned memSizeMemoExpr ;

   int memExpandBlock ;         /* Expanding block memory allocation */
   int memExpandData ;          /* Expanding data file allocation */
   int memExpandIndex ;         /* Expanding index file allocation */
   int memExpandTag ;           /* Expanding index file allocation */

   unsigned memStartData ;           /* Initial data file allocation */
   unsigned memStartBlock ;          /* Initial block memory allocation for index files */
   unsigned mem_start_index ;          /* Initial index file allocation */
   unsigned mem_start_tag ;            /* Initial index file allocation */
   unsigned mem_start_buffer ;
   unsigned long memStartMax ;

   /* True/False Flags */
   int  autoOpen ;            /* Automatic production index file opening */
   int  createError ;         /* Do 'file4create' error ? */
   int  offError ;            /* Show error messages? */
   int  exclusive ;            /* how should files be opened? */
   int  file_flush ;           /* force hard file flush during write */
   int  exprError ;
   int  fieldNameError ;
   int  goError ;             /* Do 'd4go' error ? */
   int  openError ;           /* Do 'file4open' error ? */
   int  optimize ;             /* should files be automatically bufferred? */
   int  readLock ;            /* Do lock when reading database ? */
   int  readOnly ;
   int  relateError ;         /* do relate4terminate error when no match and relate4terminate selected */
   int  safety ;               /* File create with safety ? */
   int  skipError ;           /* Do 'DataIndex::skip' error ? */
   int  tagNameError;

   int  lockAttempts ;        /* How many times to attempt locks. */

   MEM4 S4PTR *index_memory ;
   MEM4 S4PTR *data_memory ;
   MEM4 S4PTR *tag_memory ;
   MEM4 S4PTR *calc_memory ;
   MEM4 S4PTR *bitmap_memory ;
   LIST4 calc_list ;

   MEM4 S4PTR *total_memory ;
   LIST4  total_list ;       /* A list of T4TOTAL */
   LIST4  font_list;
   int  num_reports ;
   int  s4cr2;

   int  errorCode ;
   int  debug_int ;            /* used to check structure integrity (set to 0x5281) */

   short pageno;

   char S4PTR *field_buffer ;    /* used by the f4str() function */
   unsigned  buf_len ;

   char S4PTR *expr_work_buf ;   /* used by expression parsing */
   unsigned expr_buf_len ;

   char S4PTR *stored_key ;      /* used by the expr4key() function */
   unsigned stored_key_len ;

   int bitmap_disable ;     /* used for testing purposes to test disabled bitmaps */
   int do_index_verify ;    /* for internal purposes only at this point */

   #ifndef S4OPTIMIZE_OFF
      int  optimizeWrite ;
      OPT4 opt ;
      int  has_opt, do_opt ;
      unsigned int   mode ;
   #endif

   #ifdef S4CLIPPER
      int numeric_str_len ;    /* the default length for clipper index files */
      int decimals ;           /* the default # decimals for clipper index files */
   #endif

   #ifdef S4OS2SEM
   #ifdef S4OS2
   #ifndef S4IBMOS2
   #ifndef __WATCOMC__
      HMTX hmtx_mem, hmtx_expr ;
   #endif
   #endif
   #endif
   #endif
   char saved_key[I4MAX_KEY_SIZE + 2 * sizeof(long)] ;       /* used by i4remove.c, i4tag.c and i4addtag.c, i4version_check, t4version_check */
#ifdef __cplusplus
} ;
#else
} CODE4 ;
#endif

#ifdef __cplusplus
class S4CLASS FILE4
{
public:
#else
typedef struct FILE4_st
{
#endif
   #ifndef S4OPTIMIZE_OFF
      LINK4  link ;            /* set to 0 if file not optimized */
      long   hash_init ;
      long   len ;             /* internal if optimized */
      char   type ;            /* dbf, index, other */
      char   buffer_writes ;   /* are writes being bufferred? */
      int    do_buffer ;       /* is the file bufferring on? */
   #endif
   char S4PTR *name ;
   CODE4 S4PTR *code_base ;
   int   hand ;
   char  is_temp ;             /* True if it is a temporary file */

   /* is_exclusive and is_readOnly both on indicate 'r' attribute on file */
   /* is_readOnly only on indicates user access is limited to read only */
   /* if is_exclusive, will do full bufferring, if is_readOnly, will avoid */
   /* performing any disk writes */

   char  is_exclusive ;        /* True if opened exclusive */
   char  is_readOnly ;        /* True if file is read only */
   char  do_alloc_free ;
   char  write_buffer ;        /* buffer writes where possible */
   char  file_created ;        /* false if the file has not been created yet - i.e. if a memory-only file */


   #ifdef S4DEBUG_DEV
      char dup_name[255] ;
      int  has_dup ;
      int  in_use ;
   #endif
#ifdef __cplusplus
} ;
#else
} FILE4 ;
#endif

typedef struct
{
   FILE4     file ;
   short      block_size ;               /* Bytes */
   struct DATA4_st  S4PTR *data ;
   int      file_lock ;                  /* True if file is locked */
} MEMO4FILE ;

#ifdef __cplusplus
class S4CLASS FILE4SEQ_READ
{
public:
#else
typedef struct
{
#endif
   FILE4 S4PTR *file ;

   long  pos ;          /* The next position to read from */
   char S4PTR *buffer ;
   unsigned next_read_len ;
   unsigned total ;     /* Total buffer length */
   unsigned working ;   /* Temporary working buffer length (to help align write) */
   unsigned avail ;     /* # of bytes currently available */
#ifdef __cplusplus
} ;
#else
} FILE4SEQ_READ ;
#endif

#ifdef __cplusplus
class S4CLASS FILE4SEQ_WRITE
{
public:
#else
typedef struct
{
#endif
   FILE4 S4PTR *file ;

   long  pos ;          /* The next position to read from */
   char S4PTR *buffer ;
   unsigned total ;     /* Total buffer length */
   unsigned working ;   /* Temporary working buffer length (to help align write) */
   unsigned avail ;     /* # of bytes left in working buffer */
#ifdef __cplusplus
} ;
#else
} FILE4SEQ_WRITE ;
#endif

typedef struct  /* Data File Format */
{
   char     name[11] ;
   char     type ;
   char     filler[4] ;
   unsigned char  len ;
   unsigned char  dec ;
   char     filler2[13] ;
   char    has_tag ;
} FIELD4IMAGE ;

typedef struct  /* Internal Structure and Field Routines. */
{
   char        name[11] ;
   unsigned    len ;
   int         dec ;
   int         type ;
   int         offset ;
   struct DATA4_st  S4PTR *data ;
   struct F4MEMO_st  S4PTR *memo ;
   #ifdef S4VBASIC
      int  debug_int ;         /* used to check structure integrity (set to 0x5281) */
   #endif
} FIELD4 ;

typedef struct F4MEMO_st
{
   int    is_changed ;

   int    status ;           /* 0 - Current contents, 1 - Unknown */
   char  S4PTR *contents ;
   unsigned  len ;
   unsigned  len_max ;
   FIELD4  S4PTR *field ;
} F4MEMO ;

typedef struct  /* Creating Data File */
{
   char S4PTR  *name ;
   int         type ;
   unsigned int len ;
   unsigned int dec ;
} FIELD4INFO ;

typedef struct
{
   /* Database Header Information */
   char     version ;        /* 83H with .dbt, 03H without */
   char     yy ;             /* Last Update */
   char     mm ;
   char     dd ;
   long     num_recs ;
   unsigned short header_len; /* Header Length, Indicates start of data */
   unsigned short record_len;
   char     zero[16] ;
   char     has_mdx ;    /* 1 for true */
   char     dummy ;
   char     zero2[2] ;
} DATA4HEADER_FULL ;

typedef struct DATA4_st
{
   LINK4  link ;

   /* Database Header Information */
   char     version ;        /* 83H with .dbt, 03H without */
   char     yy ;             /* Last Update */
   char     mm ;
   char     dd ;
   long     num_recs ;
   unsigned short header_len; /* Header Length, Indicates start of data */

   char S4PTR *record ;              /* Data allocated with 'u4alloc' */
   char S4PTR *record_old ;          /* Data allocated with 'u4alloc' */
                                 /* Extra byte added for temporary CTRL_Z */
   unsigned record_width ;
   int      record_changed ;      /* T/F */
   long     rec_num ;             /* Record number; -1 unknown; 0 for append */
   long     rec_num_old ;         /* Record number, -1 none present; 0 for append */

   FILE4    file ;
   char     alias[11] ;

   char     memo_validated ; /* Can we be sure memo id #'s are up to date. */

   CODE4 S4PTR *code_base ;
   char     has_mdx ;        /* Has an MDX file attached to it */

   FIELD4  S4PTR *fields ;        /* An array of field pointers */
   int      n_fields ;       /* The number of data fields in the database */

   F4MEMO   S4PTR *fields_memo ;    /* A list of fields to be flushed */
   int      n_fields_memo ;  /* The number of memo files in the database */

   long     locked_record ;  /* 'locks' data when 'n_locks <= 1' */
   long     S4PTR *locks ;
   int      n_locks ;        /* Number of elements in 'locks' allocated */
   int      num_locked ;     /* Number of records locked */
   int      file_lock ;      /* True if entire file is locked */
   int      append_lock ;    /* True if the file is locked for appending */

   int      file_changed ;   /* True if the file has been changed since */
                                /* the header has been updated. */

   LIST4    indexes ;
   int      bof_flag, eof_flag ;    /* Beginning/End of File flags */

   short    block_size ;
   MEMO4FILE   memo_file ;      /* Memo file handle */
   #ifdef S4VBASIC
      int   debug_int ;      /* used to check structure integrity (set to 0x5281) */
   #endif
   #ifndef S4SINGLE
      long  minCount ;    /* used as a minimum record count for various functions */
   #endif
   long     count ;  /* a fairly current record count on the database */
                     /* used by relate module, by put here for compatibility with report writer... */
} DATA4 ;

typedef void S4OPERATOR(void) ;

typedef struct E4INFO_st
{
   FIELD4 S4PTR *field_ptr ;
   char S4PTR *p1 ;
   int   len ;         /* Length */
   int   num_entries ; /* Number of entries in sub-expression */
   int   num_parms ;
   int   result_pos ;  /* The position in the result array for result. */
   int   i1 ;          /* Could be constant position. 'i1' and 'result_pos'
                       and 'function_i' and 'function'
                       must be at last position due to memcmp() in e4is_tag() */
   int   function_i ;
   S4OPERATOR S4PTR *function ;
} E4INFO ;

typedef struct e4expr_st
{
   E4INFO S4PTR *info ;
   int      info_n ;
   char S4PTR *source ;
   char S4PTR *constants ;
   int      len ;
   int      type ;
   #ifdef S4CLIPPER
      int   key_dec ;         /* used for CLIPPER version */
      int   key_len ;
   #endif
   DATA4  S4PTR *data ;
   CODE4  S4PTR *code_base ;

   int   len_eval ;        /* This is the length of the buffer needed for evaluation. */
   int   num_parms ;       /* This is the # of parameter positions used in evaluation. */
   char  has_trim ;        /* special case for key evaluation */
} EXPR4 ;

#ifdef N4OTHER
   typedef struct
   {
      long  pointer ;    /* =0L if record, not pointer */
      long  num ;
      char  value[1] ;  /* The key size is variable */
   } B4KEY_DATA ;
#else
   typedef struct
   {
      long  num ;
      char  value[1] ;  /* The key size is variable */
   } B4KEY_DATA ;
#endif

#ifdef S4FOX
/* the following structure is used only on the leaf nodes of the tree structure */
typedef struct
{
   short            free_space ;        /* # bytes available in node */
   unsigned char    rec_num_mask[4] ;      /* record number mask */
   unsigned char    dup_byte_cnt ;      /* duplicate byte mask count */
   unsigned char    trail_byte_cnt ;    /* Trailing byte mask count */
   unsigned char    rec_num_len ;       /* # bits used for record number */
   unsigned char    dup_cnt_len ;       /* # bits used for duplicate count */
   unsigned char    trail_cnt_len ;     /* # bits used for trail count */
   unsigned char    info_len ;          /* # bytes for holding record number, */
} B4NODE_HEADER ;

typedef struct
{
      short      node_attribute ;    /* 0=index, 1=root, 2=leaf */
      short      n_keys ;            /* Block Image starts here */
      long       left_node ;         /* -1 if not present */
      long       right_node ;        /* -1 if not present */
} B4STD_HEADER ;

typedef struct
{
   LINK4 link ;
   struct TAG4_st  S4PTR *tag ;

   int   changed ;
   long  file_block ;     /* Identifies block within index file */
   int   key_on ;         /* The current key within the block */
   int   cur_trail_cnt ;  /* current value used for seeking */
   int   cur_dup_cnt ;    /* current value used for seeking */
   int   dup_pos ;        /* bit offset into the info for the duplicate data */
   int   trail_pos ;      /* bit offset into the info for the trail data */
   int   rec_pos ;        /* bit offset into the info for the record # data */
   char  *cur_pos ;       /* current position into the data (starts at end) */

   int   built_on ;       /* the 'current' key value (i.e. key really 'on') */
   char  S4PTR *built_pos ;     /* position where built on */
   B4KEY_DATA S4PTR *built_key ;

   B4STD_HEADER header ;
   B4NODE_HEADER node_hdr ;    /* only if the block is a leaf */
   char  data[1] ;        /* the remaining data */
} B4BLOCK ;

#endif  /* ifdef S4FOX  */

#ifndef S4FOX

#ifdef S4NDX
   typedef struct
   {
      long     root ;
      long     eof ;
      char     n1_dummy ;
      char     type ;
      char     n2_dummy[2] ;
      short    key_len  ;
      short    keys_max ;           /* Maximum # of keys per block;  <= 100 */
      short    int_or_date ;        /* TRUE (1) if Numeric or Date Key */
      short    group_len ;          /* key_len plus 8 increased to a multiple of 2 */
      short    dummy ;
      short    unique ;             /* TRUE if Unique */

 /*   char     expression[256] ; */
 /*   long     version ; */
   } I4IND_HEAD_WRITE;
#else
   #ifdef S4CLIPPER
      typedef struct
      {
         short          sign ;
         short          version ;
         long           root ;          /* Root Block */
         long           eof ;           /* First Free Block Pointer */
         short          group_len ;     /* Key Length + 2*sizeof(long) */
         short          key_len ;       /* Key Length */
         short          key_dec ;       /* Number of Decimals in Key */
         short          keys_max ;      /* Maximum # of keys per block;  <= 100 */
         short          keys_half ;     /* Maximum # of keys per half block */

    /*   char           expression[256];   The index expression corresponding to the database. */
    /*   short          unique   ;         TRUE if Unique */
      } I4IND_HEAD_WRITE;
   #endif
#endif

#ifdef S4NDX
typedef struct
{
   long     old_version ;
   int      header_offset ;

   long     root ;
   long     eof ;
   char     n1_dummy ;
   char     type ;
   char     n2_dummy[2] ;
   short    key_len  ;
   short    keys_max ;           /* Maximum # of keys per block;  <= 100 */
   short    int_or_date ;        /* TRUE (1) if Numeric or Date Key */
   short    group_len ;          /* key_len plus 8 increased to a multiple of 2 */
   short    dummy ;
   short    unique ;             /* TRUE if Unique */
/* char     expression[256] ; */
   long     version ;
}  T4HEADER ;
#else
   #ifdef S4CLIPPER
      typedef struct
      {
         long       old_version ;
         int        header_offset ;
         long       virtual_eof ;   /* The next available file block */
         short      sign ;
         short      version ;
         long       root ;          /* Root Block */
         long       eof ;           /* First Free Block Pointer */
         short      group_len ;     /* Key Length + 2*sizeof(long) */
         short      key_len ;       /* Key Length */
         short      key_dec ;       /* Number of Decimals in Key */
         short      keys_max ;      /* Maximum # of keys per block;  <= 100 */
         short      keys_half ;     /* Maximum # of keys per half block */
      /* char       expression[256] ;   The index expression corresponding to the database. */
         short      unique   ;      /* TRUE if Unique */
         short      descending ;     /* The descending flag corresponding to the index file */
      /* char       filter[256] ;   The filter(for) expression corresponding to the database. */
      }  T4HEADER ;
   #else
      typedef struct
      {
         char   two ;                /* Version number (currently 2) */
         char   yymmdd[3] ;          /* Date of last reindex */
         char   data_name[12] ;      /* Name of associated data file */
         char   dummy1[4] ;          /* extra 4 bytes for data-names-not used in DOS */
         short  block_chunks ;       /* Block Size 1 to 32 (512 byte chunks) */
         short  block_rw ;           /* Block Read/Write Size in bytes */
         char   is_production ;      /* 1 if production index, else 0 */
         char   num_slots ;          /* number possible tags (48) */
         short  slot_size ;          /* number bytes/tag slot (32) */
         short  num_tags ;
         short  dummy2 ;
         long   eof ;
         long   free_list ;          /* start of the free list */
         char   zero[4] ;
         char   create_date[3];      /* not used by CodeBase */
         char   blank ;
      }  I4HEADER ;
   #endif     /*   ifdef S4CLIPPER  */
#endif     /*   ifdef S4NDX   */
#endif     /*   ifndef S4FOX  */


#ifndef S4FOX
typedef struct
{
   LINK4 link ;
   struct TAG4_st  S4PTR *tag ;

   long  file_block ;  /* Identifies block within index file */
   int   changed ;
   int   key_on ;      /* The current key within the block */

   short      n_keys ; /* Block Image starts here */
   #ifdef S4NDX
      char       dummy[2] ;
      B4KEY_DATA data ;
   #else
      #ifdef S4CLIPPER
         short pointers[( B4BLOCK_SIZE / 2 - 1 )] ;
         B4KEY_DATA S4PTR *data ;
      #else
         char       dummy[6] ;
         B4KEY_DATA info ;
      #endif
   #endif
} B4BLOCK ;

typedef struct
{
   long  header_pos ;          /* Header position (in 512 byte chunks) */
   char  tag[10] ;
   short x1000 ;               /* used for dBASE/SQL expression type - dBASE only allowed for CBPP 1.0x */
   char  left_chld ;
   char  right_chld ;
   char  parent ;
   char  x2 ;
   char  index_type ;
   char  zeros[11] ;
} T4DESC ;
#endif     /* ifndef  S4FOX  */

#ifndef N4OTHER
typedef struct
{
#ifdef S4FOX
   long           root ;            /* -1 means unknown */
   long           free_list ;       /* start of the free list (-1 if none) */
   unsigned long  version ;         /* used multi-user only */
   short          key_len ;         /* Key Length */
   unsigned char  type_code;        /* 0x01 Uniq; 0x08 For Clause; 0x32 Compact; 0x80 Compound */
   unsigned char  signature ;       /* unused */

/* char           dummy2[482] ;        unused */
   char           dummy3[4] ;
   short          descending   ;    /* 1 = descending, 0 = ascending */
   short          filter_pos ;      /* not used, == to expr_len */
   short          filter_len ;      /* length of filter clause */
   short          expr_pos ;        /* not used, == to 0  */
   short          expr_len ;        /* length of expression */
/* char           expr_pool[512] ;  expression and filter pool */
#else
   long           root ;            /* -1 means unknown */
   char           dummy1[4] ;
   char           type_code;        /* 0x10 Normal; 0x58 Uniq,Desc; 0x50 Uniq; 0x18 Desc */
   char           type ;            /* N,D, or C (F is type N)  */
   char           dummy2[2] ;
   short          key_len ;
   short          keys_max ;        /* Maximum # of keys per block;  <= 100  */
   char           dummy3[2] ;       /* 1 if Numeric or Date Key (NDX only) */
   short          group_len ;       /* key_len plus 4 (MDX); plus 8 (NDX) */
   unsigned char  version ;
   char           dummy4 ;
   short          unique   ;        /* 0x4000 (TRUE)if Unique */

   /* Note, 'expr_key[220]' comes after 'unique' and */
   /*       'expr_filter[220]' comes at position 0x2FA */
#endif
}  T4HEADER ;
#endif   /*  ifndef N4OTHER  */

typedef struct TAG4_st
{
   LINK4           link ;

   EXPR4   S4PTR  *expr ;
   EXPR4   S4PTR  *filter ;
   int             unique_error; /* Is rewriting a unique key an error ? */
   struct INDEX4_st S4PTR *index ;
   S4CMP_FUNCTION *cmp ;
   C4STOK S4PTR   *stok ;               /* Conversion for 'seek' */
   C4DTOK S4PTR   *dtok ;               /* Conversion for 'seek' */
   CODE4 S4PTR    *code_base ;
   char            alias[11] ;
   char            has_keys ;
   LIST4           blocks ;

   FILE4        file ;
   int          file_locked ;

   T4HEADER    header ;
   long        header_offset ;     /* Offset in file to the tag's header info. */
   int         root_write ;        /* True if 'header.root' needs to be written */
   int         key_dec ;
   LIST4       saved ;
   int         debug_int ;         /* used to check structure integrity (set to 0x5281) */
   #ifdef S4MDX
      char            had_keys ;
   #endif
   #ifdef S4FOX
      char         p_char ;
   #endif

   #ifdef S4CLIPPER
      long         check_eof ;     /* used for debug purposes to verify eof length */
   #endif

   #ifdef S4MDX
      long  changed ;
   #endif
   #ifdef S4UNIX
      int key_type ;
   #endif
} TAG4 ;

typedef struct
{
   char S4PTR  *name ;
   char S4PTR  *expression ;
   char S4PTR  *filter ;
   int unique ;
   unsigned int descending ;
} TAG4INFO ;

typedef struct INDEX4_st
{
   LINK4  link ;

   FILE4  file ;
   DATA4 S4PTR *data ;
   CODE4 S4PTR *code_base ;
   LIST4  tags ;
   char alias[11] ;

   #ifdef S4FOX
      TAG4 S4PTR *tag_index ;    /* the tags are a tag in the index file! */
      long  eof ;
   #else
      #ifdef N4OTHER
         char S4PTR *path ;
      #else
         I4HEADER header ;
      #endif
   #endif

   MEM4 S4PTR *block_memory ;
   int   file_locked ;    /* True if locked */
   #ifdef S4FOX
      unsigned long  version_old ;
   #else
      #ifdef S4MDX
         long  changed ;
      #else
         long  version_old ;
      #endif
   #endif
} INDEX4 ;


/* Memo File Structures */

typedef struct
{
   #ifdef S4MFOX
      long  next_block ;  /* Memo Entry 1,2, ... */
      char  usused[2] ;
      short block_size ;  /* Bytes */
   #else
      #ifdef S4MNDX
         long  next_block ;  /* Memo Entry 1,2, ... */
      #else
         long  next_block ;  /* Memo Entry 1,2, ... */
         long  zero ;
         char  file_name[8] ;
         short zero2 ;
         short x102 ;
         short block_size ;  /* Bytes */
         short zero3 ;
      #endif
   #endif
} MEMO4HEADER ;

#ifndef S4MNDX
#ifndef S4MFOX
typedef struct
{
   long  next ;       /* The next free block area */
   long  num ;        /* The number of free blocks in the free block area */
   int   to_disk ;    /* TRUE if this information needs to be written to disk */
   long  block_no ;   /* The current block number */
} MEMO4CHAIN_ENTRY ;
#endif /*  ifndef S4MFOX  */

typedef struct
{
   #ifdef S4MFOX
      long  type ;         /* 0 for picture, 1 for text -- picture not supported */
      long  num_chars ;    /* Including the 'MemoBlock' */
   #else
      short minus_one ;    /* '-1' for dBASE IV */
      short start_pos ;
      long  num_chars ;    /* Including the 'MemoBlock' */
   #endif
} MEMO4BLOCK ;
#endif  /*  ifndef S4MNDX  */

typedef struct
{
   unsigned  char  sig_dig ;  /* The number of significant digits; 52 is zero */
   unsigned  char  digit_info ; /* contains one, len and sign */
   unsigned  char  bcd[10] ;
}  C4BCD ;
