/*
GUI.C
*/


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/application.h>
#include <proto/icon.h>
#include <proto/graphics.h>
//#include <proto/locale.h>
#include <proto/listbrowser.h>
#include <proto/clicktab.h>
#include <proto/chooser.h>
#include <proto/layout.h>

#include <workbench/icon.h>
#include <libraries/keymap.h> // RAWKEY_#? codes
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
#include <gadgets/slider.h>
#include <images/label.h>
#include <images/bitmap.h>

#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
extern struct LocaleInfo li;
#include "vbagui_strings.h"

#include "includes.h"
#include "vbagui_rev.h"
#include "debug.h"


extern void parse_commandline(void);
extern int32 GetRoms(STRPTR romsdir);//, struct List *);
extern void openGamepadWin(void);

//extern struct ExecIFace *IExec;
//extern struct DOSIFace *IDOS;
extern struct ApplicationIFace *IApplication;
extern struct PrefsObjectsIFace *IPrefsObjects;
extern struct IconIFace *IIcon;
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
extern struct ClickTabIFace *IClickTab;
extern struct LayoutIFace *ILayout;
extern struct ChooserIFace *IChooser;

extern struct MsgPort *VBAPort;
extern struct Screen *scr;

/*
**	Application & preferences data.
*/
extern ULONG appID;
extern PrefsObject *VBAPrefs;

extern int videoptr,yuvptr,filterptr;
extern int fskipptr,fskipvalue;
extern int throttleptr,throttlevalue;
extern int ifbptr,fsizeptr,pwiptr;
extern int rtcptr,mmxptr,ipsptr;
extern int sesptr,savetypeptr;

extern int port,agbptr,verboseptr,debugptr,gdbptr;

extern STRPTR gamefile;
extern STRPTR biosfile;
extern STRPTR exefile;
extern STRPTR ipsfile;

extern struct List *romlist;


STRPTR VideoModes[6];
STRPTR YUVModes[7];

struct List FilterModes_list, FrameSkip_list, SpeedLabel_list, IFBlending_list,
            GdbProto_list, Verbose_list,
            FlashSize_list, Save_list;

struct Window *window[WID_LAST];
Object *gadget[GID_LAST];
Object *object[OID_LAST];
struct List labels; // clicktab/page labels
struct ColumnInfo *columninfo;
char romfile_sel[MAX_FULLFILEPATH];
//struct Hook *fskip_lvlHook;
//char fskip_hk[6] = ""; // used in fskip_lvlFunc()
uint32 res_prev; // avoid "reload" already selected ROM

/*
**	Game Options Parser.
*/
BOOL gdbport = TRUE;


void append_tab(uint16 num, STRPTR label)
{
	struct Node *node = IClickTab->AllocClickTabNode(TNA_Number,num, TNA_Text,label, TAG_DONE);
	if(node) {
		IExec->AddTail(&labels, node);
	}
}

/*BOOL make_chooser_list(BOOL mode, struct List *list, char **strings)
{
	struct Node *node;

	if(mode == NEW_LIST) { IExec->NewList(list); }

	while(*strings)
	{
		//if(**strings == '_')
		// node = IChooser->AllocChooserNode(CNA_Separator,TRUE, TAG_DONE);
		//else
			node = IChooser->AllocChooserNode(CNA_Text,*strings, TAG_DONE);

		if(node) { IExec->AddTail(list, node); }
		else { return FALSE; }

		strings++;
	}

	return TRUE;
}*/

BOOL make_chooser_list2(BOOL mode, struct List *list, int32 str_num, int32 index)
{
	struct Node *node;
	int32 j;

	if(mode == NEW_LIST) { IExec->NewList(list); }
	for(j=0; j<index; j++)
	{
		node = IChooser->AllocChooserNode(CNA_CopyText, TRUE,
		                                  CNA_Text, GetString(&li, str_num+j),
		                                 TAG_DONE);
		if(node) { IExec->AddTail(list, node); }
		else { return FALSE; }
	}

	return TRUE;
}

void free_chooser_list(struct List *list)
{
	struct Node *node;

	while( (node=IExec->RemTail(list)) ) {
		IChooser->FreeChooserNode(node);
	}
}

void updateList(void)
{
//	STRPTR romdrawer = NULL;
	uint32 res_tot;
DBUG("updateList()\n",NULL);
	// Detach the listbrowser list first
	IIntuition->SetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Labels,NULL, TAG_END);

	// Re-generate romlist with new rom drawer
//	IIntuition->GetAttr(GETFILE_Drawer, gadget[GID_GAME], (uint32*)&romdrawer);
//	FreeString(&gui->myTT.romsdrawer);
//	gui->myTT.romsdrawer = DupStr(romdrawer, -1);
	res_tot = GetRoms( (STRPTR)gamefile );
	/*if(res_tot == 0) { // "clear" savestates chooser list
		struct Node *node = IChooser->AllocChooserNode(CNA_Text,GetString(&li,MSG_GUI_SAVESTATES_NO), TAG_DONE);
		// Detach chooser list
		IIntuition->SetAttrs(OBJ(OID_SAVESTATES), CHOOSER_Labels,NULL, TAG_DONE);
		// Remove previous savestates chooser list
		free_chooserlist_nodes(gui->savestates_list);
	// Add NO string (MSG_GUI_SAVESTATES_NO) at top/head of the list..
		IExec->AddHead(gui->savestates_list, node);
		//..and re-attach chooser list
		IIntuition->RefreshSetGadgetAttrs(GAD(OID_SAVESTATES), gui->win[WID_MAIN], NULL,
		                                  CHOOSER_Labels,gui->savestates_list, GA_Disabled,FALSE, TAG_DONE);
	}*/
