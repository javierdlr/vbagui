/*
**	VisualBoy Advance GUI module
**	Freeware software
**	© 2005, by Vicente 'Ami603' Gimeno
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
*/

char *version_cookie __attribute__((used)) = "$VER:  VBA_GUI 1.07 (22.3.08)©2005-2008,Vicente'Ami603'Gimeno\n\0";

#define ALL_REACTION_CLASSES

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/application.h>
#include <proto/locale.h>
#include <libraries/gadtools.h>

#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "vbagui_cat.h"

/*
**	Library and interfaces initialization
*/

struct Library				*ApplicationBase;
struct Library				*LocaleBase;

struct ApplicationIFace		*IApplication;
struct PrefsObjectsIFace		*IPrefsObjects;
struct LocaleIFace			*ILocale;

struct Catalog *vbaguiCatalog;

STRPTR Pages[5];
STRPTR VideoModes[6] ; 
STRPTR YUVModes[7] ; 
STRPTR FilterModes[15]; 
STRPTR FrameSkip[4] ;
STRPTR SpeedLabel[4] ;
STRPTR IFBlending[4];
STRPTR GdbProto[4] ;
STRPTR Verbose[11];

STRPTR FlashSize[3] ;
STRPTR Save[7] ;

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

struct MsgPort *VBAPort;
struct Screen *scr;
struct Window *window[WID_LAST];
Object *gadget[GID_LAST];
Object *object[OID_LAST];

void parse_commandline(void) ;
void destroy_objects(void);

/*
**	Visual Options Parser.
*/


STRPTR video[] = {"--video-1x ",	"--video-2x ","--video-3x ","--video-4x ","--fullscreen ", NULL};
STRPTR yuv[] = {" ","--yuv=0 ","--yuv=1 ","--yuv=2 ","--yuv=3 ","--yuv=4 ",NULL};
STRPTR filter[] = {"--filter-normal ","--filter-tv-mode ","--filter-2xsai ","--filter-super-2xsai ","--filter-super-eagle ","--filter-pixelate ","--filter-motion-blur ",
			"--filter-advmame ","--filter-simple2x ","--filter-bilinear ","--filter-bilinear+ ","--filter-scanlines ","--filter-hq2x ","--filter-lq2x ",NULL};

STRPTR fskip[] = {"--auto-frameskip ","--frameskip=","--no-auto-frameskip ",	NULL};
STRPTR throt[] ={"--no-throttle ",	"--throttle=",NULL};
STRPTR ifb[] = {	"--ifb-none ","--ifb-motion-blur ",	"--ifb-smart ",NULL};
STRPTR pwi[] = {"--no-pause-when-inactive ","--pause-when-inactive ",NULL};
STRPTR sspeed[] = {"--no-show-speed ","--show-speed-normal ","--show-speed-detailed ",NULL};

/*
**	Game Options Parser.
*/

STRPTR bios = "--bios=";

BOOL gdbport=TRUE;
STRPTR config = "--config=";

STRPTR fsize[] = {"--flash-64k ","--flash-128k ",NULL};
STRPTR savetype[] = {"--save-auto ","--save-eeprom ","--save-sram ","--save-flash ","--save-sensor ","--save-none ",NULL};
STRPTR ipsptch[] = {" ","--ips=",NULL};
STRPTR mmxen[] = {" ","--no-mmx ",	NULL};
STRPTR rtcen[] = {"--no-rtc ","--rtc ",NULL};

/*
**	Debug options parser
*/

STRPTR agbprt[] = {"--no-agb-print ","--agb-print ",NULL};

STRPTR verbose[] = {"-v 1 ","-v 2 ","-v 4 ","-v 8 ","-v 16 ","-v 32 ","-v 64 ","-v 128 ","-v 256 ","-v 512 ",NULL};
STRPTR debug[] = {"--no-debug ","--debug ",NULL};
STRPTR gdb[] = {"-G tcp ","-G tcp:","-G pipe ",NULL};

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
**	Locale Strings
*/

