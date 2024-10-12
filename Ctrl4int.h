/* ctrl4int.h  (c)Copyright Sequiter Software Inc., 1990-1994.  All rights reserved. */
       
#ifndef min   
   #define  min(a,b)  (((a) < (b)) ? (a) : (b))
   #define  max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

             
#ifndef  WM_SYSTIMER
   #define  WM_SYSTIMER       280
#endif

#include "ctrl4.h"

/***************************************************************\
\***************************************************************/

typedef  struct
{
      DATA4       *data;
      EXPR4       *validation;
      FIELD4      *field;
      COLORREF    backcolor;
      COLORREF    highlightcolor;
      COLORREF    negcolor;
      COLORREF    shadowcolor;
      COLORREF    textcolor;
      HFONT       font;
      long        alignment;
      int         border;
      int         direction;
      int         enabled;
      int         hscroll;
      int         lowercase;
      int         maxlength;
      int         nohidesel;
      int         outsideborder;
      int         password;
      int         readonly;
      int         stripclipformatting;
      int         stripdataformatting;
      int         thickness;
      int         type;
      int         uppercase;
      int         visible;
      int         vscroll;
      char        *text;
      int         multiline;
      char        *mask;
      char        *leaderneg;
      char        *leaderpos;
      char        *trailerneg;
      char        *trailerpos;
      int         dec;
      int         posonly;
      int         separator;
      char        decchar;
      char        sepchar;
      HINSTANCE   hInstance;
      HWND        edithwnd;
      HWND        hwnd;
      HWND        oldhwnd;
      HWND        master;
      HWND        parent;
      HWND        combo;
      HWND        thunder;
      RECT        bounds;
      HBRUSH      backbrush;
      int         top,left,height,width;
      int         len;
		int         wantreturns;
		int			wanttabs;
      int         insert;
      int         contentlen;
      int         fullscreen;
      int         setmultiline;
      int         cntrl,shift,sysfont;
      int         picloaded;
      int         leadersize;
      int         trailersize;
      int         neg;
      int         autohscroll;
      int         autovscroll;
      int         maxdigits;
      int         daypos,daynum,monthpos,monthnum,yearpos,yearnum,centurypos,centurynum;
      int         validate;
      int         autoclose;
      long        recno;
      long        otherstyles;
      int         deallocfont;
      int         suppress;
      int         memosize;
        int         newfont;

} EDIT4;

#define  CTRL4LEADERSIZE   4
#define  CTRL4TEXTSIZE     45

typedef struct
{
      COLORREF    bkcolor;
      COLORREF    hlcolor;
      COLORREF    txcolor;
      COLORREF    ngcolor;
      COLORREF    shcolor;
      char        posleader[CTRL4LEADERSIZE];
      char        negleader[CTRL4LEADERSIZE];
      char        postrailer[CTRL4LEADERSIZE];
      char        negtrailer[CTRL4LEADERSIZE];
      int         maxlength;
      int         thickness;
      int         border;
      int         direction;
      int         sep;
      int         dec;
      int         posonly;
      int         type;
      char        mask[CTRL4TEXTSIZE];
      char        text[CTRL4TEXTSIZE];
      int         outsideborder;

} CTRL4INFO;

/***************************************************************\
\***************************************************************/