//DBUG("  romlist = 0x%08lx\n",romlist);
	// Re-attach the listbrowser
	IIntuition->SetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_SortColumn,COL_ROM,
	                     LISTBROWSER_Labels,romlist, TAG_DONE);
	//IIntuition->RefreshSetGadgetAttrs((struct Gadget *)gadget[GID_TOTALROMS], window[WID_MAIN], NULL,
	//                                  BUTTON_VarArgs,&res_tot, TAG_DONE);
	IIntuition->SetAttrs(gadget[GID_TOTALROMS], BUTTON_VarArgs,&res_tot, TAG_DONE);
	ILayout->RefreshPageGadget((struct Gadget *)gadget[GID_GAMELIST], object[OID_PAGE3], window[WID_MAIN], NULL);
}

uint32 selectListEntry(struct Window *pw, uint32 res_val)
{
	IIntuition->SetAttrs(gadget[GID_LISTBROWSER],
	                     LISTBROWSER_Selected,res_val,
	                     LISTBROWSER_MakeVisible,res_val, TAG_DONE);
	//IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_LISTBROWSER], pw, NULL);
	ILayout->RefreshPageGadget((struct Gadget *)gadget[GID_LISTBROWSER], object[OID_PAGE3], pw, NULL);

	return res_val;
}

uint32 selectListEntryNode(struct Window *pw, struct Node *n)
{
	uint32 res_val;
	IIntuition->SetAttrs(gadget[GID_LISTBROWSER],
                      LISTBROWSER_SelectedNode,n,
                      LISTBROWSER_MakeNodeVisible,n, TAG_DONE);
	//IIntuition->RefreshGadgets(GAD(OID_LISTBROWSER), pw, NULL);
	ILayout->RefreshPageGadget((struct Gadget *)gadget[GID_LISTBROWSER], object[OID_PAGE3], pw, NULL);
	IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Selected,&res_val, TAG_DONE);

	return res_val;
}

void setROMFile(void)
{
	struct Node *n;
	STRPTR res_s, res_e;
DBUG("setROMFile()\n",NULL);
	IIntuition->GetAttr(LISTBROWSER_SelectedNode, gadget[GID_LISTBROWSER], (uint32*)&n);
	IListBrowser->GetListBrowserNodeAttrs(n, LBNA_Column,COL_ROM, LBNCA_Text,&res_s, TAG_DONE);
	IListBrowser->GetListBrowserNodeAttrs(n, LBNA_Column,COL_FMT, LBNCA_Text,&res_e, TAG_DONE);
//DBUG("    node=0x%08lx -> res_s='%s' (res_e='%s')\n",res,res_s,res_e);
	IUtility->Strlcpy(romfile_sel, gamefile, MAX_FULLFILEPATH);
	IDOS->AddPart(romfile_sel, res_s, MAX_FULLFILEPATH);
	IUtility->SNPrintf(romfile_sel, MAX_FULLFILEPATH, "%s.%s",romfile_sel,res_e);
DBUG("    '%s'\n",romfile_sel);
}


uint32 findEntryLB(char c_keyb, struct Node *next_n1)
{
	struct Node *n1 = IExec->GetHead(romlist);
	STRPTR str;
	char c_node1;
	uint32 res = 0;
DBUG("findEntryLB()\n",NULL);
	for(; n1!=NULL; n1=IExec->GetSucc(n1) ) {
		IListBrowser->GetListBrowserNodeAttrs(n1, LBNA_Column,COL_ROM, LBNCA_Text,&str, TAG_DONE);
		c_node1 = IUtility->ToUpper(*str); // uppercased (romfile 1st letter)
		if(c_node1==c_keyb  &&  n1!=next_n1) {
			res = selectListEntryNode(window[WID_MAIN], n1);
			n1 = NULL;
		}
	}

	return res;
}

// FramSkip slider level formatting
/*CONST_STRPTR fskip_lvlFunc(struct Hook *hook, APTR slider, struct TagItem *tags)
{
	uint32 val = IUtility->GetTagData(SLIDER_Level, 0, tags);
DBUG("fskip_lvlFunc() 0x%08lx (0x%08lx)\n",slider,val);
	if(val == -1) { return GetString(&li, MSG_GUI_FSKIP_AUTO); }
	if(val == 10) { return GetString(&li, MSG_GUI_FSKIP_OFF); }

	IUtility->SNPrintf(fskip_hk, sizeof(fskip_hk), "%ld",val);
	return (CONST_STRPTR)fskip_hk;
}*/


