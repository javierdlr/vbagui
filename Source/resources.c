/*
RESOURCES.c
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/application.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/locale.h>
#include <proto/amigainput.h>
#include <proto/listbrowser.h>

#include <classes/requester.h>

#define CATCOMP_NUMBERS
#define CATCOMP_BLOCK
#define CATCOMP_CODE
struct LocaleInfo li;
#include "vbagui_strings.h"

#include "includes.h"
#include "debug.h"


extern struct Window *window[WID_LAST];
extern Object *object[OID_LAST];
extern struct List *romlist;


/*
**	Library and interfaces initialization
*/
struct Library *ApplicationBase = NULL;
struct Library *AIN_Base = NULL;
struct Library *IconBase = NULL;
//struct Library *DOSBase;
struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;
struct Library *UtilityBase = NULL;
struct Library *LocaleBase = NULL;
//extern struct ExecIFace *IExec;
//extern struct DOSIFace *IDOS;
struct ApplicationIFace *IApplication = NULL;
struct PrefsObjectsIFace *IPrefsObjects = NULL;
struct AIN_IFace *IAIN = NULL;
struct IconIFace *IIcon = NULL;
struct IntuitionIFace *IIntuition = NULL;
struct GraphicsIFace *IGraphics = NULL;
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
struct ListBrowserIFace *IListBrowser = NULL;
struct ClickTabIFace *IClickTab = NULL;
struct LayoutIFace *ILayout = NULL;
struct ChooserIFace *IChooser = NULL;
// gamepad stuff
static CONST_STRPTR kw_array[] = {"Joy0_L", "Joy0_R", "Joy0_B", "Joy0_A", "Joy0_Select", "Joy0_Start", NULL};
STRPTR cfg_file;
int32 cfg_value[TOT_CFG]; // settings loaded/saved from/to 'VisualBoyAdvance.cfg'


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

	GfxBase = IExec->OpenLibrary("graphics.library", 52);
	IGraphics = (struct GraphicsIFace *)IExec->GetInterface(GfxBase, "main", 1, NULL);

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

	li.li_Catalog = NULL;
	if( (LocaleBase=IExec->OpenLibrary("locale.library", 50))
	   &&  (ILocale=(struct LocaleIFace *)IExec->GetInterface(LocaleBase, "main", 1, NULL)) )
	{
		li.li_ILocale = ILocale;
		li.li_Catalog = ILocale->OpenCatalog(NULL, "vbagui.catalog",
		                                     OC_BuiltInLanguage, "english",
		                                     OC_PreferExternal, TRUE,
		                                    TAG_END);
	}
	//else { IDOS->PutErrStr("Failed to use catalog system. Using built-in strings.\n"); }

	AIN_Base = IExec->OpenLibrary("AmigaInput.library", 52);
	IAIN = (struct AIN_IFace *)IExec->GetInterface(AIN_Base, "main", 1, NULL);

	RequesterBase = IIntuition->OpenClass("requester.class", 52, &RequesterClass);
	IntegerBase = IIntuition->OpenClass("gadgets/integer.gadget", 52, &IntegerClass);
	SpaceBase = IIntuition->OpenClass("gadgets/space.gadget", 52, &SpaceClass);
	//StringBase = IIntuition->OpenClass("gadgets/string.gadget", 52, &StringClass);
	CheckBoxBase = IIntuition->OpenClass("gadgets/checkbox.gadget", 52, &CheckBoxClass);
	ButtonBase = IIntuition->OpenClass("gadgets/button.gadget", 52, &ButtonClass);
	BitMapBase = IIntuition->OpenClass("images/bitmap.image", 52, &BitMapClass);
	LabelBase = IIntuition->OpenClass("images/label.image", 52, &LabelClass);
	GetFileBase = IIntuition->OpenClass("gadgets/getfile.gadget", 52, &GetFileClass);
	SliderBase = IIntuition->OpenClass("gadgets/slider.gadget", 52, &SliderClass);
	RadioButtonBase = IIntuition->OpenClass("gadgets/radiobutton.gadget", 52, &RadioButtonClass);
	WindowBase = IIntuition->OpenClass("window.class", 52, &WindowClass);

	ListBrowserBase = (struct Library *)IIntuition->OpenClass("gadgets/listbrowser.gadget", 52, &ListBrowserClass);
	ClickTabBase = (struct Library *)IIntuition->OpenClass("gadgets/clicktab.gadget", 52, &ClickTabClass);
	LayoutBase = (struct Library *)IIntuition->OpenClass("gadgets/layout.gadget", 52, &LayoutClass);
	ChooserBase = (struct Library *)IIntuition->OpenClass("gadgets/chooser.gadget", 52, &ChooserClass);

	if(RequesterBase==NULL  ||  IntegerBase==NULL  ||  SpaceBase==NULL  /*||  StringBase==NULL*/
	   ||  CheckBoxBase==NULL  ||  ButtonBase==NULL  ||  BitMapBase==NULL  ||  LabelBase==NULL
	   ||  GetFileBase==NULL  ||  WindowBase==NULL  ||  ListBrowserBase==NULL  ||  ClickTabBase==NULL
	   ||  SliderBase==NULL  ||  RadioButtonBase==NULL  ||  LayoutBase==NULL  ||  ChooserBase==NULL) { return -1; }

	IListBrowser = (struct ListBrowserIFace *)IExec->GetInterface( (struct Library *)ListBrowserBase, "main", 1, NULL );
	IClickTab = (struct ClickTabIFace *)IExec->GetInterface( (struct Library *)ClickTabBase, "main", 1, NULL );
	ILayout = (struct LayoutIFace *)IExec->GetInterface( (struct Library *)LayoutBase, "main", 1, NULL );
	IChooser = (struct ChooserIFace *)IExec->GetInterface( (struct Library *)ChooserBase, "main", 1, NULL );

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
		IIntuition->CloseClass(SliderBase);
		IIntuition->CloseClass(BitMapBase);
		IIntuition->CloseClass(ButtonBase);
		IIntuition->CloseClass(CheckBoxBase);
		IIntuition->CloseClass(WindowBase);

		IExec->DropInterface( (struct Interface *)IChooser );
		IIntuition->CloseClass( (struct ClassLibrary *)ChooserBase );
		IExec->DropInterface( (struct Interface *)ILayout );
		IIntuition->CloseClass( (struct ClassLibrary *)LayoutBase );
		IExec->DropInterface( (struct Interface *)IClickTab );
		IIntuition->CloseClass( (struct ClassLibrary *)ClickTabBase );
		IExec->DropInterface( (struct Interface *)IListBrowser );
		IIntuition->CloseClass( (struct ClassLibrary *)ListBrowserBase );
	}

	IExec->DropInterface( (struct Interface *)IAIN );
	IExec->CloseLibrary(AIN_Base);

	if(ILocale)
	{
		ILocale->CloseCatalog(li.li_Catalog);
		IExec->DropInterface( (struct Interface *)ILocale );
	}
	IExec->CloseLibrary( (struct Library *)LocaleBase );

	if (IApplication)		IExec->DropInterface((struct Interface *)IApplication);
	if (IPrefsObjects)		IExec->DropInterface((struct Interface *)IPrefsObjects);

	IExec->DropInterface( (struct Interface *)IIcon );
	IExec->CloseLibrary(IconBase);
	IExec->DropInterface( (struct Interface *)IGraphics );
	IExec->CloseLibrary(GfxBase);
	IExec->DropInterface( (struct Interface *)IIntuition );
	IExec->CloseLibrary(IntuitionBase);

	IExec->DropInterface( (struct Interface *)IUtility );
	IExec->CloseLibrary(UtilityBase);

	IExec->DropInterface( (struct Interface *)IDOS );
	IExec->CloseLibrary(DOSBase);

	if (ApplicationBase)	IExec->CloseLibrary(ApplicationBase);
	if (LocaleBase)		IExec->CloseLibrary(LocaleBase);
}

