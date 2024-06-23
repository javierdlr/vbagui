#ifndef INCLUDE_H
#define INCLUDE_H


#define ROMS  "PROGDIR:ROMS"
#define CONFIG_INI "PROGDIR:vba/VisualBoyAdvance.cfg"
#define GPAD_IMG "PROGDIR:vba/keymap.png"
#define GPAD_BG  "PROGDIR:vba/keymap_bg.png"
#define GPAD_STR "PROGDIR:vba/keymap_str.png"
//#define DONATE_IMG "PROGDIR:vba/donation.png"

#define MAX_FULLFILEPATH  MAX_DOS_PATH+MAX_DOS_FILENAME // GetRoms()

// gui.c: 'make_chooser_list(<mode>,..)' & 'make_chooser_list2(<mode>,..)'
#define ADD_LIST 0
#define NEW_LIST 1

enum {
 COL_ROM = 0,
 COL_FMT, // file extension
 LAST_COL
};

/*
**	Enumeration of Gadgets/Objects needed
*/
enum
{
	GID_MAIN = 0,
//PAGE1 (about)
GID_DONATE_BTN,
//PAGE2 (visual)
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
//PAGE2 (game)
	GID_EXE,
	GID_GAME,
	GID_BIOS,
	GID_FLASH,
	GID_SAVE,
	GID_MMX,
GID_GAMEPAD_BTN,
	GID_IPS,
	GID_IPSFILE,
	GID_RTC,
GID_GAMELIST,
GID_LISTBROWSER,
GID_TOTALROMS,
//PAGE3 (debug)
	GID_GDBPROTO,
	GID_GDBPORT,
	GID_DEBUG,
	GID_VERBOSE,
	GID_AGBPRINT,	
// Gamepad gadgets
GID_GPAD_L,
GID_GPAD_R,
GID_GPAD_B,
GID_GPAD_A,
GID_GPAD_SEL,
GID_GPAD_STA,
GID_GPAD_TEXT,
GID_GPAD_IMG,
GID_GPAD_STR,
GID_GPAD_SAVE,
GID_GPAD_CANCEL,
// Buttons
	GID_SAVECONF,
	GID_PLAY,
	GID_QUIT,

GID_DEBUG_PAGE,

	GID_LAST
};

enum
{
	WID_MAIN = 0,
	WID_GPAD, // Gamepad window
	WID_LAST
};
enum
{
	OID_MAIN = 0,
	OID_CLICKTAB,
	OID_ABOUT, // about image
	OID_PAGE1,
	OID_PAGE2,
	OID_PAGE3,
	OID_PAGE4,
	OID_REQ,
OID_GPAD, // Gamepad object
//OID_DONATE_IMG, // Donate image

	OID_LAST
};

#define CFG_FILE_SIZE 4096 // max. size of config.ini in memory (array)

enum { // must MATCH kw_array[] from resources.c
L = 0,
R,
B,
A,
SEL,
STA,
TOT_CFG
};

#define GPAD_BTN_OFFSET  (GID_GPAD_L - L) // "automagically" calculates offset
#define GPAD_NO_BTN      0xFFFF


#endif