/*
**	VisualBoy Advance GUI module
**	Freeware software
**	� 2005, by Vicente 'Ami603' Gimeno
**	Bugs are also copyrighted, so take care ;)
**
**	V1.00 -.Initial Version, No prefs saving, no debug page
**
**	V1.01 -.First public version, Now it does use Application.library for storing preferences easily.No debug options yet,although the page is already implemented,
**	i've disabled it to allow a earlier release.Mind you, i have my own wedding in less than 2 months, and maybe i won't code again in a reasonable amount of time.
**
**	V1.02 -.Fixed a many moons old bug that caused problems when selecting files other than on the main executable drawer.
**
**	V1.03 -. Changed the minimum required Application.library version,that will allow the public releases to run  this app.
**	Changed a little and old joke that i forgot to remove.
**
**	V1.04 -. Locale Support Added,Error messages not localized, but you got the idea.
**	Debug page is active, but not usable yet.
**
**	V1.05 -. Final Version.Hopefully it  will be enough for those that are using it.Debug page active.
**	Also removed some output when using the default language.
**	Cleared GFX status on some gadgets with actual loaded preferences
**
**	V1.06 -. Latest Rebuild.Almost no changes,Just some GFX relayouting.
**
**	V1.07 -. Latest Rebuild Using GCC 4.2.3.Most warnings removed.
**
**	V1.8 [2024.06.11] Code reworked and changed GUI layout (jabierdlr@gmail.com)
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/application.h>
//#include <proto/locale.h>

//#include <libraries/gadtools.h>
#include <classes/window.h>

#include "vbagui_cat.h"

#include "vbagui_rev.h"
#include "includes.h"


char *version_cookie __attribute__((used)) = VERSTAG" (c)2005-2008,Vicente'Ami603'Gimeno\n\0";


//extern struct ExecIFace *IExec;
//extern struct DOSIFace *IDOS;
extern struct ApplicationIFace *IApplication;
extern struct PrefsObjectsIFace *IPrefsObjects;
//extern struct IconIFace *IIcon;
extern struct IntuitionIFace *IIntuition;
//extern struct GraphicsIFace *IGraphics;
extern struct UtilityIFace *IUtility;
// the class pointer
/*extern Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *GetFileClass,
             *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
             *RequesterClass, *SpaceClass, *IntegerClass, *GetFileClass, *SliderClass;*/
// some interfaces needed
//extern struct ListBrowserIFace *IListBrowser;
//extern struct ClickTabIFace *IClickTab;
//extern struct LayoutIFace *ILayout;
//extern struct ChooserIFace *IChooser;


extern int openlibs(void);
extern void closelibs(void);
extern int create_objects(void);
extern void do_events(void);
extern void destroy_objects(void);

extern struct FC_String vbagui_Strings[];
extern struct Window *window[WID_LAST];
//Object *gadget[GID_LAST];
extern Object *object[OID_LAST];


struct MsgPort *VBAPort;
struct Screen *scr;

/*
**	Visual Options Parser.
*/
STRPTR video[]  = {"--video-1x ", "--video-2x ", "--video-3x ", "--video-4x ", "--fullscreen ", NULL};
STRPTR yuv[]    = {" ","--yuv=0 ", "--yuv=1 ", "--yuv=2 ", "--yuv=3 ", "--yuv=4 ", NULL};
STRPTR filter[] = {"--filter-normal ", "--filter-tv-mode ", "--filter-2xsai ", "--filter-super-2xsai ", "--filter-super-eagle ", "--filter-pixelate ", "--filter-motion-blur ",
                   "--filter-advmame ", "--filter-simple2x ", "--filter-bilinear ", "--filter-bilinear+ ", "--filter-scanlines ", "--filter-hq2x ", "--filter-lq2x ", NULL};

STRPTR fskip[]  = {"--auto-frameskip ", "--frameskip=", "--no-auto-frameskip ", NULL};
STRPTR throt[]  = {"--no-throttle ", "--throttle=", NULL};
STRPTR ifb[]    = {"--ifb-none ", "--ifb-motion-blur ", "--ifb-smart ", NULL};
STRPTR pwi[]    = {"--no-pause-when-inactive ", "--pause-when-inactive ", NULL};
STRPTR sspeed[] = {"--no-show-speed ", "--show-speed-normal ", "--show-speed-detailed ", NULL};

