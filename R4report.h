/* r4report.h   (c)Copyright Sequiter Software Inc., 1992-1994.  All rights reserved. */

#ifndef __REPORT4HEADER
#define __REPORT4HEADER

/* #include <windows.h> */

#define CREP_MASK 0x15  /* This masks the first byte of a .rep file */

#define START_VERSION_MASK 0x20
#define VERSION_MASK 0x26
#define STYLE_MASK 0x16

/* defined types used for specifying displayed units of measurement */
#define UNIT4POINTS    1
#define UNIT4INCHES    2
#define UNIT4CM        3

/* defined total types used for internal flags */
#define total4lowest  11
#define total4highest 12
#define total4count   13
#define total4average 14
#define total4sum     15

/* defined types for report output objects */
#define obj4type_field       400
#define obj4type_expr        401
#define obj4type_total       402
#define obj4type_calc        403
#define obj4type_text        404
#define obj4type_bitmap1     405
#define obj4type_bitmap2     409
#define obj4type_bitmap3     410
#define obj4type_hline       406
#define obj4type_vline       407
#define obj4type_frame       408

/* defines used for specifying text alignment within an output object */
#define justify4left   0
#define justify4right  1
#define justify4center 2

/* defines used for specifying display format for numeric values  */
#define obj4numNumber   0
#define obj4numExponent 1
#define obj4numCurrency 2
#define obj4numPercent  3

/* this define forces the use of the default index extension regardless of
   what extension that was saved in the report file  */
#define S4DEFAULT_INDEX 1

/* defines used for specifying current index format: these are for report
   function use only  */
#define r4mdx 1
#define r4ndx 2
#define r4cli 3
#define r4fox 4

/* these defines are for running the report module under windows  */
#ifdef S4WINDOWS
   /* user defined message sent by the report popup output window
      to the report parent window upon closing the output window */
   #define CRM_REPORTCLOSED (WM_USER+1000)

   #ifdef S4WIN32
      /* These macros are used for parsing commands in wind procs and are used
         for portability between windows and NT */
      #define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(wp)
      #define GET_WM_COMMAND_ID(wp, lp)       LOWORD(wp)
      #define GET_WM_COMMAND_HWND(wp, lp)     ((HWND)lp)

      /* offsets into the extra bytes of the popup output window */
      #define PWIN_OFF_REPORT 0
      #define PWIN_OFF_BDC    4
      #define PWIN_OFF_HBIT   8
      #define PWIN_OFF_PGWIN  12
      #define PWIN_OFF_OLDOBJ 16

      /* offsets into the extra bytes of the output page window */
      #define PGWIN_OFF_BDC   0
      #define PGWIN_OFF_HBIT  4
      #define PGWIN_OFF_YOFF  8

      /* portability macros used for setting and getting info in the window
         extra bytes */
      #define MGetWinP(a,b)  GetWindowLong( (HWND)a, (int)b )
      #define MSetWinP(a,b,c)  SetWindowLong( (HWND)a, (int)b, (LONG)c )
   #else
      /* These macros are used for parsing commands in wind procs and are used
         for portability between windows and NT */
      #define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(lp)
      #define GET_WM_COMMAND_ID(wp, lp)       (wp)
      #define GET_WM_COMMAND_HWND(wp, lp)     ((HWND)LOWORD(lp))

      /* offsets into the extra bytes of the popup output window */
      #define PWIN_OFF_REPORT 0
      #define PWIN_OFF_BDC    4
      #define PWIN_OFF_HBIT   6
      #define PWIN_OFF_PGWIN  8
      #define PWIN_OFF_OLDOBJ 10

      /* offsets into the extra bytes of the output page window */
      #define PGWIN_OFF_BDC   0
      #define PGWIN_OFF_HBIT  2
      #define PGWIN_OFF_YOFF  4

      /* portability macros used for setting and getting info in the window
         extra bytes */
      #define MGetWinP(a,b) GetWindowWord( (HWND)a, (int)b )
      #define MSetWinP(a,b,c) SetWindowWord( (HWND)a, (int)b, (WORD)c )
      #define MoveToEx(a, b, c, d) MoveTo( a, b, c )
   #endif

typedef struct{
   long dtStyle;
   BYTE dtItemCount;
   int  dtX;
   int  dtY;
   int  dtCX;
   int  dtCY;
} DlgTemplateHeader;

typedef struct{
   int  dtilX;
   int  dtilY;
   int  dtilCX;
   int  dtilCY;
   int  dtilID;
   long dtilStyle;
}DlgItemTemplateHeader;

typedef struct{
   HANDLE hBmi;
   HANDLE hDIBits;
}R4DIBINFO, *PR4DIBINFO;

GLOBALHANDLE NewDlgTemplate( DWORD, WORD, WORD, WORD, WORD,
                             LPSTR, LPSTR, LPSTR,
                             LPSTR, WORD);