typedef  struct
{
      HWND        hwnd;
      HWND        listhwnd;
      HWND        parent;
      HWND        scrollhwnd;
      HWND        combo;
      HWND        thunder;
      HWND        master;
      HINSTANCE   hInstance;
      RECT        bounds;
      TEXTMETRIC  textmetrics;
      COLORREF    backcolor;
      COLORREF    highlightcolor;
      COLORREF    shadowcolor;
      COLORREF    textcolor;
      HFONT       font;
      HBRUSH      backbrush;
      long        otherstyles;
      int         visible;
      int         border;
      int         outsideborder;
      int         direction;
      int         enabled;
      int         control_id;
      int         thickness;
      int         columns;
      int         scrollwidth;
      int         scrollheight;
      int         vscrollwidth;
      int         hscrollheight;
      int         stripdataformatting;
      int         top,left,height,width,orgheight;
      int         multiselect;
      int         button;
      int         sorted;
      int         displaymax;
      long        total;
      int         selected;
      int         scroll;
      long        position;
      long        offset;
      long        offsetbottom;
      int         autoclose;
      char        search[256];
      int         singlesearch;
      int         searchlen;
      char        *mask;
      char        *leaderneg;
      char        *leaderpos;
      char        *trailerneg;
      char        *trailerpos;
      int         dec;
      int         separator;
      char        decchar;
      char        sepchar;
      int         leadersize; 
      int         trailersize;
      int         type;
      int         strip,suppress,masterupdate;
      int         deallocfont;
      CODE4       *code;
      DATA4       *data;
      EXPR4       *expr;
      TAG4        *tag;

} LISTBOX4;

/***************************************************************\
\***************************************************************/

typedef struct
{
      HWND        hwnd;
      HWND        button;
      HWND        edithwnd;
      HWND        listhwnd;
      HWND        thunder;
      EDIT4       *edit;
      LISTBOX4    *list;
      HWND        parent;
      HINSTANCE   hInstance;
      RECT        bounds;
      COLORREF    backcolor;
      COLORREF    highlightcolor;
      COLORREF    shadowcolor;
      COLORREF    textcolor;
      HBRUSH      backbrush;
      HFONT       font;
      int         oldheight;
      int         border;
      int         outsideborder;
      int         direction;
      int         enabled;
      long        alignment;
      int         control_id;
      int         thickness;
      int         top,left,height,width;
      int         visible;
      int         style;
      int         buttonheight;
      int         sort;
      long        recno;
      long        flags,otherstyles;
      int         type;
      int         close;
      int         hscroll;
      int         semophore;
      int         nohide,hide;
      int         autoclose;
      int         deallocfont;
      int         suppress;
      int         listscrollkey;
      char        *search;
      CODE4       *code;
      DATA4       *editdata,*listdata;
      FIELD4      *field;
      EXPR4       *expr;
      CTRL4INFO   info;

} COMBO4;

typedef  struct
{
      int         enabled;
      int         but,drawfocus;
      int         bitmap;
      HWND        hwnd;
      
} BUTTON4;

/***************************************************************\
\***************************************************************/

typedef  struct
{
      LINK4       link;
      HWND        control;
      int         type,id;
      
} MASTER4LIST;


typedef  struct
{
      HWND        hwnd,tophwnd,bottomhwnd,nexthwnd,prevhwnd,searchhwnd,addhwnd,delhwnd,undohwnd;
      HWND        thunder,searchCombo;
      COLORREF    highlightcolor;
      COLORREF    shadowcolor;
      COLORREF    textcolor;
      COLORREF    backcolor;
      HWND        parent;
      HBRUSH      bkbrush;
      HINSTANCE   hInstance;
      RECT        bounds;
      long        otherstyles;
      long        recno;
      int         border;
      int         reply;
      int         outsideborder;
      int         direction;
      int         enabled;
      int         thickness;
      int         top,left,height,width;
      int         visible,buttonwidth,buttonheight;
      int         readLock,lockAttempts,autoclose;
      int         size;
      int         nocancel,lockcode;
      int         tp,prev,next,bot,search,add,del,undo;
      int         tp2,prev2,next2,bot2,search2,add2,del2,undo2;
      int         suppress,suppressEditChange;
      long        oldrecno;
      int         deleted;
      HWND        *fields;
      DATA4       *data;
      EXPR4       *expr,*validation;
      LIST4       list;
      TAG4        *tag;

} MASTER4;


/***********************************\
* Private Functions                 *
\***********************************/

/* combo4pub.c */