/*
**	Game Options Parser.
*/
STRPTR bios = "--bios=";
//BOOL gdbport=TRUE;
STRPTR config = "--config=";

STRPTR fsize[]    = {"--flash-64k ", "--flash-128k ", NULL};
STRPTR savetype[] = {"--save-auto ", "--save-eeprom ", "--save-sram ", "--save-flash ", "--save-sensor ", "--save-none ", NULL};
STRPTR ipsptch[]  = {" ", "--ips=", NULL};
STRPTR mmxen[]    = {" ", "--no-mmx ", NULL};
STRPTR rtcen[]    = {"--no-rtc ", "--rtc ", NULL};

/*
**	Debug options parser
*/
STRPTR agbprt[] = {"--no-agb-print ", "--agb-print ", NULL};

STRPTR verbose[] = {"-v 1 ", "-v 2 ", "-v 4 ", "-v 8 ", "-v 16 ", "-v 32 ", "-v 64 ", "-v 128 ", "-v 256 ", "-v 512 ", NULL};
STRPTR debug[]   = {"--no-debug ", "--debug ", NULL};
STRPTR gdb[]     = {"-G tcp ", "-G tcp:", "-G pipe ", NULL};

/*
**	Application & preferences data.
*/
ULONG appID;
PrefsObject *VBAPrefs;

int videoptr,yuvptr,filterptr;
int fskipptr,fskipvalue;
int throttleptr,throttlevalue;
int ifbptr,fsizeptr,pwiptr;
int rtcptr,mmxptr,ipsptr;
int sesptr,savetypeptr;

int port,agbptr,verboseptr,debugptr,gdbptr;

CONST_STRPTR gamefile;
CONST_STRPTR biosfile;
CONST_STRPTR exefile;
CONST_STRPTR ipsfile;


/*
**	Here we setup a buffer with all the desired options, and launch them via IDOS->Execute()
**	running it async is not needed here,as the gui won't be needed again until we stop playing.
**	Also as a second purpose it allows us to iconify the window while playing, and opening it as
**	soon as we finish the gaming session.
*/
void parse_commandline(void)
{
	BOOL res;
	STRPTR vidbuffer,gamebuffer,buffer, fskipbuff, throttlebuff,debugbuffer,gdbbuffer;

	if (fskipptr==1)		fskipbuff = IUtility->ASPrintf("%s%ld ",fskip[fskipptr],fskipvalue);
	else				fskipbuff = IUtility->ASPrintf("%s ",fskip[fskipptr]);

	if (throttleptr==1)	throttlebuff = IUtility->ASPrintf("%s%ld ",throt[throttleptr],throttlevalue);
	else				throttlebuff = IUtility->ASPrintf("%s ",throt[throttleptr]);
	
	if (gdbptr==1)		gdbbuffer = IUtility->ASPrintf("%s%ld",gdb[gdbptr],port);
	else				gdbbuffer = IUtility->ASPrintf("%s",gdb[gdbptr]);

	vidbuffer = IUtility->ASPrintf("%s%s%s%s%s%s%s%s",video[videoptr],yuv[yuvptr],filter[filterptr],fskipbuff,throttlebuff,ifb[ifbptr],pwi[pwiptr],sspeed[sesptr]);
	gamebuffer = IUtility->ASPrintf("%s%s%s%s\"%s\"",fsize[fsizeptr],savetype[savetypeptr],mmxen[mmxptr],rtcen[rtcptr],gamefile);
	debugbuffer = IUtility->ASPrintf("%s %s %s %s",agbprt[agbptr],verbose[verboseptr],debug[debugptr],gdbbuffer);
	if (debugptr) 		buffer = IUtility->ASPrintf("%s %s%s %s",exefile,vidbuffer,gamebuffer,debugbuffer);
	else				buffer = IUtility->ASPrintf("%s %s%s",exefile,vidbuffer,gamebuffer);

	//res = RA_Iconify(object[OID_MAIN]);
IIntuition->IDoMethod(object[OID_MAIN], WM_ICONIFY);
	window[WID_MAIN] = NULL;
	IDOS->SystemTags(buffer,(BPTR)NULL,(BPTR)NULL);
	//window[WID_MAIN] = RA_OpenWindow(object[OID_MAIN]);
window[WID_MAIN] = (struct Window *)IIntuition->IDoMethod(object[OID_MAIN], WM_OPEN, NULL);

	IExec->FreeVec(buffer);
	IExec->FreeVec(vidbuffer);
	IExec->FreeVec(gamebuffer);
	IExec->FreeVec(throttlebuff);
	IExec->FreeVec(fskipbuff);
	IExec->FreeVec(debugbuffer);
	IExec->FreeVec(gdbbuffer);
}