int32 GetRoms(STRPTR romsdir)
{
	APTR context;
	int32 resul = 0;
	char rom[MAX_DOS_FILENAME];
	STRPTR pattern_ms = IExec->AllocVecTags(256, TAG_END);
//	       romfullpath = IExec->AllocVecTags(MAX_FULLFILEPATH, AVT_ClearWithValue,0, TAG_END);
DBUG("GetRoms() '%s'\n",romsdir);

	// Clear listbrowser list
	IListBrowser->FreeListBrowserList(romlist);

	IDOS->ParsePatternNoCase("#?.(zip|gb|gba)", pattern_ms, 64);
	//IUtility->Strlcpy(romfullpath, romsdir, MAX_FULLFILEPATH);
	//IDOS->AddPart(romfullpath, romsdir, MAX_FULLFILEPATH);
	context = IDOS->ObtainDirContextTags(EX_StringNameInput, romsdir,//romfullpath,
	                                     EX_DataFields, (EXF_NAME|EXF_TYPE),
	                                     EX_MatchString, pattern_ms,
	                                    TAG_END);
	if(context)
	{
		struct ExamineData *dat;
		struct Node *n;
		int32 len, newpos;
		char ext[4] = ""; // extension + '\0'

		while( (dat=IDOS->ExamineDir(context)) )
		{
			if( EXD_IS_FILE(dat) )
			{
				len = IUtility->Strlen(dat->Name) - sizeof(ext);
				newpos = IDOS->SplitName(dat->Name, '.', rom, len, MAX_DOS_FILENAME);
DBUG("  rom = '%s'\n",dat->Name);
				// Get name
				IUtility->Strlcpy(rom, dat->Name, newpos);
DBUG("    name = '%s'\n",rom);
				// Get extension
				ext[0] = IUtility->ToUpper( *(dat->Name + newpos) );
				ext[1] = IUtility->ToUpper( *(dat->Name + newpos + 1) );
				ext[2] = IUtility->ToUpper( *(dat->Name + newpos + 2) ); // can be '\0' -> 'GB'
DBUG("    extension = '%s'\n",ext);
				n = IListBrowser->AllocListBrowserNode(LAST_COL,
				                                       LBNA_Column,COL_ROM,
				                                         LBNCA_CopyText,TRUE, LBNCA_Text,rom,
				                                       LBNA_Column,COL_FMT,
				                                         LBNCA_CopyText,TRUE, LBNCA_Text,ext,
				                                         LBNCA_HorizJustify, LCJ_CENTER,
				                                      TAG_DONE);
				IExec->AddTail(romlist, n);
				resul++;
DBUG("  %4ld:'%s' [0x%08lx] added\n",resul,dat->Name,n);
			}
		}

		if(IDOS->IoErr() != ERROR_NO_MORE_ENTRIES)
		{
			IDOS->Fault(IDOS->IoErr(), NULL, rom, MAX_DOS_FILENAME);
			//DoMessage(rom, REQIMAGE_ERROR, NULL);
			IIntuition->SetAttrs(object[OID_REQ], REQ_BodyText,rom, TAG_DONE);
			IIntuition->IDoMethod(object[OID_REQ],RM_OPENREQ,NULL,window[WID_MAIN],NULL,TAG_DONE);
		}
	} // END if(context)
	else
	{
		IDOS->Fault(IDOS->IoErr(), NULL, pattern_ms, MAX_DOS_FILENAME); // 'pattern_ms' used as temp-buffer
		IUtility->SNPrintf(rom, 1024, GetString(&li,MSG_ERROR_ROMDRAWER),pattern_ms);
		//DoMessage(rom, REQIMAGE_ERROR, NULL);
		IIntuition->SetAttrs(object[OID_REQ], REQ_BodyText,rom, TAG_DONE);
		IIntuition->IDoMethod(object[OID_REQ],RM_OPENREQ,NULL,window[WID_MAIN],NULL,TAG_DONE);
	}

	IDOS->ReleaseDirContext(context);
	//IExec->FreeVec(romfullpath);
	IExec->FreeVec(pattern_ms);
DBUG("GetRoms() = %ld\n",resul);
	return resul;
}