/*
**	This is the function where we
**	create all the layout objects.
*/
int create_objects(void)
{
	struct DiskObject *iconify;
	// Arrays strings on choosers, 'NULL' is replaced by translation (last NULL is like a "EOF")
	STRPTR VideoModes[] = { "Video 1x", "Video 2x", "Video 3x", "Video 4x", NULL, NULL };
	STRPTR YUVModes[]   = { NULL, "YV12", "UYVY", "YVYU", "YUY2", "IYUV", NULL };
	STRPTR SpeedLabel[] = { NULL, NULL, NULL, NULL };
	WORD max_w_ext = IGraphics->TextLength(&scr->RastPort, GetString(&li,MSG_GUI_TITLE_COL_FMT), IUtility->Strlen(GetString(&li,MSG_GUI_TITLE_COL_FMT))+1); // max rom extension pixel width

	// Reset icon X/Y positions so it iconifies properly on Workbench
	if( (iconify=IIcon->GetIconTags("PROGDIR:vbagui", ICONGETA_FailIfUnavailable,FALSE, TAG_END)) ) {
		iconify->do_CurrentX = NO_ICON_POSITION;
		iconify->do_CurrentY = NO_ICON_POSITION;
	}

	columninfo = IListBrowser->AllocLBColumnInfo(LAST_COL,
	                            LBCIA_Column,COL_ROM, LBCIA_Title, GetString(&li, MSG_GUI_TITLE_COL_ROM), //"Game",
	                                                  LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                  LBCIA_Weight, 100,
	                            LBCIA_Column,COL_FMT, LBCIA_Title, GetString(&li, MSG_GUI_TITLE_COL_FMT), //"Format",
	                                                  LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                  LBCIA_Width, max_w_ext,
	                           TAG_DONE);

	//fskip_lvlHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry,fskip_lvlFunc, TAG_END); // Frameski

	// "localization" of gadgets
	IExec->NewList(&labels);
	append_tab( 0, (STRPTR)GetString(&li,msg_about)  );
	append_tab( 1, (STRPTR)GetString(&li,msg_visual) );
	append_tab( 2, (STRPTR)GetString(&li,msg_game)   );
	append_tab( 3, (STRPTR)GetString(&li,msg_debug)  );

	// Visual options (arrays)
	VideoModes[4] = (STRPTR)GetString(&li, msg_fullscreen);
	YUVModes[0] = (STRPTR)GetString(&li, msg_yuvnone);
	//ToDo: make_radiobutton_list2(NEW_LIST, &SpeedLabel_list, msg_show1, 3);
	SpeedLabel[0] = (STRPTR)GetString(&li, msg_show1);
	SpeedLabel[1] = (STRPTR)GetString(&li, msg_show2);
	SpeedLabel[2] = (STRPTR)GetString(&li, msg_show3);
	// Visual options (lists)
	make_chooser_list2(NEW_LIST, &FilterModes_list, msg_filter1, 14);
	make_chooser_list2(NEW_LIST, &FrameSkip_list, msg_fskauto, 3);
	make_chooser_list2(NEW_LIST, &IFBlending_list, msg_ifb1, 3);
	// Debug options
	make_chooser_list2(NEW_LIST, &GdbProto_list, msg_gdbproto1, 3);
	make_chooser_list2(NEW_LIST, &Verbose_list, msg_verbose1, 10);
	//Game Options
	make_chooser_list2(NEW_LIST, &FlashSize_list, msg_flash1, 2);
	make_chooser_list2(NEW_LIST, &Save_list, msg_savetype1, 6);

	object[OID_ABOUT] = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
		BITMAP_SourceFile,  "PROGDIR:vba_banner.png",
		BITMAP_Transparent, TRUE,
		BITMAP_Masking,     TRUE,
		BITMAP_Screen,      scr,
		TAG_DONE);

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
			LABEL_Text,      GetString(&li, msg_bigabout),
		TAG_DONE),
		LAYOUT_AddChild, gadget[GID_DONATE_BTN] = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
			GA_ID,        GID_DONATE_BTN,
			GA_RelVerify, TRUE,
			GA_Text,      (STRPTR)GetString(&li, MSG_ABOUT_DONATE),
			//BUTTON_Transparent, TRUE,
			//BUTTON_BevelStyle,  BVS_NONE,
			/*BUTTON_RenderImage, object[OID_DONATE_IMG] = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
				BITMAP_SourceFile, DONATE_IMG
				BITMAP_Screen,     scr,
				BITMAP_Masking,    TRUE,
			TAG_DONE),*/
		TAG_DONE),
		CHILD_WeightedWidth,  0,
		CHILD_WeightedHeight, 0,

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

	LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,   LAYOUT_ORIENT_VERT,

			LAYOUT_AddChild,gadget[GID_VIDEO] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray, VideoModes,
				GA_ID, GID_VIDEO,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected, videoptr,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_videomode),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_YUV] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				CHOOSER_LabelArray, YUVModes,
				GA_ID, GID_YUV,
				GA_RelVerify,       TRUE,
				GA_Disabled, TRUE,
				CHOOSER_Selected, yuvptr,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_yuv),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_FILTER] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				//CHOOSER_LabelArray, FilterModes,
				CHOOSER_Labels, &FilterModes_list,
				CHOOSER_MaxLabels, 15,
				CHOOSER_Selected, filterptr,
				GA_ID, GID_FILTER,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_filter),
			TAG_DONE),

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild,gadget[GID_FSKIP] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
					//CHOOSER_LabelArray, FrameSkip,
				CHOOSER_Labels, &FrameSkip_list,
					CHOOSER_Selected, fskipptr,
					GA_ID, GID_FSKIP,
					GA_RelVerify,       TRUE,
				TAG_DONE),
				//CHILD_WeightedWidth, 0,
				CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
					LABEL_Text, GetString(&li, msg_fskip),
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_FSKIPSET] = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_RelVerify, TRUE,
					GA_ID, GID_FSKIPSET,
					GA_Disabled, (BOOL) !(fskipptr==1),
					INTEGER_Maximum, 9,
					INTEGER_Number, fskipvalue,
					INTEGER_Minimum, 0,
					INTEGER_Arrows, TRUE,
					INTEGER_MinVisible, 3,
					INTEGER_MaxChars,   1,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
			TAG_DONE),
//CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
//	LABEL_Text, GetString(&li, msg_fskip),
//TAG_DONE),
			CHILD_WeightedHeight, 0,

			/*LAYOUT_AddChild, gadget[GID_FSKIP] = IIntuition->NewObject(SliderClass, NULL,
				GA_ID,         GID_FSKIP,
				GA_RelVerify,  TRUE,
				SLIDER_Level, fskipvalue+1,
				SLIDER_Min,  -1,
				SLIDER_Max, 10,
				SLIDER_Ticks,      6+1,
				SLIDER_ShortTicks, TRUE,
				SLIDER_Orientation, SORIENT_HORIZ,
				SLIDER_LevelMaxLen, sizeof(fskip_hk),
				//SLIDER_LevelDomain, GetString(&li, MSG_GUI_FSKIP_AUTO),
				SLIDER_LevelHook,   fskip_lvlHook,
				SLIDER_LevelFormat, "%s",
				//SLIDER_LevelPlace,  PLACETEXT_RIGHT,
				SLIDER_LevelJustify, SLJ_LEFT,
			TAG_DONE),
			//CHILD_WeightedWidth, 0,
			CHILD_WeightedHeight, 0,
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_fskip),
			TAG_DONE),*/

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild,gadget[GID_THROTTLE] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_RelVerify,       TRUE,
					GA_Selected, (BOOL) throttleptr,
					GA_ID, GID_THROTTLE,
					GA_Text, GetString(&li, msg_throttle),
					CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),
				//CHILD_WeightedWidth, 0,
				LAYOUT_AddChild,gadget[GID_THROTTLESET] = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_RelVerify, TRUE,
					GA_ID, GID_THROTTLESET,
					GA_Disabled, (BOOL) !throttleptr,
					INTEGER_Maximum, 1000,
					INTEGER_Number, throttlevalue,
					INTEGER_Minimum, 5,
					INTEGER_Arrows, TRUE,
					INTEGER_MinVisible, 5,
					INTEGER_MaxChars,   4,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
			TAG_DONE),
			CHILD_WeightedHeight, 0,

			LAYOUT_AddChild,gadget[GID_IFBLEND] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				//CHOOSER_LabelArray, IFBlending,
				CHOOSER_Labels, &IFBlending_list,
				GA_ID, GID_IFBLEND,
				GA_RelVerify,       TRUE,
				CHOOSER_Selected, ifbptr,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_ifb),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_PWINAC] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_ID, GID_PWINAC,
				GA_Selected, (BOOL *)pwiptr,
				GA_Text, GetString(&li, msg_pwi),
				//CHECKBOX_TextPlace, PLACETEXT_LEFT,
			TAG_DONE),
			LAYOUT_AddChild, gadget[GID_SPEED] = IIntuition->NewObject(RadioButtonClass, NULL, //"radiobutton.gadget",
				GA_Text,        SpeedLabel,
				GA_RelVerify, TRUE,
				RADIOBUTTON_Selected, sesptr,
				RADIOBUTTON_LabelPlace, PLACETEXT_RIGHT,
				GA_ID,GID_SPEED,
		TAG_DONE),

	TAG_DONE),
	CHILD_WeightedHeight, 0,

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

	LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,   LAYOUT_ORIENT_VERT,

			LAYOUT_AddChild,gadget[GID_EXE] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
				GA_ID, GID_EXE,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly, TRUE,
				GETFILE_FullFile, exefile,
				GETFILE_TitleText, GetString(&li, msg_exeASL),
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_exe),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_GAME] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
				GA_ID, GID_GAME,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly, TRUE,
				GETFILE_FullFileExpand, TRUE,
				GETFILE_DrawersOnly,    TRUE,
				GETFILE_Drawer,         gamefile,
				//GETFILE_FullFile, gamefile,
				GETFILE_TitleText, GetString(&li, MSG_ROMSDRAWER_ASL),//msg_gameASL),
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, MSG_GUI_ROMSDRAWER),//msg_gamefile),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_BIOS] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
				GA_ID, GID_BIOS,
				GA_RelVerify,       TRUE,
				GETFILE_ReadOnly, TRUE,
				GETFILE_FullFile, biosfile,
				GETFILE_TitleText, GetString(&li, msg_biosASL),
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_biosfile),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_FLASH] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				//CHOOSER_LabelArray, FlashSize,
				CHOOSER_Labels, &FlashSize_list,
				CHOOSER_Selected, fsizeptr,
				GA_ID, GID_FLASH,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_flash),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_SAVE] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				//CHOOSER_LabelArray, Save,
				CHOOSER_Labels, &Save_list,
				CHOOSER_Selected, savetypeptr,
				GA_ID, GID_SAVE,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_savetype),
			TAG_DONE),

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild,gadget[GID_IPS] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_RelVerify,       TRUE,
					GA_ID, GID_IPS,
					GA_Selected, (BOOL )ipsptr,
					GA_Text, GetString(&li, msg_ips),
					CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
				LAYOUT_AddChild,gadget[GID_IPSFILE] = IIntuition->NewObject(GetFileClass, NULL, //"getfile.gadget",
					GA_Disabled, (BOOL ) !ipsptr,
					GA_RelVerify, TRUE,
					GA_ID, GID_IPSFILE,
					GETFILE_ReadOnly, TRUE,
					GETFILE_FullFile, ipsfile,
					GETFILE_TitleText, GetString(&li, msg_ipsASL),
				TAG_DONE),
			TAG_DONE),
			CHILD_WeightedHeight, 0,
			//CHILD_WeightedHeight, 0,

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
			//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
			//LAYOUT_BevelStyle, BVS_GROUP,
				/*LAYOUT_AddChild,gadget[GID_MMX] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_RelVerify,       TRUE,
					GA_ID, GID_MMX,
					GA_Text, GetString(&li, msg_mmx),
					GA_Selected, (BOOL *)mmxptr,
//CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),*/
				LAYOUT_AddChild,gadget[GID_RTC] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_RelVerify,       TRUE,
					GA_ID, GID_RTC,
					GA_Selected, (BOOL *)rtcptr,
					GA_Text, GetString(&li, msg_rtc),
					//CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),
				//CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_ID,        GID_GAMEPAD_BTN,
					GA_RelVerify, TRUE,
					GA_Text,      GetString(&li, MSG_GUI_GAMEPAD_BTN),//"Gamepad buttons..."
				TAG_DONE),
			TAG_DONE),
			CHILD_WeightedHeight, 0,

			LAYOUT_AddChild, gadget[GID_GAMELIST] = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				LAYOUT_BevelStyle, BVS_SBAR_VERT,
				LAYOUT_Label, GetString(&li, MSG_GUI_GAMESLIST), //"Games List",
				LAYOUT_AddChild, gadget[GID_LISTBROWSER] = IIntuition->NewObject(ListBrowserClass, NULL, //"listbrowser.gadget",
					GA_ID,        GID_LISTBROWSER,
					GA_RelVerify, TRUE,
					//LISTBROWSER_SortColumn,     COL_ROM,
					//LISTBROWSER_AutoFit,        TRUE,
					LISTBROWSER_Labels,         NULL,
					LISTBROWSER_ColumnInfo,     columninfo,
					LISTBROWSER_ColumnTitles,   TRUE,
					LISTBROWSER_ShowSelected,   TRUE,
					LISTBROWSER_Selected,       -1,
					LISTBROWSER_MinVisible,     5,
					//LISTBROWSER_Striping,       LBS_ROWS,
					LISTBROWSER_TitleClickable, TRUE,
					//LISTBROWSER_HorizontalProp, TRUE,
				TAG_DONE), // END of ROM LIST 
				LAYOUT_AddChild, gadget[GID_TOTALROMS] = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					//GA_ID,         GID_TOTALROMS,
					//GA_RelVerify,  TRUE,
					GA_ReadOnly,   TRUE,
					GA_Underscore, 0,
					GA_Text,       GetString(&li, MSG_GUI_TOTALROMS),
					BUTTON_Justification, BCJ_RIGHT,
					BUTTON_BevelStyle,    BVS_NONE,
					BUTTON_Transparent,   TRUE,
				TAG_DONE),
				//CHILD_WeightedWidth,  0,
				CHILD_WeightedHeight, 0,
			TAG_DONE),

	TAG_DONE),
	//CHILD_WeightedHeight, 0,

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

	LAYOUT_AddChild, gadget[GID_DEBUG_PAGE] = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation,   LAYOUT_ORIENT_VERT,
			LAYOUT_AddChild,gadget[GID_DEBUG] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_ID, GID_DEBUG,
				GA_Selected, (BOOL)debugptr,
				GA_Text, GetString(&li, msg_endebug),
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_AGBPRINT] = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
				GA_RelVerify,       TRUE,
				GA_Disabled, (BOOL) !debugptr,
				GA_ID, GID_AGBPRINT,
				GA_Text, GetString(&li, msg_agbprint),
				GA_Selected, (BOOL)agbptr,
			TAG_DONE),
			LAYOUT_AddChild,gadget[GID_VERBOSE] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				//CHOOSER_LabelArray, Verbose,
				CHOOSER_Labels, &Verbose_list,
				CHOOSER_Selected, GetString(&li, verboseptr),
				GA_Disabled, (BOOL) !debugptr,
				GA_ID, GID_VERBOSE,
				GA_RelVerify,       TRUE,
			TAG_DONE),
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
				LABEL_Text, GetString(&li, msg_verbose),
			TAG_DONE),

			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				//LAYOUT_BevelStyle, BVS_GROUP,
				LAYOUT_AddChild, gadget[GID_GDBPROTO] = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
					//CHOOSER_LabelArray, GdbProto,
					CHOOSER_Labels, &GdbProto_list,
					CHOOSER_Selected, gdbptr,
					GA_Disabled, (BOOL) !debugptr,
					GA_ID, GID_GDBPROTO,
					GA_RelVerify,       TRUE,
				TAG_DONE),
				CHILD_Label, IIntuition->NewObject(LabelClass, NULL, //"label.image",
					LABEL_Text, GetString(&li, msg_gdbproto),
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_GDBPORT] = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_RelVerify, TRUE,
					GA_Disabled, !((BOOL)debugptr && (gdbptr==1)),
					GA_ID, GID_GDBPORT,
					INTEGER_Maximum, 65535,
					INTEGER_Number, port,
					INTEGER_Minimum, 0,
					INTEGER_Arrows, TRUE,
					INTEGER_MinVisible, 6,
					INTEGER_MaxChars,   5,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
			TAG_DONE),
			//CHILD_WeightedHeight, 0,

		TAG_DONE),
		CHILD_WeightedHeight, 0,

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
		//GA_Text,            Pages,
		CLICKTAB_Labels,    &labels,
		CLICKTAB_Current,   0,
		//CLICKTAB_LabelTruncate, TRUE,
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
		WA_ScreenTitle,  VERS" "DATE,
		WA_Title,        "VisualBoy Advance GUI",
		WA_DragBar,      TRUE,
		WA_SmartRefresh, TRUE,
		WA_CloseGadget,  TRUE,
		WA_DepthGadget,  TRUE,
		WA_SizeGadget,   TRUE,
		WA_CustomScreen, scr,
		WA_Activate,     TRUE,
		WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_RAWKEY,
		WINDOW_IconifyGadget, TRUE,
		WINDOW_Icon,          iconify,
		WINDOW_AppPort, VBAPort,
		WINDOW_Position, WPOS_CENTERSCREEN,
		WINDOW_ParentGroup, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
			LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
			LAYOUT_HorizAlignment, LALIGN_CENTER,
			//LAYOUT_EvenSize,   FALSE,
			LAYOUT_AddChild, object[OID_CLICKTAB],
			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				LAYOUT_EvenSize,   TRUE,
				LAYOUT_AddChild,gadget[GID_SAVECONF]=IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_RelVerify, TRUE,
					GA_Text, GetString(&li, msg_save),
					GA_ID, GID_SAVECONF,
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_PLAY]=IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_RelVerify, TRUE,
					GA_Text, GetString(&li, msg_launch),
					GA_ID, GID_PLAY,
				TAG_DONE),
				LAYOUT_AddChild,gadget[GID_QUIT]=IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_RelVerify, TRUE,
					GA_Text, GetString(&li, msg_exit),
					GA_ID, GID_QUIT,
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
		REQ_TitleText,  GetString(&li, msg_warning),
		REQ_BodyText,   NULL,//GetString(&li, msg_warningtxt),
		REQ_Image,      REQIMAGE_WARNING,
		REQ_GadgetText, GetString(&li, msg_ok),
		TAG_DONE);

	if (!object[OID_REQ])
		{
		IDOS->Printf("Cannot create Requester object\n");
		return -1;
		}

	updateList();

	return 0;
}


