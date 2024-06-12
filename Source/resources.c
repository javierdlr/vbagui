/*
RESOURCES.c
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/application.h>
#include <proto/locale.h>
//#include <libraries/gadtools.h>

#include "vbagui_cat.h"

#include "includes.h"


/*
**	Library and interfaces initialization
*/
struct Library *ApplicationBase = NULL;
//struct Library *AIN_Base = NULL;
struct Library *IconBase = NULL;
//struct Library *DOSBase;
struct Library *IntuitionBase = NULL;
//struct Library *GfxBase = NULL;
struct Library *UtilityBase = NULL;
struct Library *LocaleBase = NULL;
//extern struct ExecIFace *IExec;
//extern struct DOSIFace *IDOS;
struct ApplicationIFace *IApplication = NULL;
struct PrefsObjectsIFace *IPrefsObjects = NULL;
//struct AIN_IFace *IAIN = NULL;
struct IconIFace *IIcon = NULL;
struct IntuitionIFace *IIntuition = NULL;
//struct GraphicsIFace *IGraphics = NULL;
struct UtilityIFace *IUtility = NULL;
struct LocaleIFace *ILocale = NULL;
struct Library *ClickTabBase = NULL, *ListBrowserBase = NULL, *LayoutBase = NULL,
               *ChooserBase = NULL;
// the class library base
struct ClassLibrary *ButtonBase = NULL, *BitMapBase = NULL, *CheckBoxBase = NULL,
                    *LabelBase = NULL, *WindowBase = NULL, *StringBase = NULL,
                    *RequesterBase = NULL, *SpaceBase = NULL, *IntegerBase = NULL,
                    *GetFileBase = NULL, *SliderBase = NULL, *RadioButtonBase = NULL;
// the class pointer
Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *StringClass,
      *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
      *RequesterClass, *SpaceClass, *IntegerClass, *GetFileClass, *SliderClass,
      *RadioButtonClass;
// some interfaces needed
//struct ListBrowserIFace *IListBrowser = NULL;
//struct ClickTabIFace *IClickTab = NULL;
//struct LayoutIFace *ILayout = NULL;
//struct ChooserIFace *IChooser = NULL;


