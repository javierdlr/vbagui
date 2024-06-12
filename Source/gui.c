/*
GUI.C
*/


//#define ALL_REACTION_CLASSES

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/application.h>
//#include <proto/locale.h>

#include <libraries/gadtools.h>
#include <classes/window.h>
#include <classes/requester.h>
#include <gadgets/clicktab.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/checkbox.h>
#include <gadgets/chooser.h>
//#include <gadgets/string.h>
#include <gadgets/space.h>
#include <gadgets/integer.h>
#include <gadgets/getfile.h>
//#include <gadgets/slider.h>
#include <gadgets/radiobutton.h>
#include <images/label.h>
#include <images/bitmap.h>

//#include <strings.h>

#include "vbagui_cat.h"

#include "vbagui_rev.h"
#include "includes.h"
#include "debug.h"


extern void parse_commandline(void);

//extern struct ExecIFace *IExec;
//extern struct DOSIFace *IDOS;
extern struct ApplicationIFace *IApplication;
extern struct PrefsObjectsIFace *IPrefsObjects;
//extern struct IconIFace *IIcon;
extern struct IntuitionIFace *IIntuition;
//extern struct GraphicsIFace *IGraphics;
extern struct UtilityIFace *IUtility;
// the class pointer
extern Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *GetFileClass,
             *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
             *RequesterClass, *SpaceClass, *IntegerClass, *GetFileClass, *SliderClass,
             *RadioButtonClass;
// some interfaces needed
//extern struct ListBrowserIFace *IListBrowser;
//extern struct ClickTabIFace *IClickTab;
//extern struct LayoutIFace *ILayout;
//extern struct ChooserIFace *IChooser;

extern struct FC_String vbagui_Strings[];
extern struct MsgPort *VBAPort;
extern struct Screen *scr;

/*
**	Application & preferences data.
*/
//ULONG appID;
extern PrefsObject *VBAPrefs;

extern int videoptr,yuvptr,filterptr;
extern int fskipptr,fskipvalue;
extern int throttleptr,throttlevalue;
extern int ifbptr,fsizeptr,pwiptr;
extern int rtcptr,mmxptr,ipsptr;
extern int sesptr,savetypeptr;

extern int port,agbptr,verboseptr,debugptr,gdbptr;

extern CONST_STRPTR gamefile;
extern CONST_STRPTR biosfile;
extern CONST_STRPTR exefile;
extern CONST_STRPTR ipsfile;


STRPTR Pages[5];
STRPTR VideoModes[6];
STRPTR YUVModes[7];
STRPTR FilterModes[15];
STRPTR FrameSkip[4];
STRPTR SpeedLabel[4];
STRPTR IFBlending[4];
STRPTR GdbProto[4];
STRPTR Verbose[11];

STRPTR FlashSize[3];
STRPTR Save[7];

struct Window *window[WID_LAST];
Object *gadget[GID_LAST];
Object *object[OID_LAST];

/*
**	Game Options Parser.
*/
BOOL gdbport=TRUE;


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
STRPTR VideoModes[]  = { "Video 1X", "Video 2X", "Video 3X", "Video 4X", msg_fullscreen, NULL}; 
STRPTR YUVModes[]    = { msg_yuvnone, "YV12", "UYVY", "YVYU", "YUY2", "IYUV", NULL}; 
STRPTR FilterModes[] = { msg_filter1, msg_filter2, msg_filter3, msg_filter4, msg_filter5, msg_filter6, msg_filter7, msg_filter8,
                         msg_filter9, msg_filter10, msg_filter11, msg_filter12, msg_filter13, msg_filter14, NULL}; 
STRPTR FrameSkip[]   = { msg_fskauto, msg_fskcustom, msg_fskdisab, NULL};
STRPTR SpeedLabel[]  = { msg_show1, msg_show2, msg_show3, NULL};
STRPTR IFBlending[]  = { msg_ifb1, msg_ifb2, msg_ifb3, NULL};

/*
**	Debug Options
*/
STRPTR GdbProto[] = { msg_gdbproto1, msg_gdbproto2, msg_gdbproto3, NULL};
STRPTR Verbose[]  = { msg_verbose1, msg_verbose2, msg_verbose3, msg_verbose4, msg_verbose5,
                      msg_verbose6, msg_verbose7, msg_verbose8, msg_verbose9, msg_verbose10, NULL};