BOOL AddDlgItem( GLOBALHANDLE, DWORD, WORD, WORD, WORD,
                 WORD, LPSTR, LPSTR, BYTE, LPSTR,
                 int );

int TemplateSize( GLOBALHANDLE );

/*typedef struct tagLOGFONT R4LOGFONT;*/
/*typedef R4LOGFONT* PR4LOGFONT;*/
typedef HFONT R4HFONT;
typedef COLORREF R4COLORREF;
typedef BYTE R4BYTE;
typedef WORD R4WORD;
typedef DWORD R4DWORD;

#endif

#ifndef S4WINDOWS
   #define TRUE 1
   #define FALSE 0
   #define LF_FACESIZE 32

   typedef unsigned char R4BYTE;
   typedef unsigned short R4HFONT;
   typedef unsigned short R4WORD;
   typedef unsigned long  R4DWORD;
   #ifndef S4OS2
      typedef int BOOL;
   #endif
   typedef long R4COLORREF;

#endif

/* This structure is the same as the windows LOGFONT structure, I define it here
   so that it can be referred to in both windows and dos code */
   typedef struct r4tagLOGFONT
   {
      short    lfHeight;
      short    lfWidth;
      short    lfEscapement;
      short    lfOrientation;
      short    lfWeight;
      R4BYTE   lfItalic;
      R4BYTE    lfUnderline;
      R4BYTE    lfStrikeOut;
      R4BYTE    lfCharSet;
      R4BYTE    lfOutPrecision;
      R4BYTE    lfClipPrecision;
      R4BYTE    lfQuality;
      R4BYTE    lfPitchAndFamily;
      R4BYTE    lfFaceName[32];
   } R4LOGFONT, *PR4LOGFONT ;

/* macros for creating and parsing a colorref value */
#define R4GETBVALUE(rgb) ((R4BYTE)((rgb)>>16))
#define R4GETGVALUE(rgb) ((R4BYTE)(((R4WORD)(rgb)) >> 8))
#define R4GETRVALUE(rgb) ((R4BYTE)(rgb))
#define R4RGB(r,g,b)     ((R4COLORREF)(((R4BYTE)(r)|((R4WORD)(g)<<8))|(((R4DWORD)(R4BYTE)(b))<<16)))

/* pre-declarations for the report and object structures */
struct REPORT4_st;
struct _OBJECT_struct;


/******************************************************************************/



/* this structure is used for supplying equivalent names to the windows font
   names for use on other platforms */
typedef struct STYLE4EQUIVFONTst
{
        LINK4 link;
        char winfont[33];
   char equivfont[33];
} STYLE4EQUIVFONT, S4PTR *PSTYLE4EQUIVFONT;



/******************************************************************************/

/* This structure is wrapped around an EXPR4CALC calculation structure to create
   a total.  Note:  a total structure is then wrapped by a total output object
   */
typedef struct TOTAL4st
{
   LINK4 link ;
   EXPR4CALC *calc_ptr ; /* A total is a calculation for easy
                              removal/name changes */

   /* These two members are used to keep track of when the total should be reset
      each time the reset expression is evaluated it's value is compared to the
      value stored in last_reset_value, if they are different the total is reset
      and the stored value is updated */
   EXPR4 *reset_expression;
   char  *last_reset_value;

   /* normally a total is 'incremented' appropriately for every record in the
      relation set.  In some cases you only want a subset of these records to
      apply to the total.  The add_condition and last_add_value function
      similarly to the reset condition but selectively enable or disable
      updating the total for the current record */
   EXPR4 *add_condition;
   char  *last_add_value;
   short logcon;      /* flag which indicates that the conditional total is
                         based on a logical condition, not a changed value */
   short donce;

   struct _OBJECT_struct *obj; /* report object associated with the total */

   struct REPORT4_st *report;  /* report containing the total */

   short total_type ;  /* total4lowest, total4highest, total4sum, */
                      /* total4count, total4average */

   short  lookahead;

   double total ;     /* Running Total */
   double low ;       /* Lowest Value */
   double high ;      /* Hightest Value */
   long count;        /* used internally for computing the average */
} TOTAL4, S4PTR *PTOTAL4 ;


/******************************************************************************/

/* The STYLE4 structure contains a style definition.  It contains information
   for both windows and non-windows cases*/
typedef struct STYLE4_st
{
   LINK4 link;

   char name[20];    /* name of the style ex. 'Bold Blue' */

   /* These four members are used to hold the windows font/color info */
   R4LOGFONT lfont;   /* windows logical font structure used to create fonts */
   R4HFONT   screen_font, printer_font; /* handles for fonts created */
   R4COLORREF color; /* color of the styles text */

   /* members used by the codereporter exe for manipulating the style menu */
   int menu_id;
   short position;

   short point_size;

   /* members used for holding additional (non-windows) style info */
   short codes_before_len, codes_after_len;
   char *codes_before, *codes_after;

   #ifdef S4WINDOWS
   #ifdef S4WIN32
   LOGFONT ntFont;
   #endif
   #endif

} STYLE4, S4PTR *PSTYLE4;