int      PUBLIC4 combo4activateInfo(COMBO4 *combo,CTRL4INFO *info);
COMBO4   *combo4checkHwnd(HWND hwnd);
int      combo4master(HWND hwnd,HWND master,DATA4 *data, FIELD4 *field);
int      combo4masterList(HWND hwnd,HWND master,DATA4 *data, EXPR4 *expr,TAG4 *tag);
int      combo4refresh(HWND hwnd);
int      combo4setBevel(HWND hwnd,COLORREF bkcolor,COLORREF shcolor,COLORREF hlcolor,COLORREF txcolor,int thickness,int direction,int border);
int      combo4setBevelThickness(HWND hwnd,int thickness);
int      combo4setBorder(HWND hwnd,int border);
int      combo4setDirection(HWND hwnd,int direction);
int      combo4setFont(HWND hwnd,HFONT font);
int      combo4setHighlightColor(HWND hwnd,COLORREF color);
int      combo4setShadowColor(HWND hwnd,COLORREF color);

/* combo4int.c */

int      combo4activateInfoInternal(COMBO4 *combo,CTRL4INFO *info,char *str);
void     combo4calcBounds(COMBO4 *combo);
void     combo4checkStyle(COMBO4 *combo,long flags);
int      combo4create(HWND hwnd,CREATESTRUCT *cs);
void     combo4editChange(COMBO4 *combo);
void     combo4editSetFocus(COMBO4 *combo);
void     combo4free(COMBO4 *combo);
void     combo4hideList(COMBO4 *combo);
void     combo4infoSetInternal(COMBO4 *combo,CTRL4INFO *info);
void     combo4infoSetMask(COMBO4 *combo,CTRL4INFO *info);
void     combo4init(COMBO4 *combo,HWND parent,HINSTANCE hinst);
void     combo4moveList(COMBO4 *combo);
void     combo4notify(COMBO4 *combo,int msg);
void     combo4paint(COMBO4 *combo);
void     combo4resize(COMBO4 *combo,int width,int height);
void     combo4selectionChange(COMBO4 *combo);
void     combo4showList(COMBO4 *combo);
void     combo4stripBlanks(char *str);

/* ctrl4int.c */

int      button4setbitmap(HWND hwnd,int bitmap);
int      button4setdrawfocus(HWND hwnd,int drawfocus);
int      button4setenabled(HWND hwnd,int enablbed);

COLORREF    ctrl4colorDiddle(COLORREF oldcolor);
DATA4    *ctrl4dataOpen(CODE4 *code,char *str);
void     ctrl4drawBevel(int thickness, int direction,int border,COLORREF backcolor,COLORREF shadowcolor,COLORREF highlightcolor,HDC hdc,RECT *rect,RECT *invalid);
void     ctrl4drawBitmap(HDC hdc,HBITMAP hBitmap,int xStart,int yStart,DWORD rop,int drawfocus);
int      ctrl4getType(HWND hwnd);
int      ctrl4addDateFormat(char *to,char *from,int size,char *mask);
int      ctrl4addNumericFormat(char *text,char *from,int size,int dec,int neg,int separator,char decchar,char sepchar,char *leaderpos,char *leaderneg,char *trailerpos,char *trailerneg);
int      ctrl4addMaskFormat(char *to,char *from,int size,char *mask);
HINSTANCE FAR PASCAL _export ctrl4hInst(void);
int      PUBLIC4 ctrl4infoToStr(CTRL4INFO *info,char *str);
void     PUBLIC4 ctrl4registerAppClass(HINSTANCE hInstance);
void     PUBLIC4 ctrl4registerDLLClass(HINSTANCE hInstance,int global);
int      ctrl4readOnly(DATA4 *data);
void 		ctrl4setMessageQueue(void );
int      ctrl4stripDateFormat(char *to,char *from,int size ,char *mask);
int      ctrl4stripNumericFormat(char *to,char *from,int size,int dec,int pos,char decchar);
int      ctrl4stripMaskFormat(char *to,char *from,int size,char *mask);
int      PUBLIC4 ctrl4strToInfo(CTRL4INFO *info,char *str);
void     ctrl4assign(HWND master,HWND edithwnd,FIELD4 *field,char *ptr);
void     ctrl4assign_double(HWND master,HWND edithwnd,FIELD4 *field,double value);
void     ctrl4assign_n(HWND master,HWND edithwnd,FIELD4 *field,char *ptr,unsigned ptr_len);
int      master4recallCurrentRecord(MASTER4 *master);
void     ctrl4setRecFlag(MASTER4 *master,FIELD4 *field,HWND hwnd);
HWND     ctrl4getRecFlag(MASTER4 *master,FIELD4 *field);