struct FC_String vbagui_Strings[80] = {
	{ (STRPTR) "Acerca de...", 0 },
	{ (STRPTR) "Opciones Visuales", 1 },
	{ (STRPTR) "Opciones de Juego", 2 },
	{ (STRPTR) "Opciones de Depuracion", 3 },
	{ (STRPTR) "Modo de video", 4 },
	{ (STRPTR) "Pantalla Completa", 5 },
	{ (STRPTR) "Modo YUV", 6 },
	{ (STRPTR) "Ninguno", 7 },
	{ (STRPTR) "Filtros", 8 },
	{ (STRPTR) "Modo Normal", 9 },
	{ (STRPTR) "Modo TV", 10 },
	{ (STRPTR) "2xSal", 11 },
	{ (STRPTR) "Super 2xSal", 12 },
	{ (STRPTR) "Super Eagle", 13 },
	{ (STRPTR) "Pixelado", 14 },
	{ (STRPTR) "Motion Blur", 15 },
	{ (STRPTR) "AdvanceMAME escala 2x", 16 },
	{ (STRPTR) "Simple 2x", 17 },
	{ (STRPTR) "Bilinear", 18 },
	{ (STRPTR) "Bilinear Mas", 19 },
	{ (STRPTR) "Lineas de Scan", 20 },
	{ (STRPTR) "Alta Calidad 2x", 21 },
	{ (STRPTR) "Baja Calidad 2x", 22 },
	{ (STRPTR) "Saltar Frames", 23 },
	{ (STRPTR) "Automatico", 24 },
	{ (STRPTR) "Personalizado", 25 },
	{ (STRPTR) "Desactivado", 26 },
	{ (STRPTR) "Activar Sacudida", 27 },
	{ (STRPTR) "Blending entre Frames", 28 },
	{ (STRPTR) "Ninguno", 29 },
	{ (STRPTR) "Motion Blur", 30 },
	{ (STRPTR) "Inteligente", 31 },
	{ (STRPTR) "Pausar cuando Inactivo", 32 },
	{ (STRPTR) "No Mostrar velocidad de emulacion", 33 },
	{ (STRPTR) "Mostrar velocidad de emulacion", 34 },
	{ (STRPTR) "Mostrar datos detallados de velocidad", 35 },
	{ (STRPTR) "Ejecutable del programa", 36 },
	{ (STRPTR) "Por favor seleccione un ejecutable valido de VBA", 37 },
	{ (STRPTR) "Fichero de juego", 38 },
	{ (STRPTR) "Por favor seleccione un fichero valido de juego", 39 },
	{ (STRPTR) "Fichero BIOS", 40 },
	{ (STRPTR) "Por favor seleccione un fichero valido de BIOS", 41 },
	{ (STRPTR) "Tamaño de Flash", 42 },
	{ (STRPTR) "Flash 64k", 43 },
	{ (STRPTR) "Flash 128k", 44 },
	{ (STRPTR) "Modo de Grabacion", 45 },
	{ (STRPTR) "Automatico", 46 },
	{ (STRPTR) "EEPROM", 47 },
	{ (STRPTR) "SRAM", 48 },
	{ (STRPTR) "FLASH", 49 },
	{ (STRPTR) "EEPROM + Sensor", 50 },
	{ (STRPTR) "Ninguno", 51 },
	{ (STRPTR) "Activar Parcheo IPS", 52 },
	{ (STRPTR) "Por favor seleccione un fichero valido de IPS", 53 },
	{ (STRPTR) "Desactivar MMX", 54 },
	{ (STRPTR) "Activar RTC", 55 },
	{ (STRPTR) "Activar Depuracion", 56 },
	{ (STRPTR) "Salida de Depuracion", 57 },
	{ (STRPTR) "1-SWI", 58 },
	{ (STRPTR) "2-Accesos a memoria no alineados", 59 },
	{ (STRPTR) "4-Escritura a memoria ilegal", 60 },
	{ (STRPTR) "8-Lectura de memoria ilegal", 61 },
	{ (STRPTR) "16-DMA 0", 62 },
	{ (STRPTR) "32-DMA 1", 63 },
	{ (STRPTR) "64-DMA 2", 64 },
	{ (STRPTR) "128-DMA 3", 65 },
	{ (STRPTR) "256-Instruccion no definida", 66 },
	{ (STRPTR) "512-Mensajes AGBPrint", 67 },
	{ (STRPTR) "Protocolo GDB", 68 },
	{ (STRPTR) "Puerto TCP por defecto", 69 },
	{ (STRPTR) "Puerto TCP especifico", 70 },
	{ (STRPTR) "Transporte por PIPE", 71 },
	{ (STRPTR) "Activar AGBPrint", 72 },
	{ (STRPTR) "Guardar", 73 },
	{ (STRPTR) "Lanzar", 74 },
	{ (STRPTR) "Salir", 75 },
	{ (STRPTR) "Atencion!", 76 },
	{ (STRPTR) "Por favor compruebe que ha seleccionado tanto el ejecutable\n del emulador como el fichero de juego antes de lanzar el emulador\n", 77 },
	{ (STRPTR) "De Acuerdo", 78 },
	{ (STRPTR) "1.07 AmigaOS4 Nativa\n\nCodigo: Vicente 'Ami603'Gimeno\nLogotipo: Kai 'Restore' Thorsberg,\nPort nativo OS4 de VisualBoy Advance:\nNicolas 'nicomen' Mendoza\n\nEste GUI 'Rapido y sucio' no hubiese\nvisto la luz del sol si Tony 'ToAks' Aksnes\nno me lo hubiese 'pedido amablemente' ;)\nSaludotes a aGas ;)\n" ,79 }
};



