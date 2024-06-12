#ifndef INCLUDE_H
#define INCLUDE_H


/*
**	Enumeration of Gadgets/Objects needed
*/
enum
{
	GID_MAIN=0,
	GID_VIDEO,
	GID_YUV,
	GID_FILTER,
	GID_FSKIP,
	GID_FSKIPSET,
	GID_THROTTLE,
	GID_THROTTLESET,
	GID_SPEED,
	GID_IFBLEND,
	GID_PWINAC,
	
	GID_GDBPROTO,
	GID_GDBPORT,
	GID_DEBUG,
	GID_VERBOSE,
	GID_AGBPRINT,	

	GID_EXE,
	GID_GAME,
	GID_BIOS,
	GID_FLASH,
	GID_SAVE,
	GID_MMX,
	GID_IPS,
	GID_IPSFILE,
	GID_RTC,

	GID_SAVECONF,
	GID_PLAY,
	GID_QUIT,
	
	GID_LAST
};

enum
{
	WID_MAIN=0,
	WID_LAST
};
enum
{
	OID_MAIN=0,
	OID_CLICKTAB,
	OID_ABOUT,
	OID_PAGE1,
	OID_PAGE2,
	OID_PAGE3,
	OID_PAGE4,
	OID_REQ,
	OID_LAST
};


#endif