void destroy_objects(void)
{
	if (object[OID_MAIN])		IIntuition->DisposeObject(object[OID_MAIN]);
	if (object[OID_ABOUT])	IIntuition->DisposeObject(object[OID_ABOUT]);
	if (object[OID_REQ])		IIntuition->DisposeObject(object[OID_REQ]);
	//if (object[OID_DONATE_IMG])		IIntuition->DisposeObject(object[OID_DONATE_IMG]);

	IClickTab->FreeClickTabList(&labels);
	IListBrowser->FreeLBColumnInfo(columninfo);
	//IExec->FreeSysObject(ASOT_HOOK, fskip_lvlHook);

	free_chooser_list(&FilterModes_list);
	free_chooser_list(&FrameSkip_list);
	//ToDo: free_radiobutton_list(&SpeedLabel_list);
	free_chooser_list(&IFBlending_list);
	free_chooser_list(&GdbProto_list);
	free_chooser_list(&Verbose_list);
	free_chooser_list(&FlashSize_list);
	free_chooser_list(&Save_list);
}


void do_events(void)
{
	ULONG wait, signal, app = (1L << window[WID_MAIN]->UserPort->mp_SigBit),
	      done = FALSE,
	      res_totnode = 0, res_temp = 0,
	      res, result;
	UWORD code;
	STRPTR res_str = NULL;

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
						IIntuition->IDoMethod(object[OID_MAIN], WM_ICONIFY);
						window[WID_MAIN] = NULL;
						break;
					case WMHI_UNICONIFY:
						window[WID_MAIN] =(struct Window *)IIntuition->IDoMethod(object[OID_MAIN], WM_OPEN, NULL);
						if (window[WID_MAIN])
							{
							//IIntuition->GetAttr(WINDOW_SigMask, object[OID_MAIN], &signal);
							scr = window[WID_MAIN]->WScreen;
							IIntuition->ScreenToFront(scr);
							}
						else { done = TRUE; }
						break;

		case WMHI_VANILLAKEY:
		{
			struct Node *node1, *next_node1;
			STRPTR node_val, next_n_val;
			char char_keyb;
DBUG("[WMHI_VANILLAKEY] = 0x%lx (0x%lx)\n",code,result&WMHI_KEYMASK);
			if(code == 0x1b) // ESC
			{
				done = TRUE;
				break;
			}

			IIntuition->GetAttr(CLICKTAB_Current, object[OID_CLICKTAB], &res);
			if(res != 2) { break; } // NOT in GAME_SETTINGS tab/page

			if(code == 0x0d) // ENTER/RETURN
			{
				IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Selected,&res, TAG_DONE);
				if(res!=-1  &&  exefile[0]!='\0') {
					setROMFile();
					parse_commandline();
				}
				else {
					IIntuition->SetAttrs(object[OID_REQ], REQ_BodyText,GetString(&li,msg_warningtxt), TAG_DONE);
					IIntuition->IDoMethod(object[OID_REQ],RM_OPENREQ,NULL,window[WID_MAIN],NULL,TAG_DONE);
				}
				break;
			}

			char_keyb = IUtility->ToUpper(result & WMHI_KEYMASK); // uppercase'd (key pressed)
DBUG("[WMHI_VANILLAKEY] '%lc' (0x%lx)\n",char_keyb,char_keyb);
			IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_SelectedNode,&node1, TAG_DONE);
			if(node1 == NULL) {
				res_prev = findEntryLB(char_keyb, NULL);//next_node1);
				break;
			}
			IListBrowser->GetListBrowserNodeAttrs(node1, LBNA_Column,COL_ROM, LBNCA_Text,&node_val, TAG_DONE);
			next_node1 = IExec->GetSucc(node1);
			IListBrowser->GetListBrowserNodeAttrs(next_node1, LBNA_Column,COL_ROM, LBNCA_Text,&next_n_val, TAG_DONE);