/*
**	Game Options
*/
STRPTR FlashSize[] = { msg_flash1, msg_flash2, NULL};
STRPTR Save[]      = { msg_savetype1, msg_savetype2, msg_savetype3, msg_savetype4, msg_savetype5, msg_savetype6, NULL};

object[OID_ABOUT] = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
		BITMAP_SourceFile,  "PROGDIR:vba_banner.png",
		BITMAP_Transparent, TRUE,
		BITMAP_Masking,     TRUE,
		BITMAP_Screen,      scr,
		TAG_DONE);

/*if (!object[OID_ABOUT])
	{
	IDOS->Printf("Cannot load program logo\n");
	return -1;
	}*/

/*
**	This is the first Page layout
**	Where all program credits
**	belongs to ;).
*/
object[OID_PAGE1] = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
		LAYOUT_BevelStyle,     BVS_FIELD,
		LAYOUT_HorizAlignment, LALIGN_CENTER,
		LAYOUT_AddImage, IIntuition->NewObject(LabelClass, NULL, //"label.image",
			//LABEL_Justification, LJ_CENTRE,
			object[OID_ABOUT]? LABEL_Image : TAG_IGNORE, object[OID_ABOUT],
		TAG_DONE),
		LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
		CHILD_WeightedHeight, 15,
		LAYOUT_AddImage, IIntuition->NewObject(LabelClass, NULL, //"label.image",
			LABEL_Justification, LJ_CENTRE,
			LABEL_SoftStyle, FSF_BOLD,
			LABEL_Text,      VERS" "DATE,
			LABEL_SoftStyle, FS_NORMAL,
			LABEL_Text,      msg_bigabout,
		TAG_DONE),
		LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
TAG_DONE);

if (!object[OID_PAGE1])
	{
	IDOS->Printf("Cannot create 1st Clicktab page\n");
	return -1;
	}

/*
**	This is the second Page layout
**	All the Visual Options ends
**	there.
*/
object[OID_PAGE2] = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",

//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
//CHILD_WeightedWidth, 15,

	LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,   LAYOUT_ORIENT_VERT,

			LAYOUT_AddChild,gadget[GID_VIDEO] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray,VideoModes ,
				GA_ID, GID_VIDEO,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected,videoptr,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text,msg_videomode, 
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_YUV] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray,YUVModes ,
				GA_ID, GID_YUV,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected,yuvptr,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_yuv, 
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_FILTER] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray,FilterModes ,
				CHOOSER_MaxLabels,15,
				CHOOSER_Selected,filterptr,
				GA_ID, GID_FILTER,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_filter, 
			TAG_DONE),

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild,gadget[GID_FSKIP] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
					CHOOSER_LabelArray,FrameSkip ,
					CHOOSER_Selected,fskipptr,
					GA_ID, GID_FSKIP,
					GA_RelVerify,       TRUE,
				TAG_DONE),
					CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
						LABEL_Text, msg_fskip, 
					TAG_DONE),
				LAYOUT_AddChild,gadget[GID_FSKIPSET] = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_RelVerify,TRUE,
					GA_ID,GID_FSKIPSET,
					GA_Disabled,(BOOL) !(fskipptr==1),
					INTEGER_Maximum,9,
					INTEGER_Number,fskipvalue,
					INTEGER_Minimum,0,
					INTEGER_Arrows,TRUE,
INTEGER_MinVisible, 3,
INTEGER_MaxChars,   1,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
			TAG_DONE),
			CHILD_WeightedHeight, 0,

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild,gadget[GID_THROTTLE] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_RelVerify,       TRUE,
					GA_Selected,(BOOL )throttleptr,
					GA_ID,GID_THROTTLE,
					GA_Text,msg_throttle,
CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_THROTTLESET] = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_RelVerify,TRUE,
					GA_ID,GID_THROTTLESET,
					GA_Disabled,(BOOL) !throttleptr,
					INTEGER_Maximum,1000,
					INTEGER_Number,throttlevalue,
					INTEGER_Minimum,5,
					INTEGER_Arrows,TRUE,