/* edit4pub.c */

EDIT4   *edit4checkHwnd(HWND hwnd);
double   edit4getDouble(HWND hwnd);
HFONT    edit4getFont(HWND hwnd);
int      edit4getInt(HWND hwnd);
long     edit4getLong(HWND hwnd);
int      edit4getMaxLength(HWND hwnd);
int      edit4getText(HWND hwnd,char *text,int len,int strip);
int      edit4getTextLen(HWND hwnd);
int      edit4master(HWND hwnd,HWND master,DATA4 *data, FIELD4 *field);
int      edit4refresh(HWND hwnd);
int      edit4setAlignment(HWND hwnd,int alignment);
int      edit4setBackColor(HWND hwnd,COLORREF color);
int      edit4setBevel(HWND hwnd,COLORREF bkcolor,COLORREF shcolor,COLORREF hlcolor,COLORREF txcolor,int thickness,int direction,int border);
int      edit4setBevelThickness(HWND hwnd,int thickness);
int      edit4setBorder(HWND hwnd,int border);
int      edit4setDecimals(HWND hwnd,int dec);
int      edit4setDirection(HWND hwnd,int direction);
int      edit4setDouble(HWND hwnd,double value);
int      edit4setEnabled(HWND hwnd,int enable);
int      edit4setFont(HWND hwnd,HFONT font);
int      edit4setFormatStripping(HWND hwnd,int enable);
int      edit4setHighlightColor(HWND hwnd,COLORREF color);
int      edit4setInt(HWND hwnd,int value);
int      edit4setLong(HWND hwnd,long value);
int      edit4setMaxLength(HWND hwnd,int size);
int      edit4setNegativeColor(HWND hwnd,COLORREF color);
int      edit4setMask(HWND hwnd,char *pic);
int      edit4setPositiveOnly(HWND hwnd,int pos);
int      edit4setRecno(HWND hwnd,long record);
int      edit4setSeparator(HWND hwnd,int sep);
int      edit4setShadowColor(HWND hwnd,COLORREF color);
int      edit4setText(HWND hwnd,char *text,int stripped);
int      edit4setVscroll(HWND hwnd,int vscroll);
int      edit4setHscroll(HWND hwnd,int hscroll);
int      edit4setTextColor(HWND hwnd,COLORREF color);
int      edit4setType(HWND hwnd,int type);
int      edit4setValidation(HWND hwnd,char *validation);

/* edit4int.c */
long     edit4emLimitText(EDIT4 *edit,int len );
long     edit4emReplaceSel(EDIT4 *edit,WPARAM wParam,LPARAM lParam);
long     edit4emSetReadOnly(EDIT4 *edit,BOOL readonly );
long     edit4wmClear(EDIT4 *edit);
long     edit4wmCreate(HWND hwnd,CREATESTRUCT FAR *cs);
long     edit4wmCtlColor(EDIT4 *edit,WPARAM wParam,LPARAM lParam);
long     edit4wmGetText(EDIT4 *edit,WPARAM wParam,LPARAM lParam,int strip);
long     edit4wmCommand(EDIT4 *edit, UINT message,WPARAM wParam,LPARAM lParam);