BOOL loadConfigToMemory(void)
{
	int32 i, res_int;
	BPTR fhConfFile = IDOS->FOpen(CONFIG_INI, MODE_OLDFILE, 0);
DBUG("loadConfigToMemory()\n",NULL);

	if(fhConfFile != ZERO) {
		struct FReadLineData *frld = IDOS->AllocDosObjectTags(DOS_FREADLINEDATA, 0);

		cfg_file = (STRPTR)IExec->AllocVecTags(CFG_FILE_SIZE, AVT_ClearWithValue,0, TAG_END);

		while(IDOS->FReadLine(fhConfFile, frld) > 0) {
			if(frld->frld_LineLength > 1) {
//DBUG("Line (%ld bytes): %s", frld->frld_LineLength, frld->frld_Line);
				i = TOT_CFG;
				if(frld->frld_Line[0] != '#') { // skip lines starting with such char(s)
					char kw[32] = "";
					int32 pos = IDOS->SplitName( frld->frld_Line, '=', kw, 0, sizeof(kw) ); // get KEYWORD without VALUE ("keyUp = 1")

					for(i=0; i!=TOT_CFG; i++) {
						int32 len = IUtility->Strlen(kw);

						if(kw[len-1] == ' ') { --len; /*kw[len] = '\0';*/ }
//DBUG("  \"%s\" (%s) %ld\n",kw_array[i],kw,len);

						if( (len==IUtility->Strlen(kw_array[i]))
						   &&  (IUtility->Strnicmp(kw,kw_array[i],len)==0) ) {
							if(pos!=-1  &&  IDOS->StrToLong(frld->frld_Line+pos, &res_int)!=-1) {
DBUG("  keyword:[%ld] \"%s\"\n",i,kw_array[i]);
								cfg_value[i] = res_int - 1080; // 1080 -> https://www.ngemu.com/threads/baffling-joystick-configuration-problem.28555/
DBUG("    value: %ld\n",cfg_value[i]);
							}
							break; // exit FOR()
						}
					}
				} // END if(frld->frld_Line[0]!='#'..

				if(i == TOT_CFG) { IUtility->Strlcat(cfg_file, frld->frld_Line, CFG_FILE_SIZE); }
			} // END if(frld->frld_LineLength..
		} // END while(IDOS->FReadLine(f..

		IDOS->FreeDosObject(DOS_FREADLINEDATA, frld);
		IDOS->FClose(fhConfFile);
	} // END if(fhConfFile..

//DBUG("\n'%s'\n",cfg_file);
	return cfg_file? TRUE : FALSE;
}