DBUG("  Actual Node -> Next Node\n",NULL);
DBUG("   0x%08lx -> 0x%08lx\n",node1,next_node1);
DBUG("          '%lC' -> '%lC'\n",*node_val,*next_n_val);
			// SELECT IT: NEXT node starts with KEY pressed and NEXT node = ACTUAL node
			if( char_keyb==IUtility->ToUpper(*next_n_val)  &&  IUtility->ToUpper(*next_n_val)==IUtility->ToUpper(*node_val) )
			{
				res_prev = selectListEntryNode(window[WID_MAIN], next_node1);
			}
			// GO TO KEY PRESSED FIRST NODE: 1)ACTUAL node starts with KEY pressed and NEXT node != ACTUAL node
			// OR 2)pressed another KEY OR 3)reached end of listbrowser (next_node1=NULL)
			if( (char_keyb==IUtility->ToUpper(*node_val)  &&  IUtility->ToUpper(*next_n_val)!=IUtility->ToUpper(*node_val))
			   ||  char_keyb!=IUtility->ToUpper(*node_val)  ||  next_node1==NULL )
			{
				next_node1 = node1; // avoid refreshing/reloading single ROM filename entries
				res_prev = findEntryLB(char_keyb, next_node1);
			}

		}
		break;

					case WMHI_RAWKEY:
					{
						int32 sel_entry = -1; // -1: key pressed not valid
DBUG("[WMHI_RAWKEY] 0x%lx  window[WID_MAIN]=0x%08lx)\n",code,window[WID_MAIN]);
						IIntuition->GetAttr(CLICKTAB_Current, object[OID_CLICKTAB], &res);
						if(res != 2) { break; } // NOT in GAME_SETTINGS tab/page

						IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Selected,&res,
						                     LISTBROWSER_TotalNodes,&res_totnode, TAG_DONE);