int      PUBLIC4 edit4activateInfo(EDIT4 *edit,CTRL4INFO *info);
int      edit4activateInfoInternal(EDIT4 *edit,CTRL4INFO *info,char *str);
void     edit4calcBounds(EDIT4 *edit);
void     edit4checkMultiline(EDIT4 *edit);
void     edit4changed(EDIT4 *edit);
int      edit4create(HWND hwnd,CREATESTRUCT *cs);
void     edit4cut(EDIT4 *edit);
int      edit4deleteCharacter(EDIT4 *edit,UINT key);
void     edit4free(EDIT4 *edit);
int      edit4go(EDIT4 *edit,DATA4 *data,long recno);
int      edit4handledCharacter(EDIT4 *edit,UINT key);
void     edit4infoSetInternal(EDIT4 *edit,CTRL4INFO *info);
void     edit4infoSetMask(EDIT4 *edit,CTRL4INFO *info);
void     edit4init(EDIT4 *edit,HWND parent,HINSTANCE hinst);
void     edit4insertCharacter(EDIT4 *edit,UINT key);
int      edit4killFocus(EDIT4 *edit,HWND newhwnd);
int      edit4locateNext(EDIT4 *edit, UINT *key,int pos);
void     edit4paint(EDIT4 *edit);
void     edit4paste(EDIT4 *edit);
void     edit4resize(EDIT4 *edit,int width,int height);
long     edit4setStyle(EDIT4 *edit);
int      edit4setWinText(EDIT4 *edit,char *str);
int      edit4valid(HWND parent,HWND hwnd,EDIT4 *edit);

/* edit4pic.c */

int      pic4addFormatting(EDIT4 *edit,char *to,char *from);
char     pic4alpha(UINT key);
char     pic4alphaNumeric(UINT key);
int      pic4autoDate(EDIT4 *edit,int pos,char *key);
UINT     pic4convertCharacter(EDIT4 *edit,int pos,UINT key);
UINT     pic4convertCharacterMask(int type,char pic,UINT key);
void     pic4deleteSelection(EDIT4 *edit,int start, int end);
int      pic4findNextFormat(EDIT4 *edit,int *pos);
int      pic4insertString(EDIT4 *edit,char *to,char *from);
int      pic4isFormat(EDIT4 *edit,int pos);
int      pic4isFormatMask(int type,char pic);
int      pic4loadMonth(EDIT4 *edit,int pos,char *key);
char     pic4lower(UINT key);
char     pic4numeric(UINT key);
void     pic4parseDate(EDIT4 *edit);
void     pic4shiftLeft(EDIT4 *edit,int pos,char *text);
void     pic4shiftRight(EDIT4 *edit,int pos,char *text);
int      pic4stripFormatting(EDIT4 *edit,char *from,char *to,int start,int end);
char     pic4upper(UINT key);

/* edit4num.c */

void     num4addFormat(EDIT4 *edit, char *from);
int      num4countDigits(EDIT4 *edit,int pos);
int      num4insert(EDIT4 *edit,char key,int pos);
int      num4locateDecimal(EDIT4 *edit);
int      num4locatePosition(EDIT4 *edit,int count);
void     num4deleteSelection(EDIT4 *edit,int start,int end);
void     num4setDecimal(EDIT4 *edit,int dec);
void     num4setDecimalCharacter(EDIT4 *edit,char dec);
void     num4setNegative(EDIT4 *edit,int neg);
void     num4setMask(EDIT4 *edit);
void     num4setSeparatorCharacter(EDIT4 *edit,char sep);
void     num4setLeader(EDIT4 *edit,char *pos,char *neg);
void     num4setLeaderInternal(char **leaderpos,char **leaderneg,char *pos,char *neg,int *leadersize);
void     num4stripFormat(EDIT4 *edit,char *to, char *from);
int      num4stripLeadingZeros(EDIT4 *edit,char *str);
void     num4setTrailer(EDIT4 *edit,char *pos,char *neg);
void     num4setTrailerInternal(char **trailerpos,char **trailerneg,char *pos,char *neg,int *trailersize);

