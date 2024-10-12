#ifndef __GL_FA_H
#define __GL_FA_H

#define ID_FULLYEAR   1036
_CLASSDEF(TGLFixedAssets)
class TGLFixedAssets : public TMainDialog
{
 public:
 
 Tglfa    *glfa;
 Tfatype  *fatype;
 TAssetList      *ASSET;
 TAssetTypesList *TYPES;

 BOOL bTypes;
 BOOL bAsset;
 BOOL bTypeList;
 TGLFixedAssets(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog (AParent,AName){};
 TGLFixedAssets(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL): TMainDialog (AParent,ResourceId)
 {
 
 };

 virtual void SetupWindow();
 virtual void Search   (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
 virtual void Types    (RTMessage Msg) = [ID_FIRST + ID_FA];
 virtual void SetData  (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void Ok       (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel   (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Update   (RTMessage Msg) = [CM_FIRST + ID_FAUPDATE];
 virtual void Next     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Top      (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom   (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void AssetList (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST];
 virtual void New        (RTMessage Msg) = [ID_FIRST + ID_NEW];
 virtual void UpdateType (RTMessage Msg) = [CM_FIRST + CM_UPDATETYPES];
 virtual void Current    (RTMessage Msg) = [CM_FIRST + CM_UPDATE]{UpdateWindow(Msg);}
 virtual void DepMethod  (RTMessage Msg) = [ID_FIRST + ID_DEPRECIATION];
 virtual void FullYear   (RTMessage Msg) = [ID_FIRST + ID_FULLYEAR];
 virtual void UpdateWindow (RTMessage Msg) = [ID_FIRST + ID_UPDATE];
 virtual void CurrentFA  (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH]{SetData(Msg);}
 char *GetDepMethod();
 BOOL TestDatabase();
 void UpdateData();
 /*
 BOOL SetField(int ID,Field4 *field);
 BOOL SetField(int ID,double field);
 BOOL SetField(int ID,char *field);
 */
 BOOL dCheckField(int ID);
 BOOL szCheckField(int ID);
 BOOL CheckInput();
 double dAccum(void);

};

_CLASSDEF (TGLAssetDelete)
class TGLAssetDelete: public TGLFixedAssets
{
 public:
 TGLAssetDelete(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TGLFixedAssets(AParent,AName){};
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

#endif