struct Catalog *vbaguiCatalog;
/*
**	Locale Strings
*/
struct FC_String vbagui_Strings[80] = {
	{ (STRPTR) "About", 0 },
	{ (STRPTR) "Visual Settings", 1 },
	{ (STRPTR) "Game Settings", 2 },
	{ (STRPTR) "Debug Settings", 3 },
	{ (STRPTR) "Video Mode", 4 },
	{ (STRPTR) "Full Screen", 5 },
	{ (STRPTR) "YUV Mode", 6 },
	{ (STRPTR) "None", 7 },
	{ (STRPTR) "Filters", 8 },
	{ (STRPTR) "Normal Mode", 9 },
	{ (STRPTR) "TV Mode", 10 },
	{ (STRPTR) "2xSal", 11 },
	{ (STRPTR) "Super 2xSal", 12 },
	{ (STRPTR) "Super Eagle", 13 },
	{ (STRPTR) "Pixelate", 14 },
	{ (STRPTR) "Motion Blur", 15 },
	{ (STRPTR) "AdvanceMAME scale 2x", 16 },
	{ (STRPTR) "Simple 2x", 17 },
	{ (STRPTR) "Bilinear", 18 },
	{ (STRPTR) "Bilinear Plus", 19 },
	{ (STRPTR) "Scanlines", 20 },
	{ (STRPTR) "High Quality 2x", 21 },
	{ (STRPTR) "Low Quality 2x", 22 },
	{ (STRPTR) "FrameSkip", 23 },
	{ (STRPTR) "Automatic", 24 },
	{ (STRPTR) "Custom", 25 },
	{ (STRPTR) "Disabled", 26 },
	{ (STRPTR) "Enable Throttle", 27 },
	{ (STRPTR) "InterFrame Blending", 28 },
	{ (STRPTR) "None", 29 },
	{ (STRPTR) "Motion Blur", 30 },
	{ (STRPTR) "Smart", 31 },
	{ (STRPTR) "Pause When Inactive", 32 },
	{ (STRPTR) "Don't show emulation speed", 33 },
	{ (STRPTR) "Show emulation speed", 34 },
	{ (STRPTR) "Show detailed speed data", 35 },
	{ (STRPTR) "Program Exe", 36 },
	{ (STRPTR) "Select the main VBA executable", 37 },
	{ (STRPTR) "Game File", 38 },
	{ (STRPTR) "Select a suitable game file", 39 },
	{ (STRPTR) "BIOS File", 40 },
	{ (STRPTR) "Select a suitable BIOS file", 41 },
	{ (STRPTR) "Flash Size", 42 },
	{ (STRPTR) "64k Flash", 43 },
	{ (STRPTR) "128k Flash", 44 },
	{ (STRPTR) "Save Type", 45 },
	{ (STRPTR) "Automatic", 46 },
	{ (STRPTR) "EEPROM", 47 },
	{ (STRPTR) "SRAM", 48 },
	{ (STRPTR) "FLASH", 49 },
	{ (STRPTR) "EEPROM + Sensor", 50 },
	{ (STRPTR) "None", 51 },
	{ (STRPTR) "IPS Patching", 52 },
	{ (STRPTR) "Select a suitable IPS patch file", 53 },
	{ (STRPTR) "Disable MMX", 54 },
	{ (STRPTR) "Enable RTC", 55 },
	{ (STRPTR) "Enable Debugging", 56 },
	{ (STRPTR) "Verbose Output", 57 },
	{ (STRPTR) "1-SWI", 58 },
	{ (STRPTR) "2-Unaligned Mem Access", 59 },
	{ (STRPTR) "4-Illegal Mem Write", 60 },
	{ (STRPTR) "8-Illegal Mem Read", 61 },
	{ (STRPTR) "6-DMA 0", 62 },
	{ (STRPTR) "32-DMA 1", 63 },
	{ (STRPTR) "64-DMA 2", 64 },
	{ (STRPTR) "128-DMA 3", 65 },
	{ (STRPTR) "256-Undefined Instruction", 66 },
	{ (STRPTR) "512-AGBPrint Messages", 67 },
	{ (STRPTR) "GDB Protocol", 68 },
	{ (STRPTR) "Default TCP Port", 69 },
	{ (STRPTR) "Custom TCP Port", 70 },
	{ (STRPTR) "PIPE Transport", 71 },
	{ (STRPTR) "Enable AGBPrint", 72 },
	{ (STRPTR) "Save", 73 },
	{ (STRPTR) "Launch", 74 },
	{ (STRPTR) "Exit", 75 },
	{ (STRPTR) "Warning!", 76 },
	{ (STRPTR) "\033cPlease check that you already selected game and\nexecutable path before launching the emulator\n", 77 },
	{ (STRPTR) "Ok", 78 },
	{ (STRPTR) "\n\nMain Code: Vicente 'Ami603' Gimeno.\nAbout Logo: Kai 'Restore' Thorsberg.\nNative OS4 VisualBoy Advance porting:\nNicolas 'nicomen' Mendoza and\nupdated by smarkusg.\n\nThis 'Quick'nDirty' GUI wouldn't never seen\nthe light of the day if Tony 'ToAks' Aksnes\ndidn't 'kindly asked me' for it ;)\nGreetings to aGas Group ;)\n", 79 }
};