/*
**	Libraries&interfaces Stuff
*/

int openlibs(void)
{
	ApplicationBase = IExec->OpenLibrary("application.library", 50);
	if (ApplicationBase)
		{
		IApplication  = (struct ApplicationIFace *)IExec->GetInterface(ApplicationBase, "application", 2, NULL);
		IPrefsObjects = (struct PrefsObjectsIFace *)IExec->GetInterface(ApplicationBase, "prefsobjects", 2, NULL);
		}
	if (!IApplication || !IPrefsObjects)
		{
		IDOS->Printf("Error Obtaining Application.library interfaces\n");
		return -1;
		}
	LocaleBase = IExec->OpenLibrary("locale.library",50);
	if (LocaleBase)
		{
		ILocale = (struct LocaleIFace *)IExec->GetInterface(LocaleBase, "main",1,NULL);
		}
	if (!ILocale)
		{
		IDOS->Printf("Error Obtaining Locale.library interfaces\n");
		return -1;
		}

	vbaguiCatalog = ILocale->OpenCatalog(NULL, (STRPTR) "vbagui.catalog",
				     OC_BuiltInLanguage, "español",
				     OC_Version, 0,
				     TAG_DONE);

	if (vbaguiCatalog)
		{
		struct FC_String *fc;
		int i;
		for (i = 0, fc = vbagui_Strings;  i < 80;  i++, fc++)
			{
			fc->msg = ILocale->GetCatalogStr(vbaguiCatalog, fc->id,(CONST_STRPTR) fc->msg);
			}
		}
	return 0;
}

void closelibs(void)
{
	if (vbaguiCatalog)	ILocale->CloseCatalog(vbaguiCatalog);

	if (IApplication)		IExec->DropInterface((struct Interface *)IApplication);
	if (IPrefsObjects)		IExec->DropInterface((struct Interface *)IPrefsObjects);
	if (ILocale)		IExec->DropInterface((struct Interface *)ILocale);

	if (ApplicationBase)	IExec->CloseLibrary(ApplicationBase);
	if (LocaleBase)		IExec->CloseLibrary(LocaleBase);
}

/*
**	This is the function where we
**	create all the layout objects.
*/

