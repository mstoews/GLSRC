/* ctrl4.h  (c)Copyright Sequiter Software Inc., 1990-1994.  All rights reserved. */
  
#ifdef S4VBX
   #define  PUBLIC4
#else                    
   #define PUBLIC4  FAR PASCAL _export
#endif

#define  CTRL4NORMAL          0
#define  CTRL4MASK            1
#define  CTRL4DATE            2
#define  CTRL4NUMERIC         4

#define  CTRL4RAISED          1
#define  CTRL4INSET           0

#define  CTRL4LEFT            (int) ES_LEFT
#define  CTRL4RIGHT           (int) ES_RIGHT
#define  CTRL4CENTER          (int) ES_CENTER

#define  CTRL4STRIPNONE       0
#define  CTRL4STRIPDATA       8
#define  CTRL4STRIPCLIP       16
#define  CTRL4STRIPALL        32

#define  COMBO4DROPDOWN       0
#define  COMBO4SIMPLE         1
#define  COMBO4DROPDOWNLIST   2

#define  CTRL4VGA             0
#define  CTRL4SVGA            1

#define  CTRL4COMBO           1
#define  CTRL4EDIT            2
#define  CTRL4LISTBOX         4
#define  CTRL4MASTER          8

#define  CTRL4SEPCHAR         '@'
#define  BUTTON4OFFSET        7
#define  MASTER4NORMAL        0
#define  MASTER4SUPERVGA      1
#define  MASTER4HEIGHT        24
#define  MASTER4WIDTH         30
#define  MASTER4SVGAHEIGHT    30
#define  MASTER4SVGAWIDTH     40

#define  MASTER4ABORT         3
#define  MASTER4RETRY         4
#define  MASTER4IGNORE        5
#define  MASTER4VALID         2
#define  MASTER4INVALID       1

#define  MASTER4TOP           1
#define  MASTER4PREV          2
#define  MASTER4PREVIOUS      2
#define  MASTER4SEARCH        4
#define  MASTER4NEXT          8
#define  MASTER4BOTTOM        16
#define  MASTER4ADD           32
#define  MASTER4APPEND        32
#define  MASTER4DEL           64
#define  MASTER4DELETE        64
#define  MASTER4UNDO          128
#define  MASTER4GO            1056
#define  MASTER4SKIP          288
#define  MASTER4FLUSH         544
#define  MASTER4ALL           255

#define  MS_TOP               0x00000001L
#define  MS_PREV              0x00000002L
#define  MS_SEARCH            0x00000004L
#define  MS_NEXT              0x00000008L
#define  MS_BOTTOM            0x00000010L
#define  MS_ADD               0x00000020L
#define  MS_DEL               0x00000040L
#define  MS_UNDO              0x00000080L
#define  MS_VGA               0x00000200L
#define  MS_SUPERVGA          0x00000100L

#define  CBS_LEFT             0x0000
#define  CBS_CENTER           0x1000
#define  CBS_RIGHT            0x2000

#define  r4valid              100
#define  r4invalid            110

#define  EDIT4UPDATE          WM_USER + 501

#define  CWN_LOCKED           WM_USER + 502
#define  CWN_NOUPDATE         WM_USER + 503
#define  CWM_LISTSCROLL       WM_USER + 505
#define  CWM_SEARCHSTR        WM_USER + 506
#define  CWM_VALIDATE         WM_USER + 507
#define  CWM_KEYDOWN          WM_USER + 508
#define  CWM_KEYPRESS         WM_USER + 509
#define  CWM_KEYUP            WM_USER + 510
#define  CWN_SETFOCUS         WM_USER + 511
#define  CWN_KILLFOCUS        WM_USER + 512
#define  CWN_APPEND           WM_USER + 513
#define  CWN_BOTTOM           WM_USER + 514
#define  CWN_DELETE           WM_USER + 515
#define  CWN_NEXT             WM_USER + 516
#define  CWN_PREVIOUS         WM_USER + 517
#define  CWN_SEARCH           WM_USER + 518
#define  CWN_TOP              WM_USER + 519
#define  CWN_UNDO             WM_USER + 520
#define  CWN_REPOSITION       WM_USER + 521
#define  CWM_ERROR            WM_USER + 522
#define  CWN_UNIQUE           WM_USER + 523

#define  BTN4LEFT             WM_USER + 540
#define  BTN4RIGHT            WM_USER + 541