INTEGER_MinVisible, 5,
INTEGER_MaxChars,   4,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
			TAG_DONE),
			CHILD_WeightedHeight, 0,

			LAYOUT_AddChild,gadget[GID_IFBLEND] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray, IFBlending,
				GA_ID, GID_IFBLEND,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected,ifbptr,
			TAG_DONE),
				CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
					LABEL_Text, msg_ifb, 
				TAG_DONE),
			LAYOUT_AddChild,gadget[GID_PWINAC] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_ID,GID_PWINAC,
				GA_Selected,(BOOL *)pwiptr,
				GA_Text,msg_pwi,
//CHECKBOX_TextPlace, PLACETEXT_LEFT,
			TAG_DONE),
			LAYOUT_AddChild,    gadget[GID_SPEED] = IIntuition->NewObject(RadioButtonClass, NULL, //"radiobutton.gadget",
				GA_Text,        SpeedLabel,
				GA_RelVerify,	TRUE,
				RADIOBUTTON_Selected,sesptr,
				RADIOBUTTON_LabelPlace,PLACETEXT_RIGHT,
				GA_ID,GID_SPEED,
		TAG_DONE),

	TAG_DONE),
	CHILD_WeightedHeight, 0,

//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
//CHILD_WeightedWidth, 15,

	TAG_DONE);

if (!object[OID_PAGE2])
	{
	IDOS->Printf("Cannot create 2nd Clicktab page\n");
	return -1;
	}

/*
**	This is the third Page layout
**	Where all Gaming Options
**	are about to appear.
*/
object[OID_PAGE3] = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         //LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,

//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
//CHILD_WeightedWidth, 15,

	LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,   LAYOUT_ORIENT_VERT,

			LAYOUT_AddChild,gadget[GID_EXE] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
				GA_ID, GID_EXE,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly,TRUE,
				GETFILE_FullFile,exefile,
				GETFILE_TitleText,msg_exeASL,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_exe, 
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_GAME] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
				GA_ID, GID_GAME,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly,TRUE,
				GETFILE_FullFile,gamefile,
				GETFILE_TitleText,msg_gameASL,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_gamefile, 
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_BIOS] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
				GA_ID, GID_BIOS,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly,TRUE,
				GETFILE_FullFile,biosfile,
				GETFILE_TitleText,msg_biosASL,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_biosfile, 
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_FLASH] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray,FlashSize ,
				CHOOSER_Selected,fsizeptr,
				GA_ID, GID_FLASH,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_flash, 
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_SAVE] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray,Save,
				CHOOSER_Selected,savetypeptr,
				GA_ID, GID_SAVE,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_savetype, 
			TAG_DONE),

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild,gadget[GID_IPS] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_RelVerify,       TRUE,
					GA_ID,GID_IPS,
					GA_Selected,(BOOL )ipsptr,
					GA_Text,msg_ips,
CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
				LAYOUT_AddChild,gadget[GID_IPSFILE] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
					GA_Disabled,(BOOL )!ipsptr,
					GA_RelVerify,TRUE,
					GA_ID,GID_IPSFILE,
					GETFILE_ReadOnly,TRUE,
					GETFILE_FullFile,ipsfile,
					GETFILE_TitleText,msg_ipsASL,
				TAG_DONE),
			TAG_DONE),
			CHILD_WeightedHeight, 0,
			//CHILD_WeightedHeight, 0,

			LAYOUT_AddChild,gadget[GID_MMX] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_ID,GID_MMX,
				GA_Text,msg_mmx,
				GA_Selected,(BOOL *)mmxptr,
//CHECKBOX_TextPlace, PLACETEXT_LEFT,
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_RTC] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_ID,GID_RTC,
				GA_Selected,(BOOL *)rtcptr,
				GA_Text,msg_rtc,
//CHECKBOX_TextPlace, PLACETEXT_LEFT,
		TAG_DONE),

	TAG_DONE),
	CHILD_WeightedHeight, 0,

//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
//CHILD_WeightedWidth, 15,

TAG_DONE);