DBUG("  sel=%ld  nodes=%ld\n",res,res_totnode);
						// HOME key
						if(code==RAWKEY_HOME  &&  res!=0) { sel_entry = 0; }
						// END key
						if(code==RAWKEY_END  &&  res!=res_totnode-1) { sel_entry = res_totnode - 1; }
						// CuRSOR UP key
						if(code==CURSORUP  &&  res!=0) { sel_entry = res - 1; }
						// PAGE UP key
						if(code==RAWKEY_PAGEUP  &&  res!=0) {
							IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Top,&res, LISTBROWSER_Bottom,&res_temp, TAG_DONE);
							sel_entry = res_temp - res;
							sel_entry = res - sel_entry;
//DBUG("  %ld\n",sel_entry);
							if(sel_entry < 0) { sel_entry = 0; }
						}
						// CURSOR DOWN key
						if(code==CURSORDOWN  &&  res!=res_totnode-1) {
							sel_entry = res + 1;
						}
						// PAGE DOWN key
						if(code==RAWKEY_PAGEDOWN  &&  res!=res_totnode-1) {
							IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Bottom,&res, TAG_DONE);
							sel_entry = res;
						}

						if(sel_entry != -1) {
							res_prev = selectListEntry(window[WID_MAIN], sel_entry);
						}
					}
					break;

					case WMHI_GADGETUP:
						switch (result & WMHI_GADGETMASK)
							{
// About
							case GID_DONATE_BTN:
							{
								BPTR handle = IDOS->Open("URL:https://www.paypal.com/donate/?hosted_button_id=KPGSWS9XBBJ34", MODE_OLDFILE);
//DBUG("  [GID_DONATE_BTN]\n",NULL);
								if(handle) { IDOS->Close(handle); }
								else {
									//DoMessage((STRPTR)GetString(&li,MSG_ERROR_URLOPEN), REQIMAGE_ERROR, NULL);
									IIntuition->SetAttrs(object[OID_REQ], REQ_BodyText,(STRPTR)GetString(&li,MSG_ERROR_URLOPEN), TAG_DONE);
									IIntuition->IDoMethod(object[OID_REQ],RM_OPENREQ,NULL,window[WID_MAIN],NULL,TAG_DONE);
								}
							}
							break;
// Visual Settings
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
								/*fskipptr = 1; // custom
								if(code == -1) { fskipptr = 0; code = 0; } // auto
								if(code == 10) { fskipptr = 2; code = 0; } // off
								fskipvalue = code;*/
								IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_FSKIPSET],window[WID_MAIN],NULL,GA_Disabled,!code,TAG_DONE);
								/*if (code==1)
									{
 									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_FSKIPSET],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									}
								else
									{
 									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_FSKIPSET],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									}*/
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
// Game Settings
							case GID_EXE:
								res = IIntuition->IDoMethod((Object *)gadget[GID_EXE],GFILE_REQUEST,window[WID_MAIN]);
								if(res) {
									BPTR seglist;
									IIntuition->GetAttr(GETFILE_FullFile, gadget[GID_EXE], (uint32*)&res_str);
									IUtility->Strlcpy(exefile, res_str, MAX_FULLFILEPATH);
									IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_EXE],window[WID_MAIN],NULL);
									// Check if using old VBA -> enables 'YUV Mode' gadget
									if( (seglist=IDOS->LoadSeg(res_str)) ) {
										STRPTR str_ver;
										IDOS->GetSegListInfoTags(seglist, GSLI_VersionString,&str_ver, TAG_DONE);
										IIntuition->SetAttrs(gadget[GID_YUV], GA_Disabled,str_ver? TRUE : FALSE,  CHOOSER_Selected,str_ver? 0 : yuvptr , TAG_DONE);
										IDOS->UnLoadSeg(seglist);
									}
								}
								break;
							case GID_GAME:
								res = IIntuition->IDoMethod((Object *)gadget[GID_GAME],GFILE_REQUEST,window[WID_MAIN]);
								if(res) {
									//IIntuition->GetAttrs(gadget[GID_GAME],GETFILE_FullFile, &gamefile, TAG_DONE);
									IIntuition->GetAttr(GETFILE_Drawer, gadget[GID_GAME], (uint32*)&res_str);
									IUtility->Strlcpy(gamefile, res_str, MAX_FULLFILEPATH);
									updateList();
									//IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_GAME],window[WID_MAIN],NULL);
									IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_LISTBROWSER], window[WID_MAIN], NULL);
								}
								break;
							case GID_BIOS:
								res = IIntuition->IDoMethod((Object *)gadget[GID_BIOS],GFILE_REQUEST,window[WID_MAIN]);
								if(res) {
									IIntuition->GetAttr(GETFILE_FullFile, gadget[GID_BIOS], (uint32*)&res_str);
									IUtility->Strlcpy(biosfile, res_str, MAX_FULLFILEPATH);
									IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_BIOS],window[WID_MAIN],NULL);
								}
								break;
							case GID_FLASH:
								fsizeptr=code;
								break;
							case GID_SAVE:
								savetypeptr=code;
								break;
							/*case GID_MMX:
								if (code==TRUE)	mmxptr=1;
								else				mmxptr=0;
								break;*/
							case GID_IPS:
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_IPSFILE], window[WID_MAIN], NULL, GA_Disabled,!code, TAG_DONE);
								/*if (code)
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_IPSFILE],window[WID_MAIN],NULL,GA_Disabled,FALSE,TAG_DONE);
									}
								else
									{
									IIntuition->SetGadgetAttrs((struct Gadget *)gadget[GID_IPSFILE],window[WID_MAIN],NULL,GA_Disabled,TRUE,TAG_DONE);
									}*/
								break;
							case GID_IPSFILE:
								IIntuition->IDoMethod((Object *)gadget[GID_IPSFILE],GFILE_REQUEST ,window[WID_MAIN]);
								IIntuition->GetAttr(GETFILE_FullFile, gadget[GID_IPSFILE], (uint32*)&res_str);
								IUtility->Strlcpy(ipsfile, res_str, MAX_FULLFILEPATH);
								IIntuition->RefreshGadgets((struct Gadget *)gadget[GID_IPSFILE],window[WID_MAIN],NULL);
								break;
							case GID_RTC:
								if (code==TRUE)	rtcptr=1;
								else				rtcptr=0;
								break;
							case GID_GAMEPAD_BTN:
