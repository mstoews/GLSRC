/*
_CLASSDEF(Tarjsum)
class Tarjsum : public Data4
{
 public:
  Field4 account;
  Field4 dept;
  Field4 debit;
  Field4 credit;
  Field4 mode;
  void init()
  {
	account   .init(data, 1);
	dept      .init(data, 2);
	debit     .init(data, 11);
	credit    .init(data, 12);
	mode      .init(data, 4);
  }

  Tarjsum(){};
  Tarjsum(Code4 *cb,LPSTR name=0):Data4(cb,"ar_jsum")
  {
	 if(isValid())
		 init();
  };

  ~Tarjsum(){close();};
	double Debit()   { return double(Str4ptr(debit.str()));};
	double Credit()  { return double(Str4ptr(credit.str ()));};

 };


*/