if (!object[OID_PAGE3])
	{
	IDOS->Printf("Cannot create 3rd Clicktab page\n");
	return -1;
	}

/*
**	Last, but not least, the fourth page
**	Debug Options, going into the unknown
*/
object[OID_PAGE4] = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",

//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
//CHILD_WeightedWidth, 15,

	LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,   LAYOUT_ORIENT_VERT,
			LAYOUT_AddChild,gadget[GID_DEBUG] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_ID,GID_DEBUG,
				GA_Selected,(BOOL)debugptr,
				GA_Text,msg_endebug,
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_AGBPRINT] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_Disabled,(BOOL)!debugptr,
				GA_ID,GID_AGBPRINT,
				GA_Text,msg_agbprint,
				GA_Selected,(BOOL)agbptr,
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_VERBOSE] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray,Verbose,
				CHOOSER_Selected,verboseptr,
				GA_Disabled,(BOOL)!debugptr,
				GA_ID, GID_VERBOSE,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, msg_verbose, 
			TAG_DONE),

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild, gadget[GID_GDBPROTO] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
					CHOOSER_LabelArray,GdbProto ,
					CHOOSER_Selected,gdbptr,
					GA_Disabled,(BOOL)!debugptr,
					GA_ID, GID_GDBPROTO,
					GA_RelVerify,       TRUE,
				TAG_DONE),
				CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
					LABEL_Text, msg_gdbproto, 
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_GDBPORT] = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_RelVerify,TRUE,
					GA_Disabled,!((BOOL)debugptr && (gdbptr==1)),
					GA_ID,GID_GDBPORT,
					INTEGER_Maximum,65535,
					INTEGER_Number,port,
					INTEGER_Minimum,0,
					INTEGER_Arrows,TRUE,
INTEGER_MinVisible, 6,
INTEGER_MaxChars,   5,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
			TAG_DONE),
			//CHILD_WeightedHeight, 0,

		TAG_DONE),
		CHILD_WeightedHeight, 0,

//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
//CHILD_WeightedWidth, 15,

	TAG_DONE);

if (!object[OID_PAGE4])
	{
	IDOS->Printf("Cannot create 4th Clicktab page\n");
	return -1;
	}

/*
**	Now we made a grouped clicktab
**	with all the previous pages
*/
object[OID_CLICKTAB] = IIntuition->NewObject(ClickTabClass, NULL, //"clicktab.gadget",
		GA_Text,            Pages,
		CLICKTAB_Current,   0,
		CLICKTAB_LabelTruncate,TRUE,
		CLICKTAB_PageGroup, IIntuition->NewObject(NULL, "page.gadget",
			PAGE_Add,       object[OID_PAGE1],
			PAGE_Add,       object[OID_PAGE2],
			PAGE_Add,       object[OID_PAGE3],
			PAGE_Add,       object[OID_PAGE4],
		TAG_DONE),
	TAG_DONE);

if (!object[OID_CLICKTAB])
	{
	IDOS->Printf("Cannot create Clicktab object\n");
	return -1;
	}

/*
**	And finally the window Layout
**	with the last horizontal layout
**	storing the most useful options ;).
*/
object[OID_MAIN] = IIntuition->NewObject(WindowClass, NULL, //"window.class",
		WA_Title,           "VisualBoy Advance GUI",
		WA_DragBar,         TRUE,
		WA_SmartRefresh,    TRUE,
		WA_CloseGadget,     TRUE,
		WA_DepthGadget,     TRUE,
WA_SizeGadget, TRUE,
		WA_CustomScreen,scr,
		WINDOW_IconifyGadget,TRUE,
		WINDOW_AppPort,VBAPort,
		WINDOW_Position, WPOS_CENTERSCREEN,
		WINDOW_ParentGroup, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
			LAYOUT_HorizAlignment,LALIGN_CENTER,
			LAYOUT_EvenSize,   FALSE,
			//LAYOUT_FixedVert,FALSE,
			//LAYOUT_FixedHoriz,FALSE,
			//LAYOUT_ShrinkWrap,TRUE,
			LAYOUT_AddChild, object[OID_CLICKTAB],
			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         //LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				LAYOUT_EvenSize,   TRUE,
				LAYOUT_AddChild,gadget[GID_SAVECONF]=IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_RelVerify,TRUE,
					GA_Text,msg_save,
					GA_ID,GID_SAVECONF,
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_PLAY]=IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_RelVerify,TRUE,
					GA_Text,msg_launch,
					GA_ID,GID_PLAY,
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_QUIT]=IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_RelVerify,TRUE,
					GA_Text,msg_exit,
					GA_ID,GID_QUIT,
				TAG_DONE),
			TAG_DONE),
			CHILD_WeightedHeight, 0,
		TAG_DONE),
	TAG_DONE);