int create_objects(void)
{

STRPTR Pages[] = { msg_about, msg_visual, msg_game, msg_debug, NULL}; 

/*
**	Visual Options
*/

STRPTR VideoModes[] = { "Video 1X", "Video 2X", "Video 3X", "Video 4X", msg_fullscreen, NULL}; 
STRPTR YUVModes[] = { msg_yuvnone, "YV12", "UYVY", "YVYU", "YUY2", "IYUV", NULL}; 
STRPTR FilterModes[] = 
{ 
	msg_filter1,	msg_filter2, msg_filter3, msg_filter4, msg_filter5, msg_filter6, msg_filter7, msg_filter8,
	msg_filter9, msg_filter10, msg_filter11, msg_filter12, msg_filter13,	msg_filter14,NULL
}; 

STRPTR FrameSkip[] = { msg_fskauto, msg_fskcustom, msg_fskdisab, NULL};
STRPTR SpeedLabel[] = { msg_show1, msg_show2, msg_show3, NULL};
STRPTR IFBlending[] = { msg_ifb1, msg_ifb2, msg_ifb3, NULL};

/*
**	Debug Options
*/

STRPTR GdbProto[] = { msg_gdbproto1, msg_gdbproto2, msg_gdbproto3, NULL};
STRPTR Verbose[] = 
{
	msg_verbose1, msg_verbose2, msg_verbose3, msg_verbose4,msg_verbose5,
	msg_verbose6,msg_verbose7,msg_verbose8,msg_verbose9,msg_verbose10,NULL
};

/*
**	Game Options
*/

STRPTR FlashSize[] = { msg_flash1, msg_flash2, NULL};
STRPTR Save[] = { msg_savetype1, msg_savetype2, msg_savetype3, msg_savetype4, msg_savetype5, msg_savetype6, NULL};

object[OID_ABOUT] = BitMapObject,
					BITMAP_SourceFile,"PROGDIR:images/visual.png",
					BITMAP_Transparent,TRUE,
					BITMAP_Masking,TRUE,
					BITMAP_Screen,scr,
				BitMapEnd;

if (!object[OID_ABOUT])
	{
	IDOS->Printf("Cannot load program logo\n");
	return -1;
	}
/*
**	This is the first Page layout
**	Where all program credits
**	belongs to ;).
*/


object[OID_PAGE1] = VLayoutObject,
		LAYOUT_BevelStyle,      BVS_FIELD,
		LAYOUT_AddChild,HLayoutObject,
			LAYOUT_AddImage, LabelObject,
				LABEL_Justification,LJ_CENTRE,
				LABEL_Image,object[OID_ABOUT],
			LabelEnd,
		LAYOUT_AddChild,HLayoutObject,
		LAYOUT_HorizAlignment,LALIGN_CENTER,
			LAYOUT_AddImage, LabelObject,
				LABEL_Justification,LJ_CENTRE,
				LABEL_Text, msg_bigabout,
			LabelEnd,
		LayoutEnd,
	LayoutEnd,
End;

if (!object[OID_PAGE1])
	{
	IDOS->Printf("Cannot create 1st clicktab Page\n");
	return -1;
	}
/*
**	This is the second Page layout
**	All the Visual Options ends
**	there.
*/

object[OID_PAGE2] = HLayoutObject,
		LAYOUT_HorizAlignment,LALIGN_CENTER,
		LAYOUT_FixedVert,FALSE,
		LAYOUT_FixedHoriz,FALSE,
		LAYOUT_AddChild,VLayoutObject,
			LAYOUT_AddChild,gadget[GID_VIDEO] = ChooserObject,
				CHOOSER_LabelArray,VideoModes ,
				GA_ID, GID_VIDEO,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected,videoptr,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text,msg_videomode, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_YUV] = ChooserObject,
				CHOOSER_LabelArray,YUVModes ,
				GA_ID, GID_YUV,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected,yuvptr,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_yuv, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_FILTER] = ChooserObject,
				CHOOSER_LabelArray,FilterModes ,
				CHOOSER_MaxLabels,15,
				CHOOSER_Selected,filterptr,
				GA_ID, GID_FILTER,
				GA_RelVerify,       TRUE,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_filter, 
				LabelEnd,
			LAYOUT_AddChild,HLayoutObject,
				LAYOUT_AddChild,gadget[GID_FSKIP] = ChooserObject,
					CHOOSER_LabelArray,FrameSkip ,
					CHOOSER_Selected,fskipptr,
					GA_ID, GID_FSKIP,
					GA_RelVerify,       TRUE,
				ChooserEnd,
					CHILD_Label, LabelObject, 
						LABEL_Text, msg_fskip, 
					LabelEnd,
				LAYOUT_AddChild,gadget[GID_FSKIPSET] = IntegerObject,
					GA_RelVerify,TRUE,
					GA_ID,GID_FSKIPSET,
					GA_Disabled,(BOOL) !(fskipptr==1),
					INTEGER_Maximum,9,
					INTEGER_Number,fskipvalue,
					INTEGER_Minimum,0,
					INTEGER_Arrows,TRUE,
				IntegerEnd,
			LayoutEnd,
			LAYOUT_AddChild,HLayoutObject,
				LAYOUT_AddChild,gadget[GID_THROTTLE] = CheckBoxObject,
					GA_RelVerify,       TRUE,
					GA_Selected,(BOOL )throttleptr,
					GA_ID,GID_THROTTLE,
					GA_Text,msg_throttle,
				CheckBoxEnd,
				LAYOUT_AddChild,gadget[GID_THROTTLESET] = IntegerObject,
					GA_RelVerify,TRUE,
					GA_ID,GID_THROTTLESET,
					GA_Disabled,(BOOL) !throttleptr,
					INTEGER_Maximum,1000,
					INTEGER_Number,throttlevalue,
					INTEGER_Minimum,5,
					INTEGER_Arrows,TRUE,
				IntegerEnd,
			LayoutEnd,
		LayoutEnd,
		LAYOUT_AddChild,VLayoutObject,
			LAYOUT_AddChild,gadget[GID_IFBLEND] = ChooserObject,
				CHOOSER_LabelArray, IFBlending,
				GA_ID, GID_IFBLEND,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected,ifbptr,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_ifb, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_PWINAC] = CheckBoxObject,
				GA_RelVerify,       TRUE,
				GA_ID,GID_PWINAC,
				GA_Selected,(BOOL *)pwiptr,
				GA_Text,msg_pwi,
			CheckBoxEnd,
			LAYOUT_AddChild,    gadget[GID_SPEED] = RadioButtonObject,
				GA_Text,        SpeedLabel,
				GA_RelVerify,	TRUE,
				RADIOBUTTON_Selected,sesptr,
				RADIOBUTTON_LabelPlace,PLACETEXT_RIGHT,
				GA_ID,GID_SPEED,

			RadioButtonEnd,
		LayoutEnd,
	End;

if (!object[OID_PAGE2])
	{
	IDOS->Printf("Cannot create 2nd clicktab Page\n");
	return -1;
	}

/*
**	This is the third Page layout
**	Where all Gaming Options
**	are about to appear.
*/

