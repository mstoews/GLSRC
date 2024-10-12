/* ctrl4bor.h  (c)Copyright Sequiter Software Inc., 1990-1994.  All rights reserved. */

#define MVGA	361
#define MSVGA	362
#define MTOP	353
#define MPREV	354
#define MSEARCH	355
#define MNEXT	356
#define MBOTTOM	357
#define MAPPEND	358
#define MDELETED	359
#define MUNDO	360
#define CSIMPLE	253
#define CDROPDOWN	254
#define CDROPDOWNLIST	255
#define	CTRL4CCVERSION	0x0100

#define LNOTIFY	153
#define LSORT	154
#define LMULTI	155
#define LNOREDRAW	156
#define LTAB	157
#define LMULTICOL	158
#define LKEY	159
#define LEXSEL	160
#define LSCROLL	161
#define EMAX	113
#define NPOS	201
#define NNEG	205
#define NPOSLEADER	206
#define NPOSTRAILER	209
#define NNEGLEADER	207
#define NNEGTRAILER	208
#define NDEC	210
#define NSEP	202
#define NPOSONLY	204
#define NNEGCOLOR	45
#define BINSET	104
#define BRAISED	105
#define BINBORDER	106
#define BOUTBORDER	107
#define BTEXTCOLOR	46
#define BBACKCOLOR	47
#define BHIGHCOLOR	43
#define BSHADOWCOLOR	44
#define BBORDER		120
#define BNOBORDER		121
#define BTHICKNESS	102
#define JLEFT	120
#define JCENTER	121
#define JRIGHT	122
#define ETEXT 103
#define EPICTURE 104
#define EID 130
#define EMULTILINE	105
#define ESINGLELINE	106
#define ECASEINSENSITIVE	107
#define EUPPERCASE	108
#define ELOWERCASE	109
#define EAUTOHSCROLL	110
#define EAUTOVSCROLL	111
#define ENORMAL	137
#define EDATE	138
#define EPIC	139
#define ENUMERIC	140
#define EPASSWORD	116
#define ECONVERTOEM	117
#define EKEEPSELECTION	119
#define EHSCROLLBARS	114
#define EVSCROLLBARS	115
#define ETABSTOP	144
#define EGROUP	145
#define EVISIBLE	146
#define EDISABLE	147
#define EREADONLY	148
#define EWANTRETURNS	149
#define EFONT	48
#define EBEVEL	50
#define ENUM	49

typedef struct	CTRL4FLAG
{
	long	Flag;
	char	*Symbol;
} CTRL4FLAG;

typedef struct	CTRL4INITDIALOG
{
	BOOL			Extended;
	HGLOBAL  	StyleData;
	LPFNSTRTOID StrToId;
	LPFNIDTOSTR	IdToStr;
} CTRL4INITDIALOG;


