/*
GAMEPAD.C
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/amigainput.h>

#include <libraries/gadtools.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/space.h>
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


extern void saveToConfig(void);

//extern struct ExecIFace *IExec;
//extern struct DOSIFace *IDOS;
extern struct AIN_IFace *IAIN;
//extern struct IconIFace *IIcon;
extern struct IntuitionIFace *IIntuition;
//extern struct GraphicsIFace *IGraphics;
//extern struct UtilityIFace *IUtility;
// the class pointer
extern Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *GetFileClass,
             *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
             *RequesterClass, *SpaceClass, *IntegerClass, *GetFileClass, *SliderClass,
             *RadioButtonClass;

extern struct Window *window[WID_LAST];
extern Object *gadget[GID_LAST];
extern Object *object[OID_LAST];

extern struct MsgPort *ai_port;
extern STRPTR cfg_file;
extern int32 cfg_value[]; // settings loaded/saved from/to 'VisualBoyAdvance.cfg'


/*
 * Using parts of code:
 * E-UAE - The portable Amiga Emulator
 * AmigaInput joystick driver
 * Copyright 2005 Richard Drummond
 */
#define MAX_INPUT_DEVICES  6
#define MAX_JOYSTICKS  MAX_INPUT_DEVICES
#define MAX_AXES       2
#define MAX_BUTTONS    12

BOOL enumerateJoysticks(AIN_Device *, void *UserData);
void close_joysticks(void);
unsigned int get_joystick_count(void);
STRPTR get_joystick_name(unsigned int);
int acquire_joy(unsigned int, int);
void unacquire_joy(unsigned int);

// A handy container to encapsulate the information we
// need when enumerating joysticks on the system.
struct enumPacket {
 APTR             context;
 uint32          *count;
 struct joystick *joyList;
};
// Per-joystick data private to driver
struct joystick {
 AIN_DeviceID     id;
 STRPTR           name;
 AIN_DeviceHandle *handle;
 APTR             context;
 uint32           axisCount;
 uint32           buttonCount;
// uint32           axisBufferOffset[MAX_AXES];
// int32            axisData[MAX_AXES];
 uint32           buttonBufferOffset[MAX_BUTTONS];
// int32            buttonData[MAX_BUTTONS];
};

static APTR joystickContext;
static uint32 joystickCount;
static struct joystick joystickList[MAX_JOYSTICKS];


uint32 getGamepadButton(unsigned int joynum)
{
	struct joystick *joy = &joystickList[joynum-1];
	uint32 ret = GPAD_NO_BTN;

	if(joy->handle != NULL) {
		AIN_InputEvent *event = NULL;
		while( (event=IAIN->AIN_GetEvent(joy->context)) ) {
//DBUG("AIN_GetEvent() 0x%08lx\n",event->Type);
			/*switch(event->Type) {
				case AINET_BUTTON:
					ret = event->Index - joy->buttonBufferOffset[0];
DBUG("Button #%2ld [%ld]\n",ret, event->Value);
				break;
				default: break;
			}*/
			if(event->Type == AINET_BUTTON) {
				ret = event->Index - joy->buttonBufferOffset[0];
DBUG("Button #%2ld [%ld]\n",ret, event->Value);
			}
			IAIN->AIN_FreeEvent(joy->context, event);
		}
	}

	return ret;
}

