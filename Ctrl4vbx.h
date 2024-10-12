/* ctrl4vbx.h  (c)Copyright Sequiter Software Inc., 1990-1994.  All rights reserved. */

#define CTRL4POPUP         WM_USER + 100

#ifndef S4VBX
   #define S4VBX 
#endif

#ifdef __cplusplus
   extern "C"
   {
#endif
      void        FAR PASCAL _export ctrl4initVBX(CODE4 *code,HINSTANCE hInstance,int initialize);
      void        FAR PASCAL _export ctrl4initVBXUndo(CODE4 *code,HINSTANCE hInstance);
#ifdef __cplusplus
   }
#endif

CODE4 *     FAR PASCAL _export ctrl4initVB( void );
int         FAR PASCAL _export ctrl4initUndoVB(CODE4 *code );
#ifdef S4CBPP
   void ctrl4init( Code4 &code, HINSTANCE hInstance ) { ctrl4initVBX( &code, hInstance, 0);}
   void ctrl4initUndo( Code4 &code, HINSTANCE hInstance ) { ctrl4initVBXUndo( &code, hInstance);}
#else
   #define ctrl4init(c,h)  (ctrl4initVBX((c),(h),1))
   #define ctrl4initUndo(c,h)  (ctrl4initVBXUndo((c),(h)))
#endif







