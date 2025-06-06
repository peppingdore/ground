#pragma once

#include "../grd_reflect.h"

#define GRD_WINDOWS_MESSAGE_CODE_LIST\
	X(_WM_NULL, 0x0000)\
	X(_WM_CREATE, 0x0001)\
	X(_WM_DESTROY, 0x0002)\
	X(_WM_MOVE, 0x0003)\
	X(_WM_SIZE, 0x0005)\
	X(_WM_ACTIVATE, 0x0006)\
	X(_WM_SETFOCUS, 0x0007)\
	X(_WM_KILLFOCUS, 0x0008)\
	X(_WM_ENABLE, 0x000a)\
	X(_WM_SETREDRAW, 0x000b)\
	X(_WM_SETTEXT, 0x000c)\
	X(_WM_GETTEXT, 0x000d)\
	X(_WM_GETTEXTLENGTH, 0x000e)\
	X(_WM_PAINT, 0x000f)\
	X(_WM_CLOSE, 0x0010)\
	X(_WM_QUERYENDSESSION, 0x0011)\
	X(_WM_QUIT, 0x0012)\
	X(_WM_QUERYOPEN, 0x0013)\
	X(_WM_ERASEBKGND, 0x0014)\
	X(_WM_SYSCOLORCHANGE, 0x0015)\
	X(_WM_ENDSESSION, 0x0016)\
	X(_WM_SHOWWINDOW, 0x0018)\
	X(_WM_CTLCOLOR, 0x0019)\
	X(_WM_WININICHANGE, 0x001a)\
	X(_WM_DEVMODECHANGE, 0x001b)\
	X(_WM_ACTIVATEAPP, 0x001c)\
	X(_WM_FONTCHANGE, 0x001d)\
	X(_WM_TIMECHANGE, 0x001e)\
	X(_WM_CANCELMODE, 0x001f)\
	X(_WM_SETCURSOR, 0x0020)\
	X(_WM_MOUSEACTIVATE, 0x0021)\
	X(_WM_CHILDACTIVATE, 0x0022)\
	X(_WM_QUEUESYNC, 0x0023)\
	X(_WM_GETMINMAXINFO, 0x0024)\
	X(_WM_PAINTICON, 0x0026)\
	X(_WM_ICONERASEBKGND, 0x0027)\
	X(_WM_NEXTDLGCTL, 0x0028)\
	X(_WM_SPOOLERSTATUS, 0x002a)\
	X(_WM_DRAWITEM, 0x002b)\
	X(_WM_MEASUREITEM, 0x002c)\
	X(_WM_DELETEITEM, 0x002d)\
	X(_WM_VKEYTOITEM, 0x002e)\
	X(_WM_CHARTOITEM, 0x002f)\
	X(_WM_SETFONT, 0x0030)\
	X(_WM_GETFONT, 0x0031)\
	X(_WM_SETHOTKEY, 0x0032)\
	X(_WM_GETHOTKEY, 0x0033)\
	X(_WM_QUERYDRAGICON, 0x0037)\
	X(_WM_COMPAREITEM, 0x0039)\
	X(_WM_GETOBJECT, 0x003d)\
	X(_WM_COMPACTING, 0x0041)\
	X(_WM_COMMNOTIFY, 0x0044)\
	X(_WM_WINDOWPOSCHANGING, 0x0046)\
	X(_WM_WINDOWPOSCHANGED, 0x0047)\
	X(_WM_POWER, 0x0048)\
	X(_WM_COPYGLOBALDATA, 0x0049)\
	X(_WM_COPYDATA, 0x004a)\
	X(_WM_CANCELJOURNAL, 0x004b)\
	X(_WM_NOTIFY, 0x004e)\
	X(_WM_INPUTLANGCHANGEREQUEST, 0x0050)\
	X(_WM_INPUTLANGCHANGE, 0x0051)\
	X(_WM_TCARD, 0x0052)\
	X(_WM_HELP, 0x0053)\
	X(_WM_USERCHANGED, 0x0054)\
	X(_WM_NOTIFYFORMAT, 0x0055)\
	X(_WM_CONTEXTMENU, 0x007b)\
	X(_WM_STYLECHANGING, 0x007c)\
	X(_WM_STYLECHANGED, 0x007d)\
	X(_WM_DISPLAYCHANGE, 0x007e)\
	X(_WM_GETICON, 0x007f)\
	X(_WM_SETICON, 0x0080)\
	X(_WM_NCCREATE, 0x0081)\
	X(_WM_NCDESTROY, 0x0082)\
	X(_WM_NCCALCSIZE, 0x0083)\
	X(_WM_NCHITTEST, 0x0084)\
	X(_WM_NCPAINT, 0x0085)\
	X(_WM_NCACTIVATE, 0x0086)\
	X(_WM_GETDLGCODE, 0x0087)\
	X(_WM_SYNCPAINT, 0x0088)\
	X(_WM_NCMOUSEMOVE, 0x00a0)\
	X(_WM_NCLBUTTONDOWN, 0x00a1)\
	X(_WM_NCLBUTTONUP, 0x00a2)\
	X(_WM_NCLBUTTONDBLCLK, 0x00a3)\
	X(_WM_NCRBUTTONDOWN, 0x00a4)\
	X(_WM_NCRBUTTONUP, 0x00a5)\
	X(_WM_NCRBUTTONDBLCLK, 0x00a6)\
	X(_WM_NCMBUTTONDOWN, 0x00a7)\
	X(_WM_NCMBUTTONUP, 0x00a8)\
	X(_WM_NCMBUTTONDBLCLK, 0x00a9)\
	X(_WM_NCXBUTTONDOWN, 0x00ab)\
	X(_WM_NCXBUTTONUP, 0x00ac)\
	X(_WM_NCXBUTTONDBLCLK, 0x00ad)\
	X(_EM_GETSEL, 0x00b0)\
	X(_EM_SETSEL, 0x00b1)\
	X(_EM_GETRECT, 0x00b2)\
	X(_EM_SETRECT, 0x00b3)\
	X(_EM_SETRECTNP, 0x00b4)\
	X(_EM_SCROLL, 0x00b5)\
	X(_EM_LINESCROLL, 0x00b6)\
	X(_EM_SCROLLCARET, 0x00b7)\
	X(_EM_GETMODIFY, 0x00b8)\
	X(_EM_SETMODIFY, 0x00b9)\
	X(_EM_GETLINECOUNT, 0x00ba)\
	X(_EM_LINEINDEX, 0x00bb)\
	X(_EM_SETHANDLE, 0x00bc)\
	X(_EM_GETHANDLE, 0x00bd)\
	X(_EM_GETTHUMB, 0x00be)\
	X(_EM_LINELENGTH, 0x00c1)\
	X(_EM_REPLACESEL, 0x00c2)\
	X(_EM_SETFONT, 0x00c3)\
	X(_EM_GETLINE, 0x00c4)\
	X(_EM_LIMITTEXT, 0x00c5)\
	X(_EM_SETLIMITTEXT, 0x00c5)\
	X(_EM_CANUNDO, 0x00c6)\
	X(_EM_UNDO, 0x00c7)\
	X(_EM_FMTLINES, 0x00c8)\
	X(_EM_LINEFROMCHAR, 0x00c9)\
	X(_EM_SETWORDBREAK, 0x00ca)\
	X(_EM_SETTABSTOPS, 0x00cb)\
	X(_EM_SETPASSWORDCHAR, 0x00cc)\
	X(_EM_EMPTYUNDOBUFFER, 0x00cd)\
	X(_EM_GETFIRSTVISIBLELINE, 0x00ce)\
	X(_EM_SETREADONLY, 0x00cf)\
	X(_EM_SETWORDBREAKPROC, 0x00d0)\
	X(_EM_GETWORDBREAKPROC, 0x00d1)\
	X(_EM_GETPASSWORDCHAR, 0x00d2)\
	X(_EM_SETMARGINS, 0x00d3)\
	X(_EM_GETMARGINS, 0x00d4)\
	X(_EM_GETLIMITTEXT, 0x00d5)\
	X(_EM_POSFROMCHAR, 0x00d6)\
	X(_EM_CHARFROMPOS, 0x00d7)\
	X(_EM_SETIMESTATUS, 0x00d8)\
	X(_EM_GETIMESTATUS, 0x00d9)\
	X(_SBM_SETPOS, 0x00e0)\
	X(_SBM_GETPOS, 0x00e1)\
	X(_SBM_SETRANGE, 0x00e2)\
	X(_SBM_GETRANGE, 0x00e3)\
	X(_SBM_ENABLE_ARROWS, 0x00e4)\
	X(_SBM_SETRANGEREDRAW, 0x00e6)\
	X(_SBM_SETSCROLLINFO, 0x00e9)\
	X(_SBM_GETSCROLLINFO, 0x00ea)\
	X(_SBM_GETSCROLLBARINFO, 0x00eb)\
	X(_BM_GETCHECK, 0x00f0)\
	X(_BM_SETCHECK, 0x00f1)\
	X(_BM_GETSTATE, 0x00f2)\
	X(_BM_SETSTATE, 0x00f3)\
	X(_BM_SETSTYLE, 0x00f4)\
	X(_BM_CLICK, 0x00f5)\
	X(_BM_GETIMAGE, 0x00f6)\
	X(_BM_SETIMAGE, 0x00f7)\
	X(_BM_SETDONTCLICK, 0x00f8)\
	X(_WM_INPUT, 0x00ff)\
	X(_WM_KEYDOWN, 0x0100)\
	X(_WM_KEYFIRST, 0x0100)\
	X(_WM_KEYUP, 0x0101)\
	X(_WM_CHAR, 0x0102)\
	X(_WM_DEADCHAR, 0x0103)\
	X(_WM_SYSKEYDOWN, 0x0104)\
	X(_WM_SYSKEYUP, 0x0105)\
	X(_WM_SYSCHAR, 0x0106)\
	X(_WM_SYSDEADCHAR, 0x0107)\
	X(_WM_UNICHAR, 0x0109)\
	X(_WM_WNT_CONVERTREQUESTEX, 0x0109)\
	X(_WM_CONVERTREQUEST, 0x010a)\
	X(_WM_CONVERTRESULT, 0x010b)\
	X(_WM_INTERIM, 0x010c)\
	X(_WM_IME_STARTCOMPOSITION, 0x010d)\
	X(_WM_IME_ENDCOMPOSITION, 0x010e)\
	X(_WM_IME_COMPOSITION, 0x010f)\
	X(_WM_IME_KEYLAST, 0x010f)\
	X(_WM_INITDIALOG, 0x0110)\
	X(_WM_COMMAND, 0x0111)\
	X(_WM_SYSCOMMAND, 0x0112)\
	X(_WM_TIMER, 0x0113)\
	X(_WM_HSCROLL, 0x0114)\
	X(_WM_VSCROLL, 0x0115)\
	X(_WM_INITMENU, 0x0116)\
	X(_WM_INITMENUPOPUP, 0x0117)\
	X(_WM_SYSTIMER, 0x0118)\
	X(_WM_MENUSELECT, 0x011f)\
	X(_WM_MENUCHAR, 0x0120)\
	X(_WM_ENTERIDLE, 0x0121)\
	X(_WM_MENURBUTTONUP, 0x0122)\
	X(_WM_MENUDRAG, 0x0123)\
	X(_WM_MENUGETOBJECT, 0x0124)\
	X(_WM_UNINITMENUPOPUP, 0x0125)\
	X(_WM_MENUCOMMAND, 0x0126)\
	X(_WM_CHANGEUISTATE, 0x0127)\
	X(_WM_UPDATEUISTATE, 0x0128)\
	X(_WM_QUERYUISTATE, 0x0129)\
	X(_WM_LBTRACKPOINT, 0x0131)\
	X(_WM_CTLCOLORMSGBOX, 0x0132)\
	X(_WM_CTLCOLOREDIT, 0x0133)\
	X(_WM_CTLCOLORLISTBOX, 0x0134)\
	X(_WM_CTLCOLORBTN, 0x0135)\
	X(_WM_CTLCOLORDLG, 0x0136)\
	X(_WM_CTLCOLORSCROLLBAR, 0x0137)\
	X(_WM_CTLCOLORSTATIC, 0x0138)\
	X(_CB_GETEDITSEL, 0x0140)\
	X(_CB_LIMITTEXT, 0x0141)\
	X(_CB_SETEDITSEL, 0x0142)\
	X(_CB_ADDSTRING, 0x0143)\
	X(_CB_DELETESTRING, 0x0144)\
	X(_CB_DIR, 0x0145)\
	X(_CB_GETCOUNT, 0x0146)\
	X(_CB_GETCURSEL, 0x0147)\
	X(_CB_GETLBTEXT, 0x0148)\
	X(_CB_GETLBTEXTLEN, 0x0149)\
	X(_CB_INSERTSTRING, 0x014a)\
	X(_CB_RESETCONTENT, 0x014b)\
	X(_CB_FINDSTRING, 0x014c)\
	X(_CB_SELECTSTRING, 0x014d)\
	X(_CB_SETCURSEL, 0x014e)\
	X(_CB_SHOWDROPDOWN, 0x014f)\
	X(_CB_GETITEMDATA, 0x0150)\
	X(_CB_SETITEMDATA, 0x0151)\
	X(_CB_GETDROPPEDCONTROLRECT, 0x0152)\
	X(_CB_SETITEMHEIGHT, 0x0153)\
	X(_CB_GETITEMHEIGHT, 0x0154)\
	X(_CB_SETEXTENDEDUI, 0x0155)\
	X(_CB_GETEXTENDEDUI, 0x0156)\
	X(_CB_GETDROPPEDSTATE, 0x0157)\
	X(_CB_FINDSTRINGEXACT, 0x0158)\
	X(_CB_SETLOCALE, 0x0159)\
	X(_CB_GETLOCALE, 0x015a)\
	X(_CB_GETTOPINDEX, 0x015b)\
	X(_CB_SETTOPINDEX, 0x015c)\
	X(_CB_GETHORIZONTALEXTENT, 0x015d)\
	X(_CB_SETHORIZONTALEXTENT, 0x015e)\
	X(_CB_GETDROPPEDWIDTH, 0x015f)\
	X(_CB_SETDROPPEDWIDTH, 0x0160)\
	X(_CB_INITSTORAGE, 0x0161)\
	X(_CB_MULTIPLEADDSTRING, 0x0163)\
	X(_CB_GETCOMBOBOXINFO, 0x0164)\
	X(_CB_MSGMAX, 0x0165)\
	X(_WM_MOUSEFIRST, 0x0200)\
	X(_WM_MOUSEMOVE, 0x0200)\
	X(_WM_LBUTTONDOWN, 0x0201)\
	X(_WM_LBUTTONUP, 0x0202)\
	X(_WM_LBUTTONDBLCLK, 0x0203)\
	X(_WM_RBUTTONDOWN, 0x0204)\
	X(_WM_RBUTTONUP, 0x0205)\
	X(_WM_RBUTTONDBLCLK, 0x0206)\
	X(_WM_MBUTTONDOWN, 0x0207)\
	X(_WM_MBUTTONUP, 0x0208)\
	X(_WM_MBUTTONDBLCLK, 0x0209)\
	X(_WM_MOUSELAST, 0x0209)\
	X(_WM_MOUSEWHEEL, 0x020a)\
	X(_WM_XBUTTONDOWN, 0x020b)\
	X(_WM_XBUTTONUP, 0x020c)\
	X(_WM_XBUTTONDBLCLK, 0x020d)\
	X(_WM_MOUSEHWHEEL, 0x020e)\
	X(_WM_PARENTNOTIFY, 0x0210)\
	X(_WM_ENTERMENULOOP, 0x0211)\
	X(_WM_EXITMENULOOP, 0x0212)\
	X(_WM_NEXTMENU, 0x0213)\
	X(_WM_SIZING, 0x0214)\
	X(_WM_CAPTURECHANGED, 0x0215)\
	X(_WM_MOVING, 0x0216)\
	X(_WM_POWERBROADCAST, 0x0218)\
	X(_WM_DEVICECHANGE, 0x0219)\
	X(_WM_MDICREATE, 0x0220)\
	X(_WM_MDIDESTROY, 0x0221)\
	X(_WM_MDIACTIVATE, 0x0222)\
	X(_WM_MDIRESTORE, 0x0223)\
	X(_WM_MDINEXT, 0x0224)\
	X(_WM_MDIMAXIMIZE, 0x0225)\
	X(_WM_MDITILE, 0x0226)\
	X(_WM_MDICASCADE, 0x0227)\
	X(_WM_MDIICONARRANGE, 0x0228)\
	X(_WM_MDIGETACTIVE, 0x0229)\
	X(_WM_MDISETMENU, 0x0230)\
	X(_WM_ENTERSIZEMOVE, 0x0231)\
	X(_WM_EXITSIZEMOVE, 0x0232)\
	X(_WM_DROPFILES, 0x0233)\
	X(_WM_MDIREFRESHMENU, 0x0234)\
	X(_WM_IME_REPORT, 0x0280)\
	X(_WM_IME_SETCONTEXT, 0x0281)\
	X(_WM_IME_NOTIFY, 0x0282)\
	X(_WM_IME_CONTROL, 0x0283)\
	X(_WM_IME_COMPOSITIONFULL, 0x0284)\
	X(_WM_IME_SELECT, 0x0285)\
	X(_WM_IME_CHAR, 0x0286)\
	X(_WM_IME_REQUEST, 0x0288)\
	X(_WM_IMEKEYDOWN, 0x0290)\
	X(_WM_IME_KEYDOWN, 0x0290)\
	X(_WM_IMEKEYUP, 0x0291)\
	X(_WM_IME_KEYUP, 0x0291)\
	X(_WM_NCMOUSEHOVER, 0x02a0)\
	X(_WM_MOUSEHOVER, 0x02a1)\
	X(_WM_NCMOUSELEAVE, 0x02a2)\
	X(_WM_MOUSELEAVE, 0x02a3)\
	X(_WM_CUT, 0x0300)\
	X(_WM_COPY, 0x0301)\
	X(_WM_PASTE, 0x0302)\
	X(_WM_CLEAR, 0x0303)\
	X(_WM_UNDO, 0x0304)\
	X(_WM_RENDERFORMAT, 0x0305)\
	X(_WM_RENDERALLFORMATS, 0x0306)\
	X(_WM_DESTROYCLIPBOARD, 0x0307)\
	X(_WM_DRAWCLIPBOARD, 0x0308)\
	X(_WM_PAINTCLIPBOARD, 0x0309)\
	X(_WM_VSCROLLCLIPBOARD, 0x030a)\
	X(_WM_SIZECLIPBOARD, 0x030b)\
	X(_WM_ASKCBFORMATNAME, 0x030c)\
	X(_WM_CHANGECBCHAIN, 0x030d)\
	X(_WM_HSCROLLCLIPBOARD, 0x030e)\
	X(_WM_QUERYNEWPALETTE, 0x030f)\
	X(_WM_PALETTEISCHANGING, 0x0310)\
	X(_WM_PALETTECHANGED, 0x0311)\
	X(_WM_HOTKEY, 0x0312)\
	X(_WM_PRINT, 0x0317)\
	X(_WM_PRINTCLIENT, 0x0318)\
	X(_WM_APPCOMMAND, 0x0319)\
	X(_WM_HANDHELDFIRST, 0x0358)\
	X(_WM_HANDHELDLAST, 0x035f)\
	X(_WM_AFXFIRST, 0x0360)\
	X(_WM_AFXLAST, 0x037f)\
	X(_WM_PENWINFIRST, 0x0380)\
	X(_WM_RCRESULT, 0x0381)\
	X(_WM_HOOKRCRESULT, 0x0382)\
	X(_WM_GLOBALRCCHANGE, 0x0383)\
	X(_WM_PENMISCINFO, 0x0383)\
	X(_WM_SKB, 0x0384)\
	X(_WM_HEDITCTL, 0x0385)\
	X(_WM_PENCTL, 0x0385)\
	X(_WM_PENMISC, 0x0386)\
	X(_WM_CTLINIT, 0x0387)\
	X(_WM_PENEVENT, 0x0388)\
	X(_WM_PENWINLAST, 0x038f)\
	X(_DDM_SETFMT, 0x0400)\
	X(_DM_GETDEFID, 0x0400)\
	X(_NIN_SELECT, 0x0400)\
	X(_TBM_GETPOS, 0x0400)\
	X(_WM_PSD_PAGESETUPDLG, 0x0400)\
	X(_WM_USER, 0x0400)\
	X(_CBEM_INSERTITEMA, 0x0401)\
	X(_DDM_DRAW, 0x0401)\
	X(_DM_SETDEFID, 0x0401)\
	X(_HKM_SETHOTKEY, 0x0401)\
	X(_PBM_SETRANGE, 0x0401)\
	X(_RB_INSERTBANDA, 0x0401)\
	X(_SB_SETTEXTA, 0x0401)\
	X(_TB_ENABLEBUTTON, 0x0401)\
	X(_TBM_GETRANGEMIN, 0x0401)\
	X(_TTM_ACTIVATE, 0x0401)\
	X(_WM_CHOOSEFONT_GETLOGFONT, 0x0401)\
	X(_WM_PSD_FULLPAGERECT, 0x0401)\
	X(_CBEM_SETIMAGELIST, 0x0402)\
	X(_DDM_CLOSE, 0x0402)\
	X(_DM_REPOSITION, 0x0402)\
	X(_HKM_GETHOTKEY, 0x0402)\
	X(_PBM_SETPOS, 0x0402)\
	X(_RB_DELETEBAND, 0x0402)\
	X(_SB_GETTEXTA, 0x0402)\
	X(_TB_CHECKBUTTON, 0x0402)\
	X(_TBM_GETRANGEMAX, 0x0402)\
	X(_WM_PSD_MINMARGINRECT, 0x0402)\
	X(_CBEM_GETIMAGELIST, 0x0403)\
	X(_DDM_BEGIN, 0x0403)\
	X(_HKM_SETRULES, 0x0403)\
	X(_PBM_DELTAPOS, 0x0403)\
	X(_RB_GETBARINFO, 0x0403)\
	X(_SB_GETTEXTLENGTHA, 0x0403)\
	X(_TBM_GETTIC, 0x0403)\
	X(_TB_PRESSBUTTON, 0x0403)\
	X(_TTM_SETDELAYTIME, 0x0403)\
	X(_WM_PSD_MARGINRECT, 0x0403)\
	X(_CBEM_GETITEMA, 0x0404)\
	X(_DDM_END, 0x0404)\
	X(_PBM_SETSTEP, 0x0404)\
	X(_RB_SETBARINFO, 0x0404)\
	X(_SB_SETPARTS, 0x0404)\
	X(_TB_HIDEBUTTON, 0x0404)\
	X(_TBM_SETTIC, 0x0404)\
	X(_TTM_ADDTOOLA, 0x0404)\
	X(_WM_PSD_GREEKTEXTRECT, 0x0404)\
	X(_CBEM_SETITEMA, 0x0405)\
	X(_PBM_STEPIT, 0x0405)\
	X(_TB_INDETERMINATE, 0x0405)\
	X(_TBM_SETPOS, 0x0405)\
	X(_TTM_DELTOOLA, 0x0405)\
	X(_WM_PSD_ENVSTAMPRECT, 0x0405)\
	X(_CBEM_GETCOMBOCONTROL, 0x0406)\
	X(_PBM_SETRANGE32, 0x0406)\
	X(_RB_SETBANDINFOA, 0x0406)\
	X(_SB_GETPARTS, 0x0406)\
	X(_TB_MARKBUTTON, 0x0406)\
	X(_TBM_SETRANGE, 0x0406)\
	X(_TTM_NEWTOOLRECTA, 0x0406)\
	X(_WM_PSD_YAFULLPAGERECT, 0x0406)\
	X(_CBEM_GETEDITCONTROL, 0x0407)\
	X(_PBM_GETRANGE, 0x0407)\
	X(_RB_SETPARENT, 0x0407)\
	X(_SB_GETBORDERS, 0x0407)\
	X(_TBM_SETRANGEMIN, 0x0407)\
	X(_TTM_RELAYEVENT, 0x0407)\
	X(_CBEM_SETEXSTYLE, 0x0408)\
	X(_PBM_GETPOS, 0x0408)\
	X(_RB_HITTEST, 0x0408)\
	X(_SB_SETMINHEIGHT, 0x0408)\
	X(_TBM_SETRANGEMAX, 0x0408)\
	X(_TTM_GETTOOLINFOA, 0x0408)\
	X(_CBEM_GETEXSTYLE, 0x0409)\
	X(_CBEM_GETEXTENDEDSTYLE, 0x0409)\
	X(_PBM_SETBARCOLOR, 0x0409)\
	X(_RB_GETRECT, 0x0409)\
	X(_SB_SIMPLE, 0x0409)\
	X(_TB_ISBUTTONENABLED, 0x0409)\
	X(_TBM_CLEARTICS, 0x0409)\
	X(_TTM_SETTOOLINFOA, 0x0409)\
	X(_CBEM_HASEDITCHANGED, 0x040a)\
	X(_RB_INSERTBANDW, 0x040a)\
	X(_SB_GETRECT, 0x040a)\
	X(_TB_ISBUTTONCHECKED, 0x040a)\
	X(_TBM_SETSEL, 0x040a)\
	X(_TTM_HITTESTA, 0x040a)\
	X(_WIZ_QUERYNUMPAGES, 0x040a)\
	X(_CBEM_INSERTITEMW, 0x040b)\
	X(_RB_SETBANDINFOW, 0x040b)\
	X(_SB_SETTEXTW, 0x040b)\
	X(_TB_ISBUTTONPRESSED, 0x040b)\
	X(_TBM_SETSELSTART, 0x040b)\
	X(_TTM_GETTEXTA, 0x040b)\
	X(_WIZ_NEXT, 0x040b)\
	X(_CBEM_SETITEMW, 0x040c)\
	X(_RB_GETBANDCOUNT, 0x040c)\
	X(_SB_GETTEXTLENGTHW, 0x040c)\
	X(_TB_ISBUTTONHIDDEN, 0x040c)\
	X(_TBM_SETSELEND, 0x040c)\
	X(_TTM_UPDATETIPTEXTA, 0x040c)\
	X(_WIZ_PREV, 0x040c)\
	X(_CBEM_GETITEMW, 0x040d)\
	X(_RB_GETROWCOUNT, 0x040d)\
	X(_SB_GETTEXTW, 0x040d)\
	X(_TB_ISBUTTONINDETERMINATE, 0x040d)\
	X(_TTM_GETTOOLCOUNT, 0x040d)\
	X(_CBEM_SETEXTENDEDSTYLE, 0x040e)\
	X(_RB_GETROWHEIGHT, 0x040e)\
	X(_SB_ISSIMPLE, 0x040e)\
	X(_TB_ISBUTTONHIGHLIGHTED, 0x040e)\
	X(_TBM_GETPTICS, 0x040e)\
	X(_TTM_ENUMTOOLSA, 0x040e)\
	X(_SB_SETICON, 0x040f)\
	X(_TBM_GETTICPOS, 0x040f)\
	X(_TTM_GETCURRENTTOOLA, 0x040f)\
	X(_RB_IDTOINDEX, 0x0410)\
	X(_SB_SETTIPTEXTA, 0x0410)\
	X(_TBM_GETNUMTICS, 0x0410)\
	X(_TTM_WINDOWFROMPOINT, 0x0410)\
	X(_RB_GETTOOLTIPS, 0x0411)\
	X(_SB_SETTIPTEXTW, 0x0411)\
	X(_TBM_GETSELSTART, 0x0411)\
	X(_TB_SETSTATE, 0x0411)\
	X(_TTM_TRACKACTIVATE, 0x0411)\
	X(_RB_SETTOOLTIPS, 0x0412)\
	X(_SB_GETTIPTEXTA, 0x0412)\
	X(_TB_GETSTATE, 0x0412)\
	X(_TBM_GETSELEND, 0x0412)\
	X(_TTM_TRACKPOSITION, 0x0412)\
	X(_RB_SETBKCOLOR, 0x0413)\
	X(_SB_GETTIPTEXTW, 0x0413)\
	X(_TB_ADDBITMAP, 0x0413)\
	X(_TBM_CLEARSEL, 0x0413)\
	X(_TTM_SETTIPBKCOLOR, 0x0413)\
	X(_RB_GETBKCOLOR, 0x0414)\
	X(_SB_GETICON, 0x0414)\
	X(_TB_ADDBUTTONSA, 0x0414)\
	X(_TBM_SETTICFREQ, 0x0414)\
	X(_TTM_SETTIPTEXTCOLOR, 0x0414)\
	X(_RB_SETTEXTCOLOR, 0x0415)\
	X(_TB_INSERTBUTTONA, 0x0415)\
	X(_TBM_SETPAGESIZE, 0x0415)\
	X(_TTM_GETDELAYTIME, 0x0415)\
	X(_RB_GETTEXTCOLOR, 0x0416)\
	X(_TB_DELETEBUTTON, 0x0416)\
	X(_TBM_GETPAGESIZE, 0x0416)\
	X(_TTM_GETTIPBKCOLOR, 0x0416)\
	X(_RB_SIZETORECT, 0x0417)\
	X(_TB_GETBUTTON, 0x0417)\
	X(_TBM_SETLINESIZE, 0x0417)\
	X(_TTM_GETTIPTEXTCOLOR, 0x0417)\
	X(_RB_BEGINDRAG, 0x0418)\
	X(_TB_BUTTONCOUNT, 0x0418)\
	X(_TBM_GETLINESIZE, 0x0418)\
	X(_TTM_SETMAXTIPWIDTH, 0x0418)\
	X(_RB_ENDDRAG, 0x0419)\
	X(_TB_COMMANDTOINDEX, 0x0419)\
	X(_TBM_GETTHUMBRECT, 0x0419)\
	X(_TTM_GETMAXTIPWIDTH, 0x0419)\
	X(_RB_DRAGMOVE, 0x041a)\
	X(_TBM_GETCHANNELRECT, 0x041a)\
	X(_TB_SAVERESTOREA, 0x041a)\
	X(_TTM_SETMARGIN, 0x041a)\
	X(_RB_GETBARHEIGHT, 0x041b)\
	X(_TB_CUSTOMIZE, 0x041b)\
	X(_TBM_SETTHUMBLENGTH, 0x041b)\
	X(_TTM_GETMARGIN, 0x041b)\
	X(_RB_GETBANDINFOW, 0x041c)\
	X(_TB_ADDSTRINGA, 0x041c)\
	X(_TBM_GETTHUMBLENGTH, 0x041c)\
	X(_TTM_POP, 0x041c)\
	X(_RB_GETBANDINFOA, 0x041d)\
	X(_TB_GETITEMRECT, 0x041d)\
	X(_TBM_SETTOOLTIPS, 0x041d)\
	X(_TTM_UPDATE, 0x041d)\
	X(_RB_MINIMIZEBAND, 0x041e)\
	X(_TB_BUTTONSTRUCTSIZE, 0x041e)\
	X(_TBM_GETTOOLTIPS, 0x041e)\
	X(_TTM_GETBUBBLESIZE, 0x041e)\
	X(_RB_MAXIMIZEBAND, 0x041f)\
	X(_TBM_SETTIPSIDE, 0x041f)\
	X(_TB_SETBUTTONSIZE, 0x041f)\
	X(_TTM_ADJUSTRECT, 0x041f)\
	X(_TBM_SETBUDDY, 0x0420)\
	X(_TB_SETBITMAPSIZE, 0x0420)\
	X(_TTM_SETTITLEA, 0x0420)\
	X(_MSG_FTS_JUMP_VA, 0x0421)\
	X(_TB_AUTOSIZE, 0x0421)\
	X(_TBM_GETBUDDY, 0x0421)\
	X(_TTM_SETTITLEW, 0x0421)\
	X(_RB_GETBANDBORDERS, 0x0422)\
	X(_MSG_FTS_JUMP_QWORD, 0x0423)\
	X(_RB_SHOWBAND, 0x0423)\
	X(_TB_GETTOOLTIPS, 0x0423)\
	X(_MSG_REINDEX_REQUEST, 0x0424)\
	X(_TB_SETTOOLTIPS, 0x0424)\
	X(_MSG_FTS_WHERE_IS_IT, 0x0425)\
	X(_RB_SETPALETTE, 0x0425)\
	X(_TB_SETPARENT, 0x0425)\
	X(_RB_GETPALETTE, 0x0426)\
	X(_RB_MOVEBAND, 0x0427)\
	X(_TB_SETROWS, 0x0427)\
	X(_TB_GETROWS, 0x0428)\
	X(_TB_GETBITMAPFLAGS, 0x0429)\
	X(_TB_SETCMDID, 0x042a)\
	X(_RB_PUSHCHEVRON, 0x042b)\
	X(_TB_CHANGEBITMAP, 0x042b)\
	X(_TB_GETBITMAP, 0x042c)\
	X(_MSG_GET_DEFFONT, 0x042d)\
	X(_TB_GETBUTTONTEXTA, 0x042d)\
	X(_TB_REPLACEBITMAP, 0x042e)\
	X(_TB_SETINDENT, 0x042f)\
	X(_TB_SETIMAGELIST, 0x0430)\
	X(_TB_GETIMAGELIST, 0x0431)\
	X(_TB_LOADIMAGES, 0x0432)\
	X(_EM_CANPASTE, 0x0432)\
	X(_TTM_ADDTOOLW, 0x0432)\
	X(_EM_DISPLAYBAND, 0x0433)\
	X(_TB_GETRECT, 0x0433)\
	X(_TTM_DELTOOLW, 0x0433)\
	X(_EM_EXGETSEL, 0x0434)\
	X(_TB_SETHOTIMAGELIST, 0x0434)\
	X(_TTM_NEWTOOLRECTW, 0x0434)\
	X(_EM_EXLIMITTEXT, 0x0435)\
	X(_TB_GETHOTIMAGELIST, 0x0435)\
	X(_TTM_GETTOOLINFOW, 0x0435)\
	X(_EM_EXLINEFROMCHAR, 0x0436)\
	X(_TB_SETDISABLEDIMAGELIST, 0x0436)\
	X(_TTM_SETTOOLINFOW, 0x0436)\
	X(_EM_EXSETSEL, 0x0437)\
	X(_TB_GETDISABLEDIMAGELIST, 0x0437)\
	X(_TTM_HITTESTW, 0x0437)\
	X(_EM_FINDTEXT, 0x0438)\
	X(_TB_SETSTYLE, 0x0438)\
	X(_TTM_GETTEXTW, 0x0438)\
	X(_EM_FORMATRANGE, 0x0439)\
	X(_TB_GETSTYLE, 0x0439)\
	X(_TTM_UPDATETIPTEXTW, 0x0439)\
	X(_EM_GETCHARFORMAT, 0x043a)\
	X(_TB_GETBUTTONSIZE, 0x043a)\
	X(_TTM_ENUMTOOLSW, 0x043a)\
	X(_EM_GETEVENTMASK, 0x043b)\
	X(_TB_SETBUTTONWIDTH, 0x043b)\
	X(_TTM_GETCURRENTTOOLW, 0x043b)\
	X(_EM_GETOLEINTERFACE, 0x043c)\
	X(_TB_SETMAXTEXTROWS, 0x043c)\
	X(_EM_GETPARAFORMAT, 0x043d)\
	X(_TB_GETTEXTROWS, 0x043d)\
	X(_EM_GETSELTEXT, 0x043e)\
	X(_TB_GETOBJECT, 0x043e)\
	X(_EM_HIDESELECTION, 0x043f)\
	X(_TB_GETBUTTONINFOW, 0x043f)\
	X(_EM_PASTESPECIAL, 0x0440)\
	X(_TB_SETBUTTONINFOW, 0x0440)\
	X(_EM_REQUESTRESIZE, 0x0441)\
	X(_TB_GETBUTTONINFOA, 0x0441)\
	X(_EM_SELECTIONTYPE, 0x0442)\
	X(_TB_SETBUTTONINFOA, 0x0442)\
	X(_EM_SETBKGNDCOLOR, 0x0443)\
	X(_TB_INSERTBUTTONW, 0x0443)\
	X(_EM_SETCHARFORMAT, 0x0444)\
	X(_TB_ADDBUTTONSW, 0x0444)\
	X(_EM_SETEVENTMASK, 0x0445)\
	X(_TB_HITTEST, 0x0445)\
	X(_EM_SETOLECALLBACK, 0x0446)\
	X(_TB_SETDRAWTEXTFLAGS, 0x0446)\
	X(_EM_SETPARAFORMAT, 0x0447)\
	X(_TB_GETHOTITEM, 0x0447)\
	X(_EM_SETTARGETDEVICE, 0x0448)\
	X(_TB_SETHOTITEM, 0x0448)\
	X(_EM_STREAMIN, 0x0449)\
	X(_TB_SETANCHORHIGHLIGHT, 0x0449)\
	X(_EM_STREAMOUT, 0x044a)\
	X(_TB_GETANCHORHIGHLIGHT, 0x044a)\
	X(_EM_GETTEXTRANGE, 0x044b)\
	X(_TB_GETBUTTONTEXTW, 0x044b)\
	X(_EM_FINDWORDBREAK, 0x044c)\
	X(_TB_SAVERESTOREW, 0x044c)\
	X(_EM_SETOPTIONS, 0x044d)\
	X(_TB_ADDSTRINGW, 0x044d)\
	X(_EM_GETOPTIONS, 0x044e)\
	X(_TB_MAPACCELERATORA, 0x044e)\
	X(_EM_FINDTEXTEX, 0x044f)\
	X(_TB_GETINSERTMARK, 0x044f)\
	X(_EM_GETWORDBREAKPROCEX, 0x0450)\
	X(_TB_SETINSERTMARK, 0x0450)\
	X(_EM_SETWORDBREAKPROCEX, 0x0451)\
	X(_TB_INSERTMARKHITTEST, 0x0451)\
	X(_EM_SETUNDOLIMIT, 0x0452)\
	X(_TB_MOVEBUTTON, 0x0452)\
	X(_TB_GETMAXSIZE, 0x0453)\
	X(_EM_REDO, 0x0454)\
	X(_TB_SETEXTENDEDSTYLE, 0x0454)\
	X(_EM_CANREDO, 0x0455)\
	X(_TB_GETEXTENDEDSTYLE, 0x0455)\
	X(_EM_GETUNDONAME, 0x0456)\
	X(_TB_GETPADDING, 0x0456)\
	X(_EM_GETREDONAME, 0x0457)\
	X(_TB_SETPADDING, 0x0457)\
	X(_EM_STOPGROUPTYPING, 0x0458)\
	X(_TB_SETINSERTMARKCOLOR, 0x0458)\
	X(_EM_SETTEXTMODE, 0x0459)\
	X(_TB_GETINSERTMARKCOLOR, 0x0459)\
	X(_EM_GETTEXTMODE, 0x045a)\
	X(_TB_MAPACCELERATORW, 0x045a)\
	X(_EM_AUTOURLDETECT, 0x045b)\
	X(_TB_GETSTRINGW, 0x045b)\
	X(_EM_GETAUTOURLDETECT, 0x045c)\
	X(_TB_GETSTRINGA, 0x045c)\
	X(_EM_SETPALETTE, 0x045d)\
	X(_EM_GETTEXTEX, 0x045e)\
	X(_EM_GETTEXTLENGTHEX, 0x045f)\
	X(_EM_SHOWSCROLLBAR, 0x0460)\
	X(_EM_SETTEXTEX, 0x0461)\
	X(_TAPI_REPLY, 0x0463)\
	X(_ACM_OPENA, 0x0464)\
	X(_BFFM_SETSTATUSTEXTA, 0x0464)\
	X(_CDM_FIRST, 0x0464)\
	X(_CDM_GETSPEC, 0x0464)\
	X(_EM_SETPUNCTUATION, 0x0464)\
	X(_IPM_CLEARADDRESS, 0x0464)\
	X(_WM_CAP_UNICODE_START, 0x0464)\
	X(_ACM_PLAY, 0x0465)\
	X(_BFFM_ENABLEOK, 0x0465)\
	X(_CDM_GETFILEPATH, 0x0465)\
	X(_EM_GETPUNCTUATION, 0x0465)\
	X(_IPM_SETADDRESS, 0x0465)\
	X(_PSM_SETCURSEL, 0x0465)\
	X(_UDM_SETRANGE, 0x0465)\
	X(_WM_CHOOSEFONT_SETLOGFONT, 0x0465)\
	X(_ACM_STOP, 0x0466)\
	X(_BFFM_SETSELECTIONA, 0x0466)\
	X(_CDM_GETFOLDERPATH, 0x0466)\
	X(_EM_SETWORDWRAPMODE, 0x0466)\
	X(_IPM_GETADDRESS, 0x0466)\
	X(_PSM_REMOVEPAGE, 0x0466)\
	X(_UDM_GETRANGE, 0x0466)\
	X(_WM_CAP_SET_CALLBACK_ERRORW, 0x0466)\
	X(_WM_CHOOSEFONT_SETFLAGS, 0x0466)\
	X(_ACM_OPENW, 0x0467)\
	X(_BFFM_SETSELECTIONW, 0x0467)\
	X(_CDM_GETFOLDERIDLIST, 0x0467)\
	X(_EM_GETWORDWRAPMODE, 0x0467)\
	X(_IPM_SETRANGE, 0x0467)\
	X(_PSM_ADDPAGE, 0x0467)\
	X(_UDM_SETPOS, 0x0467)\
	X(_WM_CAP_SET_CALLBACK_STATUSW, 0x0467)\
	X(_BFFM_SETSTATUSTEXTW, 0x0468)\
	X(_CDM_SETCONTROLTEXT, 0x0468)\
	X(_EM_SETIMECOLOR, 0x0468)\
	X(_IPM_SETFOCUS, 0x0468)\
	X(_PSM_CHANGED, 0x0468)\
	X(_UDM_GETPOS, 0x0468)\
	X(_CDM_HIDECONTROL, 0x0469)\
	X(_EM_GETIMECOLOR, 0x0469)\
	X(_IPM_ISBLANK, 0x0469)\
	X(_PSM_RESTARTWINDOWS, 0x0469)\
	X(_UDM_SETBUDDY, 0x0469)\
	X(_CDM_SETDEFEXT, 0x046a)\
	X(_EM_SETIMEOPTIONS, 0x046a)\
	X(_PSM_REBOOTSYSTEM, 0x046a)\
	X(_UDM_GETBUDDY, 0x046a)\
	X(_EM_GETIMEOPTIONS, 0x046b)\
	X(_PSM_CANCELTOCLOSE, 0x046b)\
	X(_UDM_SETACCEL, 0x046b)\
	X(_EM_CONVPOSITION, 0x046c)\
	X(_MCIWNDM_GETZOOM, 0x046d)\
	X(_PSM_UNCHANGED, 0x046d)\
	X(_UDM_SETBASE, 0x046d)\
	X(_PSM_APPLY, 0x046e)\
	X(_UDM_GETBASE, 0x046e)\
	X(_PSM_SETTITLEA, 0x046f)\
	X(_UDM_SETRANGE32, 0x046f)\
	X(_PSM_SETWIZBUTTONS, 0x0470)\
	X(_UDM_GETRANGE32, 0x0470)\
	X(_WM_CAP_DRIVER_GET_NAMEW, 0x0470)\
	X(_PSM_PRESSBUTTON, 0x0471)\
	X(_UDM_SETPOS32, 0x0471)\
	X(_WM_CAP_DRIVER_GET_VERSIONW, 0x0471)\
	X(_PSM_SETCURSELID, 0x0472)\
	X(_UDM_GETPOS32, 0x0472)\
	X(_PSM_SETFINISHTEXTA, 0x0473)\
	X(_PSM_GETTABCONTROL, 0x0474)\
	X(_PSM_ISDIALOGMESSAGE, 0x0475)\
	X(_MCIWNDM_REALIZE, 0x0476)\
	X(_PSM_GETCURRENTPAGEHWND, 0x0476)\
	X(_MCIWNDM_SETTIMEFORMATA, 0x0477)\
	X(_PSM_INSERTPAGE, 0x0477)\
	X(_EM_SETLANGOPTIONS, 0x0478)\
	X(_MCIWNDM_GETTIMEFORMATA, 0x0478)\
	X(_PSM_SETTITLEW, 0x0478)\
	X(_WM_CAP_FILE_SET_CAPTURE_FILEW, 0x0478)\
	X(_EM_GETLANGOPTIONS, 0x0479)\
	X(_MCIWNDM_VALIDATEMEDIA, 0x0479)\
	X(_PSM_SETFINISHTEXTW, 0x0479)\
	X(_WM_CAP_FILE_GET_CAPTURE_FILEW, 0x0479)\
	X(_EM_GETIMECOMPMODE, 0x047a)\
	X(_EM_FINDTEXTW, 0x047b)\
	X(_MCIWNDM_PLAYTO, 0x047b)\
	X(_WM_CAP_FILE_SAVEASW, 0x047b)\
	X(_EM_FINDTEXTEXW, 0x047c)\
	X(_MCIWNDM_GETFILENAMEA, 0x047c)\
	X(_EM_RECONVERSION, 0x047d)\
	X(_MCIWNDM_GETDEVICEA, 0x047d)\
	X(_PSM_SETHEADERTITLEA, 0x047d)\
	X(_WM_CAP_FILE_SAVEDIBW, 0x047d)\
	X(_EM_SETIMEMODEBIAS, 0x047e)\
	X(_MCIWNDM_GETPALETTE, 0x047e)\
	X(_PSM_SETHEADERTITLEW, 0x047e)\
	X(_EM_GETIMEMODEBIAS, 0x047f)\
	X(_MCIWNDM_SETPALETTE, 0x047f)\
	X(_PSM_SETHEADERSUBTITLEA, 0x047f)\
	X(_MCIWNDM_GETERRORA, 0x0480)\
	X(_PSM_SETHEADERSUBTITLEW, 0x0480)\
	X(_PSM_HWNDTOINDEX, 0x0481)\
	X(_PSM_INDEXTOHWND, 0x0482)\
	X(_MCIWNDM_SETINACTIVETIMER, 0x0483)\
	X(_PSM_PAGETOINDEX, 0x0483)\
	X(_PSM_INDEXTOPAGE, 0x0484)\
	X(_DL_BEGINDRAG, 0x0485)\
	X(_MCIWNDM_GETINACTIVETIMER, 0x0485)\
	X(_PSM_IDTOINDEX, 0x0485)\
	X(_DL_DRAGGING, 0x0486)\
	X(_PSM_INDEXTOID, 0x0486)\
	X(_DL_DROPPED, 0x0487)\
	X(_PSM_GETRESULT, 0x0487)\
	X(_DL_CANCELDRAG, 0x0488)\
	X(_PSM_RECALCPAGESIZES, 0x0488)\
	X(_MCIWNDM_GET_SOURCE, 0x048c)\
	X(_MCIWNDM_PUT_SOURCE, 0x048d)\
	X(_MCIWNDM_GET_DEST, 0x048e)\
	X(_MCIWNDM_PUT_DEST, 0x048f)\
	X(_MCIWNDM_CAN_PLAY, 0x0490)\
	X(_MCIWNDM_CAN_WINDOW, 0x0491)\
	X(_MCIWNDM_CAN_RECORD, 0x0492)\
	X(_MCIWNDM_CAN_SAVE, 0x0493)\
	X(_MCIWNDM_CAN_EJECT, 0x0494)\
	X(_MCIWNDM_CAN_CONFIG, 0x0495)\
	X(_IE_GETINK, 0x0496)\
	X(_IE_MSGFIRST, 0x0496)\
	X(_MCIWNDM_PALETTEKICK, 0x0496)\
	X(_IE_SETINK, 0x0497)\
	X(_IE_GETPENTIP, 0x0498)\
	X(_IE_SETPENTIP, 0x0499)\
	X(_IE_GETERASERTIP, 0x049a)\
	X(_IE_SETERASERTIP, 0x049b)\
	X(_IE_GETBKGND, 0x049c)\
	X(_IE_SETBKGND, 0x049d)\
	X(_IE_GETGRIDORIGIN, 0x049e)\
	X(_IE_SETGRIDORIGIN, 0x049f)\
	X(_IE_GETGRIDPEN, 0x04a0)\
	X(_IE_SETGRIDPEN, 0x04a1)\
	X(_IE_GETGRIDSIZE, 0x04a2)\
	X(_IE_SETGRIDSIZE, 0x04a3)\
	X(_IE_GETMODE, 0x04a4)\
	X(_IE_SETMODE, 0x04a5)\
	X(_IE_GETINKRECT, 0x04a6)\
	X(_WM_CAP_SET_MCI_DEVICEW, 0x04a6)\
	X(_WM_CAP_GET_MCI_DEVICEW, 0x04a7)\
	X(_WM_CAP_PAL_OPENW, 0x04b4)\
	X(_WM_CAP_PAL_SAVEW, 0x04b5)\
	X(_IE_GETAPPDATA, 0x04b8)\
	X(_IE_SETAPPDATA, 0x04b9)\
	X(_IE_GETDRAWOPTS, 0x04ba)\
	X(_IE_SETDRAWOPTS, 0x04bb)\
	X(_IE_GETFORMAT, 0x04bc)\
	X(_IE_SETFORMAT, 0x04bd)\
	X(_IE_GETINKINPUT, 0x04be)\
	X(_IE_SETINKINPUT, 0x04bf)\
	X(_IE_GETNOTIFY, 0x04c0)\
	X(_IE_SETNOTIFY, 0x04c1)\
	X(_IE_GETRECOG, 0x04c2)\
	X(_IE_SETRECOG, 0x04c3)\
	X(_IE_GETSECURITY, 0x04c4)\
	X(_IE_SETSECURITY, 0x04c5)\
	X(_IE_GETSEL, 0x04c6)\
	X(_IE_SETSEL, 0x04c7)\
	X(_CDM_LAST, 0x04c8)\
	X(_EM_SETBIDIOPTIONS, 0x04c8)\
	X(_IE_DOCOMMAND, 0x04c8)\
	X(_MCIWNDM_NOTIFYMODE, 0x04c8)\
	X(_EM_GETBIDIOPTIONS, 0x04c9)\
	X(_IE_GETCOMMAND, 0x04c9)\
	X(_EM_SETTYPOGRAPHYOPTIONS, 0x04ca)\
	X(_IE_GETCOUNT, 0x04ca)\
	X(_EM_GETTYPOGRAPHYOPTIONS, 0x04cb)\
	X(_IE_GETGESTURE, 0x04cb)\
	X(_MCIWNDM_NOTIFYMEDIA, 0x04cb)\
	X(_EM_SETEDITSTYLE, 0x04cc)\
	X(_IE_GETMENU, 0x04cc)\
	X(_EM_GETEDITSTYLE, 0x04cd)\
	X(_IE_GETPAINTDC, 0x04cd)\
	X(_MCIWNDM_NOTIFYERROR, 0x04cd)\
	X(_IE_GETPDEVENT, 0x04ce)\
	X(_IE_GETSELCOUNT, 0x04cf)\
	X(_IE_GETSELITEMS, 0x04d0)\
	X(_IE_GETSTYLE, 0x04d1)\
	X(_MCIWNDM_SETTIMEFORMATW, 0x04db)\
	X(_EM_OUTLINE, 0x04dc)\
	X(_MCIWNDM_GETTIMEFORMATW, 0x04dc)\
	X(_EM_GETSCROLLPOS, 0x04dd)\
	X(_EM_SETSCROLLPOS, 0x04de)\
	X(_EM_SETFONTSIZE, 0x04df)\
	X(_EM_GETZOOM, 0x04e0)\
	X(_MCIWNDM_GETFILENAMEW, 0x04e0)\
	X(_EM_SETZOOM, 0x04e1)\
	X(_MCIWNDM_GETDEVICEW, 0x04e1)\
	X(_EM_GETVIEWKIND, 0x04e2)\
	X(_EM_SETVIEWKIND, 0x04e3)\
	X(_EM_GETPAGE, 0x04e4)\
	X(_MCIWNDM_GETERRORW, 0x04e4)\
	X(_EM_SETPAGE, 0x04e5)\
	X(_EM_GETHYPHENATEINFO, 0x04e6)\
	X(_EM_SETHYPHENATEINFO, 0x04e7)\
	X(_EM_GETPAGEROTATE, 0x04eb)\
	X(_EM_SETPAGEROTATE, 0x04ec)\
	X(_EM_GETCTFMODEBIAS, 0x04ed)\
	X(_EM_SETCTFMODEBIAS, 0x04ee)\
	X(_EM_GETCTFOPENSTATUS, 0x04f0)\
	X(_EM_SETCTFOPENSTATUS, 0x04f1)\
	X(_EM_GETIMECOMPTEXT, 0x04f2)\
	X(_EM_ISIME, 0x04f3)\
	X(_EM_GETIMEPROPERTY, 0x04f4)\
	X(_EM_GETQUERYRTFOBJ, 0x050d)\
	X(_EM_SETQUERYRTFOBJ, 0x050e)\
	X(_FM_GETFOCUS, 0x0600)\
	X(_FM_GETDRIVEINFOA, 0x0601)\
	X(_FM_GETSELCOUNT, 0x0602)\
	X(_FM_GETSELCOUNTLFN, 0x0603)\
	X(_FM_GETFILESELA, 0x0604)\
	X(_FM_GETFILESELLFNA, 0x0605)\
	X(_FM_REFRESH_WINDOWS, 0x0606)\
	X(_FM_RELOAD_EXTENSIONS, 0x0607)\
	X(_FM_GETDRIVEINFOW, 0x0611)\
	X(_FM_GETFILESELW, 0x0614)\
	X(_FM_GETFILESELLFNW, 0x0615)\
	X(_WLX_WM_SAS, 0x0659)\
	X(_SM_GETSELCOUNT, 0x07e8)\
	X(_UM_GETSELCOUNT, 0x07e8)\
	X(_WM_CPL_LAUNCH, 0x07e8)\
	X(_SM_GETSERVERSELA, 0x07e9)\
	X(_UM_GETUSERSELA, 0x07e9)\
	X(_WM_CPL_LAUNCHED, 0x07e9)\
	X(_SM_GETSERVERSELW, 0x07ea)\
	X(_UM_GETUSERSELW, 0x07ea)\
	X(_SM_GETCURFOCUSA, 0x07eb)\
	X(_UM_GETGROUPSELA, 0x07eb)\
	X(_SM_GETCURFOCUSW, 0x07ec)\
	X(_UM_GETGROUPSELW, 0x07ec)\
	X(_SM_GETOPTIONS, 0x07ed)\
	X(_UM_GETCURFOCUSA, 0x07ed)\
	X(_UM_GETCURFOCUSW, 0x07ee)\
	X(_UM_GETOPTIONS, 0x07ef)\
	X(_UM_GETOPTIONS2, 0x07f0)\
	X(_LVM_FIRST, 0x1000)\
	X(_LVM_GETBKCOLOR, 0x1000)\
	X(_LVM_SETBKCOLOR, 0x1001)\
	X(_LVM_GETIMAGELIST, 0x1002)\
	X(_LVM_SETIMAGELIST, 0x1003)\
	X(_LVM_GETITEMCOUNT, 0x1004)\
	X(_LVM_GETITEMA, 0x1005)\
	X(_LVM_SETITEMA, 0x1006)\
	X(_LVM_INSERTITEMA, 0x1007)\
	X(_LVM_DELETEITEM, 0x1008)\
	X(_LVM_DELETEALLITEMS, 0x1009)\
	X(_LVM_GETCALLBACKMASK, 0x100a)\
	X(_LVM_SETCALLBACKMASK, 0x100b)\
	X(_LVM_GETNEXTITEM, 0x100c)\
	X(_LVM_FINDITEMA, 0x100d)\
	X(_LVM_GETITEMRECT, 0x100e)\
	X(_LVM_SETITEMPOSITION, 0x100f)\
	X(_LVM_GETITEMPOSITION, 0x1010)\
	X(_LVM_GETSTRINGWIDTHA, 0x1011)\
	X(_LVM_HITTEST, 0x1012)\
	X(_LVM_ENSUREVISIBLE, 0x1013)\
	X(_LVM_SCROLL, 0x1014)\
	X(_LVM_REDRAWITEMS, 0x1015)\
	X(_LVM_ARRANGE, 0x1016)\
	X(_LVM_EDITLABELA, 0x1017)\
	X(_LVM_GETEDITCONTROL, 0x1018)\
	X(_LVM_GETCOLUMNA, 0x1019)\
	X(_LVM_SETCOLUMNA, 0x101a)\
	X(_LVM_INSERTCOLUMNA, 0x101b)\
	X(_LVM_DELETECOLUMN, 0x101c)\
	X(_LVM_GETCOLUMNWIDTH, 0x101d)\
	X(_LVM_SETCOLUMNWIDTH, 0x101e)\
	X(_LVM_GETHEADER, 0x101f)\
	X(_LVM_CREATEDRAGIMAGE, 0x1021)\
	X(_LVM_GETVIEWRECT, 0x1022)\
	X(_LVM_GETTEXTCOLOR, 0x1023)\
	X(_LVM_SETTEXTCOLOR, 0x1024)\
	X(_LVM_GETTEXTBKCOLOR, 0x1025)\
	X(_LVM_SETTEXTBKCOLOR, 0x1026)\
	X(_LVM_GETTOPINDEX, 0x1027)\
	X(_LVM_GETCOUNTPERPAGE, 0x1028)\
	X(_LVM_GETORIGIN, 0x1029)\
	X(_LVM_UPDATE, 0x102a)\
	X(_LVM_SETITEMSTATE, 0x102b)\
	X(_LVM_GETITEMSTATE, 0x102c)\
	X(_LVM_GETITEMTEXTA, 0x102d)\
	X(_LVM_SETITEMTEXTA, 0x102e)\
	X(_LVM_SETITEMCOUNT, 0x102f)\
	X(_LVM_SORTITEMS, 0x1030)\
	X(_LVM_SETITEMPOSITION32, 0x1031)\
	X(_LVM_GETSELECTEDCOUNT, 0x1032)\
	X(_LVM_GETITEMSPACING, 0x1033)\
	X(_LVM_GETISEARCHSTRINGA, 0x1034)\
	X(_LVM_SETICONSPACING, 0x1035)\
	X(_LVM_SETEXTENDEDLISTVIEWSTYLE, 0x1036)\
	X(_LVM_GETEXTENDEDLISTVIEWSTYLE, 0x1037)\
	X(_LVM_GETSUBITEMRECT, 0x1038)\
	X(_LVM_SUBITEMHITTEST, 0x1039)\
	X(_LVM_SETCOLUMNORDERARRAY, 0x103a)\
	X(_LVM_GETCOLUMNORDERARRAY, 0x103b)\
	X(_LVM_SETHOTITEM, 0x103c)\
	X(_LVM_GETHOTITEM, 0x103d)\
	X(_LVM_SETHOTCURSOR, 0x103e)\
	X(_LVM_GETHOTCURSOR, 0x103f)\
	X(_LVM_APPROXIMATEVIEWRECT, 0x1040)\
	X(_LVM_SETWORKAREAS, 0x1041)\
	X(_LVM_GETSELECTIONMARK, 0x1042)\
	X(_LVM_SETSELECTIONMARK, 0x1043)\
	X(_LVM_SETBKIMAGEA, 0x1044)\
	X(_LVM_GETBKIMAGEA, 0x1045)\
	X(_LVM_GETWORKAREAS, 0x1046)\
	X(_LVM_SETHOVERTIME, 0x1047)\
	X(_LVM_GETHOVERTIME, 0x1048)\
	X(_LVM_GETNUMBEROFWORKAREAS, 0x1049)\
	X(_LVM_SETTOOLTIPS, 0x104a)\
	X(_LVM_GETITEMW, 0x104b)\
	X(_LVM_SETITEMW, 0x104c)\
	X(_LVM_INSERTITEMW, 0x104d)\
	X(_LVM_GETTOOLTIPS, 0x104e)\
	X(_LVM_FINDITEMW, 0x1053)\
	X(_LVM_GETSTRINGWIDTHW, 0x1057)\
	X(_LVM_GETCOLUMNW, 0x105f)\
	X(_LVM_SETCOLUMNW, 0x1060)\
	X(_LVM_INSERTCOLUMNW, 0x1061)\
	X(_LVM_GETITEMTEXTW, 0x1073)\
	X(_LVM_SETITEMTEXTW, 0x1074)\
	X(_LVM_GETISEARCHSTRINGW, 0x1075)\
	X(_LVM_EDITLABELW, 0x1076)\
	X(_LVM_GETBKIMAGEW, 0x108b)\
	X(_LVM_SETSELECTEDCOLUMN, 0x108c)\
	X(_LVM_SETTILEWIDTH, 0x108d)\
	X(_LVM_SETVIEW, 0x108e)\
	X(_LVM_GETVIEW, 0x108f)\
	X(_LVM_INSERTGROUP, 0x1091)\
	X(_LVM_SETGROUPINFO, 0x1093)\
	X(_LVM_GETGROUPINFO, 0x1095)\
	X(_LVM_REMOVEGROUP, 0x1096)\
	X(_LVM_MOVEGROUP, 0x1097)\
	X(_LVM_MOVEITEMTOGROUP, 0x109a)\
	X(_LVM_SETGROUPMETRICS, 0x109b)\
	X(_LVM_GETGROUPMETRICS, 0x109c)\
	X(_LVM_ENABLEGROUPVIEW, 0x109d)\
	X(_LVM_SORTGROUPS, 0x109e)\
	X(_LVM_INSERTGROUPSORTED, 0x109f)\
	X(_LVM_REMOVEALLGROUPS, 0x10a0)\
	X(_LVM_HASGROUP, 0x10a1)\
	X(_LVM_SETTILEVIEWINFO, 0x10a2)\
	X(_LVM_GETTILEVIEWINFO, 0x10a3)\
	X(_LVM_SETTILEINFO, 0x10a4)\
	X(_LVM_GETTILEINFO, 0x10a5)\
	X(_LVM_SETINSERTMARK, 0x10a6)\
	X(_LVM_GETINSERTMARK, 0x10a7)\
	X(_LVM_INSERTMARKHITTEST, 0x10a8)\
	X(_LVM_GETINSERTMARKRECT, 0x10a9)\
	X(_LVM_SETINSERTMARKCOLOR, 0x10aa)\
	X(_LVM_GETINSERTMARKCOLOR, 0x10ab)\
	X(_LVM_SETINFOTIP, 0x10ad)\
	X(_LVM_GETSELECTEDCOLUMN, 0x10ae)\
	X(_LVM_ISGROUPVIEWENABLED, 0x10af)\
	X(_LVM_GETOUTLINECOLOR, 0x10b0)\
	X(_LVM_SETOUTLINECOLOR, 0x10b1)\
	X(_LVM_CANCELEDITLABEL, 0x10b3)\
	X(_LVM_MAPINDEXTOID, 0x10b4)\
	X(_LVM_MAPIDTOINDEX, 0x10b5)\
	X(_LVM_ISITEMVISIBLE, 0x10b6)\
	X(_LVM_GETEMPTYTEXT, 0x10cc)\
	X(_LVM_GETFOOTERRECT, 0x10cd)\
	X(_LVM_GETFOOTERINFO, 0x10ce)\
	X(_LVM_GETFOOTERITEMRECT, 0x10cf)\
	X(_LVM_GETFOOTERITEM, 0x10d0)\
	X(_LVM_GETITEMINDEXRECT, 0x10d1)\
	X(_LVM_SETITEMINDEXSTATE, 0x10d2)\
	X(_LVM_GETNEXTITEMINDEX, 0x10d3)\
	X(_CCM_FIRST, 0x2000)\
	X(_OCM__BASE, 0x2000)\
	X(_CCM_SETBKCOLOR, 0x2001)\
	X(_CCM_SETCOLORSCHEME, 0x2002)\
	X(_CCM_GETCOLORSCHEME, 0x2003)\
	X(_CCM_GETDROPTARGET, 0x2004)\
	X(_CCM_SETUNICODEFORMAT, 0x2005)\
	X(_LVM_SETUNICODEFORMAT, 0x2005)\
	X(_CCM_GETUNICODEFORMAT, 0x2006)\
	X(_LVM_GETUNICODEFORMAT, 0x2006)\
	X(_CCM_SETVERSION, 0x2007)\
	X(_CCM_GETVERSION, 0x2008)\
	X(_CCM_SETNOTIFYWINDOW, 0x2009)\
	X(_CCM_SETWINDOWTHEME, 0x200b)\
	X(_CCM_DPISCALE, 0x200c)\
	X(_OCM_CTLCOLOR, 0x2019)\
	X(_OCM_DRAWITEM, 0x202b)\
	X(_OCM_MEASUREITEM, 0x202c)\
	X(_OCM_DELETEITEM, 0x202d)\
	X(_OCM_VKEYTOITEM, 0x202e)\
	X(_OCM_CHARTOITEM, 0x202f)\
	X(_OCM_COMPAREITEM, 0x2039)\
	X(_OCM_NOTIFY, 0x204e)\
	X(_OCM_COMMAND, 0x2111)\
	X(_OCM_HSCROLL, 0x2114)\
	X(_OCM_VSCROLL, 0x2115)\
	X(_OCM_CTLCOLORMSGBOX, 0x2132)\
	X(_OCM_CTLCOLOREDIT, 0x2133)\
	X(_OCM_CTLCOLORLISTBOX, 0x2134)\
	X(_OCM_CTLCOLORBTN, 0x2135)\
	X(_OCM_CTLCOLORDLG, 0x2136)\
	X(_OCM_CTLCOLORSCROLLBAR, 0x2137)\
	X(_OCM_CTLCOLORSTATIC, 0x2138)\
	X(_OCM_PARENTNOTIFY, 0x2200)\
	X(_WM_APP, 0x8000)\
	X(_WM_RASDIALEVENT, 0xcccd)\

#pragma push_macro("X")
enum GrdWindowsMessageCode {
	#define X(name, code) GRD_WIN##name = code,
	GRD_WINDOWS_MESSAGE_CODE_LIST
	#undef X
};

GRD_REFLECT(GrdWindowsMessageCode) {
	#define X(name, code) GRD_ENUM_VALUE(GRD_WIN##name);
	GRD_WINDOWS_MESSAGE_CODE_LIST
	#undef X
}
#pragma pop_macro("X")
