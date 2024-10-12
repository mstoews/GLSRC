#ifndef __GL_FIELD_H
#define __GL_FIELD_H
class S4CLASS Field : public  Field4
{
 public:
 Field (Data4 data,int iField):Field4(data,iField){field =  d4field_j( data.data, iField ) ;};
 HWND   hWnd;
 void   AssignField(HWND hWnd,UINT ID);
 void   SetField(HWND hWnd,UINT ID);
 char   *GetField(HWND hWnd,UINT ID);
 char   *GetString(HWND hWnd,UINT ID);
 double dGetDouble(HWND hWnd,UINT ID);
 long   lGetLong(HWND hWnd,UINT ID);
 char   *GetDate(HWND hWnd,UINT ID);
};

#endif