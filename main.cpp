#include <windows.h>
#include <bits/stdc++.h>
#include <commctrl.h>
#include <commdlg.h>
#include <io.h>
#include <conio.h>
#include <tchar.h>
#include "main.h"
#include "SciLexer.h"
#include "Scintilla.h"

using namespace std;
HWND hwnd;
string codealltmp = "";
int wordsizepos = 2;
int wsizes[16] = {4,8,11,12,14,16,18,20,22,24,30,36,48,60,72,96};
string fontname = "Consolas";
bool fsaved=0, fopend=0, fcompiled=0;
bool programmeexiterrorstatusflag = 1;
unsigned long long variMsgCnt = 0;
HINSTANCE g_hInst;
char szFileName[MAX_PATH*10]="Untitled";
HWND g_hStatusBar, g_hToolBar;
bool hasstartopenfile = 0;
char commandline[MAX_PATH*10] = "";
string lasttimestr;
POINT cursorpoint;
const char* g_szKeywords="asm auto bool break case catch char class const const_cast continue default delete do double dynamic_cast else enum explicit extern false finally float for friend goto if inline int long mutable namespace new operator private protected public register reinterpret_cast register return short signed sizeof static static_cast struct switch template this throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while";

string output_time() {
	time_t rawtime;
   	time(&rawtime);   
   	char pblgtime[20];
	strftime(pblgtime, 20, "%Y-%m-%d %H-%M-%S", localtime(&rawtime));
	string tttmps="";
	tttmps.insert(0, pblgtime);
	return tttmps;
}

void Addinfo(char info[]) {
	char nativetext[100000];
	GetDlgItemText(hwnd, ID_COMPILERES, nativetext, 100000);
	string aftertext;
	aftertext.clear();
	aftertext += "-------------------------\r\n";
	aftertext += "Time: ";
	aftertext += output_time();
	aftertext += "\r\nInfo: ";
	aftertext += info;
	aftertext += "\r\n\r\n";
	aftertext += nativetext;
	SetDlgItemText(hwnd, ID_COMPILERES, aftertext.c_str());
}

void SafeGetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo)  
    {  
        if (NULL==lpSystemInfo)    return;  
        typedef VOID (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);  
        LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress( GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");;  
        if (NULL != fnGetNativeSystemInfo)  
        {  
            fnGetNativeSystemInfo(lpSystemInfo);  
        }  
        else  
        {  
            GetSystemInfo(lpSystemInfo);  
        }  
    } 
BOOL runprocess(char szCommandLine[], int fwait, int fshow) {
	BOOL ret = system(szCommandLine);
	
	/*
	if (fshow && (!fwait)) {
		::ShellExecute(hwnd, "open", szCommandLine, NULL, NULL, SW_SHOWMAXIMIZED);
		return 0;
	}
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION  pi; 
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.wShowWindow = fshow?TRUE:FALSE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	MessageBox(0, szCommandLine, "Caption", 0);
	BOOL ret = ::CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	//if (fwait) {
		BOOL wret2 = ::WaitForSingleObject(pi.hProcess, INFINITE);
	//}
	char errorname[1000];
	sprintf(errorname, "ERROR:ret=%d; LastError=%d; WaitRet=%d", ret, GetLastError(), wret2);
	MessageBox(0, errorname, "", 0);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	*/
	return ret;
}


BOOL LoadFile(HWND hEdit, LPSTR pszFileName) {
	HANDLE hFile;
	BOOL bSuccess = FALSE;

	hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(hFile != INVALID_HANDLE_VALUE) {
		DWORD dwFileSize;
		dwFileSize = GetFileSize(hFile, NULL);
		if(dwFileSize != 0xFFFFFFFF) {
			LPSTR pszFileText;
			pszFileText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
			if(pszFileText != NULL) {
				DWORD dwRead;
				if(ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
					pszFileText[dwFileSize] = 0; // Null terminator
					if(/*SetWindowText(hEdit, pszFileText)*/SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETTEXT, SCI_NULL, (LPARAM)pszFileText)) {
						bSuccess = TRUE; // It worked!
						fopend=1;
						SendMessage(g_hStatusBar, SB_SETTEXT, 4, (LPARAM)szFileName); 
					}
				}
				GlobalFree(pszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL SaveFile(HWND hEdit, LPSTR pszFileName) {
	HANDLE hFile;
	BOOL bSuccess = FALSE;

	hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile != INVALID_HANDLE_VALUE) {
		DWORD dwTextLength;
		dwTextLength = GetWindowTextLength(hEdit);
		if(dwTextLength > 0) {
			LPSTR pszText;
			pszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
			if(pszText != NULL) {
				if(GetWindowText(hEdit, pszText, dwTextLength + 1)) {
					DWORD dwWritten;
					dwTextLength = SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_GETLENGTH, SCI_NULL, SCI_NULL);
					SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_GETTEXT, dwTextLength+1, (LPARAM)pszText);
					if(WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL)) {
						bSuccess = TRUE;
						fsaved=1;
					}
				}
				GlobalFree(pszText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL DoFileOpenSave(HWND hwnd, BOOL bSave) {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	szFileName[0] = 0;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = (bSave ? "C++ Files (*.cpp; *.c++)\0*.cpp;*.c++\0C++ Header Files (*.hpp)\0*.hpp\0Pascal Files (*.pp)\0*.pp\0Windows命令脚本 (*.bat; *.cmd)\0*.bat;*.cmd\0All Files (*.*)\0*.*\0\0" : "C++ Files (*.cpp; *.c++; *.cxx)\0*.cpp;*.c++;*.cxx\0C++ Header Files (*.hpp)\0*.hpp\0Pascal Files (*.pp)\0*.pp\0Windows命令脚本 (*.bat; *.cmd)\0*.bat;*.com;*.cmd\0ClickIDE Temporary Compilation Logs\0*_compile_tmp.log\0All Files (*.*)\0*.*\0\0");
 	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "";

	if(bSave) {
		ofn.Flags = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
		if(GetSaveFileName(&ofn)) {
			if(!SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
				MessageBox(hwnd, "Save file failed.\n(Or this is an empty file.)", "Error",MB_OK|MB_ICONEXCLAMATION);
				fsaved=0;
				return FALSE;
			}
		} else {
			return FALSE;
		}
	} else {
		ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
		if(GetOpenFileName(&ofn)) {
			if(!LoadFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
				MessageBox(hwnd, "Load of file failed.", "Error",MB_OK|MB_ICONEXCLAMATION);
				fopend=0;
				return FALSE;
			}
		} else {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL DoFileOpen(HWND hwnd, char rt[]) {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	rt[0] = 0;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "配置设置(*.ini)\0*.ini\0\0";
	ofn.lpstrFile = rt;
	ofn.nMaxFile = MAX_PATH*4;
	ofn.lpstrDefExt = "";	
	ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	if(GetOpenFileName(&ofn)) {
		;
	} else {
		return FALSE;
	}
	return TRUE;
}
string getcppfn (char yufn[]) {
	string rtttmp = "";
	for (int i = 0; i < strlen(yufn) - 4; i++) {
		rtttmp += yufn[i];
	}
	return rtttmp;
}

string i_to_str(int int_num) {
	string rt;
	char tmp[100];
	sprintf(tmp, "%d", int_num);
	rt.clear();
	rt+=tmp;
	return rt;
}


LRESULT __stdcall SendEditor(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) {
	return SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), Msg, wParam, lParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	HMENU hMenu, hFileMenu, hCompileMenu;
	ifstream wndfin;
	bool GHELPEXITFLAG = 0;
	string errreporti="";
	string titlestr01="Click 5.0 ";
	int errreportcnt = 0;
	char cmdbuf1[MAX_PATH+40];
	char cmdbuf2[MAX_PATH+40];
	char cmdbuf3[MAX_PATH+40];
	char cmdbuf4[MAX_PATH+40];
	char cmdbuf5[MAX_PATH+40];
	int iStatusWidths[] = {100, 230, 300, 320, -1};
	RECT rectStatus;
	bool isycl = 0;
	bool iszfc = 0;
	bool islfst = 1;
	bool nlycl = 0;
	bool issgzs = 0; /*//*/ //a
	bool ismtzs = 0;
	bool dontout = 0;
	RECT rctA; //定义一个RECT结构体，存储窗口的长宽高
	int wwidth = 1000, wheight = 600;
	ofstream fout;
	HFONT hFont;
	HFONT hFont_ln;
	FINDREPLACE repfindtag;
	char getallcodetmpstr[200000];
	int linecount = 0;
	string linenumtmptext;
	int inttmpnum;
	string compileordertmp;
	char szFileNametmp[MAX_PATH*10];
	char compileresult[50000];
	char szFileName2[MAX_PATH*10];
	/*4.8-- 
	if (tosetcur) {
		SetCaretPos((cursorpoint.x-6)/(wsizes[wordsizepos]/2.0), (cursorpoint.y-2)/wsizes[wordsizepos]);
		tosetcur=0;
	}
	--4.8*/ 
	switch(Message) {
		case WM_CREATE:
			GetWindowRect(hwnd,&rctA);//通过窗口句柄获得窗口的大小存储在rctA结构中
			wwidth = rctA.right - rctA.left;
			wheight = rctA.bottom - rctA.top;
			CreateWindow("Scintilla", "",WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_BORDER,60, 30, wwidth-60/*CW_USEDEFAULT*/, wheight-240,hwnd, (HMENU)IDC_MAIN_TEXT, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Welcome\nto\nClickIDE!\n\nVersion:\n5.0.0",WS_CHILD|WS_VISIBLE|WS_BORDER,0, 30, 60/*CW_USEDEFAULT*/, wheight-120,hwnd, (HMENU)IDC_LINE_NUM, GetModuleHandle(NULL), NULL);
			CreateWindow("EDIT", "g++.exe %f -o %e",WS_CHILD|WS_VISIBLE|WS_BORDER,60, wheight-115, wwidth-60/*CW_USEDEFAULT*/, 25,hwnd, (HMENU)ID_COMPILEORDER, GetModuleHandle(NULL), NULL);
			CreateWindow("EDIT", "",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|WS_VSCROLL|ES_WANTRETURN,60, wheight-210, wwidth-60/*CW_USEDEFAULT*/, 95,hwnd, (HMENU)ID_COMPILERES, GetModuleHandle(NULL), NULL);
			
			/*4.7*/hFont = CreateFont(wsizes[wordsizepos],0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,fontname.c_str());//创建字体
			/*4.7*/hFont_ln = CreateFont(14,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Consolas");//创建字体
			
			/*4.7*/SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
			/*4.7*/SendDlgItemMessage(hwnd, IDC_LINE_NUM, WM_SETFONT,(WPARAM)hFont_ln/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
			///*4.7*/SendDlgItemMessage(hwnd, IDC_QUICKFUNC, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
			
			/*5.0*/hFont = CreateFont(18,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Consolas");//创建字体
			SendDlgItemMessage(hwnd, ID_COMPILEORDER, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
			/*5.0*/hFont = CreateFont(15,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Consolas");//创建字体
			SendDlgItemMessage(hwnd, ID_COMPILERES, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
			
			/*3.10: Statusbar*/
			g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
			WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
			hwnd, (HMENU)ID_STATUSBAR, g_hInst, NULL);
			
			SendMessage(g_hStatusBar, SB_SETPARTS, 5, (LPARAM)iStatusWidths);
			SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)"Click 5.0 IDE"); 
			SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
			SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "已编译" : "未编译")); 
			SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)""); 
			SendMessage(g_hStatusBar, SB_SETTEXT, 4, (LPARAM)szFileName); 
			/*--3.10*/
			
			/*4.7: ToolBar*/
			TBADDBITMAP tbab;
			TBBUTTON tbb[14];
			g_hToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
			WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
			hwnd, (HMENU)ID_TOOLBAR, g_hInst, NULL);
			/*--4.7*/
			
			/*
			***
			*4.7 Tool Bar
			*Add Bitmap
			***
			*/
			SendMessage(g_hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			
			tbab.hInst = HINST_COMMCTRL;
			tbab.nID = IDB_STD_SMALL_COLOR;
			SendMessage(g_hToolBar, TB_ADDBITMAP, 0, (LPARAM)&tbab);
			
			ZeroMemory(tbb, sizeof(tbb));
			
			tbb[0].iBitmap = STD_FILENEW;
			tbb[0].fsState = TBSTATE_ENABLED;
			tbb[0].fsStyle = TBSTYLE_BUTTON;
			tbb[0].idCommand = CM_FILE_NEW;
			
			tbb[1].iBitmap = STD_FILEOPEN;
			tbb[1].fsState = TBSTATE_ENABLED;
			tbb[1].fsStyle = TBSTYLE_BUTTON;
			tbb[1].idCommand = CM_FILE_OPEN;
			
			tbb[2].iBitmap = STD_FILESAVE;
			tbb[2].fsState = TBSTATE_ENABLED;
			tbb[2].fsStyle = TBSTYLE_BUTTON;
			tbb[2].idCommand = CM_FILE_SAVE;
			
			tbb[3].fsStyle = TBSTYLE_SEP;
			
			tbb[4].iBitmap = STD_CUT;
			tbb[4].fsState = TBSTATE_ENABLED;
			tbb[4].fsStyle = TBSTYLE_BUTTON;
			tbb[4].idCommand = CM_EDIT_CUT;
			
			tbb[5].iBitmap = STD_COPY;
			tbb[5].fsState = TBSTATE_ENABLED;
			tbb[5].fsStyle = TBSTYLE_BUTTON;
			tbb[5].idCommand = CM_EDIT_COPY;
			
			tbb[6].iBitmap = STD_PASTE;
			tbb[6].fsState = TBSTATE_ENABLED;
			tbb[6].fsStyle = TBSTYLE_BUTTON;
			tbb[6].idCommand = CM_EDIT_PASTE;
			
			tbb[7].fsStyle = TBSTYLE_SEP;
			
			tbb[8].iBitmap = STD_UNDO;
			tbb[8].fsState = TBSTATE_ENABLED;
			tbb[8].fsStyle = TBSTYLE_BUTTON;
			tbb[8].idCommand = CM_EDIT_UNDO;
			
			tbb[9].iBitmap = STD_FIND;
			tbb[9].fsState = TBSTATE_ENABLED;
			tbb[9].fsStyle = TBSTYLE_BUTTON;
			tbb[9].idCommand = CM_EDIT_FIND;
			
			tbb[10].fsStyle = TBSTYLE_SEP;
			
			tbb[11].iBitmap = STD_HELP;
			tbb[11].fsState = TBSTATE_ENABLED;
			tbb[11].fsStyle = TBSTYLE_BUTTON;
			tbb[11].idCommand = CM_ABOUT;
			
			tbb[12].fsStyle = TBSTYLE_SEP;
			
			tbb[13].iBitmap = STD_DELETE;
			tbb[13].fsState = TBSTATE_ENABLED;
			tbb[13].fsStyle = TBSTYLE_BUTTON;
			tbb[13].idCommand = CM_FILE_EXIT;
			
			SendMessage(g_hToolBar, TB_ADDBUTTONS, 14, (LPARAM)&tbb);
			/*
			*--4.7
			*/
			if (hasstartopenfile) {
				LoadFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), commandline);
				strcpy(szFileName, commandline);
				SendMessage(g_hStatusBar, SB_SETTEXT, 4, (LPARAM)szFileName); 
			}
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETLEXER, SCLEX_CPP, NULL); //C++语法解析
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETKEYWORDS, 0, (sptr_t)g_szKeywords);//设置关键字
			// 下面设置各种语法元素前景色
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_STRING, RGB(10, 0, 255));//字符串
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_CHARACTER, RGB(91, 74, 68));//字符
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_NUMBER, RGB(144, 49, 150));//字符
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_PREPROCESSOR, RGB(84, 181, 153));//预编译开关
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_OPERATOR, RGB(255, 0, 0));//运算符
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENT, RGB(2, 122, 216));//块注释
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENTLINE, RGB(2, 122, 216));//行注释
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENTDOC, RGB(2, 122, 250));//文档注释（/**开头）
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCARETLINEVISIBLE, TRUE, 0);
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCARETLINEBACK, 0xb0ffff, 0);
			
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETMARGINTYPEN,0,(sptr_t)SC_MARGIN_NUMBER);
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETMARGINWIDTHN,0,(sptr_t)40);
			
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_CLEARCMDKEY, (WPARAM)('F'+(SCMOD_CTRL<<16)), SCI_NULL);
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCODEPAGE, SC_CP_UTF8, SCI_NULL);
			
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT),SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Consolas");
			SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT),SCI_STYLESETSIZE, STYLE_DEFAULT,(sptr_t)wsizes[wordsizepos]);
			
			SendEditor(SCI_SETTABWIDTH, 4, 0);//tab：4个空格 
			/*
			    SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"0"); 
   
    			SendEditor(SCI_SETMARGINTYPEN, MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);//页边类型 
    			SendEditor(SCI_SETMARGINMASKN, MARGIN_FOLD_INDEX, SC_MASK_FOLDERS); //页边掩码 
    			SendEditor(SCI_SETMARGINWIDTHN, MARGIN_FOLD_INDEX, 11); //页边宽度 
    			SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_FOLD_INDEX, TRUE); //响应鼠标消息 
   				
    			// 折叠标签样式 
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);  
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_CIRCLEMINUS);  
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_CIRCLEPLUSCONNECTED); 
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED); 
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE); 
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);  
    			SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE); 
   
    			// 折叠标签颜色 
    			SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0); 
    			SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0); 
    			SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0); 
   				
    			SendEditor(SCI_SETFOLDFLAGS, 16|4, 0); //如果折叠就在折叠行的上下各画一条横线 
			*/
			
			return 0;
			break;
		case WM_SIZE:
			RECT rectClient, rectStatus, rectTool;
			UINT uToolHeight, uStatusHeight, uClientAlreaHeight;
			
			GetWindowRect(hwnd,&rctA);//通过窗口句柄获得窗口的大小存储在rctA结构中
			wwidth = rctA.right - rctA.left;
			wheight = rctA.bottom - rctA.top;
			if(wParam != SIZE_MINIMIZED) {
				MoveWindow(GetDlgItem(hwnd, IDC_MAIN_TEXT), 60, 30, /*LOWORD(lParam)*/wwidth-75,/*HIWORD(lParam)*/wheight-240, TRUE);
				MoveWindow(GetDlgItem(hwnd, IDC_LINE_NUM), 0, 30, /*LOWORD(lParam)*/60,/*HIWORD(lParam)*/wheight-120, TRUE);
				MoveWindow(GetDlgItem(hwnd, ID_COMPILEORDER), 60, wheight-115, wwidth-60/*CW_USEDEFAULT*/, 25, TRUE);
				MoveWindow(GetDlgItem(hwnd, ID_COMPILERES), 60, wheight-210, wwidth-60/*CW_USEDEFAULT*/, 95, TRUE);
		    }
			SendMessage(g_hToolBar, TB_AUTOSIZE, 0, 0);
			SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
			GetWindowRect(g_hStatusBar, &rectStatus);
			
			GetClientRect(hwnd, &rectClient);
			GetWindowRect(g_hStatusBar, &rectStatus);
			GetWindowRect(g_hToolBar, &rectTool);
			uToolHeight = rectTool.bottom - rectTool.top;
			uStatusHeight = rectStatus.bottom - rectStatus.top;
			uClientAlreaHeight = rectClient.bottom;
			
			break;
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwnd, IDC_MAIN_TEXT));
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case CM_DT: {
					GetDlgItemText(hwnd, IDC_MAIN_TEXT, getallcodetmpstr, 200000);
					MessageBox(hwnd, getallcodetmpstr, "", MB_OK);
					MessageBox(NULL, i_to_str(GetScrollPos(GetDlgItem(hwnd, IDC_MAIN_TEXT), SB_VERT)).c_str(), "", MB_OK);
					break;
				}
				case CM_FILE_OPEN:
					/*
					if (MessageBox (hwnd, " If you open a new file, the unsaved contents will be lost!\n Sure to continue?", "Warning!", MB_YESNO | MB_ICONWARNING) != IDYES) {
						break;
					}
					*/
					/*settitle*/ 
					titlestr01="Click 5.0 [ Opening... ]";
					SetWindowText (hwnd, titlestr01.c_str());
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Opening..."); 
					/*end:settitle*/ 
					if (!DoFileOpenSave(hwnd, FALSE)) {
						titlestr01="Click 5.0";
						SetWindowText (hwnd, titlestr01.c_str());
						SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "已编译" : "未编译")); 
						SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
						break;
					}
					fcompiled=0;
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SetWindowText (hwnd, titlestr01.c_str());
					SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "已编译" : "未编译")); 
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					/*end:settitle*/ 
					break;
				case CM_WLARGE: {
					if (wordsizepos >= 15) {
						MessageBox(hwnd, "已经是最大字体！", "", MB_OK);
						break;
					}
					///*4.7*/hFont = CreateFont(wsizes[++wordsizepos],0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,fontname.c_str());//创建字体
					///*4.7*/SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
					SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)(wsizes[++wordsizepos]));
					break;
				}
				case CM_WSMALL: {
					if (wordsizepos <= 0) {
						MessageBox(hwnd, "已经是最小字体！", "", MB_OK);
						break;
					}
					///*4.7*/hFont = CreateFont(wsizes[--wordsizepos],0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,fontname.c_str());//创建字体
					///*4.7*/SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
					SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)(wsizes[--wordsizepos]));
					break;
				}
				case CM_CFONT: {
					if (fontname == "Consolas") {
						fontname = "Inconsolata";
					} else {
						fontname = "Consolas";
					}
					///*4.7*/hFont = CreateFont(wsizes[wordsizepos],0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,fontname.c_str());//创建字体
					///*4.7*/SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
					SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT),SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)fontname.c_str());
					break;
				}
				case CM_IMPORTSET: {
					char filenametoimport[MAX_PATH*10];
					if (!DoFileOpen(hwnd, filenametoimport)) {
						break;
					}
					wordsizepos  = GetPrivateProfileInt(TEXT("FONT"), TEXT("SIZE"), 5, filenametoimport);
					char fontnameini[100];
					GetPrivateProfileString(TEXT("FONT"), TEXT("NAME"), TEXT("Inconsolata"), fontnameini, 100, filenametoimport);
					/*4.7*/hFont = CreateFont(wsizes[wordsizepos],0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,fontnameini);//创建字体
					/*4.7*/SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
					break;
				}
				case CM_FILE_SAVEAS:
					/*settitle*/ 
					titlestr01="Click 5.0 [ Saving... ]";
					SetWindowText (hwnd, titlestr01.c_str());
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Saving..."); 
					/*end:settitle*/ 
					DoFileOpenSave(hwnd, TRUE);
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SetWindowText (hwnd, titlestr01.c_str());
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					/*end:settitle*/ 
					break;
				case CM_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case CM_FILE_SAVE:
					/*settitle*/ 
					titlestr01="Click 5.0 [ Saving... ]";
					SetWindowText (hwnd, titlestr01.c_str());
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Saving..."); 
					/*end:settitle*/ 
					if ((!fsaved && !fopend) || strcmp(szFileName, "Untitled") == 0) {
						if (!DoFileOpenSave(hwnd, TRUE)) {
							SetWindowText (hwnd, "Click 5.0");
							break;
						}
					} else {
						if(!SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
							MessageBox(hwnd, "Save file failed.\n(Or this is an empty file.)", "Error",MB_OK|MB_ICONEXCLAMATION);
							fsaved=0;
						}
					}
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					break;
				case CM_ABOUT:
					/*settitle*/ 
					SetWindowText (hwnd, "Click 5.0 [ About... ]");
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"About..."); 
					/*end:settitle*/ 
					MessageBox (hwnd, "Click IDE: 2020.4\nVersion: 5.0.-Stable\nBy: 华育中学 Eric 倪天衡.\nGUI: Win32 API.\nIntroduction: Click is an light, open-source, convenient C++ IDE which based on GNU MinGW.\nOnly for: Windows 7/8/8.1/10. You can contact us to get the XP Version.\nLicense: Apache License, Version 2.0\nTo learn more or get updates, please visit our official website: https://ericnth.cn/clickide/\nIf you meet some problems, please contact us or visit: Help->Get help..." , "About...", 0);
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					break;
				case CM_RUN:
					if (fcompiled) {
						/*settitle*/ 
						titlestr01="Click 5.0 [ Running ] [ ";
						titlestr01+=szFileName;
						titlestr01+=" ]";
						SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Running..."); 
						SetWindowText (hwnd, titlestr01.c_str());
						/*end:settitle*/ 
						strcpy(szFileName2,getcppfn(szFileName).c_str());
						strcat(szFileName2,".exe");
						if (_access(szFileName2, X_OK) == -1) {
							MessageBox(hwnd, "运行失败。请在下方信息窗口查看详情。\nFail to run program! Please look at the details in the \"Information\" Box.", "Click 5.0", MB_OK|MB_ICONHAND);
							sprintf(cmdbuf3, "Fail to run program: \r\n%s.exe\r\nMaybe it is because that you failed in compilation.\r\nAnd make sure that you added \"-o %%e\" option when inputting compiling order.", getcppfn(szFileName).c_str());
							Addinfo(cmdbuf3);
							break;
						}
						sprintf(cmdbuf3, "Running program: \r\n%s.exe", getcppfn(szFileName).c_str());
						Addinfo(cmdbuf3);
						sprintf (cmdbuf2, "start \"Click5.0-Executing [%s.exe]\" %s.exe",getcppfn(szFileName).c_str(),getcppfn(szFileName).c_str());
						runprocess (cmdbuf2, 0, 1);
					} else {
						MessageBox (hwnd, "You haven't compiled this file yet (or have failed in it),\nPlease compile it first!", "Can't Run!", MB_OK | MB_ICONERROR);
						sprintf(cmdbuf3, "Fail to run program: \r\n%s.exe\r\nPlease compile it first.", getcppfn(szFileName).c_str());
						Addinfo(cmdbuf3);
					}
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					break;
				case CM_COMPILE:
					char compileordertmptmp[800];
					GetDlgItemText(hwnd, ID_COMPILEORDER, compileordertmptmp, 800);
					compileordertmp.clear();
					compileordertmp += compileordertmptmp;
					for (int i = 0; i < compileordertmp.size(); i++) {
						if (i == compileordertmp.size()-1) {
							continue;
						}
						if (compileordertmp[i] == '%') {
							if (compileordertmp[i+1] == 'f' || compileordertmp[i+1] == 'F') {
								strcpy(szFileNametmp, "\"");
								strcat(szFileNametmp, szFileName);
								strcat(szFileNametmp, "\"");
								compileordertmp.replace(i, 2, szFileNametmp);
							}
							if (compileordertmp[i+1] == 'e' || compileordertmp[i+1] == 'E') {
								strcpy(szFileNametmp, "\"");
								strcat(szFileNametmp, getcppfn(szFileName).c_str());
								strcat(szFileNametmp, "\"");
								compileordertmp.replace(i, 2, szFileNametmp);
							}
							if (compileordertmp[i+1] == '%') {
								strcpy(szFileNametmp, "%");
								compileordertmp.replace(i, 2, szFileNametmp);
							}
						}
					}
					//MessageBox(hwnd, compileordertmp.c_str(), "", MB_OK);
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Compiling..."); 
					if ((!fsaved && !fopend) || strcmp(szFileName, "") == 0) {
						if (!DoFileOpenSave(hwnd, TRUE)) {
							break;
						}
					} else {
						if(!SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
							MessageBox(hwnd, "Save file failed.", "Error",MB_OK|MB_ICONEXCLAMATION);
							fsaved=0;
							break;
						}
					}
					/*settitle*/ 
					titlestr01="Click 5.0 [ Compiling ] [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					sprintf (cmdbuf1, "%s 2> %s_compile_tmp.log", compileordertmp.c_str(),szFileName);
					sprintf (cmdbuf2, "start \"Click5.0-Executing [%s.exe]\" /max %s.exe",getcppfn(szFileName).c_str(),getcppfn(szFileName).c_str());
					sprintf (cmdbuf3, "del \"%s.exe\"",getcppfn(szFileName).c_str());
					sprintf (cmdbuf4, "del \"%s_compile_tmp.log\"",szFileName);
					sprintf (cmdbuf5, "%s_compile_tmp.log",szFileName);
					runprocess (cmdbuf3, 1, 0);
					runprocess (cmdbuf1, 1, 0);
					wndfin.open (cmdbuf5);
					while (wndfin) {
						errreportcnt++;
						inttmpnum=wndfin.get();
						if (inttmpnum == '\n') {
							errreporti += "\r\n";
						} else {
							errreporti += (char)inttmpnum;
						}
					}
					wndfin.close();
					if (errreportcnt>1) {
						sprintf(compileresult, "Compilation Result\r\nOrder: %s\r\nCompiler output: %s\r\nResult: Failed.", cmdbuf1, errreporti.c_str());
						Addinfo(compileresult);
						SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
						break;
						fcompiled=0; 
					} else {
						fcompiled=1;
						sprintf(compileresult, "Compilation Result\r\nOrder: %s\r\nResult: Succeed.", cmdbuf1, errreporti.c_str());
						Addinfo(compileresult);
						SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					}
					runprocess (cmdbuf4, 1, 0);
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "已编译" : "未编译")); 
					/*end:settitle*/ 
					break;
				case CM_COMPILERUN:
					SendMessage(hwnd, WM_COMMAND, CM_COMPILE, NULL);
					SendMessage(hwnd, WM_COMMAND, CM_RUN, NULL);
					break;
					
				case CM_STARTCMD:
					runprocess ((char*)"start /max \"Click 5.0 [Command]\"", 0, 1);
					break;
				case CM_RUNBAT:
					/*settitle*/ 
					titlestr01="Click 5.0 [ Running (Bat) ] [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Running Bat..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					if ((!fsaved && !fopend) || strcmp(szFileName, "") == 0) {
						DoFileOpenSave(hwnd, TRUE);
					} else {
						if(!SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
							MessageBox(hwnd, "Save file failed.\n(Or this is an empty file.)", "Error",MB_OK|MB_ICONEXCLAMATION);
							fsaved=0;
						}
					}
					sprintf (cmdbuf2, "start \"Click5.0-Executing [%s]\" %s", szFileName, szFileName);
					runprocess (cmdbuf2, 0, 1);
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					break;
				case CM_DEBUG: 
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Debugging..."); 
					if ((!fsaved && !fopend) || strcmp(szFileName, "") == 0) {
						DoFileOpenSave(hwnd, TRUE);
					} else {
						if(!SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
							MessageBox(hwnd, "Save file failed.", "Error",MB_OK|MB_ICONEXCLAMATION);
							fsaved=0;
						}
					}
					sprintf (cmdbuf2, "start /max \"Click5.0-Debugging [%s]\" gdb %s.exe", getcppfn(szFileName).c_str(), getcppfn(szFileName).c_str());
					runprocess (cmdbuf2, 0, 1);
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					break;
				case CM_VVARI:
					/*settitle*/ 
					titlestr01="Click 5.0 [ Viewing Variables... ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Viewing Variables..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					sprintf (cmdbuf1, "szFileName\t= %s\nfsaved         \t= %s\nfopened      \t= %s\nfcompiled   \t= %s\nCurrentTime\t= %s\nCurrentMessage\t= %d/%d\nMessageCount\t= %lld", szFileName, (fsaved ? "True" : "False"), (fopend ? "True" : "False"), (fcompiled ? "True" : "False"), output_time().c_str(), WM_COMMAND, CM_VVARI, variMsgCnt);
					MessageBox (hwnd, cmdbuf1, "Variables...", MB_OK | MB_ICONINFORMATION);
					/*settitle*/ 
					titlestr01="Click 5.0 [ ";
					titlestr01+=szFileName;
					titlestr01+=" ]";
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
					break;
				case CM_GHELP:
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Helps..."); 
					GHELPSTARTPLACE:
					
					switch (MessageBox (0, "在您使用该软件进行编译运行前，请确保您已经将您的g++编译器bin目录和fpc编译器的bin\\i386-win32\\目录添加到环境变量PATH。（环境变量设置方法：右击“此电脑”->属性，点击左侧“高级系统设置”，在“高级”标签下单击“环境变量(N)...”，双击“系统变量”中的PATH项进行编辑，在后面添加“XXX\\FPC\\2.2.2\\bin\\i386-win32\\”和“XXX\\MinGW\\bin\\”（将XXX替换为你的安装位置，不要加引号！），然后全部点击“确定”即可。", "Help 01", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "在您打开一个文件后，可以对它进行任何操作。我们并没有禁止类似打开一个C++文件后用\"Compile Pascal File...\"来进行编译等的操作（尽管这不对），因此您在使用编译/运行这些选项时，请务必确认是否选择了正确的语言！", "Help 02", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "由于作者能力有限以及本软件向C++的偏向性，部分Pascal程序可能无法正确编译/运行，请您谅解。您也可选择使用其他Pascal编译器（只要把它的目录添加到环境变量PATH，并将软件安装时自带的FPC目录从环境变量PATH中移除即可。", "Help 03", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "由于本软件开发时间较短，因此在使用过程中由以下限制：\n  1.仅用于Windows操作系统的部分支持Win32API的版本。\n  2.C++文件仅支持.cpp, .c++, .cxx后缀名，Pascal文件仅支持.pp后缀名，C++头文件仅支持.hpp后缀名，批处理文件仅支持.bat, .com, .cmd后缀名，请谅解。如您使用其他的后缀名（字符数量不符），可能导致编译运行失败。", "Help 04", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "在您想要直接运行/调试一个程序时，必须先保存。并且，如果您想要运行/调试当前您写的程序，请先编译，否则运行/调试的是您上一次编译后生成的程序。", "Help 05", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "若您的状态条被遮挡，且您想要查看，可以选择Help > Flush StatusBar进行刷新。", "Help 06", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "本文件夹内Click5.0.exe是可以单独使用的，即：您可以把这个exe文件复制到任意位置（甚至其他Windows电脑）均可使用。但您需要自己设置MinGW和FPC库，在本发布版中已经自带（即MinGW和FPC文件夹），但您仍然可以使用自己的库。只要将它添加到环境变量即可。", "Help 07", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "其次，若看到类似结尾为_compile_tmp.log的文件，是我们在编译过程中（可能会）生成的临时日志文件，您完全可以直接删除，对ClickIDE和其他软件的运行没有任何影响。", "Help 08", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "在您编译时，若是C++程序，仅当发生错误/警告时才会发出提示，否则直接编译完成；若是Pascal程序，任何情况下都会发出编译提示，所以请认真留意提示中是否存在例如\"Fatal\"或\"Error\"之类的字眼，如有，则表明编译出错，反之，则表明编译通过。", "Help 09", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					switch (MessageBox (0, "若有其他困难，问题，意见或者建议，请您一定要及时联系作者邮箱eric_ni2008@163.com进行咨询或投诉，以便我们今后把ClickIDE做得更加完善！", "Help 10", MB_CANCELTRYCONTINUE | MB_ICONINFORMATION | MB_DEFBUTTON3)) {
						case IDCANCEL: GHELPEXITFLAG = 1;break;
						case IDCONTINUE:break;
						case IDTRYAGAIN: goto GHELPSTARTPLACE;break;
						default: GHELPEXITFLAG = 1;break;
					}
					if (GHELPEXITFLAG) {SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); break;}
					
					MessageBox (0, "没有更多提示了......", "Message", MB_OK | MB_ICONINFORMATION);
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					
					break;
				case CM_EDIT_UNDO:
					SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, EM_UNDO, 0, 0);
					break;
				case CM_EDIT_CUT:
					SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_CUT, 0, 0);
					break;
				case CM_EDIT_COPY:
					SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_COPY, 0, 0);
					break;
				case CM_EDIT_PASTE:
					SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_PASTE, 0, 0);
					break;
				case CM_EDIT_FIND:
					ZeroMemory(&repfindtag, sizeof(repfindtag));
					repfindtag.hwndOwner = hwnd;
					repfindtag.lpstrFindWhat = "\0";
					repfindtag.Flags = FR_DOWN|FR_FINDNEXT|FR_MATCHCASE;
					repfindtag.wFindWhatLen = MAX_PATH;
					repfindtag.wReplaceWithLen = MAX_PATH;
					//repfindtag.lpstrFindWhat = szFindWhat;
					repfindtag.lCustData = 0;
					repfindtag.lpfnHook = NULL;
					repfindtag.lStructSize = sizeof(repfindtag);
					FindText(&repfindtag);
					break;
				case CM_FLSTB:
					SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)"Click 5.0 IDE"); 
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "已编译" : "未编译")); 
					SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)""); 
					SendMessage(g_hStatusBar, SB_SETTEXT, 4, (LPARAM)szFileName); 
					break;
				case CM_GHTML: {
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"GeneratingHTML..."); 
					GetDlgItemText(hwnd, IDC_MAIN_TEXT, getallcodetmpstr, 200000);
					codealltmp.clear();
					codealltmp+=getallcodetmpstr;
					codealltmp+="\n     ";
					//MessageBox(hwnd, codealltmp.c_str(), "", MB_OK);
					char hlfilename[MAX_PATH*6];
					strcpy(hlfilename, szFileName);
					strcat(hlfilename, (char*)"_highlight.html");
					char titlefilename[MAX_PATH*6];
					strcpy(titlefilename, "Click5.0-");
					strcat(titlefilename, hlfilename);
					fout.open(hlfilename);
					fout << "<!DOCTYLE html>" << endl;
					fout << "<html><head><meta charset=\"utf-8\"/><title>";
					fout << titlefilename;
					fout << "</title><style>body{font-family:Consolas,Arial,Helvetica,sans-serif;}h3{font-style:italic}.operator{color:#ff2c0f;}.zfc{color:#0000ff;}.ycl{color:#80ab66}.zs{color:#0078d7;}.gjz{font-weight:bold;}.vari{font-weight:bold;color:#8e37a7;}</style></head>" << endl;
					fout << "<body><h3>Exported by ClickIDE 5.0</h3><p>";
					inttmpnum = SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_GETLENGTH, SCI_NULL, SCI_NULL);
					char codealltmptmp[inttmpnum];
					SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_GETTEXT, inttmpnum+1, (LPARAM)codealltmptmp);
					codealltmp.clear();
					codealltmp += codealltmptmp;
					for (int i = 0; i < codealltmp.size(); i++) {
						if (dontout) {
							dontout = 0;
							continue;
						}
						char tch = codealltmp[i];
						if (tch == '\"') {
							islfst=0;
							if (codealltmp[i-1] == '\\') {
								fout << "quot;";continue;
							}
							if (!iszfc) {
								fout << "<span class=\"zfc\">&quot;";
								iszfc = true;continue;
							}
							if (iszfc) {
								fout << "&quot;</span>";
								iszfc = false;continue;
							}
						}
						if (tch == '#' && islfst) {
							islfst=0;
							isycl = 1;
							fout << "<span class=\"ycl\">#";continue; 
						}
						if (tch == '/' && codealltmp[i+1] == '*') {
							ismtzs = 1;
							fout << "<span class=\"zs\">/";continue; 
						}
						if (tch == '*' && codealltmp[i+1] == '/') {
							ismtzs = 0;dontout=1;
							fout << "*/</span>";continue; 
						}
						if (tch == '/' && codealltmp[i+1] == '/') {
							issgzs = 1;dontout=1;
							fout << "<span class=\"zs\">//";continue; 
						}
						if (tch == ' ') {
							fout << "&nbsp;";continue;
						}
						if (issgzs || ismtzs) {
							if (tch == '<') {fout << "&lt;";continue;}
							if (tch == '>') {fout << "&gt;";continue;}
							if (tch == '&') {fout << "&amp;";continue;}
							if (tch == '\"') {fout << "&quot;";continue;}
							if (tch == '\t') {fout << "&nbsp;&nbsp;&nbsp;&nbsp;";continue;}
							if (tch == ' ') {fout << "&nbsp;";continue;}
							if (tch == '\n') {
								if (isycl && !nlycl) {
									isycl = 0;
									fout << "</span>"; 
								}
								if (issgzs) {
									fout << "</span>";
									issgzs = 0;
								}
								islfst = 1;fout << "<br/>";continue;
							}
							fout << tch;
							continue;
						}
						if (tch == '<') {islfst=0;if (isycl || iszfc) {fout << "&lt";} else {fout << "<span class=\"operator\">&lt</span>";}continue;}
						if (tch == '>') {islfst=0;if (isycl || iszfc) {fout << "&gt";} else {fout << "<span class=\"operator\">&gt</span>";}continue;}
						if (tch == '&') {islfst=0;if (isycl || iszfc) {fout << "&amp";} else {fout << "<span class=\"operator\">&amp;</span>";}continue;}
						if (tch == '{') {islfst=0;if (isycl || iszfc) {fout << "{";} else {fout << "<span class=\"operator\">{</span>";}continue;}
						if (tch == '}') {islfst=0;if (isycl || iszfc) {fout << "}";} else {fout << "<span class=\"operator\">}</span>";}continue;}
						if (tch == '(') {islfst=0;if (isycl || iszfc) {fout << "(";} else {fout << "<span class=\"operator\">(</span>";}continue;}
						if (tch == ')') {islfst=0;if (isycl || iszfc) {fout << ")";} else {fout << "<span class=\"operator\">)</span>";}continue;}
						if (tch == ':') {islfst=0;if (isycl || iszfc) {fout << ":";} else {fout << "<span class=\"operator\">:</span>";}continue;}
						if (tch == ';') {islfst=0;if (isycl || iszfc) {fout << ";";} else {fout << "<span class=\"operator\">;</span>";}continue;}
						if (tch == '+') {islfst=0;if (isycl || iszfc) {fout << "+";} else {fout << "<span class=\"operator\">+</span>";}continue;}
						if (tch == '-') {islfst=0;if (isycl || iszfc) {fout << "-";} else {fout << "<span class=\"operator\">-</span>";}continue;}
						if (tch == '*') {islfst=0;if (isycl || iszfc) {fout << "*";} else {fout << "<span class=\"operator\">*</span>";}continue;}
						if (tch == '/') {islfst=0;if (isycl || iszfc) {fout << "/";} else {fout << "<span class=\"operator\">/</span>";}continue;}
						if (tch == '%') {islfst=0;if (isycl || iszfc) {fout << "%";} else {fout << "<span class=\"operator\">%</span>";}continue;}
						if (tch == '~') {islfst=0;if (isycl || iszfc) {fout << "~";} else {fout << "<span class=\"operator\">~</span>";}continue;}
						if (tch == '?') {islfst=0;if (isycl || iszfc) {fout << "?";} else {fout << "<span class=\"operator\">?</span>";}continue;}
						if (tch == '^') {islfst=0;if (isycl || iszfc) {fout << "^";} else {fout << "<span class=\"operator\">^</span>";}continue;}
						if (tch == '[') {islfst=0;if (isycl || iszfc) {fout << "[";} else {fout << "<span class=\"operator\">[</span>";}continue;}
						if (tch == ']') {islfst=0;if (isycl || iszfc) {fout << "]";} else {fout << "<span class=\"operator\">]</span>";}continue;}
						if (tch == '=') {islfst=0;if (isycl || iszfc) {fout << "=";} else {fout << "<span class=\"operator\">=</span>";}continue;}
						if (tch == ',') {islfst=0;if (isycl || iszfc) {fout << ",";} else {fout << "<span class=\"operator\">,</span>";}continue;}
						if (tch == '\n') {
							if (isycl && !nlycl) {
								isycl = 0;
								fout << "</span>"; 
							}
							if (issgzs) {
								fout << "</span>";
								issgzs = 0;
							}
							islfst = 1;
							fout << "<br/>";continue;
						}
						if (tch == '\\') {
							if (codealltmp[i+1] == '\n' && isycl) {
								nlycl = 1;
							}
						}
						if (tch == '\t') {
							fout << "&nbsp;&nbsp;&nbsp;&nbsp;";continue;
						}
						fout << tch;
						islfst = 0;
					}
					fout << "</p></body></html>" << endl;
					fout.close();
					Sleep(200);
					ShellExecute(NULL,TEXT("open"), hlfilename, TEXT(""),NULL,SW_SHOWNORMAL);
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
					break;
				}
				
				case CM_ASTYLE: {
					MessageBox(hwnd, "本版本（5.0-Stable）不支持该功能。若想尝鲜，请联系作者获取内测版本。下一个正式版本（5.1-stable）将会包含此功能。\n", "Ah oh~", MB_OK);
					break;
					PostMessage(hwnd, WM_COMMAND, CM_FILE_SAVE, (LPARAM)"");
					char astylestr[MAX_PATH*6];
					sprintf(astylestr, "--recursive --style=bsd --convert-tabs --indent=spaces=4 --attach-closing-while --indent-switches --indent-namespaces --indent-continuation=4 --indent-preproc-block --indent-preproc-define --indent-preproc-cond --indent-col1-comments --pad-oper --pad-paren-in --unpad-paren --delete-empty-lines --align-pointer=name --align-reference=name --break-elseifs --add-braces >%s.astyle.log %s*", szFileName, szFileName);
					//system(astylestr)
					MessageBox(NULL, astylestr, "", MB_OK);
					ShellExecute(NULL,TEXT("open"), "AStyle.exe", TEXT(astylestr),NULL,SW_HIDE);
					sprintf(astylestr, "del %s.orig", szFileName);
					system(astylestr);
					
					break;
				}
				case CM_GITHUB: {
					ShellExecute(NULL,TEXT("open"), TEXT("https://github.com/EricNTH080103/ClickIDE-5.x"), TEXT(""),NULL,SW_SHOWNORMAL);
					break;
				}
				case CM_WEBSITE: {
					ShellExecute(NULL,TEXT("open"), TEXT("https://ericnth.cn/clickide/"), TEXT(""),NULL,SW_SHOWNORMAL);
					break;
				}
			}
			hMenu = GetMenu(hwnd);
			hFileMenu = GetSubMenu(hMenu, 0);
			hCompileMenu = GetSubMenu(hMenu, 1);
			EnableMenuItem(hFileMenu, CM_FILE_SAVE, MF_BYCOMMAND | (!(!fsaved && !fopend) || strcmp(szFileName, "Untitled") ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(hCompileMenu, CM_RUN, MF_BYCOMMAND | ((fcompiled) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(hCompileMenu, CM_DEBUG, MF_BYCOMMAND | ((fcompiled) ? MF_ENABLED : MF_GRAYED));
			char tishitext[1024];
			GetDlgItemText(hwnd, IDC_MAIN_TEXT, getallcodetmpstr, 200000);
			codealltmp.clear();
			codealltmp+=getallcodetmpstr;
			linecount = 0;
			sprintf(tishitext, "Welcome\nto\nClickIDE!\n\nVersion:\n5.0.0\n\nWords:\n%d\n\nFont size:%d", codealltmp.size(), wsizes[wordsizepos]);
			SetDlgItemText(hwnd, IDC_LINE_NUM, tishitext);
			
			
			//SetScrollPos(GetDlgItem(hwnd, IDC_MAIN_TEXT), SB_VERT, GetScrollPos(GetDlgItem(hwnd, IDC_MAIN_TEXT), SB_VERT), 1);
			
			
			
			break;
		/*
		case WM_CTLCOLOREDIT: {
      		HDC hdc = (HDC)wParam;
   			SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
   			SetBkColor((HDC)wParam,RGB(0x0, 0x0, 0x0));
   			return (long long int)((HBRUSH)GetStockObject(NULL_BRUSH));
   			break;
   		}*/
		case WM_CLOSE:
					SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Exitting..."); 
					/*settitle*/ 
					titlestr01="Click 5.0 [ Exiting... ]";
					SetWindowText (hwnd, titlestr01.c_str());
					/*end:settitle*/ 
			if (MessageBox (hwnd, "Are you sure to quit? \nThings which are not saved will be lost!", "Exiting...", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
				/*settitle*/ 
				titlestr01="Click 5.0 [ ";
				titlestr01+=szFileName;
				titlestr01+=" ]";
				SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"..."); 
				SetWindowText (hwnd, titlestr01.c_str());
				/*end:settitle*/ 
				break;
			}
			programmeexiterrorstatusflag = 0;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			/*
			if (programmeexiterrorstatusflag) {
				if (MessageBox (0, "Abnormal exit!\nDo you want to try to recover?", "Click 5.0", MB_YESNO | MB_ICONQUESTION) == IDYES) {
					break;
				}
			}
			*/
			DeleteObject(hFont);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow) {
	SYSTEM_INFO si;
	HINSTANCE hDLL; 
    SafeGetNativeSystemInfo(&si);  
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) {  
        hDLL = LoadLibrary(TEXT("SciLexer_x64.dll"));  
		if (hDLL == NULL) {
			MessageBox(NULL, "Fatal: SciLexer_x64.dll Not found.\n出现了一个致命错误：SciLexer_x64.dll找不到。\n解决方案如下：\n  1.请联系软件支持人员（可以通过邮箱：eric_ni2008@163.com或访问我们的网站ericnth.cn）。\n  2.如果您的电脑中确实有SciLexer.dll或SciLexer_x64.dll，请将它命名为SciLexer_x64.dll，并将它添加至环境变量PATH中。\n  3.如果您急于使用却不知道解决方法，请前往：ericnth.cn/clickide下载无外部依赖的4.x版本（最新4.6.5）。", "Click 5.0", MB_ICONHAND | MB_OK);
			return 0;
		}
    } else {
      	hDLL = LoadLibrary(TEXT("SciLexer_x86.dll"));  
		if (hDLL == NULL) {
			MessageBox(NULL, "Fatal: SciLexer_x86.dll Not found.\n出现了一个致命错误：SciLexer_x86.dll找不到。\n解决方案如下：\n  1.请联系软件支持人员（可以通过邮箱：eric_ni2008@163.com或访问我们的网站ericnth.cn）。\n  2.如果您的电脑中确实有SciLexer.dll或SciLexer_x86.dll，请将它命名为SciLexer_x64.dll，并将它添加至环境变量PATH中。\n  3.如果您急于使用却不知道解决方法，请前往：ericnth.cn/clickide下载无外部依赖的4.x版本（最新4.6.5）。", "Click 5.0", MB_ICONHAND | MB_OK);
			return 0; 
		}
	}
	if (strcmp(lpCmdLine, "") != 0) {
		hasstartopenfile = 1;
		if (_access(lpCmdLine, W_OK) == -1) {
			MessageBox(NULL, "文件不存在或拒绝访问！", "Click 5.0", MB_OK);
			hasstartopenfile = 0;
		} else {
			strcpy(commandline, lpCmdLine);
		}
	}
	WNDCLASSEX wc;
	//HWND hwnd;
	MSG Msg;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = "MAINMENU";
	wc.lpszClassName = "WindowClass";
	wc.hIconSm       = LoadIcon(hInstance,"A"); /* A is name used by project icons */

	if(!RegisterClassEx(&wc)) {
		MessageBox(0,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);
		return 0;
	}
	
	sprintf (szFileName, "%s", "Untitled"); 
	
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Click 5.0",WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1000,600,
		NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) {
		MessageBox(0, "Window Creation Failed!", "Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);
		return 0;
	}

	ShowWindow(hwnd,1);
	UpdateWindow(hwnd);
	

	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		variMsgCnt++;
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	
	FreeLibrary(hDLL);
	return Msg.wParam;
}