void unloadConfigFromMemory(void)
{
DBUG("unloadConfigFromMemory()\n",NULL);
	IExec->FreeVec(cfg_file);
	cfg_file = NULL;
}

void saveToConfig(void)
{
	int32 i;
	BPTR fhConfFile = IDOS->FOpen(CONFIG_INI, MODE_NEWFILE, 0);
DBUG("saveToConfig() 0x%08lx\n",fhConfFile);

	if(fhConfFile != ZERO) {
		char buf[1024] = "";
//DBUG("\n'%s'\n",cfg_file);
		IDOS->FPuts(fhConfFile, cfg_file); // write not modified 'VisualBoyAdvance.cfg' settings

		// Add keywords and values to 'VisualBoyAdvance.cfg'
		for(i=0; i!=TOT_CFG; i++) {
DBUG("  %s",kw_array[i]);
DBUGN("=%ld\n",cfg_value[i]+1080);
			IUtility->SNPrintf(buf, sizeof(buf), "%s%s=%ld\n",buf,kw_array[i],cfg_value[i]+1080); // 1080 -> https://www.ngemu.com/threads/baffling-joystick-configuration-problem.28555/
		}
		// Write modified settings to 'config.ini'
		IDOS->FPuts(fhConfFile, buf);
		IDOS->FClose(fhConfFile);
	}
}