/******************************************************************************/
/* GROUP4 structure: used for specifying the groupings of the records in the
   report
*/
typedef struct GROUP4_st
{
   LINK4 link;

   short position;  /* holds the groups position in the groups in the report */

   short is_active; /* used by CR .exe to flag the selected group */

   /* two flags used to indicate whether this group is the page header/footer
      or the title summary group.  These two groups are created automatically
      and cannot be deleted. */
   short header_footer;
   short title_summary;

   /* flag used to indicate that the title/summary has been outputted */
   short tsdone;

   /* expression and values used for determining whether or not a group should
      reset */
   EXPR4 *reset_expression;
   char  *last_reset_value, *held_reset_value;

   /* user defined name for the group */
   char group_label[26];

   /* lists containing the display areas for the groups header and footer */
   LIST4 header_areas;
   LIST4 footer_areas;

   /* flags used to specify user defined options on the group */
   short swap_header;
   short swap_footer;
   short repeat_header;
   short reset_page;
   short reset_pagenum;
   short reset_flag;
   short lookahead;

   /* temporary flags */
   short tswap_header;
   short tswap_footer;
   short trepeat_header;
   short treset_page;
   short treset_pagenum;

   /* containing report */
   struct REPORT4_st *report;

   /* in the CR executable the group has two info windows corresponding to
      the group header and footer */
   #ifdef S4WINDOWS
      HWND header_info_hWnd;
      HWND footer_info_hWnd;
   #endif
} GROUP4, S4PTR *PGROUP4;


/******************************************************************************/
/* AREA4 structure. Each group contains header and footer areas.  These areas
   have a specific size, and contain the actual report output objects.
*/
typedef struct AREA4_st
{
   LINK4 link;

   short position;   /* position in list of areas */
   long height;      /* height of the area in 1000's of an inch */
   int  height_dev;  /* height of the area in device output units */

   short is_header;  /* flag specifying whether area is a header area */
   short is_active;  /* used by CR exe to indicate selected area */

   EXPR4 *suppression_condition;

   GROUP4 *group;    /* containing group */

   short allow_pagebreaks;  /* user set flag specifying whether an area can
                               be broken across page boundaries in output */

   LIST4 objects;    /* list of output objects in the area */

   struct REPORT4_st *report;  /* containing report */

   /* in CR each area has a window associated with it */
   #ifdef S4WINDOWS
      HWND hWnd;
   #endif

} AREA4, S4PTR *PAREA4;


/******************************************************************************/
/* For non windows output each page of the report is placed in a buffer file.
   the information is in the format of a series of OBJECT4 structures.
*/
typedef struct OBJECT4_st
{
   short objtype;    /* type of object. ie. obj4text, etc. */
   short alignment;  /* alignment of objects text, ie. justify4left */
   long x, y, w, h;  /* objects position in page and size in 1000's of inches */

   long info_len;    /* length of the info block */
   void *info;       /* information for the objects output, usually a text
                        string */

   short style_index; /* index into the reports style list */

} OBJECT4, S4PTR *POBJECT4;


/******************************************************************************/
/* PAGE4 structure is used for buffering the pages of the report in
   non-windows output
*/
typedef struct PAGE4_st
{
   FILE4  file_buf;
   FILE4SEQ_READ seq_rd;
   FILE4SEQ_WRITE seq_wr;
   char fmem_buf[1024];
   char rmem_buf[1024];
   char *mem_buf, *mem_buf_pos;

   short first_read;
   long info_buf_len;

   OBJECT4 object;

} PAGE4, S4PTR *PPAGE4;


/******************************************************************************/
/* REPORT4 structure.  This is the main report structure.
*/