void processGamepadGUI(void)
{
	uint16 code = 0;
	uint32 siggot = 0, w_sigmask = 0, ai_sigmask = 1L << ai_port->mp_SigBit,
	       result = WMHI_LASTMSG, btn_GID = GPAD_NO_BTN, btn, btn_pressed = 0;
	BOOL done = TRUE;
	AIN_InputEvent *event = NULL;
	unsigned int joynum = 1;
	struct joystick *joy = &joystickList[joynum-1];

	IIntuition->GetAttr(WINDOW_SigMask, object[OID_GPAD], &w_sigmask);

	// Code taken fro AmiDog's FPSE SDK (http://www.amidog.se/amiga/bin/FPSE/20151029/FPSE-0.10.6-SDK.tar.gz)
	if(joy->handle != NULL) {
		struct TagItem tags[] = { {AINCC_Window,(ULONG)window[WID_GPAD]}, {TAG_DONE,TAG_DONE} };
//DBUG("joy: handle=0x%08lx  context=0x%08lx\n",joy->handle,joy->context);
		IAIN->AIN_SetDeviceParameter(joy->context, joy->handle, AINDP_EVENT, TRUE);
		IAIN->AIN_Set(joy->context, tags);

		while(done != FALSE) {
			siggot = IExec->Wait(w_sigmask | ai_sigmask | SIGBREAKF_CTRL_C);

			if(siggot & SIGBREAKF_CTRL_C) { done = FALSE; break; }

			while( (result=IIntuition->IDoMethod(object[OID_GPAD], WM_HANDLEINPUT, &code)) != WMHI_LASTMSG ) {
//DBUG("result=0x%lx\n",result);
				switch(result & WMHI_CLASSMASK) {
					case WMHI_CLOSEWINDOW:
						done = FALSE;
					break;
					case WMHI_GADGETUP:
DBUG("[WMHI_GADGETUP] code = %ld (0x%08lx)\n",code,code);
						switch(result & WMHI_GADGETMASK) {
							case GID_GPAD_L:
							case GID_GPAD_R:
							case GID_GPAD_B:
							case GID_GPAD_A:
							case GID_GPAD_SEL:
							case GID_GPAD_STA:
								if(btn_GID!=(result & WMHI_GADGETMASK)  &&  btn_GID!=GPAD_NO_BTN) {
									IIntuition->RefreshSetGadgetAttrs((struct Gadget *)gadget[btn_GID], window[WID_GPAD], NULL, GA_Selected,FALSE, TAG_END);
								}
								btn_GID = result & WMHI_GADGETMASK;
								IIntuition->GetAttr(GA_Selected, gadget[btn_GID], &btn_pressed);
DBUG("  btn_GID = 0x%08lx (OID_GPAD_#?)\n",btn_GID);
							break;
							case GID_GPAD_SAVE:
								saveToConfig();
							case GID_GPAD_CANCEL:
								done = FALSE;
							break;
						}
					break;
				}
			}

			btn = getGamepadButton(joynum);
			if(btn_pressed  &&  btn!=GPAD_NO_BTN  &&  btn_GID!=GPAD_NO_BTN) {
DBUG("  (%ld -> %ld)\n",cfg_value[btn_GID-GPAD_BTN_OFFSET],btn);
				cfg_value[btn_GID-GPAD_BTN_OFFSET] = btn;
				IIntuition->RefreshSetGadgetAttrs((struct Gadget *)gadget[btn_GID], window[WID_GPAD], NULL,
				                                  BUTTON_Integer,btn, GA_Selected,FALSE, TAG_END);
				btn_GID = GPAD_NO_BTN; // "reset" btn_GID
			}

		} // END while(done..

		IAIN->AIN_SetDeviceParameter(joy->context, joy->handle, AINDP_EVENT, FALSE);
		// Remove pending AI messages
		while( (event=IAIN->AIN_GetEvent(joy->context)) ) { IAIN->AIN_FreeEvent(joy->context, event); }
	}

}