object[OID_PAGE3] = HLayoutObject,
		LAYOUT_HorizAlignment,LALIGN_CENTER,
		LAYOUT_FixedVert,FALSE,
		LAYOUT_FixedHoriz,FALSE,
		LAYOUT_AddChild,VLayoutObject,
			LAYOUT_AddChild,gadget[GID_EXE] = GetFileObject,
				GA_ID, GID_EXE,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly,TRUE,
				GETFILE_FullFile,exefile,
				GETFILE_TitleText,msg_exeASL,
			GetFileEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_exe, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_GAME] = GetFileObject,
				GA_ID, GID_GAME,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly,TRUE,
				GETFILE_FullFile,gamefile,
				GETFILE_TitleText,msg_gameASL,
			GetFileEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_gamefile, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_BIOS] = GetFileObject,
				GA_ID, GID_BIOS,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly,TRUE,
				GETFILE_FullFile,biosfile,
				GETFILE_TitleText,msg_biosASL,
			GetFileEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_biosfile, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_FLASH] = ChooserObject,
				CHOOSER_LabelArray,FlashSize ,
				CHOOSER_Selected,fsizeptr,
				GA_ID, GID_FLASH,
				GA_RelVerify,       TRUE,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_flash, 
				LabelEnd,
			LAYOUT_AddChild,gadget[GID_SAVE] = ChooserObject,
				CHOOSER_LabelArray,Save,
				CHOOSER_Selected,savetypeptr,
				GA_ID, GID_SAVE,
				GA_RelVerify,       TRUE,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_savetype, 
				LabelEnd,
		LayoutEnd,
		LAYOUT_AddChild,VLayoutObject,
		LAYOUT_FixedVert,FALSE,
		LAYOUT_FixedHoriz,FALSE,
			LAYOUT_AddChild,HLayoutObject,
				LAYOUT_AddChild,gadget[GID_IPS] = CheckBoxObject,
					GA_RelVerify,       TRUE,
					GA_ID,GID_IPS,
					GA_Selected,(BOOL )ipsptr,
					GA_Text,msg_ips,
				CheckBoxEnd,
				LAYOUT_AddChild,gadget[GID_IPSFILE] = GetFileObject,
					GA_Disabled,(BOOL )!ipsptr,
					GA_RelVerify,TRUE,
					GA_ID,GID_IPSFILE,
					GETFILE_ReadOnly,TRUE,
					GETFILE_FullFile,ipsfile,
					GETFILE_TitleText,msg_ipsASL,
				GetFileEnd,
			LayoutEnd,
			LAYOUT_AddChild,HLayoutObject,
				LAYOUT_AddChild,gadget[GID_MMX] = CheckBoxObject,
					GA_RelVerify,       TRUE,
					GA_ID,GID_MMX,
					GA_Text,msg_mmx,
					GA_Selected,(BOOL *)mmxptr,
				CheckBoxEnd,
				LAYOUT_AddChild,gadget[GID_RTC] = CheckBoxObject,
					GA_RelVerify,       TRUE,
					GA_ID,GID_RTC,
				GA_Selected,(BOOL *)rtcptr,
				GA_Text,msg_rtc,
			CheckBoxEnd,
		LayoutEnd,
	LayoutEnd,
End;

if (!object[OID_PAGE3])
	{
	IDOS->Printf("Cannot create 3rd clicktab Page\n");
	return -1;
	}

/*
**	Last, but not least, the fourth page
**	Debug Options, going into the unknown
*/

object[OID_PAGE4] = HLayoutObject,
		LAYOUT_HorizAlignment,LALIGN_CENTER,
		LAYOUT_FixedVert,FALSE,
		LAYOUT_FixedHoriz,FALSE,
		LAYOUT_AddChild,VLayoutObject,
			LAYOUT_AddChild,gadget[GID_DEBUG] = CheckBoxObject,
				GA_RelVerify,       TRUE,
				GA_ID,GID_DEBUG,
				GA_Selected,(BOOL)debugptr,
				GA_Text,msg_endebug,
			CheckBoxEnd,
			LAYOUT_AddChild,gadget[GID_VERBOSE] = ChooserObject,
				CHOOSER_LabelArray,Verbose,
				CHOOSER_Selected,verboseptr,
				GA_Disabled,(BOOL)!debugptr,
				GA_ID, GID_VERBOSE,
				GA_RelVerify,       TRUE,
			ChooserEnd,
				CHILD_Label, LabelObject, 
					LABEL_Text, msg_verbose, 
				LabelEnd,
			LAYOUT_AddChild,HLayoutObject,
				LAYOUT_AddChild,gadget[GID_GDBPROTO] = ChooserObject,
					CHOOSER_LabelArray,GdbProto ,
					CHOOSER_Selected,gdbptr,
					GA_Disabled,(BOOL)!debugptr,
					GA_ID, GID_GDBPROTO,
					GA_RelVerify,       TRUE,

				ChooserEnd,
					CHILD_Label, LabelObject, 
						LABEL_Text, msg_gdbproto, 
					LabelEnd,
				LAYOUT_AddChild,gadget[GID_GDBPORT] = IntegerObject,
					GA_RelVerify,TRUE,
					GA_Disabled,!((BOOL)debugptr && (gdbptr==1)),
					GA_ID,GID_GDBPORT,
					INTEGER_Maximum,65535,
					INTEGER_Number,port,
					INTEGER_Minimum,0,
					INTEGER_Arrows,TRUE,
				IntegerEnd,
			LayoutEnd,
		LayoutEnd,
		LAYOUT_AddChild,VLayoutObject,
			LAYOUT_AddChild,gadget[GID_AGBPRINT] = CheckBoxObject,
				GA_RelVerify,       TRUE,
				GA_Disabled,(BOOL)!debugptr,
				GA_ID,GID_AGBPRINT,
				GA_Text,msg_agbprint,
				GA_Selected,(BOOL)agbptr,
			CheckBoxEnd,
		LayoutEnd,
	End;