/*
#define  CTRL4ALIGNMENT       0
#define  CTRL4BEVELWIDTH      1
#define  CTRL4BORDER          2
#define  CTRL4DECIMALCHAR     3
#define  CTRL4DECIMALS        4
#define  CTRL4DIRECTION       5
#define  CTRL4ENABLED         6
#define  CTRL4FONT            7
#define  CTRL4FORMATSTRIPPING 8
#define  CTRL4MAXLENGTH       9
#define  CTRL4POSITIVEONLY    10
#define  CTRL4SEPARATOR       11
#define  CTRL4SEPARATORCHAR   12
#define  CTRL4SORTED          13
#define  CTRL4TYPE            14

#define  LISTBOX4COLUMNS      15
#define  LISTBOX4MULTISELECT  16

#define  MASTER4LOCKATTEMPTS  17
#define  MASTER4READLOCK      18
#define  MASTER4SIZE          19

#define  CTRL4BACKCOLOR       20
#define  CTRL4HIGHLIGHTCOLOR  21
#define  CTRL4POSLEADER       22
#define  CTRL4NEGLEADER       23
#define  CTRL4NEGATIVECOLOR   24
#define  CTRL4SHADOWCOLOR     25
#define  CTRL4TEXTCOLOR       26
#define  CTRL4POSTRAILER      27
#define  CTRL4NEGTRAILER      28
*/
/***************************************************************\
\***************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
int      PUBLIC4 combo4editData(HWND hwnd,DATA4 *editdata,FIELD4 *field);
int      PUBLIC4 combo4listData(HWND hwnd,DATA4 *listdata,EXPR4 *expr);
int      PUBLIC4 combo4search(HWND hwnd,char *key);

int      PUBLIC4 ctrl4alignment(HWND hwnd,int alignment);
int      PUBLIC4 ctrl4alignmentGet(HWND hwnd);
int      PUBLIC4 ctrl4assignDouble(HWND hwnd,double value);
int      PUBLIC4 ctrl4assignInt(HWND hwnd,int value);
int      PUBLIC4 ctrl4assignLong(HWND hwnd,long value);
int      PUBLIC4 ctrl4assignStr(HWND hwnd,char *str,int stripped);
int      PUBLIC4 ctrl4backColor(HWND hwnd,COLORREF color);
COLORREF PUBLIC4 ctrl4backColorGet(HWND hwnd);
int      PUBLIC4 ctrl4bevelWidth(HWND hwnd,int width);
int      PUBLIC4 ctrl4bevelWidthGet(HWND hwnd);
int      PUBLIC4 ctrl4bevel(HWND hwnd,COLORREF bkcolor,COLORREF shcolor,COLORREF hlcolor,COLORREF txcolor,int thickness,int direction,int border);
int      PUBLIC4 ctrl4border(HWND hwnd,int border);
int      PUBLIC4 ctrl4borderGet(HWND hwnd);
int      PUBLIC4 ctrl4bottom(HWND hwnd);
void     PUBLIC4 ctrl4decimalChar(HWND hwnd,char dec);
char     PUBLIC4 ctrl4decimalCharGet(HWND hwnd);
int      PUBLIC4 ctrl4decimals(HWND hwnd,int decimals);
int      PUBLIC4 ctrl4decimalsGet(HWND hwnd);
int      PUBLIC4 ctrl4direction(HWND hwnd,int direction);
int      PUBLIC4 ctrl4directionGet(HWND hwnd);
double   PUBLIC4 ctrl4double(HWND hwnd);
void  	PUBLIC4 ctrl4double2(HWND hwnd,double *value);
int      PUBLIC4 ctrl4enabled(HWND hwnd,int enabled);
int      PUBLIC4 ctrl4font(HWND hwnd,HFONT font);
HFONT    PUBLIC4 ctrl4fontGet(HWND hwnd);
int      PUBLIC4 ctrl4formatStripping(HWND hwnd,int strip);
int      PUBLIC4 ctrl4formatStrippingGet(HWND hwnd);
int      PUBLIC4 ctrl4go(HWND hwnd,long recno);
int      PUBLIC4 ctrl4highlightColor(HWND hwnd,COLORREF color);
COLORREF PUBLIC4 ctrl4highlightColorGet(HWND hwnd);

#ifndef S4VBX
void     PUBLIC4 ctrl4init(CODE4 *code,HINSTANCE hInstance);
void     PUBLIC4 ctrl4initCPP(CODE4 *code,HINSTANCE hInstance );
int      PUBLIC4 ctrl4initUndo(CODE4 *code,HINSTANCE hInstance);
int      PUBLIC4 ctrl4initUndoCPP(CODE4 *code,HINSTANCE hInstance );
#endif

int      PUBLIC4 ctrl4int(HWND hwnd);
void     PUBLIC4 ctrl4leader(HWND hwnd,char *pos,char *neg);
void     PUBLIC4 ctrl4leaderGet(HWND hwnd,char *posBuf,char *negBuff,int buffSize);
long     PUBLIC4 ctrl4long(HWND hwnd);
int      PUBLIC4 ctrl4negativeColor(HWND hwnd,COLORREF color);
COLORREF PUBLIC4 ctrl4negativeColorGet(HWND hwnd);
int      PUBLIC4 ctrl4mask(HWND hwnd,char *mask);
void     PUBLIC4 ctrl4maskGet(HWND hwnd,char *maskBuf,int bufSize);
int      PUBLIC4 ctrl4maxLength(HWND hwnd,int len);
int      PUBLIC4 ctrl4positiveOnly(HWND hwnd,int pos);
int      PUBLIC4 ctrl4positiveOnlyGet(HWND hwnd);
long     PUBLIC4 ctrl4recNo(HWND hwnd);
int      PUBLIC4 ctrl4seek(HWND hwnd,char *key);
int      PUBLIC4 ctrl4separator(HWND hwnd,int sep);
int      PUBLIC4 ctrl4separatorGet(HWND hwnd);
void     PUBLIC4 ctrl4separatorChar(HWND hwnd,char sep);
char     PUBLIC4 ctrl4separatorCharGet(HWND hwnd);
int      PUBLIC4 ctrl4shadowColor(HWND hwnd,COLORREF color);
COLORREF PUBLIC4 ctrl4shadowColorGet(HWND hwnd);
int      PUBLIC4 ctrl4skip(HWND hwnd,long numrecs);
int      PUBLIC4 ctrl4sorted(HWND hwnd,int sort);
int      PUBLIC4 ctrl4sortedGet(HWND hwnd);
int      PUBLIC4 ctrl4str(HWND hwnd,char *text,int len,int strip);
int      PUBLIC4 ctrl4strLength(HWND hwnd);
int      PUBLIC4 ctrl4strListLength(HWND hwnd,long ref);
int      PUBLIC4 ctrl4strList(HWND hwnd,char *text,int len,long ref);
int      PUBLIC4 ctrl4tag(HWND hwnd,TAG4 *tag);
int      PUBLIC4 ctrl4tagFind(HWND hwnd,char *tag);
int      PUBLIC4 ctrl4textColor(HWND hwnd,COLORREF color);
COLORREF PUBLIC4 ctrl4textColorGet(HWND hwnd);
int      PUBLIC4 ctrl4top(HWND hwnd);
void     PUBLIC4 ctrl4trailer(HWND hwnd,char *pos,char *neg);
void     PUBLIC4 ctrl4trailerGet(HWND hwnd,char *posBuf,char *negBuff,int buffSize);
int      PUBLIC4 ctrl4type(HWND hwnd,int edittype);
int      PUBLIC4 ctrl4typeGet(HWND hwnd);
int      PUBLIC4 ctrl4update(HWND hwnd);
int      PUBLIC4 ctrl4validation(HWND hwnd,char *validation);

int      PUBLIC4 edit4data(HWND hwnd,DATA4 *data,FIELD4 *field);

int      PUBLIC4 listbox4columns(HWND hwnd,int columns);
int      PUBLIC4 listbox4data(HWND hwnd,DATA4 *data,EXPR4 *expr);
int      PUBLIC4 listbox4multiSelect(HWND hwnd,int multiselect);
int      PUBLIC4 listbox4searchType(HWND hwnd,int type);

int      PUBLIC4 master4append(HWND hwnd);
int      PUBLIC4 master4bindEdit(HWND hwnd,HWND control,FIELD4 *field);
int      PUBLIC4 master4bindList(HWND hwnd,HWND control,EXPR4 *expr,TAG4 *tag);
int      PUBLIC4 master4changed(HWND hwnd,int changed);
int      PUBLIC4 master4data(HWND hwnd,DATA4 *data,EXPR4 *expr);
int      PUBLIC4 master4delete(HWND hwnd);
void     PUBLIC4 master4displayButtons(HWND hwnd,int buttons);
int      PUBLIC4 master4flush(HWND hwnd);
int      PUBLIC4 master4lockAttempts(HWND hwnd,int lockAttempts);
int      PUBLIC4 master4readLock(HWND hwnd,int readLock);
int      PUBLIC4 master4reply(HWND hwnd,int reply);
int      PUBLIC4 master4search(HWND hwnd);
int      PUBLIC4 master4searchKey(HWND hwnd,char *key);
int      PUBLIC4 master4size(HWND hwnd,int size);
int      PUBLIC4 master4undo(HWND hwnd);
int      PUBLIC4 master4unbind(HWND hwnd,HWND control);
int      PUBLIC4 master4unbindAll(HWND hwnd);
#ifdef __cplusplus
}
#endif

#ifdef S4CBPP
   #ifndef __DLL__
   int      combo4editData(HWND hwnd,Data4 editdata, Field4 field);
   int      combo4editData(HWND hwnd);
   int      combo4listData(HWND hwnd,Data4 listdata, Expr4 expr);
   int      combo4listData(HWND hwnd);
   int      ctrl4tag(HWND hwnd,Tag4 tag);
   int      edit4data(HWND hwnd,Data4 data,Field4 field);
   int      edit4data(HWND hwnd);
   int      listbox4data(HWND hwnd,Data4 data,Expr4 expr);
   int      listbox4data(HWND hwnd);
   int      master4bindEdit(HWND hwnd,HWND control,Field4 field);
   int      master4bindList(HWND hwnd,HWND control,Expr4 expr,Tag4 tag);
   int      master4bindList(HWND hwnd,HWND control,Expr4 expr);
   int      master4data(HWND hwnd, Data4 data, Expr4 expr);
   int      master4data(HWND hwnd);

   inline int     combo4editData(HWND hwnd,Data4 editdata, Field4 field) { return combo4editData( hwnd,editdata.data, field.field );}
   inline int     combo4editData(HWND hwnd) { return combo4editData( hwnd, NULL, NULL );}
   inline int     combo4listData(HWND hwnd,Data4 listdata, Expr4 expr) { return combo4listData( hwnd, listdata.data, expr.expr );}
   inline int     combo4listData(HWND hwnd) { return combo4listData( hwnd, NULL, NULL);}
   inline int     ctrl4tag(HWND hwnd,Tag4 tag) {return ctrl4tag( hwnd, tag.tag );}
   inline int     edit4data(HWND hwnd,Data4 data,Field4 field) { return edit4data( hwnd, data.data, field.field );}
   inline int     edit4data(HWND hwnd) { return edit4data( hwnd, NULL, NULL );}
   inline int     listbox4data(HWND hwnd,Data4 data,Expr4 expr) {return listbox4data( hwnd, data.data, expr.expr );}
   inline int     listbox4data(HWND hwnd) {return listbox4data( hwnd, NULL, NULL );}
   inline int     master4bindEdit(HWND hwnd,HWND control,Field4 field) { return master4bindEdit( hwnd, control, field.field);}
   inline int     master4bindList(HWND hwnd,HWND control,Expr4 expr,Tag4 tag) {return master4bindList( hwnd, control, expr.expr, tag.tag);}
   inline int     master4bindList(HWND hwnd,HWND control,Expr4 expr) {return master4bindList( hwnd, control, expr.expr, NULL);}
   inline int     master4data(HWND hwnd, Data4 data, Expr4 expr) { return master4data( hwnd, data.data, expr.expr) ;}
   inline int     master4data(HWND hwnd) { return master4data( hwnd, NULL, NULL) ;}

      #ifndef S4VBX
         void     ctrl4init(Code4 &code,HINSTANCE hInstance);
         int      ctrl4initUndo(Code4 &code,HINSTANCE hInstance);

         inline void     ctrl4init(Code4 &code,HINSTANCE hInstance)     { ctrl4initCPP( (CODE4 *) &code, hInstance );}
         inline int      ctrl4initUndo(Code4 &code,HINSTANCE hInstance) { return ctrl4initUndo( (CODE4 *) &code, hInstance );}
      #endif
   #endif 
   
#endif









