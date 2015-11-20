#ifndef		__VAR_SIZE_DEF__

#define		__VAR_SIZE_DEF__

typedef		unsigned long		uint32 ;
typedef		unsigned short		uint16 ;
typedef		unsigned char		uchar8 ;

typedef		long				sint32 ;
typedef		short int			sint16 ;
typedef		char				schar8 ;

typedef		unsigned short *	uint16p ;

/* 汎用フラグ型 */
union FLAG_TYPE {
	uchar8 BYTE ;			/*	BYTE Access */
	struct {				/*	BIT	 Access */
		uchar8 B7:1;
		uchar8 B6:1;
		uchar8 B5:1;
		uchar8 B4:1;
		uchar8 B3:1;
		uchar8 B2:1;
		uchar8 B1:1;
		uchar8 B0:1;
	} BIT ;
};

#define		ACCMODE		0	/* 0: 通常モード	1: 車速ｻｰﾎﾞ系のみ	*/
#ifdef ON
#    undef ON
#endif

#ifdef OFF
#    undef OFF
#endif
#define OFF   0
#define ON    1

#endif