typedef struct REPORT4_st
{
   CODE4 *code_base;  /* applications CODE4 structure */
   RELATE4 *relate;   /* the reports associated relation */

   char *report_name;         /* name of the report */
   char *report_file_name;    /* name of the file holding the report, with path */

   char *printer_name;
   #ifdef S4WINDOWS
      HDC printerDC;
   #endif

   char *stylesheet_name;
   char *report_caption;
   char default_dateFormat[20];

   GROUP4 *title_summary; /* GROUP4 holding title and summary areas.  This
                             group is also in the groups list */

   GROUP4 *page_header_footer; /* GROUP4 holding page header and footer. This
                                  group is not in the group list */

   LIST4 styles;               /* STYLE4 structs associated with report*/
   LIST4 groups;               /* list of the reports GROUP4 structs */
   LIST4 active_objects;       /* a list of the selected OBJ4 structures. This
                                  list is used by the CR .exe */

   LIST4 obj_type_info;        /* a list of OBJ4TYPE structures */
   LIST4 *obj_type_list;

   STYLE4 *active_style;

   long  sensitivity_x, sensitivity_y;
   int   dsx, dsy;
   short adjust_all;
   short units;
   short hide_info;
   short screen_breaks;
   short pgbrk_title;
   short pathnames;
   short bitmaps;

   int output_handle, use_styles;

   long report_width, report_height;
   long margin_top, margin_bottom, margin_right, margin_left;
   long hard_margin_top, hard_margin_bottom, hard_margin_right, hard_margin_left;
   long page_no, start_page, end_page, page_count;
   short page_nums;

   char decimal_point, thousand_separator, currency_symbol[11];

   /* info used in generating report output */
   short hard_resets_flag;
   short first;
   int output_code;
   long dev_page_height, dev_page_width;
   long dev_margin_left, dev_margin_top, dev_margin_right, dev_margin_bottom;
   long dev_hard_margin_top, dev_hard_margin_bottom, dev_hard_margin_right, dev_hard_margin_left;
   long disp_bottom;
   long ypos, break_height;
   short in_header, end_report, broken, tdone;
   GROUP4 *group_on, *group_first;
   AREA4  *area_on;

   /* for use in generating a data file from report output */
   short   for_dbf;
   LIST4   output_objs;
   PGROUP4 output_group;
   FILE4SEQ_WRITE dfile_seq;
   char *dfile_name, *dfile_buf;
   DATA4 *data_file;
   long rcount;

   /* for non-windows report output */
   PAGE4 page_buf;

   #ifdef S4WINDOWS
      HWND hWnd;
      HWND hWnd2;
      HWND pagewin;
   #endif
} REPORT4, S4PTR *PREPORT4;


/******************************************************************************/

/* pre declarations */
struct  _OBJ_TYPE_struct;
struct  _OBJECT_struct;

/* OBJ4TYPE structure is used to hold information for each object type.
   currently this is limited to the object type name, windows class name,
   and the menu and cursor to be used in CR.  In the future this structure
   will be used to allow the import of third party or custom output objects */
typedef struct _OBJ_TYPE_struct
{
   LINK4 link;

   char name[26];
   char wndclass_name[46];
   int obj_type;
   #ifdef S4WINDOWS
   HINSTANCE lib_instance;

   HMENU hMenu;
   HCURSOR hCursor;
   #endif
} OBJ4TYPE;


/******************************************************************************/


typedef struct _OBJECT_struct
{
   LINK4 link;
   LINK4 link2;

   /* position within the containing area, and size, in 1000's of an inch */
   long x, y;
   long w, h;

   int num_chars;

   /* flags used by CR .exe for tagging selected objects */
   short is_active, is_first;

   /* flag and expression used for display once functionality.  this works in
      a similar fashion to the group reset */
   short display_once;
   EXPR4 *display_once_expr;
   char  *last_display_val;

   AREA4 *area; /* containing area */

   /* this is the text displayed in the object in the CR layout screen. In the
      case of a text object it is also the text for output */
   char *wintext;

   char *dateFormat;
   int date_changed;

   /* text to be output and length of text*/
   char *eval_text;
   int eval_len;

   /* style to use for objects output */
   STYLE4 *style;
   short style_location;

   short lookahead;

   OBJ4TYPE *obj_type;
   struct _OBJECT_struct *container;  /* pointer to containing object */

   LIST4 contained;           /* list of pointers to contained objects */

   short obj_type_num;

   short alignment; /* justify4left, justify4right, justify4center,
                       0=horizontal line, 1 = vertical line
                       0=squared frame, 1 = rounded frame */

   short numeric_type, display_zero, dec, use_brackets, leading_zero;
   short mod_type, background;

   void *data; /* generic pointer used to hold object specific info. eg. for
                  an expression object this will be an expression */

   long dev_x, dev_y, dev_w, dev_h;

   /* these six members are used for sending report output to an data file */
   double dval;
   FIELD4 *field;
   char field_name[11];
   char field_type;
   int  field_len;
   int  field_dec;

   #ifdef S4WINDOWS
      HWND hWnd;
   #endif

} OBJ4, S4PTR *POBJ4;


/******************************************************************************/


typedef struct OUTOBJLIST_st
{
   LINK4 link;
   POBJ4 obj;
} OUT4OBJ, S4PTR *POUT4OBJ;


/******************************************************************************/


typedef struct INDEXLIST_st
{
   LINK4 link;
   char  *index_name;
   int   name_length;
}INAME4;


/******************************************************************************/


typedef struct N4CHANGE_st
{
   LINK4 link;
   char  *old_name;
   char  *new_name;
} N4CHANGE, S4PTR *PN4CHANGE;


/******************************************************************************/

/* structure used by CR .exe for creating an instant report */
#ifdef S4CR2
   typedef struct _inst4report
   {
      REPORT4 *report;
      LIST4   groups;
      LIST4   fields;
      LIST4   totals;
      char    *title, *sort_expr, *query_expr;
      int     hspace, vspace, max_obj_width;
      BOOL    vert_wrap, labels, labels_ingroup;
   } INST4REPORT, S4PTR *PINST4REPORT;


