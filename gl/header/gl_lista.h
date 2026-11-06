#ifndef _GL_LIST_H
#define _GL_LIST_H
#include "array.h"
_CLASSDEF(TListArray)
class TListArray : public Array
{
public:
  TListArray(int upper, int lower = 0, sizeType aDelta = 0)
    : Array(upper, lower, aDelta){};
  virtual classType isA() const { return __firstUserClass + 1; }
  virtual Pchar nameOf() const { return "TListArray"; }
};


_CLASSDEF(TListItem)
class TListItem : public Object
{
 public:
  int index,rec;
  TListItem(int Aindex,int Arec){index = Aindex, rec = Arec;}
  virtual classType isA() const { return __firstUserClass; }
  virtual Pchar nameOf() const { return "TListItem"; }
  virtual hashValueType hashValue() const { return 0; }
  virtual int isEqual(RCObject AListItem) const
    { return index == ((RTListItem)AListItem).index && rec == ((RTListItem)AListItem).rec; }
  virtual void printOn(Rostream outputStream) const
    { outputStream << "(" << index << "," << rec << ")"; }

};
#endif