if (!object[OID_PAGE4])
	{
	IDOS->Printf("Cannot create 4th clicktab Page\n");
	return -1;
	}

/*
**	Now we made a grouped clicktab
**	with all the previous pages
*/

object[OID_CLICKTAB] = ClickTabObject,
		GA_Text,            Pages,
		CLICKTAB_Current,   0,
		CLICKTAB_LabelTruncate,TRUE,
		CLICKTAB_PageGroup, PageObject,
			PAGE_Add,       object[OID_PAGE1],
			PAGE_Add,       object[OID_PAGE2],
			PAGE_Add,       object[OID_PAGE3],
			PAGE_Add,       object[OID_PAGE4],
		ClickTabEnd,
	End;

if (!object[OID_CLICKTAB])
	{
	IDOS->Printf("Cannot create clicktab Object\n");
	return -1;
	}

/*
**	And finally the window Layout
**	with the last horizontal layout
**	storing the most useful options ;).
*/

object[OID_MAIN] = WindowObject,
		WA_Title,           "VisualBoy Advance GUI",
		WA_DragBar,         TRUE,
		WA_SmartRefresh,    TRUE,
		WA_CloseGadget,     TRUE,
		WA_DepthGadget,     TRUE,
		WA_CustomScreen,scr,
		WINDOW_IconifyGadget,TRUE,
		WINDOW_AppPort,VBAPort,
		WINDOW_Position, WPOS_CENTERSCREEN,
		WINDOW_ParentGroup, VLayoutObject,
			LAYOUT_HorizAlignment,LALIGN_CENTER,
			LAYOUT_EvenSize,   FALSE,
			LAYOUT_FixedVert,FALSE,
			LAYOUT_FixedHoriz,FALSE,
			LAYOUT_ShrinkWrap,TRUE,
			LAYOUT_AddChild, object[OID_CLICKTAB],
			LAYOUT_AddChild,HLayoutObject,
				LAYOUT_EvenSize,   TRUE,
				LAYOUT_AddChild,gadget[GID_SAVECONF]=ButtonObject,
					GA_RelVerify,TRUE,
					GA_Text,msg_save,
					GA_ID,GID_SAVECONF,
				ButtonEnd,
				LAYOUT_AddChild,gadget[GID_PLAY]=ButtonObject,
					GA_RelVerify,TRUE,
					GA_Text,msg_launch,
					GA_ID,GID_PLAY,
				ButtonEnd,
				LAYOUT_AddChild,gadget[GID_QUIT]=ButtonObject,
					GA_RelVerify,TRUE,
					GA_Text,msg_exit,
					GA_ID,GID_QUIT,
				ButtonEnd,
			LayoutEnd,
		LayoutEnd,
	WindowEnd;     

if (!object[OID_MAIN])
	{
	IDOS->Printf("Cannot create Window Object\n");
	return -1;
	}

/*
**	This requester will warn the user if he wants to play no game, or without emulator ;)
*/

object[OID_REQ] = IIntuition->NewObject(IRequester->REQUESTER_GetClass(),NULL,
	REQ_Type,	REQTYPE_INFO,
	REQ_TitleText,	msg_warning,
	REQ_BodyText,	msg_warningtxt,
	REQ_Image,REQIMAGE_WARNING,
	REQ_GadgetText,	msg_ok,
	TAG_DONE);

if (!object[OID_REQ])
	{
	IDOS->Printf("Cannot create Requester Object\n");
	return -1;
	}

return 0;
}