/* list4pub.c */

int      listbox4addFormat(LISTBOX4 *list,char *str,char **result);
long     listbox4addString(LISTBOX4 *list,char *str);
int      listbox4getText(HWND hwnd,long index,char *buffer,int bufferlen,int strip);
int      listbox4getTextLength(HWND hwnd,long index,int strip);
long     listbox4insertString(LISTBOX4 *list,char *str,UINT index);
int      listbox4master(HWND hwnd,HWND master,DATA4 *data, EXPR4 *expr,TAG4 *tag);
int      listbox4refresh(HWND hwnd);
int      listbox4seekInternal(LISTBOX4 *list,char *key,int strip);
int      listbox4setBackColor(HWND hwnd,COLORREF color);
int      listbox4setBevel(HWND hwnd,COLORREF bkcolor,COLORREF shcolor,COLORREF hlcolor,COLORREF txcolor,int thickness,int direction,int border);
int      listbox4setBevelThickness(HWND hwnd,int thickness);
int      listbox4setBorder(HWND hwnd,int border);
int      listbox4setDecimals(HWND hwnd,int dec);
int      listbox4setDecimalCharacter(HWND hwnd,char decchar);
int      listbox4setDirection(HWND hwnd,int direction);
int      listbox4enabled(HWND hwnd,int enable);
int      listbox4setFont(HWND hwnd,HFONT font);
int      listbox4setFormatStripping(HWND hwnd,int strip);
int      listbox4setHighlightColor(HWND hwnd,COLORREF color);
void     listbox4setLeader(LISTBOX4 *list,char *pos,char *neg);
int      listbox4setMask(HWND hwnd,char *mask);
int      listbox4setSeparator(HWND hwnd,int sep);
int      listbox4setSeparatorCharacter(HWND hwnd,char sepchar);
int      listbox4setShadowColor(HWND hwnd,COLORREF color);
int      listbox4setSorted(HWND hwnd,int sort);
int      listbox4setTag(HWND hwnd,TAG4 *tag);
int      listbox4setTagOpen(HWND hwnd,char *tag);
int      listbox4setTextColor(HWND hwnd,COLORREF color);
void     listbox4setTrailer(LISTBOX4 *list,char *pos,char *neg);
int      listbox4setType(HWND hwnd,int type);
char     *listbox4stripFormat(LISTBOX4 *list, char *str);

/* list4int.c */
int      listbox4go(LISTBOX4 *list,DATA4 *data,long recno);
void     listbox4tagSelect(LISTBOX4 *list);
int      PUBLIC4 listbox4activateInfo(LISTBOX4 *list,CTRL4INFO *info);
int      listbox4activateInfoInternal(LISTBOX4 *list,CTRL4INFO *info,char *str);
void     listbox4addItem(LISTBOX4 *list,char *value);
void     listbox4boxMouseMove(LISTBOX4 *list,int keys,int y);
void     listbox4boxLineDown(LISTBOX4 *list,int key);
void     listbox4boxLineUp(LISTBOX4 *list,int key);
void     listbox4boxPageDown(LISTBOX4 *list);
void     listbox4boxPageUp(LISTBOX4 *list);
void     listbox4calcBounds(LISTBOX4 *list);
int      listbox4calcHeight(LISTBOX4 *list);
LISTBOX4 *listbox4checkhwnd(HWND hwnd);
void     listbox4checkScroll(LISTBOX4 *list);
int      listbox4create(HWND hwnd,CREATESTRUCT *cs);
void     listbox4free(LISTBOX4 *list);
void     listbox4freeStr(char *str);
char     *listbox4getStr(LISTBOX4 *list);
int      listbox4handledChar(LISTBOX4 *list,int key);
void     listbox4infoSetInternal(LISTBOX4 *list,CTRL4INFO *info);
void     listbox4infoSetMask(LISTBOX4 *list,CTRL4INFO *info);
void     listbox4init(LISTBOX4 *list,HWND parent,HINSTANCE hinst);
void     listbox4initList(LISTBOX4 *list);
void     listbox4lineUp(LISTBOX4 *list);
void     listbox4lineDown(LISTBOX4 *list);
void     listbox4loadDataList(LISTBOX4 *list);
long     listbox4noData(LISTBOX4 *list,UINT message,UINT wParam,LONG lParam);
void     listbox4pageDown(LISTBOX4 *list);
void     listbox4pageUp(LISTBOX4 *list);
void     listbox4paint(LISTBOX4 *list);
void     listbox4resize(LISTBOX4 *list,int width,int height);
int      listbox4seek(LISTBOX4 *list);
void     listbox4setSelection(LISTBOX4 *list);
void     listbox4selChange(LISTBOX4 *list);
long     listbox4selectString(LISTBOX4 *list,UINT index,char *str,int strip);
void     listbox4thumbPos(LISTBOX4 *list,int position);
void     listbox4updatePos(LISTBOX4 *list);