Object *labelButton(uint32 objID, CONST_STRPTR lbl, uint32 alignH, uint32 lblPos)
{
//DBUG("labelButton() [%ld] = %ld\n",objID-GPAD_BTN_OFFSET,cfg_value[objID-GPAD_BTN_OFFSET]);
	return IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_SpaceOuter,     TRUE,
		LAYOUT_HorizAlignment, alignH,
		LAYOUT_LabelColumn,    lblPos,
		LAYOUT_AddChild, gadget[objID] = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
			GA_ID,        objID,
			GA_RelVerify, TRUE,
			BUTTON_PushButton,   TRUE,
			BUTTON_DomainString, "88",
			BUTTON_Integer,      cfg_value[objID-GPAD_BTN_OFFSET],
		TAG_DONE),
		CHILD_WeightedWidth, 0,
		CHILD_Label, IIntuition->NewObject(LabelClass, NULL, LABEL_Text,lbl, TAG_DONE),
	TAG_DONE);
}

void createGamepadGUI(void)
{
	struct ExamineData *dat_bg = IDOS->ExamineObjectTags(EX_StringNameInput,GPAD_BG, TAG_END),
	                   *dat = IDOS->ExamineObjectTags(EX_StringNameInput,GPAD_IMG, TAG_END),
	                   *dat_str = IDOS->ExamineObjectTags(EX_StringNameInput,GPAD_STR, TAG_END);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat_bg);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat_str);

	object[OID_GPAD] = IIntuition->NewObject(WindowClass, NULL, //"window.class",
        WA_ScreenTitle, VERS" "DATE,
        WA_Title,       get_joystick_name(joystickCount),
        //WA_CustomScreen, window[WID_MAIN]->WScreen,
        //WA_PubScreenFallBack, TRUE,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        //WA_SizeGadget,  TRUE,
        dat_bg? WINDOW_BackFillName : TAG_IGNORE, GPAD_BG,//"keymap_bg.png"
        WA_DepthGadget, TRUE,
        WA_Activate,    TRUE,
        //WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_RAWKEY,
        //WINDOW_IconifyGadget, TRUE,
        //WINDOW_AppPort,       gAppPort,
        //WINDOW_Icon,          iconify,
        WINDOW_RefWindow, window[WID_MAIN],
        WINDOW_Position,  WPOS_CENTERWINDOW,
        //WINDOW_PopupGadget, TRUE,
        //WINDOW_JumpScreensMenu, TRUE,
        //WINDOW_UniqueID,    "vbaGUI_Gamepad_win",
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,  TRUE,

// Left + Right
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          LAYOUT_SpaceOuter,    TRUE,
          LAYOUT_VertAlignment, LALIGN_TOP,
          LAYOUT_AddChild, labelButton(GID_GPAD_L, "_L", LALIGN_LEFT, PLACETEXT_RIGHT),
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
          LAYOUT_AddChild, labelButton(GID_GPAD_R, "_R", LALIGN_RIGHT, PLACETEXT_LEFT),
         TAG_DONE), // END of Left + Right
         CHILD_WeightedHeight, 0,

         LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
           BUTTON_Transparent, TRUE,
           BUTTON_BevelStyle,  BVS_NONE,
           dat? BUTTON_RenderImage : TAG_IGNORE, gadget[GID_GPAD_STR] = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
             BITMAP_SourceFile,  GPAD_STR,//"keymap_str.png",
             BITMAP_Screen,      window[WID_MAIN]->WScreen,
             BITMAP_Masking,     TRUE,
             BITMAP_Transparent, TRUE,
           TAG_DONE),
         TAG_DONE),