DBUG("  [GID_GAMEPAD_BTN]\n",NULL);
								openGamepadWin(); // open gamepad window
								break;
							case GID_LISTBROWSER:
DBUG("  [GID_LISTBROWSER] %ld\n",code);
								IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_RelEvent,&res, /*LISTBROWSER_Selected,&res_temp,*/ TAG_DONE);
								setROMFile();
								if(res == LBRE_DOUBLECLICK) { parse_commandline(); }
								break;
// Debug Settings
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
//DBUG("[GID_DEBUG] %ld\n",code);
								IIntuition->SetAttrs(gadget[GID_VERBOSE], GA_Disabled,!code, TAG_DONE);
								IIntuition->SetAttrs(gadget[GID_GDBPROTO], GA_Disabled,!code, TAG_DONE);
								IIntuition->SetAttrs(gadget[GID_AGBPRINT], GA_Disabled,!code, TAG_DONE);
								IIntuition->SetAttrs(gadget[GID_GDBPORT], GA_Disabled,code? !(gdbptr==1) : TRUE, TAG_DONE);
								ILayout->RefreshPageGadget((struct Gadget *)gadget[GID_DEBUG_PAGE], object[OID_PAGE4], window[WID_MAIN], NULL);
								debugptr = code;
								/*if (code)
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
									}*/
								break;
							case GID_VERBOSE:
								verboseptr = code;
								break;
							case GID_AGBPRINT:
								agbptr = code;
								break;
// Buttons
							case GID_SAVECONF:
DBUG("[GID_SAVECONF]\n",NULL);
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
								IPrefsObjects->WritePrefs(VBAPrefs, WRITEPREFS_AppID,appID, WRITEPREFS_WriteENVARC,TRUE, TAG_DONE);
								break;
							case GID_PLAY:
								IIntuition->GetAttrs(gadget[GID_LISTBROWSER], LISTBROWSER_Selected,&res, TAG_DONE);
DBUG("  [GID_PLAY] %ld\n",res);
								//if (strcmp(gamefile,"")&&strcmp(exefile,""))
								if(res!=-1  &&  exefile[0]!='\0') {
									parse_commandline();
								}
								else {
									IIntuition->SetAttrs(object[OID_REQ], REQ_BodyText,GetString(&li,msg_warningtxt), TAG_DONE);
									IIntuition->IDoMethod(object[OID_REQ],RM_OPENREQ,NULL,window[WID_MAIN],NULL,TAG_DONE);
								}
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