/*
**	Main Routine, rocking already ;)
**
*/
int main(int argc, char *argv[])
{
	VBAPort= IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	if (!VBAPort)
		{
		IDOS->Printf("Cannot create Message Port.\n");
		closelibs();
		return -1;
		}

	if (openlibs()!=0)
		{
		IDOS->Printf("Error opening libraries/interfaces.\n");
		closelibs();
		IExec->FreeSysObject(ASOT_PORT,VBAPort);
		return -1;
		}

	appID=IApplication->RegisterApplication(NULL, 
			REGAPP_URLIdentifier,     "Ami603.es",
			REGAPP_LoadPrefs,         TRUE,
			REGAPP_SavePrefs,         TRUE,
			REGAPP_FileName,          "VBAGUI",
			REGAPP_NoIcon,            TRUE,
			REGAPP_UniqueApplication, TRUE,
			TAG_DONE);
	if (!appID)
		{
		IDOS->Printf("Error registering the Application.\n");
		IApplication->UnregisterApplication(appID, NULL);
		closelibs();
		IExec->FreeSysObject(ASOT_PORT,VBAPort);
		return -1;
		}

	if (IApplication->GetApplicationAttrs(appID, APPATTR_MainPrefsDict, (ULONG)&VBAPrefs, TAG_DONE))
		{
		videoptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Video Mode", 0);
		yuvptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "YUV Mode", 0);
		filterptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Filter Mode", 0);
		fskipptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "FrameSkip Mode", 0);
		fskipvalue = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "FrameSkip Value", 0);
		throttleptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Throttle Mode", 0);
		throttlevalue = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Throttle Value", 5);
		ifbptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "InterFrame Blending", 0);
		pwiptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Pause When Inactive", 0);
		sesptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Show Emulation Speed", 0);

		fsizeptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Flash size", 0);
		savetypeptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Save Type", 0);
		rtcptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "RTC", 0);
		mmxptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "MMX", 0);
		ipsptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "IPS", 0);
		exefile =(STRPTR) IPrefsObjects->DictGetStringForKey(VBAPrefs, "Executable", "VisualBoyAdvance");
		biosfile = (STRPTR)IPrefsObjects->DictGetStringForKey(VBAPrefs, "Bios File", "");
		ipsfile = (STRPTR)IPrefsObjects->DictGetStringForKey(VBAPrefs, "ips File", "");
		gamefile =(STRPTR)IPrefsObjects->DictGetStringForKey(VBAPrefs, "Game File", "");

		port = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Port", 55555);
		agbptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "AGB", 0);
		verboseptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Verbose",0);
		debugptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "Debug", 0);
		gdbptr = IPrefsObjects->DictGetIntegerForKey(VBAPrefs, "GDB", 0);
		}

	scr = IIntuition->LockPubScreen(NULL);

	if (create_objects()==0)
		{
		window[WID_MAIN] = (struct Window *)IIntuition->IDoMethod(object[OID_MAIN], WM_OPEN, NULL);
		IIntuition->UnlockScreen(scr);
		scr = NULL;
		do_events();
		}

	IIntuition->UnlockScreen(scr);
	destroy_objects();
	IApplication->UnregisterApplication(appID, NULL);
	closelibs();
	IExec->FreeSysObject(ASOT_PORT,VBAPort);

	return 0;
}
