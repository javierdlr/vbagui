#ifndef vbagui_CAT_H
#define vbagui_CAT_H


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/*
**  Prototypes
*/
#if !defined(__GNUC__)  && !defined(__SASC)  &&  !defined(_DCC)
extern VOID OpenvbaguiCatalog(VOID);
extern VOID ClosevbaguiCatalog(VOID);
#endif


struct FC_String {
    CONST_STRPTR msg;
    LONG id;
};

#define msg_about (vbagui_Strings[0].msg)
#define _msg_about (vbagui_Strings+0)
#define msg_visual (vbagui_Strings[1].msg)
#define _msg_visual (vbagui_Strings+1)
#define msg_game (vbagui_Strings[2].msg)
#define _msg_game (vbagui_Strings+2)
#define msg_debug (vbagui_Strings[3].msg)
#define _msg_debug (vbagui_Strings+3)
#define msg_videomode (vbagui_Strings[4].msg)
#define _msg_videomode (vbagui_Strings+4)
#define msg_fullscreen (vbagui_Strings[5].msg)
#define _msg_fullscreen (vbagui_Strings+5)
#define msg_yuv (vbagui_Strings[6].msg)
#define _msg_yuv (vbagui_Strings+6)
#define msg_yuvnone (vbagui_Strings[7].msg)
#define _msg_yuvnone (vbagui_Strings+7)
#define msg_filter (vbagui_Strings[8].msg)
#define _msg_filter (vbagui_Strings+8)
#define msg_filter1 (vbagui_Strings[9].msg)
#define _msg_filter1 (vbagui_Strings+9)
#define msg_filter2 (vbagui_Strings[10].msg)
#define _msg_filter2 (vbagui_Strings+10)
#define msg_filter3 (vbagui_Strings[11].msg)
#define _msg_filter3 (vbagui_Strings+11)
#define msg_filter4 (vbagui_Strings[12].msg)
#define _msg_filter4 (vbagui_Strings+12)
#define msg_filter5 (vbagui_Strings[13].msg)
#define _msg_filter5 (vbagui_Strings+13)
#define msg_filter6 (vbagui_Strings[14].msg)
#define _msg_filter6 (vbagui_Strings+14)
#define msg_filter7 (vbagui_Strings[15].msg)
#define _msg_filter7 (vbagui_Strings+15)
#define msg_filter8 (vbagui_Strings[16].msg)
#define _msg_filter8 (vbagui_Strings+16)
#define msg_filter9 (vbagui_Strings[17].msg)
#define _msg_filter9 (vbagui_Strings+17)
#define msg_filter10 (vbagui_Strings[18].msg)
#define _msg_filter10 (vbagui_Strings+18)
#define msg_filter11 (vbagui_Strings[19].msg)
#define _msg_filter11 (vbagui_Strings+19)
#define msg_filter12 (vbagui_Strings[20].msg)
#define _msg_filter12 (vbagui_Strings+20)
#define msg_filter13 (vbagui_Strings[21].msg)
#define _msg_filter13 (vbagui_Strings+21)
#define msg_filter14 (vbagui_Strings[22].msg)
#define _msg_filter14 (vbagui_Strings+22)
#define msg_fskip (vbagui_Strings[23].msg)
#define _msg_fskip (vbagui_Strings+23)
#define msg_fskauto (vbagui_Strings[24].msg)
#define _msg_fskauto (vbagui_Strings+24)
#define msg_fskcustom (vbagui_Strings[25].msg)
#define _msg_fskcustom (vbagui_Strings+25)
#define msg_fskdisab (vbagui_Strings[26].msg)
#define _msg_fskdisab (vbagui_Strings+26)
#define msg_throttle (vbagui_Strings[27].msg)
#define _msg_throttle (vbagui_Strings+27)
#define msg_ifb (vbagui_Strings[28].msg)
#define _msg_ifb (vbagui_Strings+28)
#define msg_ifb1 (vbagui_Strings[29].msg)
#define _msg_ifb1 (vbagui_Strings+29)
#define msg_ifb2 (vbagui_Strings[30].msg)
#define _msg_ifb2 (vbagui_Strings+30)
#define msg_ifb3 (vbagui_Strings[31].msg)
#define _msg_ifb3 (vbagui_Strings+31)
#define msg_pwi (vbagui_Strings[32].msg)
#define _msg_pwi (vbagui_Strings+32)
#define msg_show1 (vbagui_Strings[33].msg)
#define _msg_show1 (vbagui_Strings+33)
#define msg_show2 (vbagui_Strings[34].msg)
#define _msg_show2 (vbagui_Strings+34)
#define msg_show3 (vbagui_Strings[35].msg)
#define _msg_show3 (vbagui_Strings+35)
#define msg_exe (vbagui_Strings[36].msg)
#define _msg_exe (vbagui_Strings+36)
#define msg_exeASL (vbagui_Strings[37].msg)
#define _msg_exeASL (vbagui_Strings+37)
#define msg_gamefile (vbagui_Strings[38].msg)
#define _msg_gamefile (vbagui_Strings+38)
#define msg_gameASL (vbagui_Strings[39].msg)
#define _msg_gameASL (vbagui_Strings+39)
#define msg_biosfile (vbagui_Strings[40].msg)
#define _msg_biosfile (vbagui_Strings+40)
#define msg_biosASL (vbagui_Strings[41].msg)
#define _msg_biosASL (vbagui_Strings+41)
#define msg_flash (vbagui_Strings[42].msg)
#define _msg_flash (vbagui_Strings+42)
#define msg_flash1 (vbagui_Strings[43].msg)
#define _msg_flash1 (vbagui_Strings+43)
#define msg_flash2 (vbagui_Strings[44].msg)
#define _msg_flash2 (vbagui_Strings+44)
#define msg_savetype (vbagui_Strings[45].msg)
#define _msg_savetype (vbagui_Strings+45)
#define msg_savetype1 (vbagui_Strings[46].msg)
#define _msg_savetype1 (vbagui_Strings+46)
#define msg_savetype2 (vbagui_Strings[47].msg)
#define _msg_savetype2 (vbagui_Strings+47)
#define msg_savetype3 (vbagui_Strings[48].msg)
#define _msg_savetype3 (vbagui_Strings+48)
#define msg_savetype4 (vbagui_Strings[49].msg)
#define _msg_savetype4 (vbagui_Strings+49)
#define msg_savetype5 (vbagui_Strings[50].msg)
#define _msg_savetype5 (vbagui_Strings+50)
#define msg_savetype6 (vbagui_Strings[51].msg)
#define _msg_savetype6 (vbagui_Strings+51)
#define msg_ips (vbagui_Strings[52].msg)
#define _msg_ips (vbagui_Strings+52)
#define msg_ipsASL (vbagui_Strings[53].msg)
#define _msg_ipsASL (vbagui_Strings+53)
#define msg_mmx (vbagui_Strings[54].msg)
#define _msg_mmx (vbagui_Strings+54)
#define msg_rtc (vbagui_Strings[55].msg)
#define _msg_rtc (vbagui_Strings+55)
#define msg_endebug (vbagui_Strings[56].msg)
#define _msg_endebug (vbagui_Strings+56)
#define msg_verbose (vbagui_Strings[57].msg)
#define _msg_verbose (vbagui_Strings+57)
#define msg_verbose1 (vbagui_Strings[58].msg)
#define _msg_verbose1 (vbagui_Strings+58)
#define msg_verbose2 (vbagui_Strings[59].msg)
#define _msg_verbose2 (vbagui_Strings+59)
#define msg_verbose3 (vbagui_Strings[60].msg)
#define _msg_verbose3 (vbagui_Strings+60)
#define msg_verbose4 (vbagui_Strings[61].msg)
#define _msg_verbose4 (vbagui_Strings+61)
#define msg_verbose5 (vbagui_Strings[62].msg)
#define _msg_verbose5 (vbagui_Strings+62)
#define msg_verbose6 (vbagui_Strings[63].msg)
#define _msg_verbose6 (vbagui_Strings+63)
#define msg_verbose7 (vbagui_Strings[64].msg)
#define _msg_verbose7 (vbagui_Strings+64)
#define msg_verbose8 (vbagui_Strings[65].msg)
#define _msg_verbose8 (vbagui_Strings+65)
#define msg_verbose9 (vbagui_Strings[66].msg)
#define _msg_verbose9 (vbagui_Strings+66)
#define msg_verbose10 (vbagui_Strings[67].msg)
#define _msg_verbose10 (vbagui_Strings+67)
#define msg_gdbproto (vbagui_Strings[68].msg)
#define _msg_gdbproto (vbagui_Strings+68)
#define msg_gdbproto1 (vbagui_Strings[69].msg)
#define _msg_gdbproto1 (vbagui_Strings+69)
#define msg_gdbproto2 (vbagui_Strings[70].msg)
#define _msg_gdbproto2 (vbagui_Strings+70)
#define msg_gdbproto3 (vbagui_Strings[71].msg)
#define _msg_gdbproto3 (vbagui_Strings+71)
#define msg_agbprint (vbagui_Strings[72].msg)
#define _msg_agbprint (vbagui_Strings+72)
#define msg_save (vbagui_Strings[73].msg)
#define _msg_save (vbagui_Strings+73)
#define msg_launch (vbagui_Strings[74].msg)
#define _msg_launch (vbagui_Strings+74)
#define msg_exit (vbagui_Strings[75].msg)
#define _msg_exit (vbagui_Strings+75)
#define msg_warning (vbagui_Strings[76].msg)
#define _msg_warning (vbagui_Strings+76)
#define msg_warningtxt (vbagui_Strings[77].msg)
#define _msg_warningtxt (vbagui_Strings+77)
#define msg_ok (vbagui_Strings[78].msg)
#define _msg_ok (vbagui_Strings+78)
#define msg_bigabout (vbagui_Strings[79].msg)
#define _msg_bigabout (vbagui_Strings+79)


#endif
