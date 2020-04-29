#pragma once

#define CM_FILE_SAVEAS	9072
#define CM_FILE_EXIT	9071
#define CM_FILE_OPEN	9070
#define CM_ABOUT        9069
#define CM_COMPILE      9068
#define CM_RUN          9067
#define CM_COMPILERUN   9066
#define CM_FILE_SAVE	9065
#define CM_FILE_NEW 	9064
#define CM_STARTCMD     9051
#define CM_RUNBAT       9050
#define CM_DEBUG        9049
#define CM_VVARI        9048
#define CM_GHELP        9046
#define CM_EDIT_PASTE	9045
#define CM_EDIT_COPY	9044
#define CM_EDIT_CUT	    9043
#define CM_EDIT_UNDO	9042
#define CM_FLSTB		9041
#define CM_GHTML		9040
#define CM_ASTYLE		9039
#define CM_WLARGE		9038
#define CM_WSMALL		9037 
#define CM_CFONT		9036
#define CM_GITHUB		9035
#define CM_WEBSITE		9034
#define CM_IMPORTSET	9033
#define CM_EXPORTSET	9032
#define CM_EDIT_FIND	9031
#define CM_DT			9030 
#define CM_COMPILEBTN	9028
#define CM_ADDBRA		9027

#define ID_STATUSBAR    4997
#define ID_TOOLBAR		4998

#define IDC_MAIN_TEXT   1001
#define IDC_LINE_NUM	1002
#define ID_COMPILEORDER	1003
#define ID_COMPILERES	1004

#define MARGIN_FOLD_INDEX 2 

#define KEY_PRESSED(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) 