// [image] + B + A
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
          LAYOUT_VertAlignment, LALIGN_CENTER,
          //LAYOUT_HorizAlignment, LALIGN_CENTER,
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
            SPACE_MinWidth,10, TAG_DONE),
          CHILD_WeightedWidth, 0,
          LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
            BUTTON_Transparent, TRUE,
            BUTTON_BevelStyle,  BVS_NONE,
            dat? BUTTON_RenderImage : TAG_IGNORE, gadget[GID_GPAD_IMG] = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
              BITMAP_SourceFile,  GPAD_IMG,//"keymap.png",
              BITMAP_Screen,      window[WID_MAIN]->WScreen,
              BITMAP_Masking,     TRUE,
              BITMAP_Transparent, TRUE,
            TAG_DONE),
          TAG_DONE),
          dat? TAG_IGNORE : CHILD_MinWidth, 100,
          dat? TAG_IGNORE : CHILD_MinHeight, 100,
          CHILD_WeightedWidth,  0,
          CHILD_WeightedHeight, 0,
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
            SPACE_MinWidth,20, TAG_DONE),
          CHILD_WeightedWidth, 0,
          LAYOUT_AddChild, labelButton(GID_GPAD_B, "_B", LALIGN_LEFT, PLACETEXT_LEFT),
          CHILD_WeightedWidth,  0,
          CHILD_WeightedHeight, 0,
          LAYOUT_AddChild, labelButton(GID_GPAD_A, "_A", LALIGN_RIGHT, PLACETEXT_RIGHT),
          CHILD_WeightedWidth,  0,
          CHILD_WeightedHeight, 0,
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
            SPACE_MinWidth,20, TAG_DONE),
          CHILD_WeightedWidth, 0,
         TAG_DONE), // END of [image] + B + A
         //CHILD_WeightedHeight, 0,

         LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),

// Select + Start
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
          LAYOUT_SpaceOuter, TRUE,
          LAYOUT_AddChild, labelButton(GID_GPAD_SEL, "S_elect", LALIGN_RIGHT, PLACETEXT_LEFT),
          LAYOUT_AddChild, labelButton(GID_GPAD_STA, "_Start", LALIGN_LEFT, PLACETEXT_RIGHT),
         TAG_DONE), // END of Select + Start
         CHILD_WeightedHeight, 0,

         LAYOUT_AddChild, gadget[GID_GPAD_TEXT] = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
           //GA_ID,        OID_GPAD_TEXT,
           //GA_RelVerify, TRUE,
           GA_ReadOnly, TRUE,
           GA_Text,      GetString(&li, MSG_GUI_GPAD_TEXT),//"(assign gamepad buttons)",
           BUTTON_BevelStyle,   BVS_NONE,
           BUTTON_Transparent, TRUE,
         TAG_DONE),
         CHILD_WeightedHeight, 0,

// BUTTONS
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
           LAYOUT_SpaceOuter,  TRUE,
           LAYOUT_BevelStyle,  BVS_SBAR_VERT,
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        GID_GPAD_SAVE,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_GPAD_SAVE),
           TAG_DONE),
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        GID_GPAD_CANCEL,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_GPAD_CANCEL),
           TAG_DONE),
         TAG_DONE), // END of BUTTONS
         CHILD_WeightedHeight, 0,

        TAG_DONE),
	TAG_DONE);
DBUG("  gamepad obj = 0x%08lx\n",object[OID_GPAD]);
	if( object[OID_GPAD] ) {
		window[WID_GPAD] = (struct Window *)IIntuition->IDoMethod(object[OID_GPAD], WM_OPEN, NULL);
	}
}


void openGamepadWin(void)
{
	struct TagItem tags[] = { {AINCC_Port,(ULONG)ai_port}, {TAG_DONE,TAG_DONE} };
DBUG("openGamepadWin()\n",NULL);

	joystickContext = IAIN->AIN_CreateContext(1, tags);
	if(joystickContext) {
		struct enumPacket packet = {joystickContext, &joystickCount, &joystickList[0]};

		IAIN->AIN_EnumDevices(joystickContext, enumerateJoysticks, &packet);

		if( get_joystick_count() ) {
DBUG("  ->%s<-\n", get_joystick_name(joystickCount));
			acquire_joy(joystickCount, 0);

			IIntuition->SetAttrs(object[OID_MAIN], WA_BusyPointer,TRUE, TAG_DONE);
			createGamepadGUI();
DBUG("  gamepad win = 0x%08lx\n",window[WID_GPAD]);
			if(window[WID_GPAD]) { processGamepadGUI(); }
			IIntuition->SetAttrs(object[OID_MAIN], WA_BusyPointer,FALSE, TAG_DONE);

			IIntuition->DisposeObject( object[OID_GPAD] );
			IIntuition->DisposeObject( gadget[GID_GPAD_IMG] );
			IIntuition->DisposeObject( gadget[GID_GPAD_STR] );
			object[OID_GPAD] = gadget[GID_GPAD_IMG] = gadget[GID_GPAD_STR] = NULL;

			unacquire_joy(joystickCount);
		}

		close_joysticks();
	}

}


