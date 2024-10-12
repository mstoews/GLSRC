_CLASSDEF(Tglcrept)
class Tglcrept : public Data4
{
  public:
  Field4 descript;
  Field4 cname;
  Field4 fname;
  Tglcrept(){};
  inline void init()
  {
  descript.init(data,1);
  cname.init(data,2);
  fname.init(data,3);
  }

  Tglcrept(Code4 *cb,LPSTR name=0):Data4(cb,"gl_rept")
  {
	 if(!isValid())
		return;
	 init();
 }

  LPSTR GetDbfName(){ return "GL_REPT";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if( isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
  }

  ~Tglcrept()
  {
  }
};