void destroy_objects()
{
	if (object[OID_MAIN])		IIntuition->DisposeObject(object[OID_MAIN]);
	if (object[OID_ABOUT])	IIntuition->DisposeObject(object[OID_ABOUT]);
	if (object[OID_REQ])		IIntuition->DisposeObject(object[OID_REQ]);
}

void do_events(void)
{
	ULONG wait, signal, app = (1L << window[WID_MAIN]->UserPort->mp_SigBit);
	ULONG done = FALSE,
			res;
	ULONG result;
	UWORD code;
	IIntuition->GetAttr(WINDOW_SigMask, object[OID_MAIN], &signal);
	while (!done)
		{
		wait = IExec->Wait( signal | SIGBREAKF_CTRL_C | app );
		if ( wait & SIGBREAKF_CTRL_C )
			{
			done = TRUE;
			}
	   	else
			{
			while ( (result = RA_HandleInput(object[OID_MAIN], &code) ) != WMHI_LASTMSG )
				{
				switch (result & WMHI_CLASSMASK)
     					{
     					case WMHI_CLOSEWINDOW:
						window[WID_MAIN] = NULL;
						done = TRUE;
						break;
					case WMHI_ICONIFY:
						res = RA_Iconify(object[OID_MAIN]);
						window[WID_MAIN] = NULL;
						break;
					case WMHI_UNICONIFY:
						window[WID_MAIN] = (struct Window *) RA_OpenWindow(object[OID_MAIN]);
						if (window[WID_MAIN])
							{
							IIntuition->GetAttr(WINDOW_SigMask, object[OID_MAIN], &signal);
							}
						else
							{
							done = TRUE;
							}
						break;
					case WMHI_GADGETUP:
						switch (result & WMHI_GADGETMASK)
							{
							case GID_VIDEO:
								videoptr=code; 
								break;
							case GID_YUV:
								yuvptr=code;
								break;
							case GID_FILTER:
								filterptr=code;
								break;
							case GID_FSKIP:
								if (code==1)
									{
 									IIntuition->SetGadgetAttrs((struct Gadget *)(struct Gadget *)gadget[GID_FSKIPSET],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									}
								else
									{
 									IIntuition->SetGadgetAttrs((struct Gadget *)(struct Gadget *)gadget[GID_FSKIPSET],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									}

								fskipptr=code;
								break;
								case GID_FSKIPSET:
								fskipvalue=code;
								break;
							case GID_THROTTLE:
								if (code)
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_THROTTLESET],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									}
								else
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_THROTTLESET],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									}
								throttleptr=code;
								break;
							case GID_THROTTLESET:
								throttlevalue=code;
								break;
							case GID_SPEED:
								sesptr=code;
								break;
							case GID_IFBLEND:
								ifbptr=code;
								break;
							case GID_PWINAC:
								pwiptr=code;
								break;

							case GID_EXE:
								IIntuition->IDoMethod((Object *)gadget[GID_EXE],GFILE_REQUEST,window[WID_MAIN]);
								IIntuition->GetAttrs(gadget[GID_EXE],GETFILE_FullFile, &exefile, TAG_DONE);
								IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_EXE],window[WID_MAIN],NULL);
								break;
							case GID_GAME:
								IIntuition->IDoMethod((Object *)gadget[GID_GAME],GFILE_REQUEST ,window[WID_MAIN]);
								IIntuition->GetAttrs(gadget[GID_GAME],GETFILE_FullFile, &gamefile, TAG_DONE);
								IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_GAME],window[WID_MAIN],NULL);
								break;
							case GID_BIOS:
								IIntuition->IDoMethod((Object *)gadget[GID_BIOS],GFILE_REQUEST ,window[WID_MAIN]);
								IIntuition->GetAttrs(gadget[GID_BIOS],GETFILE_FullFile, &biosfile, TAG_DONE);
								IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_BIOS],window[WID_MAIN],NULL);
								break;
							case GID_FLASH:
								fsizeptr=code;
								break;
							case GID_SAVE:
								savetypeptr=code;
								break;
							case GID_MMX:
								if (code==TRUE)	mmxptr=1;
								else				mmxptr=0;
								break;
							case GID_IPS:
								if (code)
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_IPSFILE],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									}
								else
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_IPSFILE],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									}
								break;
							case GID_IPSFILE:
								IIntuition->IDoMethod((Object *)gadget[GID_IPSFILE],GFILE_REQUEST ,window[WID_MAIN]);
								IIntuition->GetAttrs(gadget[GID_IPSFILE],GETFILE_FullFile, &ipsfile, TAG_DONE);
								IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_IPSFILE],window[WID_MAIN],NULL);
								break;
							case GID_RTC:
								if (code==TRUE)	rtcptr=1;
								else				rtcptr=0;
								break;
							case GID_GDBPROTO:
								switch (code)
									{
									case 0:
										port = 55555;
										IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_GDBPORT],window[WID_MAIN],NULL,INTEGER_Number,port,TAG_DONE);
									case 2:
										gdbport=TRUE;
										break;
									case 1:
										gdbport=FALSE;

										break;
									}
								gdbptr = code;
								IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_GDBPORT],window[WID_MAIN],NULL,GA_Disabled,gdbport,TAG_DONE);
								break;
							case GID_GDBPORT:
								port = code;
								break;
							case GID_DEBUG:
								if (code)
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_VERBOSE],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_GDBPROTO],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_AGBPRINT],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_GDBPORT],window[WID_MAIN],NULL,GA_Disabled,!(gdbptr==1),TAG_DONE);
									debugptr = 1;
									}
								else
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_VERBOSE],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_GDBPROTO],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_AGBPRINT],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_GDBPORT],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									debugptr = 0;
									}
								break;
							case GID_VERBOSE:
								verboseptr = code;
								break;
							case GID_AGBPRINT:
								agbptr = code;
								break;

							case GID_SAVECONF:
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,videoptr,TAG_DONE),"Video Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,yuvptr,TAG_DONE),"YUV Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,filterptr,TAG_DONE),"Filter Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,fskipptr,TAG_DONE),"FrameSkip Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,fskipvalue,TAG_DONE),"FrameSkip Value");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,throttleptr,TAG_DONE),"Throttle Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,throttlevalue,TAG_DONE),"Throttle Value");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,ifbptr,TAG_DONE),"InterFrame Blending");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,pwiptr,TAG_DONE),"Pause When Inactive");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,sesptr,TAG_DONE),"Show Emulation Speed");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,fsizeptr,TAG_DONE),"Flash size");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,savetypeptr,TAG_DONE),"Save Type");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,rtcptr,TAG_DONE),"RTC");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,mmxptr,TAG_DONE),"MMX");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,ipsptr,TAG_DONE),"IPS");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
										IPrefsObjects->PrefsString(NULL, NULL, ALPOSTR_AllocSetString, ipsfile, TAG_DONE),"ips File");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
										IPrefsObjects->PrefsString(NULL, NULL, ALPOSTR_AllocSetString, gamefile, TAG_DONE),"Game File");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
										IPrefsObjects->PrefsString(NULL, NULL, ALPOSTR_AllocSetString, biosfile, TAG_DONE),"Bios File");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
										IPrefsObjects->PrefsString(NULL, NULL, ALPOSTR_AllocSetString, exefile, TAG_DONE),"Executable");

								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,port,TAG_DONE),"Port");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,agbptr,TAG_DONE),"AGB");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,verboseptr,TAG_DONE),"Verbose");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,debugptr,TAG_DONE),"Debug");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
									IPrefsObjects->PrefsNumber(NULL,NULL, 
										ALPONUM_AllocSetLong,gdbptr,TAG_DONE),"GDB");
								break;
							case GID_PLAY:
								if (strcmp(gamefile,"")&&strcmp(exefile,""))
									parse_commandline();
								else
									IIntuition->IDoMethod(object[OID_REQ],RM_OPENREQ,NULL,window[WID_MAIN],NULL,TAG_DONE);
								break;
							case GID_QUIT:
								done= TRUE;
								break;
							}
					break;
				}
			}
		}
	}
}
/*
** 	Here we setup a buffer with all the desired options, and launch them via IDOS->Execute()
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
	res = RA_Iconify(object[OID_MAIN]);
	window[WID_MAIN] = NULL;
	IDOS->SystemTags(buffer,(BPTR)NULL,(BPTR)NULL);
	window[WID_MAIN] = RA_OpenWindow(object[OID_MAIN]);

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

int main(void)
{
	VBAPort= IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	if (!VBAPort)
		{
		IDOS->Printf("Cannot Create Message Port.\n");
		closelibs();
		return -1;
		}

	if (openlibs()!=0)
		{
		IDOS->Printf("Error Opening Libraries/Interfaces.\n");
		closelibs();
		IExec->FreeSysObject(ASOT_PORT,VBAPort);
		return -1;
		}
	appID=IApplication->RegisterApplication(NULL, 
			REGAPP_URLIdentifier, "Ami603.es",
			REGAPP_LoadPrefs, TRUE,
			REGAPP_SavePrefs, TRUE,
			REGAPP_FileName,"VBAGUI",
			REGAPP_NoIcon,TRUE,
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
		window[WID_MAIN] = RA_OpenWindow(object[OID_MAIN]);
		IIntuition->UnlockScreen(scr);
		do_events();
		}
	destroy_objects();
	IApplication->UnregisterApplication(appID, NULL);
	closelibs();
	IExec->FreeSysObject(ASOT_PORT,VBAPort);
	return 0;
}