/*
 * Using parts of code:
 * E-UAE - The portable Amiga Emulator
 * AmigaInput joystick driver
 * Copyright 2005 Richard Drummond
 */
// Callback to enumerate joysticks
BOOL enumerateJoysticks(AIN_Device *device, void *UserData)
{
	APTR context = ((struct enumPacket *)UserData)->context;
	uint32 *count = ((struct enumPacket *)UserData)->count;
	struct joystick *joy = &((struct enumPacket *)UserData)->joyList[*count];

	BOOL result = FALSE;

	if(*count < MAX_JOYSTICKS) {
		if(device->Type == AINDT_JOYSTICK) {
			unsigned int i;

			joy->context     = context;
			joy->id          = device->DeviceID;
			joy->name        = (STRPTR)device->DeviceName;
			joy->axisCount   = device->NumAxes;
			joy->buttonCount = device->NumButtons;

			if(joy->axisCount > MAX_AXES) joy->axisCount = MAX_AXES;

			if(joy->buttonCount > MAX_BUTTONS) joy->buttonCount = MAX_BUTTONS;

			// Query offsets in ReadDevice buffer for axes' data
//			for(i=0; i<joy->axisCount; i++)
//				result = IAIN->AIN_Query(joy->context, joy->id, AINQ_AXIS_OFFSET, i, &(joy->axisBufferOffset[i]), 4);

			// Query offsets in ReadDevice buffer for buttons' data
			for(i=0; i<joy->buttonCount; i++) {
				result = /*result &&*/ IAIN->AIN_Query(joy->context, joy->id, AINQ_BUTTON_OFFSET, i, &(joy->buttonBufferOffset[i]), 4);
DBUG("AINQ_BUTTON_OFFSET #%ld = %ld\n",i,joy->buttonBufferOffset[i]);
			}

			if(result  &&  joy->id==4096) {
DBUG("Joystick #%ld (AI ID=%ld) '%s' with %ld axes, %ld buttons\n",*count, joy->id, joy->name, joy->axisCount, joy->buttonCount);
				(*count)++;
			}

		}
	}

	return result;
}

void close_joysticks(void)
{
	unsigned int i = joystickCount;

	while(i-- > 0) {
		struct joystick *joy = &joystickList[i];

		if(joy->handle) {
			IAIN->AIN_ReleaseDevice(joy->context, joy->handle);
			joy->handle = 0;
		}
	}
	joystickCount = 0;

	if(joystickContext) {
		IAIN->AIN_DeleteContext(joystickContext);
		joystickContext = NULL;
	}
}

// Query number of joysticks attached to system
unsigned int get_joystick_count(void)
{
	return joystickCount;
}

STRPTR get_joystick_name(unsigned int joynum)
{
	return (STRPTR)joystickList[joynum-1].name;
}

int acquire_joy(unsigned int joynum, int flags)
{
	struct joystick *joy = &joystickList[joynum-1];
	int result = 0;

	joy->handle = IAIN->AIN_ObtainDevice(joy->context, joy->id);
	if(joy->handle) result = 1;
	else IDOS->Printf("Failed to acquire joy\n");

	return result;
}

void unacquire_joy(unsigned int joynum)
{
	struct joystick *joy = &joystickList[joynum-1];

	if(joy->handle) {
		IAIN->AIN_ReleaseDevice(joy->context, joy->handle);
		joy->handle = 0;
	}
}