if (!object[OID_MAIN])
	{
	IDOS->Printf("Cannot create Window object\n");
	return -1;
	}

/*
**	This requester will warn the user if he wants to play no game, or without emulator ;)
*/
object[OID_REQ] = IIntuition->NewObject(RequesterClass, NULL, //"requester.class",
	REQ_Type,       REQTYPE_INFO,
	REQ_TitleText,  msg_warning,
	REQ_BodyText,   msg_warningtxt,
	REQ_Image,      REQIMAGE_WARNING,
	REQ_GadgetText, msg_ok,
	TAG_DONE);

if (!object[OID_REQ])
	{
	IDOS->Printf("Cannot create Requester object\n");
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
			while( (result=IIntuition->IDoMethod(object[OID_MAIN], WM_HANDLEINPUT, &code)) != WMHI_LASTMSG )
				{
				switch (result & WMHI_CLASSMASK)
					{
					case WMHI_CLOSEWINDOW:
						window[WID_MAIN] = NULL;
						done = TRUE;
						break;
					case WMHI_ICONIFY:
						res = IIntuition->IDoMethod(object[OID_MAIN], WM_ICONIFY);
						window[WID_MAIN] = NULL;
						break;
					case WMHI_UNICONIFY:
						window[WID_MAIN] =(struct Window *)IIntuition->IDoMethod(object[OID_MAIN], WM_OPEN, NULL);
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
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,videoptr,TAG_DONE),
								                "Video Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,yuvptr,TAG_DONE),
								                "YUV Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,filterptr,TAG_DONE),
								                "Filter Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,fskipptr,TAG_DONE),
								                "FrameSkip Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,fskipvalue,TAG_DONE),
								                "FrameSkip Value");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,throttleptr,TAG_DONE),
								                "Throttle Mode");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,throttlevalue,TAG_DONE),
								                "Throttle Value");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,ifbptr,TAG_DONE),
								                "InterFrame Blending");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,pwiptr,TAG_DONE),
								                "Pause When Inactive");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,sesptr,TAG_DONE),
								                "Show Emulation Speed");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,fsizeptr,TAG_DONE),
								                "Flash size");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,savetypeptr,TAG_DONE),
								                "Save Type");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,rtcptr,TAG_DONE),
								                "RTC");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,mmxptr,TAG_DONE),
								                "MMX");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,ipsptr,TAG_DONE),
								                "IPS");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsString(NULL,NULL,ALPOSTR_AllocSetString,ipsfile,TAG_DONE),
								                "ips File");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsString(NULL,NULL,ALPOSTR_AllocSetString,gamefile,TAG_DONE),
								                "Game File");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsString(NULL,NULL,ALPOSTR_AllocSetString,biosfile,TAG_DONE),
								                "Bios File");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsString(NULL,NULL,ALPOSTR_AllocSetString,exefile,TAG_DONE),
								                "Executable");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,port,TAG_DONE),
								                "Port");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,agbptr,TAG_DONE),
								                "AGB");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,verboseptr,TAG_DONE),
								                "Verbose");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,debugptr,TAG_DONE),
								                "Debug");
								IPrefsObjects->DictSetObjectForKey(VBAPrefs,
								                IPrefsObjects->PrefsNumber(NULL,NULL,ALPONUM_AllocSetLong,gdbptr,TAG_DONE),
								                "GDB");
								break;
							case GID_PLAY:
								//if (strcmp(gamefile,"")&&strcmp(exefile,""))
if (gamefile[0]!='\0'  &&  exefile[0]!='\0')
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