/* mstr4int.c */
int      PUBLIC4 master4activateInfo(MASTER4 *master,CTRL4INFO *info);
int      master4activateInfoInternal(MASTER4 *master,CTRL4INFO *info,char *str);
void     master4calcBounds(MASTER4 *master,int width,int height);
MASTER4  *master4checkHwnd(HWND hwnd);
void     master4checkButtons(MASTER4 *master,int rc);
void     master4checkStyle(MASTER4 *master,long flags);
int      master4create(HWND hwnd,CREATESTRUCT *cs);
void     master4delButtons(MASTER4 *master);
int      master4enabled(HWND hwnd,int button,int enabled);
void     master4free(MASTER4 *master);
void     master4init(MASTER4 *master,HWND parent,HINSTANCE hinst);
int      master4lock(MASTER4 *master,int operation,long param);
int      master4doLock(MASTER4 *master,int rc);
void     master4notify(MASTER4 *master,int msg);
void     master4paint(MASTER4 *master);
//id     master4resize(MASTER4 *master);
void     master4resize(MASTER4 *master,int width,int height);
int      master4setActive(MASTER4 *master,int button,int enabled);
void     master4storeRecord(MASTER4 *master);
int      master4tag(HWND hwnd,TAG4 *tag);
int      master4tagOpen(HWND hwnd,char *tag);
void     master4update(MASTER4 *master);
void     master4setDel(MASTER4 *master,int del);
void     master4setDlgTag(MASTER4 *master,EXPR4 *expr,HWND searchhwnd,HWND searchhwnd2);
void     master4setFocus(MASTER4 *master,HWND current,int dir);
int          master4validate(MASTER4 *master);
int           master4writeField(EDIT4 *edit);

/*mstr4pub.c */

int         master4ctrlupdate(HWND hwnd);
int         master4setBevel(HWND hwnd,COLORREF bkcolor,COLORREF shcolor,COLORREF hlcolor,int thickness,int direction,int border);
int         master4setBevelThickness(HWND hwnd,int thickness);
int         master4setBorder(HWND hwnd,int border);
int         master4bottom(HWND hwnd);
int         master4go(HWND hwnd,long recno);
int         master4seek(HWND hwnd,char *key);
int         master4setDirection(HWND hwnd,int direction);
int         master4setEnabled(HWND hwnd,int enabled);
int         master4setHighlightColor(HWND hwnd,COLORREF color);
int         master4setShadowColor(HWND hwnd,COLORREF color);
int         master4setValidation(HWND hwnd,char *validation);
int         master4skip(HWND hwnd,long numrecs);
int         master4top(HWND hwnd);