/*
**	Libraries&interfaces Stuff
*/
int openlibs(void)
{
	DOSBase = IExec->OpenLibrary("dos.library", 52);
	IDOS = (struct DOSIFace *)IExec->GetInterface(DOSBase, "main", 1, NULL);

	UtilityBase = IExec->OpenLibrary("utility.library", 52);
	IUtility = (struct UtilityIFace *)IExec->GetInterface(UtilityBase, "main", 1, NULL);

	IntuitionBase = IExec->OpenLibrary("intuition.library", 52);
	IIntuition = (struct IntuitionIFace *)IExec->GetInterface(IntuitionBase, "main", 1, NULL);

	IconBase = IExec->OpenLibrary("icon.library", 52);
	IIcon = (struct IconIFace *)IExec->GetInterface(IconBase, "main", 1, NULL);

	ApplicationBase = IExec->OpenLibrary("application.library", 50);
	if (ApplicationBase)
		{
		IApplication  = (struct ApplicationIFace *)IExec->GetInterface(ApplicationBase, "application", 2, NULL);
		IPrefsObjects = (struct PrefsObjectsIFace *)IExec->GetInterface(ApplicationBase, "prefsobjects", 2, NULL);
		}
	if (!IApplication || !IPrefsObjects)
		{
		IDOS->Printf("Error obtaining application.library interfaces\n");
		return -1;
		}
	LocaleBase = IExec->OpenLibrary("locale.library",50);
	if (LocaleBase)
		{
		ILocale = (struct LocaleIFace *)IExec->GetInterface(LocaleBase, "main",1,NULL);
		}
	if (!ILocale)
		{
		IDOS->Printf("Error obtaining locale.library interface\n");
		return -1;
		}

	vbaguiCatalog = ILocale->OpenCatalog(NULL, (STRPTR)"vbagui.catalog",
	                                     OC_BuiltInLanguage, "english",
	                                     OC_Version,0, TAG_DONE);
	if (vbaguiCatalog)
		{
		struct FC_String *fc;
		int i;
		for (i = 0, fc = vbagui_Strings;  i < 80;  i++, fc++)
			{
			fc->msg = ILocale->GetCatalogStr(vbaguiCatalog, fc->id,(CONST_STRPTR) fc->msg);
			}
		}

	RequesterBase = IIntuition->OpenClass("requester.class", 52, &RequesterClass);
	IntegerBase = IIntuition->OpenClass("gadgets/integer.gadget", 52, &IntegerClass);
	SpaceBase = IIntuition->OpenClass("gadgets/space.gadget", 52, &SpaceClass);
	//StringBase = IIntuition->OpenClass("gadgets/string.gadget", 52, &StringClass);
	CheckBoxBase = IIntuition->OpenClass("gadgets/checkbox.gadget", 52, &CheckBoxClass);
	ButtonBase = IIntuition->OpenClass("gadgets/button.gadget", 52, &ButtonClass);
	BitMapBase = IIntuition->OpenClass("images/bitmap.image", 52, &BitMapClass);
	LabelBase = IIntuition->OpenClass("images/label.image", 52, &LabelClass);
	GetFileBase = IIntuition->OpenClass("gadgets/getfile.gadget", 52, &GetFileClass);
	//SliderBase = IIntuition->OpenClass("gadgets/slider.gadget", 52, &SliderClass);
	RadioButtonBase = IIntuition->OpenClass("gadgets/radiobutton.gadget", 52, &RadioButtonClass);
	WindowBase = IIntuition->OpenClass("window.class", 52, &WindowClass);

//	ListBrowserBase = (struct Library *)IIntuition->OpenClass("gadgets/listbrowser.gadget", 52, &ListBrowserClass);
	ClickTabBase = (struct Library *)IIntuition->OpenClass("gadgets/clicktab.gadget", 52, &ClickTabClass);
	LayoutBase = (struct Library *)IIntuition->OpenClass("gadgets/layout.gadget", 52, &LayoutClass);
	ChooserBase = (struct Library *)IIntuition->OpenClass("gadgets/chooser.gadget", 52, &ChooserClass);

	return 0;
}

void closelibs(void)
{
	if(IIntuition) {
		IIntuition->CloseClass(RequesterBase);
		IIntuition->CloseClass(RadioButtonBase);
		IIntuition->CloseClass(IntegerBase);
		IIntuition->CloseClass(SpaceBase);
		//IIntuition->CloseClass(StringBase);
		IIntuition->CloseClass(LabelBase);
		IIntuition->CloseClass(GetFileBase);
		//IIntuition->CloseClass(SliderBase);
		IIntuition->CloseClass(BitMapBase);
		IIntuition->CloseClass(ButtonBase);
		IIntuition->CloseClass(CheckBoxBase);
		IIntuition->CloseClass(WindowBase);

//		IExec->DropInterface( (struct Interface *)IChooser );
		IIntuition->CloseClass( (struct ClassLibrary *)ChooserBase );
//		IExec->DropInterface( (struct Interface *)ILayout );
		IIntuition->CloseClass( (struct ClassLibrary *)LayoutBase );
//		IExec->DropInterface( (struct Interface *)IClickTab );
		IIntuition->CloseClass( (struct ClassLibrary *)ClickTabBase );
//		IExec->DropInterface( (struct Interface *)IListBrowser );
//		IIntuition->CloseClass( (struct ClassLibrary *)ListBrowserBase );
	}

	if (vbaguiCatalog)	ILocale->CloseCatalog(vbaguiCatalog);

	if (IApplication)		IExec->DropInterface((struct Interface *)IApplication);
	if (IPrefsObjects)		IExec->DropInterface((struct Interface *)IPrefsObjects);
	if (ILocale)		IExec->DropInterface((struct Interface *)ILocale);

	IExec->DropInterface( (struct Interface *)IIcon );
	IExec->CloseLibrary(IconBase);
//	IExec->DropInterface( (struct Interface *)IGraphics );
//	IExec->CloseLibrary(GfxBase);
	IExec->DropInterface( (struct Interface *)IIntuition );
	IExec->CloseLibrary(IntuitionBase);

	IExec->DropInterface( (struct Interface *)IUtility );
	IExec->CloseLibrary(UtilityBase);

	IExec->DropInterface( (struct Interface *)IDOS );
	IExec->CloseLibrary(DOSBase);

	if (ApplicationBase)	IExec->CloseLibrary(ApplicationBase);
	if (LocaleBase)		IExec->CloseLibrary(LocaleBase);
}