/******************************************************************************/


   typedef struct _inst4group
   {
      LINK4  link;
      char   *group_expr;
   }INST4GROUP, S4PTR *PINST4GROUP;


/******************************************************************************/


   typedef struct _inst4field
   {
      LINK4 link;
      FIELD4 *field;
   }INST4FIELD, S4PTR *PINST4FIELD;


/******************************************************************************/


   typedef struct _inst4total
   {
      LINK4 link;
      char *total_expr;
      PINST4GROUP igroup;
      PINST4FIELD ifield;
   }INST4TOTAL, S4PTR *PINST4TOTAL;

#endif


/******************************************************************************/


#ifdef __cplusplus
  extern "C" {
#endif

PREPORT4 S4FUNCTION   report4init( RELATE4 S4PTR *);
void S4FUNCTION       report4free( PREPORT4, int, int );

PGROUP4 S4FUNCTION    group4create( PREPORT4, char S4PTR *, char S4PTR * );
void S4FUNCTION       group4free( PGROUP4 );
void S4FUNCTION       group4positionSet( PGROUP4, int );
PGROUP4 S4FUNCTION    group4create_title_summary( PREPORT4 );
PGROUP4 S4FUNCTION    group4create_pgheader_pgfooter( PREPORT4 );
int      S4FUNCTION   group4swapHeader( PGROUP4, int );
int      S4FUNCTION   group4swapFooter( PGROUP4, int );
int      S4FUNCTION   group4repeatHeader( PGROUP4, int );
int      S4FUNCTION   group4resetPage( PGROUP4, int );
int      S4FUNCTION   group4resetPageNum( PGROUP4, int );


PAREA4 S4FUNCTION     area4create( PGROUP4, long, short, char S4PTR * );
void S4FUNCTION       area4free( PAREA4 );
void S4FUNCTION       area4add_object( PAREA4, POBJ4 );

int S4FUNCTION        report4save( PREPORT4, char S4PTR *, int );

int S4FUNCTION        relate4save( RELATE4 S4PTR *, char S4PTR *, int );
PREPORT4 S4FUNCTION  report4retrieve( CODE4 S4PTR *, char S4PTR *, int, char S4PTR * );
RELATE4 S4PTR * S4FUNCTION  relate4retrieve( CODE4 S4PTR *, char S4PTR *, int, char S4PTR * );

RELATE4 * S4FUNCTION relate4retrieve2( CODE4 *c4, char *file_name,
                      int open_files, char *pathname, char *buf, char *name_buf );
REPORT4 * S4FUNCTION  report4retrieve2(CODE4 *c4, char *file_name, int open_files,
                        char *pathname, char *buf, char *name_buf );
int S4FUNCTION   relate4save2( RELATE4 *relate, char *file_name,
                        int save_paths, char *buf, char *name_buf );


POBJ4 S4FUNCTION      obj4create( PAREA4, long, long, long, long );
void S4FUNCTION       obj4free( POBJ4 );

POBJ4 S4FUNCTION      obj4fieldCreate( PAREA4, FIELD4 S4PTR *, long, long, long, long );
void S4FUNCTION       obj4fieldFree( POBJ4 );

POBJ4 S4FUNCTION      obj4totalCreate( PAREA4, PTOTAL4, long, long, long, long );
void S4FUNCTION       obj4totalFree( POBJ4 );

POBJ4 S4FUNCTION      obj4calcCreate( PAREA4, EXPR4CALC S4PTR *, long, long, long, long );
void S4FUNCTION       obj4calcFree( POBJ4 );

POBJ4 S4FUNCTION      obj4exprCreate( PAREA4, EXPR4 S4PTR *, long, long, long, long );
void S4FUNCTION       obj4exprFree( POBJ4 );

POBJ4 S4FUNCTION      obj4textCreate( PAREA4, char S4PTR *, long, long, long, long );
void S4FUNCTION       obj4textFree( POBJ4 );

POBJ4 S4FUNCTION      obj4lineCreate( PAREA4, int, long, long, long );
void S4FUNCTION       obj4lineFree( POBJ4 );
int S4FUNCTION obj4lineWidth( POBJ4, long );

POBJ4 S4FUNCTION      obj4frameCreate( PAREA4, long, long, long, long );
void S4FUNCTION       obj4frameFree( POBJ4 );

void S4FUNCTION       obj4delete( POBJ4 );
void S4FUNCTION       obj4sort_obj_tree( POBJ4 );
void                obj4evaluate( POBJ4 );
void                obj4evaluate_lookahead( POBJ4 );
int S4FUNCTION obj4justify( POBJ4, int );
int S4FUNCTION obj4dateFormat( POBJ4, char S4PTR * );
int S4FUNCTION obj4decimals( POBJ4, int );
int S4FUNCTION obj4displayOnce( POBJ4, char S4PTR * );
int S4FUNCTION obj4displayZero( POBJ4, int );
int S4FUNCTION obj4numericType( POBJ4, int );
int S4FUNCTION obj4leadingZero( POBJ4, int );
int S4FUNCTION obj4brackets( POBJ4, int );
int S4FUNCTION obj4style( POBJ4, PSTYLE4 );

int S4FUNCTION report4querySet( PREPORT4, char S4PTR * );
int S4FUNCTION report4sortSet( PREPORT4, char S4PTR * );

int report4conv_double( POBJ4, double, char * );

PTOTAL4 S4FUNCTION    total4create( PREPORT4, char S4PTR *, char S4PTR *, int, char S4PTR * );
void S4FUNCTION       total4value_reset( PTOTAL4 );
void S4FUNCTION       total4free( PTOTAL4 );
void                  total4value_update( PTOTAL4 );

#ifdef S4WINDOWS
PSTYLE4 S4FUNCTION     style4create( PREPORT4, PR4LOGFONT, char S4PTR *, R4COLORREF, int );
#else
PSTYLE4 S4FUNCTION     style4create( PREPORT4, char S4PTR *, int, char S4PTR *, int, char S4PTR * );
#endif
PSTYLE4 S4FUNCTION     style4lookup( PREPORT4, char S4PTR * );
BOOL S4FUNCTION        style4delete( PREPORT4, char S4PTR * );
BOOL S4FUNCTION        style4free( PREPORT4, PSTYLE4 );
int S4FUNCTION         style4color( PSTYLE4, R4COLORREF );
int S4FUNCTION         style4default_set( PREPORT4, PSTYLE4 );

int S4FUNCTION report4do( PREPORT4 );

int S4FUNCTION report4save_object( FILE4SEQ_WRITE S4PTR *, POBJ4 );
int S4FUNCTION report4save_area( FILE4SEQ_WRITE S4PTR *, PAREA4 );
int S4FUNCTION report4save_group( FILE4SEQ_WRITE S4PTR *, PGROUP4 );
int S4FUNCTION report4save_style( FILE4SEQ_WRITE S4PTR *, PSTYLE4 );
int S4FUNCTION report4save_calc( FILE4SEQ_WRITE S4PTR *, EXPR4CALC S4PTR * );
int S4FUNCTION report4save_report( PREPORT4, FILE4SEQ_WRITE S4PTR * );

int S4FUNCTION report4retrieve_object( FILE4SEQ_READ S4PTR *, PAREA4 );
int S4FUNCTION report4retrieve_area( FILE4SEQ_READ S4PTR *, PGROUP4 );
int S4FUNCTION report4retrieve_group( FILE4SEQ_READ S4PTR *, PREPORT4 );
int S4FUNCTION report4retrieve_page_header_footer( FILE4SEQ_READ S4PTR *, PREPORT4 );
PSTYLE4 S4FUNCTION report4retrieve_style( FILE4SEQ_READ S4PTR *, PREPORT4 );
int S4FUNCTION report4retrieve_calc( FILE4SEQ_READ S4PTR *, PREPORT4 );
REPORT4 * S4FUNCTION   report4retrieve_report( FILE4SEQ_READ S4PTR *, RELATE4 S4PTR * );

int S4FUNCTION retrieve4string( FILE4SEQ_READ S4PTR *, char S4PTR *, int );
DATA4 S4PTR *  S4FUNCTION relate4lookup_data( RELATE4 S4PTR *, char S4PTR * );

int S4FUNCTION report4caption( PREPORT4, char S4PTR * );
int S4FUNCTION obj4frameCorners( POBJ4, int );
int S4FUNCTION obj4frameFill( POBJ4, int );
int S4FUNCTION area4pageBreak( PAREA4, int );

int S4FUNCTION report4groupHardResets( PREPORT4, int );

int report4alloc_records( PREPORT4 );
int report4swap_old_rec( PREPORT4 );
int report4make_old_rec( PREPORT4 );
int report4output_area_break( PAREA4 );

#ifdef S4WINDOWS
int S4FUNCTION report4generatePage( PREPORT4, HDC );
int report4output_object( OBJ4 *, HDC );
int report4output_area( AREA4 *, HDC, int );
void report4output_pgheader( REPORT4 *, HDC hDC );
void report4output_pgfooter( REPORT4 *, HDC hDC );
void report4output_repeat_headers( REPORT4 *, HDC, GROUP4 * );
void report4output_swapped_footer( GROUP4 *, HDC );
HDC S4FUNCTION report4get_printerDC ( void );
void report4output_screen_fonts( REPORT4 * );
int report4register_classes( PREPORT4 );
void report4calc_obj_dev( REPORT4 *, HDC );
void S4FUNCTION obj4bitmapFileFree( POBJ4 obj );
POBJ4 S4FUNCTION obj4bitmapFileCreate( PAREA4 area, char *filename, long x, long y, long w, long h );
void S4FUNCTION obj4bitmapFieldFree( POBJ4 obj );
POBJ4 S4FUNCTION obj4bitmapFieldCreate( PAREA4 area, FIELD4 *field, long x, long y, long w, long h );
HINSTANCE CALLBACK c4dll_inst(void );
#else
int S4FUNCTION report4generatePage( PREPORT4 );
int report4output_object( OBJ4 *, PAGE4 *);
int report4output_area( AREA4 *, PAGE4 *, int );
void report4output_pgheader( REPORT4 *, PAGE4 *);
void report4output_pgfooter( REPORT4 *, PAGE4 *);
void report4output_repeat_headers( REPORT4 *, PAGE4 *, GROUP4 * );
void report4output_swapped_footer( GROUP4 *, PAGE4 *);
void report4calc_obj_dev( REPORT4 * );
void style4flagsGet( PSTYLE4 style, int *pt_size, int *bold, int *italic, int *underline, int *strikeout );
int style4equivFontGet( CODE4 *code_base, PSTYLE4 style, char *buf, int bufsize );
void style4equivFontsRemove( CODE4 *code_base );
int S4FUNCTION style4equivFontRegister( CODE4 *code_base, char *winfont, char *equivfont );
#endif
void report4evaluate_lookahead( GROUP4 * );
void report4check_lookahead( REPORT4 * );
void report4check_display_once( PREPORT4 );

GROUP4 * S4FUNCTION report4calc_first_change_group( REPORT4 * );

#ifdef S4WINDOWS
HDC S4FUNCTION report4get_printerIC ( PREPORT4 );
#ifndef S4WIN32
long CALLBACK _export PreViewProc( HWND, UINT, WPARAM, LPARAM );
long CALLBACK _export OutputPageProc( HWND, UINT, WPARAM, LPARAM );
long CALLBACK _export MouseEatProc( HWND, UINT, WPARAM, LPARAM );
long CALLBACK _export PageStatProc( HWND, UINT, WPARAM, LPARAM );
#else
long CALLBACK PreViewProc( HWND, UINT, WPARAM, LPARAM );
long CALLBACK OutputPageProc( HWND, UINT, WPARAM, LPARAM );
long CALLBACK MouseEatProc( HWND, UINT, WPARAM, LPARAM );
#endif
void S4FUNCTION obj4bitmapStaticFree( POBJ4 obj );
POBJ4 S4FUNCTION obj4bitmapStaticCreate( PAREA4 area, HANDLE hDIB, long x, long y, long w, long h );

LPSTR S4FUNCTION FindDIBBits (LPSTR );
WORD S4FUNCTION PaletteSize (LPSTR );
WORD S4FUNCTION DIBNumColors (LPSTR );

BOOL S4FUNCTION WriteDIB ( LPSTR, HANDLE );
HANDLE S4FUNCTION ReadDIBFile (int);
HANDLE S4FUNCTION GetDIB ( LPSTR, CODE4 S4PTR * );

void S4FUNCTION report4printerSelect( PREPORT4 report );
int S4FUNCTION report4parent( PREPORT4, HWND );
HDC S4FUNCTION report4printerDC( PREPORT4, HDC );
#endif

int S4FUNCTION expr4calc_remove( EXPR4CALC *calc, int remove_all );
int S4FUNCTION expr4calc_modify( EXPR4CALC *calc, char *expr_source );

int S4FUNCTION report4output( PREPORT4 report, int output_handle, int use_styles );
int S4FUNCTION report4margins( PREPORT4 report, long left, long right, long top, long bottom, int unit_type );
int S4FUNCTION report4pageSize( PREPORT4 report, long pg_height, long pg_width, int unit_type );

int S4FUNCTION report4deleteCalc( PREPORT4 report, EXPR4CALC S4PTR *del_calc );

int S4FUNCTION report4styleSheetSave( REPORT4 *report, char *file_name );
int S4FUNCTION report4styleSheetLoad( REPORT4 *report, char *file_name, int override );
int S4FUNCTION report4currency( PREPORT4, char * );
int S4FUNCTION report4decimal( PREPORT4, char );
int S4FUNCTION report4separator( PREPORT4, char );

int S4FUNCTION report4pageInit( PREPORT4 report );
int S4FUNCTION report4pageFree( PREPORT4 report );
POBJECT4 S4FUNCTION report4pageObjNext( PREPORT4 report );
POBJECT4 S4FUNCTION report4pageObjFirst( PREPORT4 report );
int S4FUNCTION report4toScreen( PREPORT4 report, int to_screen );

PSTYLE4 S4FUNCTION report4styleFirst( PREPORT4 report );
PSTYLE4 S4FUNCTION report4styleNext( PREPORT4 report, PSTYLE4 style );
PSTYLE4 S4FUNCTION report4styleLast( PREPORT4 report );
PSTYLE4 S4FUNCTION report4stylePrev( PREPORT4 report, PSTYLE4 style );
int S4FUNCTION report4numStyles( PREPORT4 report );

PGROUP4 S4FUNCTION report4groupFirst( PREPORT4 report );
PGROUP4 S4FUNCTION report4groupNext( PREPORT4 report, PGROUP4 group );
PGROUP4 S4FUNCTION report4groupLast( PREPORT4 report );
PGROUP4 S4FUNCTION report4groupPrev( PREPORT4 report, PGROUP4 group );
int S4FUNCTION report4numGroups( PREPORT4 report );
PGROUP4 S4FUNCTION report4titleSummary( PREPORT4 report );
PGROUP4 S4FUNCTION report4pageHeaderFooter( PREPORT4 report );

PAREA4 S4FUNCTION group4headerFirst( PGROUP4 group );
PAREA4 S4FUNCTION group4headerNext( PGROUP4 group, PAREA4 area );
PAREA4 S4FUNCTION group4headerLast( PGROUP4 group );
PAREA4 S4FUNCTION group4eaderPrev( PGROUP4 group, PAREA4 area );
int S4FUNCTION area4numHeaders( PGROUP4 group );
PAREA4 S4FUNCTION group4footerFirst( PGROUP4 group );
PAREA4 S4FUNCTION group4footerNext( PGROUP4 group, PAREA4 area );
PAREA4 S4FUNCTION group4footerLast( PGROUP4 group );
PAREA4 S4FUNCTION group4footerPrev( PGROUP4 group, PAREA4 area );
int S4FUNCTION group4numFooters( PGROUP4 group );

int S4FUNCTION report4titlePage( PREPORT4 report, int title_page );

TOTAL4 * S4FUNCTION total4lookup( REPORT4 *report, char *name );
RELATE4 * S4FUNCTION   relate4retrieve_relate( FILE4SEQ_READ *seq, int open_files, char *spath );
int S4FUNCTION report4index_type( void );

PGROUP4 S4FUNCTION report4groupLookup( PREPORT4 report, char *group_name );
int S4FUNCTION group4resetExprSet( PGROUP4 group, char *expr_src );

PREPORT4 S4FUNCTION report4retrieve_old(CODE4 *c4,char *file_name,int open_files);

#ifdef S4CR2
int S4FUNCTION AlternateDataFile( char S4PTR *dname_buf, int len );
int S4FUNCTION AlternateIndexFile( char S4PTR *iname_buf, int len );
int S4FUNCTION AlternateTagName( char S4PTR *tname_buf, int len );
int S4FUNCTION AlternateMasterExpression( char S4PTR *expr_buf, RELATE4 S4PTR *relate, int len );
int S4FUNCTION AlternateSortExpression(  char S4PTR *expr_buf, RELATE4 S4PTR *relate, int len );
int S4FUNCTION AlternateQueryExpression( char S4PTR *expr_buf, RELATE4 S4PTR *relate, int len );
#endif

#ifdef S4VBASIC
#ifdef S4WINDOWS
   long S4FUNCTION vbRepWndProc( HWND, UINT, WPARAM, LPARAM ) ;
   void S4FUNCTION report4registerVBClass( REPORT4 * ) ;
#endif
   int S4FUNCTION report4decimal_v( PREPORT4 report, char *decimal ) ;
   int S4FUNCTION report4separator_v( PREPORT4, char * ) ;
#endif

POBJ4 S4FUNCTION area4objFirst( PAREA4 area );
POBJ4 S4FUNCTION area4objNext( PAREA4 area, POBJ4 aobj );
POBJ4 S4FUNCTION area4objPrev( PAREA4 area, POBJ4 aobj );
POBJ4 S4FUNCTION area4objLast( PAREA4 area );

void S4FUNCTION obj4remove( POBJ4 obj );
void S4FUNCTION area4sort_obj_tree( PAREA4 area );
int S4FUNCTION report4pageMarginsGet( PREPORT4, long S4PTR *, long S4PTR*, long S4PTR *, long S4PTR * );
int S4FUNCTION report4pageSizeGet( PREPORT4, long S4PTR *,long S4PTR * );
void report4nchange( CODE4 *, char **, int, int );
void report4free_name_list( void );
PSTYLE4 S4FUNCTION report4styleSelected( PREPORT4 );
int S4FUNCTION report4styleSelect( PREPORT4, PSTYLE4 );
PSTYLE4 S4FUNCTION style4index( PREPORT4, int );

int S4FUNCTION total4addCondition( PTOTAL4, char *, int );
int S4FUNCTION obj4dataFieldSet( POBJ4 obj, char *fname, char ftype, int flength, int fdec );
int S4FUNCTION report4dataFileSet( PREPORT4 report, char *fname );
int S4FUNCTION report4dataDo( PREPORT4 report );
int S4FUNCTION report4dataGroup( PREPORT4, PGROUP4 );

int S4FUNCTION obj4lookAhead( POBJ4, int );


#ifdef __cplusplus
 }
#endif

#endif /* __REPORT4HEADER */

