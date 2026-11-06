_CLASSDEF(TGLSourceCodes)
class TGLSourceCodes : public TGLAcctsDelete
{
 public:
 PTglsource glsource;
 PTglaccts  glaccts;
 TGLSourceCodes(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TGLAcctsDelete(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void HandleCode     (RTMessage Msg) = [ID_FIRST + ID_SOURCECODE];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Add            (RTMessage Msg) = [ID_FIRST + ID_ADD];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify	     (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
};
