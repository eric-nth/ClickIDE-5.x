#undef UNICODE

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
bool darktheme = 0;
HWND hwnd;
char fontname[32] = "Consolas";
bool tabwidthset = 0;
string codealltmp = "";
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
vector<string> funclists;
CHOOSEFONT cf;
LOGFONT lf;
const size_t MAX_FUNCSIZE=1000;
size_t FUNCSIZE = 0;
char g_szFuncList[MAX_FUNCSIZE][1024];
char g_szFuncDesc[MAX_FUNCSIZE][256];

LRESULT __stdcall SendEditor(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) {
    return SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), Msg, wParam, lParam);
}

void GainAdminPrivileges(LPSTR cmdline, LPSTR executedir, UINT idd, BOOL bWait){

    //strCmd.Format (_T("/adminoption %d"), idd);
    char strCmd[MAX_PATH];
    char strApp[MAX_PATH*10];
    sprintf(strCmd, "/adminoption %d", idd);
    sprintf(strApp, cmdline, idd);

    SHELLEXECUTEINFO execinfo;
    memset(&execinfo, 0, sizeof(execinfo));
    execinfo.lpFile			= strApp;
    execinfo.cbSize			= sizeof(execinfo);
    execinfo.lpVerb			= TEXT("runas");
    execinfo.fMask			= SEE_MASK_NOCLOSEPROCESS;
    execinfo.nShow			= SW_SHOWDEFAULT;
    execinfo.lpParameters	= strCmd;
    execinfo.lpDirectory    = executedir;

    ShellExecuteEx(&execinfo);

    if(bWait)
        WaitForSingleObject(execinfo.hProcess, INFINITE);
}

string output_time() {
    time_t rawtime;
    time(&rawtime);
    char pblgtime[20];
    strftime(pblgtime, 20, "%Y-%m-%d %H-%M-%S", localtime(&rawtime));
    string tttmps="";
    tttmps.insert(0, pblgtime);
    return tttmps;
}

void Addinfo(const char info[]) {
    char nativetext[100000];
    GetDlgItemText(hwnd, ID_COMPILERES, nativetext, 100000);
    string aftertext;
    aftertext.clear();
    aftertext += "-------------------------\r\n";
    aftertext += "Time: ";
    aftertext += output_time();
    aftertext += "\r\nInfo: ";
    aftertext += info;
    aftertext += "\r\n";
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
                        SendEditor(SCI_SETSAVEPOINT);
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
    char szFileNameBackup[MAX_PATH*10];
    strcpy(szFileNameBackup, szFileName);
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    szFileName[0] = 0;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = (bSave ? "C++ Files (*.cpp; *.c++)\0*.cpp;*.c++\0C++ Header Files (*.hpp)\0*.hpp\0Pascal Files (*.pp)\0*.pp\0Windows批处理文件(*.bat; *.cmd)\0*.bat;*.cmd\0All Files (*.*)\0*.*\0\0" : "C++ Files (*.cpp; *.c++; *.cxx)\0*.cpp;*.c++;*.cxx\0C++ Header Files (*.hpp)\0*.hpp\0Pascal Files (*.pp)\0*.pp\0Windows    疟  (*.bat; *.cmd)\0*.bat;*.com;*.cmd\0ClickIDE Temporary Compilation Logs\0*_compile_tmp.log\0All Files (*.*)\0*.*\0\0");
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
            strcpy(szFileName, szFileNameBackup);
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
            strcpy(szFileName, szFileNameBackup);
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
    ofn.lpstrFilter = "Ini Setting File(*.ini)\0*.ini\0\0";
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
    int iStatusWidths[] = {100, 230, 330, 335, -1};
    RECT rectStatus;
    bool isycl = 0;
    bool iszfc = 0;
    bool islfst = 1;
    bool nlycl = 0;
    bool issgzs = 0;
    bool ismtzs = 0;
    bool dontout = 0;
    RECT rctA;
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
    SCNotification* notify = (SCNotification*)lParam;
    HDC hdc,mdc;
    HBITMAP bg;
    int cursorpos;
    DWORD dwTextLength;
    int currentlinenum = 0;
    int tabcount = 0;
    char updatefilename[MAX_PATH*10];
    int brpos = 0;
    char currentpath[MAX_PATH*10];
    //char  linebuf[10000];
    //int  curLine  =  GetCurrentLineNumber();
    //int  lineLength  =  SendEditor(SCI_LINELENGTH,  curLine);
    /*4.8--
    if (tosetcur) {
        SetCaretPos((cursorpoint.x-6)/(wsizes[wordsizepos]/2.0), (cursorpoint.y-2)/wsizes[wordsizepos]);
        tosetcur=0;
    }
    --4.8*/
    switch(Message) {
        case WM_CREATE:
            GetWindowRect(hwnd,&rctA);//通     诰    么  诘拇 小 娲? rctA 峁?
            wwidth = rctA.right - rctA.left;
            wheight = rctA.bottom - rctA.top;
            CreateWindow("Scintilla", "",WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_BORDER,60, 30, wwidth-180/*CW_USEDEFAULT*/, wheight-240,hwnd, (HMENU)IDC_MAIN_TEXT, GetModuleHandle(NULL), NULL);
            CreateWindow("STATIC", "Welcome\nto\nClickIDE!\n\nVersion:\n5.0.0",WS_CHILD|WS_VISIBLE|WS_BORDER,0, 30, 60/*CW_USEDEFAULT*/, wheight-160,hwnd, (HMENU)IDC_LINE_NUM, GetModuleHandle(NULL), NULL);
            CreateWindow("EDIT", "g++.exe %f -o %e -lm",WS_CHILD|WS_VISIBLE|WS_BORDER,60, wheight-115, wwidth-180/*CW_USEDEFAULT*/, 25,hwnd, (HMENU)ID_COMPILEORDER, GetModuleHandle(NULL), NULL);
            CreateWindow("EDIT", "",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|WS_VSCROLL|ES_WANTRETURN,60, wheight-210, wwidth-180/*CW_USEDEFAULT*/, 95,hwnd, (HMENU)ID_COMPILERES, GetModuleHandle(NULL), NULL);
            CreateWindow("BUTTON", "Q",WS_CHILD|WS_VISIBLE|WS_BORDER,wwidth-120, 30, 100/*CW_USEDEFAULT*/, 95,hwnd, (HMENU)ID_COMPILENOW, GetModuleHandle(NULL), NULL);
            CreateWindow("BUTTON", "<",WS_CHILD|WS_VISIBLE|WS_BORDER,wwidth-120, 130, 100/*CW_USEDEFAULT*/, 95,hwnd, (HMENU)ID_SAVENOW, GetModuleHandle(NULL), NULL);

            /*4.7*/hFont = CreateFont(11,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,fontname);//
            /*4.7*/hFont_ln = CreateFont(14,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Consolas");//

            /*4.7*/SendDlgItemMessage(hwnd, IDC_LINE_NUM, WM_SETFONT,(WPARAM)hFont_ln/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
            ///*4.7*/SendDlgItemMessage(hwnd, IDC_QUICKFUNC, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));

            /*4.7*/hFont_ln = CreateFont(72,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Wingdings");//
            /*4.7*/SendDlgItemMessage(hwnd, ID_COMPILENOW, WM_SETFONT,(WPARAM)hFont_ln/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
            /*4.7*/SendDlgItemMessage(hwnd, ID_SAVENOW, WM_SETFONT,(WPARAM)hFont_ln/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));

            /*5.0*/hFont = CreateFont(18,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Consolas");//
            SendDlgItemMessage(hwnd, ID_COMPILEORDER, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));
            /*5.0*/hFont = CreateFont(15,0,0,0,0,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Consolas");//
            SendDlgItemMessage(hwnd, ID_COMPILERES, WM_SETFONT,(WPARAM)hFont/*GetStockObject(DEFAULT_GUI_FONT)*/, MAKELPARAM(TRUE,0));

            /*3.10: Statusbar*/
            g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                                          WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
                                          hwnd, (HMENU)ID_STATUSBAR, g_hInst, NULL);

            SendMessage(g_hStatusBar, SB_SETPARTS, 5, (LPARAM)iStatusWidths);
            SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)"Click 5.0 IDE");
            SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
            SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "Compiled" : "Not Compiled"));
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
            tbb[0].idCommand = IDM_FILE_NEW;

            tbb[1].iBitmap = STD_FILEOPEN;
            tbb[1].fsState = TBSTATE_ENABLED;
            tbb[1].fsStyle = TBSTYLE_BUTTON;
            tbb[1].idCommand = IDM_FILE_OPEN;

            tbb[2].iBitmap = STD_FILESAVE;
            tbb[2].fsState = TBSTATE_ENABLED;
            tbb[2].fsStyle = TBSTYLE_BUTTON;
            tbb[2].idCommand = IDM_FILE_SAVE;

            tbb[3].fsStyle = TBSTYLE_SEP;

            tbb[4].iBitmap = STD_CUT;
            tbb[4].fsState = TBSTATE_ENABLED;
            tbb[4].fsStyle = TBSTYLE_BUTTON;
            tbb[4].idCommand = IDM_EDIT_CUT;

            tbb[5].iBitmap = STD_COPY;
            tbb[5].fsState = TBSTATE_ENABLED;
            tbb[5].fsStyle = TBSTYLE_BUTTON;
            tbb[5].idCommand = IDM_EDIT_COPY;

            tbb[6].iBitmap = STD_PASTE;
            tbb[6].fsState = TBSTATE_ENABLED;
            tbb[6].fsStyle = TBSTYLE_BUTTON;
            tbb[6].idCommand = IDM_EDIT_PASTE;

            tbb[7].fsStyle = TBSTYLE_SEP;

            tbb[8].iBitmap = STD_UNDO;
            tbb[8].fsState = TBSTATE_ENABLED;
            tbb[8].fsStyle = TBSTYLE_BUTTON;
            tbb[8].idCommand = IDM_EDIT_UNDO;

            tbb[9].iBitmap = STD_FIND;
            tbb[9].fsState = TBSTATE_ENABLED;
            tbb[9].fsStyle = TBSTYLE_BUTTON;
            tbb[9].idCommand = IDM_EDIT_FIND;

            tbb[10].fsStyle = TBSTYLE_SEP;

            tbb[11].iBitmap = STD_HELP;
            tbb[11].fsState = TBSTATE_ENABLED;
            tbb[11].fsStyle = TBSTYLE_BUTTON;
            tbb[11].idCommand = IDM_ABOUT;

            tbb[12].fsStyle = TBSTYLE_SEP;

            tbb[13].iBitmap = STD_DELETE;
            tbb[13].fsState = TBSTATE_ENABLED;
            tbb[13].fsStyle = TBSTYLE_BUTTON;
            tbb[13].idCommand = IDM_FILE_EXIT;

            SendMessage(g_hToolBar, TB_ADDBUTTONS, 14, (LPARAM)&tbb);
            /*
            *--4.7
            */

            if (!darktheme) {
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETLEXER, SCLEX_CPP, (LPARAM) 0); //C++
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETKEYWORDS, 0, (sptr_t) g_szKeywords);//   霉丶
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_STRING, RGB(10, 0, 255));// 址
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_CHARACTER, RGB(91, 74, 68));// 址
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_NUMBER, RGB(144, 49, 150));// 址
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_PREPROCESSOR,
                            RGB(84, 181, 153));//预   肟?
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_OPERATOR, RGB(255, 0, 0));//
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENT, RGB(2, 122, 216));//  注
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENTLINE,
                            RGB(2, 122, 216));//  注
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENTDOC,
                            RGB(2, 122, 250));// 牡 注 停 /**  头
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCARETLINEVISIBLE, TRUE, 0);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCARETLINEBACK, 0xb0ffff, 0);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, STYLE_INDENTGUIDE, RGB(128, 128, 128));
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, STYLE_CONTROLCHAR, RGB(128, 128, 28));

                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETMARGINTYPEN, 0, (sptr_t) SC_MARGIN_NUMBER);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETMARGINWIDTHN, 0, (sptr_t) 40);

                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t) "Consolas");
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETSIZE, STYLE_DEFAULT, 11);
            } else {
                /*
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETLEXER, SCLEX_CPP, (LPARAM) 0); //C++
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETKEYWORDS, 0, (sptr_t) g_szKeywords);//   霉丶
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_STRING, RGB(10, 0, 255));// 址
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_CHARACTER, RGB(91, 74, 68));// 址
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_NUMBER, RGB(144, 49, 150));// 址
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_PREPROCESSOR,
                            RGB(84, 181, 153));//预   肟?
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_OPERATOR, RGB(255, 0, 0));//
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENT, RGB(2, 122, 216));//  注
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENTLINE,
                            RGB(2, 122, 216));//  注
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, SCE_C_COMMENTDOC,
                            RGB(2, 122, 250));// 牡 注 停 /**  头
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCARETLINEVISIBLE, TRUE, 0);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETCARETLINEBACK, 0xb0ffff, 0);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, STYLE_INDENTGUIDE, RGB(128, 128, 128));
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFORE, STYLE_CONTROLCHAR, RGB(128, 128, 28));

                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETMARGINTYPEN, 0, (sptr_t) SC_MARGIN_NUMBER);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_SETMARGINWIDTHN, 0, (sptr_t) 40);

                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t) "Consolas");
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETSIZE, STYLE_DEFAULT, 11);
                SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETBACK, STYLE_DEFAULT, (LPARAM)RGB(0x22, 0x27, 0x2e));*/
            }

            //Code Folding
            SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT),SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");

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

            SendEditor(SCI_SETINDENT, 0);

            if (hasstartopenfile) {
                LoadFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), commandline);
                strcpy(szFileName, commandline);
                SendMessage(g_hStatusBar, SB_SETTEXT, 4, (LPARAM)szFileName);
                PostMessage(hwnd, WM_COMMAND, IDM_LOADFILE, 0);
            }

            cf.iPointSize = 11;
            strcpy(lf.lfFaceName, fontname);


            wndfin.open("capis.txt");

            while(wndfin.good() && FUNCSIZE < MAX_FUNCSIZE && (!wndfin.eof())) {
                wndfin.getline(g_szFuncDesc[FUNCSIZE], 1024, '\n');
                strcpy(g_szFuncList[FUNCSIZE], g_szFuncDesc[FUNCSIZE]);
                strtok(g_szFuncList[FUNCSIZE], "(");
                strcat(g_szFuncList[FUNCSIZE], "(");
                FUNCSIZE++;
            }
            /*
            while(wndfin.good() && FUNCSIZE < MAX_FUNCSIZE && (!wndfin.eof())) {
                wndfin.getline(g_szFuncDesc[FUNCSIZE], 1024, '\n');
                strcpy(g_szFuncList[FUNCSIZE], strtok(g_szFuncDesc[FUNCSIZE], "("));
                MessageBox(NULL, g_szFuncDesc[FUNCSIZE], g_szFuncList[FUNCSIZE], MB_OK);
                FUNCSIZE++;
            }
            */
            wndfin.close();

            
            return 0;
            break;
        case WM_SIZE:
            RECT rectClient, rectStatus, rectTool;
            UINT uToolHeight, uStatusHeight, uClientAlreaHeight;

            GetWindowRect(hwnd,&rctA);
            wwidth = rctA.right - rctA.left;
            wheight = rctA.bottom - rctA.top;
            if(wParam != SIZE_MINIMIZED) {
                MoveWindow(GetDlgItem(hwnd, IDC_MAIN_TEXT), 60, 30, /*LOWORD(lParam)*/wwidth-160,/*HIWORD(lParam)*/wheight-240, TRUE);
                MoveWindow(GetDlgItem(hwnd, IDC_LINE_NUM), 0, 30, /*LOWORD(lParam)*/60,/*HIWORD(lParam)*/wheight-120, TRUE);
                MoveWindow(GetDlgItem(hwnd, ID_COMPILEORDER), 60, wheight-115, wwidth-160/*CW_USEDEFAULT*/, 25, TRUE);
                MoveWindow(GetDlgItem(hwnd, ID_COMPILERES), 60, wheight-210, wwidth-160/*CW_USEDEFAULT*/, 95, TRUE);
                MoveWindow(GetDlgItem(hwnd, ID_COMPILENOW), wwidth-100, 35, 80/*CW_USEDEFAULT*/, 80, TRUE);
                MoveWindow(GetDlgItem(hwnd, ID_SAVENOW), wwidth-100, 120, 80/*CW_USEDEFAULT*/, 80, TRUE);
                //MoveWindow(GetDlgItem(hwnd, ID_COMPILERES), 60, wheight-210, wwidth-180/*CW_USEDEFAULT*/, 95, TRUE);
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
                case IDM_FILE_NEW: {
                    GetModuleFileName(NULL, szFileNametmp, MAX_PATH*10);
                    ShellExecute(NULL, "open", szFileNametmp, "", "", SW_SHOWNORMAL);
                    break;
                }
                case IDM_LOADFILE: {
                    if(!LoadFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
                        MessageBox(hwnd, "Load of file failed.", "Error",MB_OK|MB_ICONEXCLAMATION);
                        fopend=0;
                        return FALSE;
                    }
                    break;
                }
                case IDM_SETTABWIDTH: {
                    SendEditor(SCI_SETTABWIDTH, 4, 0);//tab  4   崭
                    SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETBOLD, SCE_C_WORD, true);
                    SendEditor(SCI_SETCODEPAGE,SC_CP_UTF8);
                    break;
                }
                case IDM_DT: {
                    GetDlgItemText(hwnd, IDC_MAIN_TEXT, getallcodetmpstr, 200000);
                    MessageBox(hwnd, getallcodetmpstr, "", MB_OK);
                    MessageBox(NULL, i_to_str(GetScrollPos(GetDlgItem(hwnd, IDC_MAIN_TEXT), SB_VERT)).c_str(), "", MB_OK);
                    //MessageBox(NULL, i_to_str(pointtmp.x).c_str(), "", MB_OK);
                    cursorpos = SendEditor(SCI_GETCURRENTPOS);
                    dwTextLength = SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_GETLENGTH, SCI_NULL, SCI_NULL);
                    char pszText[dwTextLength];
                    SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_GETTEXT, dwTextLength+1, (LPARAM)pszText);
                    MessageBox(hwnd, i_to_str(pszText[cursorpos]).c_str(), "", MB_OK);
                    SendEditor(SCI_INSERTTEXT, -1, (LPARAM)")");
                    break;
                }
                case IDM_FILE_OPEN:
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
                        SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "Compiled" : "Not Compiled"));
                        SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                        break;
                    }
                    fcompiled=0;
                    /*settitle*/
                    titlestr01="Click 5.0 [ ";
                    titlestr01+=szFileName;
                    titlestr01+=" ]";
                    SetWindowText (hwnd, titlestr01.c_str());
                    SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "Compiled" : "Not Compiled"));
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                    /*end:settitle*/
                    break;
                case IDM_EDIT_ALL: {
                    SendEditor(SCI_SELECTALL);
                    break;
                }

                case IDM_PICKFONT:
                    cf.lStructSize = sizeof (CHOOSEFONT) ;
                    cf.hwndOwner = hwnd ;
                    cf.hDC = NULL ;
                    cf.lpLogFont = &lf ;
                    cf.iPointSize = 0 ;
                    cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_FORCEFONTEXIST;
                    cf.lCustData = 0 ;
                    cf.lpfnHook = NULL ;
                    cf.lpTemplateName = NULL ;
                    cf.hInstance = NULL ;
                    cf.lpszStyle = NULL ;
                    cf.nFontType = 0 ;
                    cf.nSizeMin = 0 ;
                    cf.nSizeMax = 0 ;
                    if (ChooseFont (&cf)) {
                        SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETSIZEFRACTIONAL, STYLE_DEFAULT, (sptr_t)cf.iPointSize * 10);
                        SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)lf.lfFaceName);
                        SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETITALIC, STYLE_DEFAULT, (sptr_t)lf.lfItalic);
                        SendMessage(GetDlgItem(hwnd, IDC_MAIN_TEXT), SCI_STYLESETWEIGHT, STYLE_DEFAULT, (sptr_t)lf.lfWeight);
                    }
                    break;
                case IDM_FILE_SAVEAS:
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
                case IDM_FILE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                case IDM_FILE_SAVE:
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
                case IDM_ABOUT:
                    /*settitle*/
                    SetWindowText (hwnd, "Click 5.0 [ About... ]");
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"About...");
                    /*end:settitle*/
                    MessageBox (hwnd, "Click IDE: 2020.4\nVersion: 5.0-Stable\nBy:23564 EricNTH.\nGUI: Win32 API.\nIntroduction: Click is an light, open-source, convenient C++ IDE which based on GNU MinGW.\nOnly for: Windows 7/8/8.1/10. You can contact us to get the XP Version.\nLicense: Apache License, Version 2.0\nTo learn more or get updates, please visit our official website: https://ericnth.cn/clickide/\nIf you meet some problems, please contact us or visit: Help->Get help..." , "About...", 0);
                    /*settitle*/
                    titlestr01="Click 5.0 [ ";
                    titlestr01+=szFileName;
                    titlestr01+=" ]";
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                    SetWindowText (hwnd, titlestr01.c_str());
                    /*end:settitle*/
                    break;
                case IDM_RUN:
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
                            MessageBox(hwnd, "Fail to run program! Please look at the details in the \"Information\" Box.", "Click 5.0", MB_OK|MB_ICONHAND);
                            sprintf(cmdbuf3, "Fail to run program: \r\n%s.exe\r\nMaybe it is because that you failed in compilation.\r\nAnd make sure that you added \"-o %%e\" option when inputting compiling order.", getcppfn(szFileName).c_str());
                            Addinfo(cmdbuf3);
                            titlestr01="Click 5.0 [ ";
                            titlestr01+=szFileName;
                            titlestr01+=" ]";
                            SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                            SetWindowText (hwnd, titlestr01.c_str());
                            break;
                        }
                        sprintf(cmdbuf3, "Running program: \r\n%s.exe", getcppfn(szFileName).c_str());
                        Addinfo(cmdbuf3);
                        sprintf (cmdbuf2, "%s.exe",getcppfn(szFileName).c_str());//sprintf (cmdbuf2, "start \"Click5.0-Executing [%s.exe]\" %s.exe",getcppfn(szFileName).c_str(),getcppfn(szFileName).c_str());
                        ShellExecute(NULL, "open", cmdbuf2, "", "", SW_SHOWNORMAL);//runprocess (cmdbuf2, 0, 1);
                    } else {
                        //MessageBox (hwnd, "You haven't compiled this file yet (or have failed in it),\nPlease compile it first!", "Can't Run!", MB_OK | MB_ICONERROR);
                        sprintf(cmdbuf3, "Unable to run program: \r\nPlease compile it (correctly) first.");
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
                case IDM_COMPILE:
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
                    SendEditor(SCI_SETCURSOR, SC_CURSORWAIT);
                    /*settitle*/
                    titlestr01="Click 5.0 [ Compiling ] [ ";
                    titlestr01+=szFileName;
                    titlestr01+=" ]";
                    SetWindowText (hwnd, titlestr01.c_str());
                    /*end:settitle*/
                    sprintf (cmdbuf1, "%s 2> %s_compile_tmp.log", compileordertmp.c_str(),szFileName);
                    sprintf (cmdbuf2, "start \"Click5.0-Executing [%s.exe]\" /max %s.exe",getcppfn(szFileName).c_str(),getcppfn(szFileName).c_str());
                    sprintf (cmdbuf3, "%s.exe",getcppfn(szFileName).c_str());
                    sprintf (cmdbuf4, "%s_compile_tmp.log",szFileName);
                    sprintf (cmdbuf5, "%s_compile_tmp.log",szFileName);
                    remove(cmdbuf3);//runprocess (cmdbuf3, 1, 0);
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
                    SendEditor(SCI_SETCURSOR, SC_CURSORNORMAL);
                    if (_access(cmdbuf3, X_OK) != -1) { //Exist
                        if (errreportcnt>1) {

                        }
                    }
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
                    remove(cmdbuf4);//runprocess (cmdbuf4, 1, 0);
                    /*settitle*/
                    titlestr01="Click 5.0 [ ";
                    titlestr01+=szFileName;
                    titlestr01+=" ]";
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                    SetWindowText (hwnd, titlestr01.c_str());
                    SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "Compiled" : "Not Compiled"));
                    /*end:settitle*/
                    break;
                case IDM_COMPILERUN:
                    SendMessage(hwnd, WM_COMMAND, IDM_COMPILE, (LPARAM)0);
                    Sleep(500);
                    SendMessage(hwnd, WM_COMMAND, IDM_RUN, (LPARAM)0);
                    break;

                case IDM_STARTCMD:
                    runprocess ((char*)"start /max \"Click 5.0 [Command]\"", 0, 1);
                    break;
                case IDM_RUNBAT:
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
                case IDM_DEBUG:
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
                case IDM_VVARI:
                    /*settitle*/
                    titlestr01="Click 5.0 [ Viewing Variables... ]";
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Viewing Variables...");
                    SetWindowText (hwnd, titlestr01.c_str());
                    /*end:settitle*/
                    sprintf (cmdbuf1, "szFileName\t= %s\nfsaved         \t= %s\nfopened      \t= %s\nfcompiled   \t= %s\nCurrentTime\t= %s\nCurrentMessage\t= %d/%d\nMessageCount\t= %lld", szFileName, (fsaved ? "True" : "False"), (fopend ? "True" : "False"), (fcompiled ? "True" : "False"), output_time().c_str(), WM_COMMAND, IDM_VVARI, variMsgCnt);
                    MessageBox (hwnd, cmdbuf1, "Variables...", MB_OK | MB_ICONINFORMATION);
                    /*settitle*/
                    titlestr01="Click 5.0 [ ";
                    titlestr01+=szFileName;
                    titlestr01+=" ]";
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                    SetWindowText (hwnd, titlestr01.c_str());
                    /*end:settitle*/
                    break;
                case IDM_GHELP:
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Helps...");
                    ShellExecute(NULL,TEXT("open"), TEXT("https://github.com/eric-nth/ClickIDE-5.x/blob/master/README.md"), TEXT(""),NULL,SW_SHOWNORMAL);
                    MessageBox(hwnd, "Please scroll to the bottom of this page to get online helps and hints!\n(If the browser hasn't brought you there, please visit: \nhttps://github.com/eric-nth/ClickIDE-5.x/blob/master/README.md)", "Helps", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");

                    break;
                case IDM_EDIT_UNDO:
                    SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, EM_UNDO, 0, 0);
                    break;
                case IDM_EDIT_CUT:
                    SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_CUT, 0, 0);
                    break;
                case IDM_EDIT_COPY:
                    SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_COPY, 0, 0);
                    break;
                case IDM_EDIT_PASTE:
                    SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_PASTE, 0, 0);
                    break;
                case IDM_EDIT_FIND:
                    ZeroMemory(&repfindtag, sizeof(repfindtag));
                    repfindtag.hwndOwner = hwnd;
                    repfindtag.lpstrFindWhat = (LPSTR)"\0";
                    repfindtag.Flags = FR_DOWN|FR_FINDNEXT|FR_MATCHCASE;
                    repfindtag.wFindWhatLen = MAX_PATH;
                    repfindtag.wReplaceWithLen = MAX_PATH;
                    //repfindtag.lpstrFindWhat = szFindWhat;
                    repfindtag.lCustData = 0;
                    repfindtag.lpfnHook = NULL;
                    repfindtag.lStructSize = sizeof(repfindtag);
                    FindText(&repfindtag);
                    //Addinfo(repfindtag.lpstrFindWhat);
                    //SendEditor(SCI_SEARCHINTARGET, SendEditor(SCI_GETTARGETTEXT, 0, (LPARAM)repfindtag.lpstrFindWhat));
                    break;
                case IDM_FLSTB:
                    SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)"Click 5.0 IDE");
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                    SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(fcompiled ? "Compiled" : "Not Compiled"));
                    SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)"");
                    SendMessage(g_hStatusBar, SB_SETTEXT, 4, (LPARAM)szFileName);
                    break;
                case IDM_GHTML: {
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

                case IDM_ASTYLE: {
                    MessageBox(hwnd, "This version(ClickIDE 5.0.0 Stable) doesn't have this function. \nPlease expect for version 5.1.", "Ah oh~", MB_OK|MB_ICONINFORMATION);
                    break;
                    PostMessage(hwnd, WM_COMMAND, IDM_FILE_SAVE, (LPARAM)"");
                    char astylestr[MAX_PATH*6];
                    sprintf(astylestr, "--recursive --style=bsd --convert-tabs --indent=spaces=4 --attach-closing-while --indent-switches --indent-namespaces --indent-continuation=4 --indent-preproc-block --indent-preproc-define --indent-preproc-cond --indent-col1-comments --pad-oper --pad-paren-in --unpad-paren --delete-empty-lines --align-pointer=name --align-reference=name --break-elseifs --add-braces >%s.astyle.log %s*", szFileName, szFileName);
                    //system(astylestr)
                    MessageBox(NULL, astylestr, "", MB_OK);
                    ShellExecute(NULL,TEXT("open"), "AStyle.exe", TEXT(astylestr),NULL,SW_HIDE);
                    sprintf(astylestr, "del %s.orig", szFileName);
                    system(astylestr);

                    break;
                }
                case IDM_GITHUB: {
                        ShellExecute(NULL,TEXT("open"), TEXT("https://github.com/eric-nth/ClickIDE-5.x"), TEXT(""),NULL,SW_SHOWNORMAL);
                    break;
                }
                case IDM_WEBSITE: {
                    ShellExecute(NULL,TEXT("open"), TEXT("https://ericnth.cn/clickide/"), TEXT(""),NULL,SW_SHOWNORMAL);
                    break;
                }
                case IDM_UPDATE: {
                    GetModuleFileName(0, currentpath, MAX_PATH*10);
                    strcat(currentpath, "\\..\\");
                    sprintf(updatefilename, "%sclickupdate.exe", currentpath);
                    GainAdminPrivileges(updatefilename, currentpath,1, 0);
                    return 0;
                }
                case IDM_ADDBRA: {
                    SendEditor(SCI_INSERTTEXT, -1, (LPARAM)lParam);
                    break;
                }
                case IDM_ADDBRAEX: {
                    SendEditor(SCI_INSERTTEXT, HIWORD(wParam), (LPARAM)lParam);
                    break;
                }
                case IDM_AUTOSAVE: {
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Autosaving...");
                    sprintf(szFileName2, "%s.clickide.autosave.txt", szFileName);
                    if ((!fsaved && !fopend) || strcmp(szFileName, "Untitled") == 0) {
                        ;
                    } else {
                        SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName2);
                        SetFileAttributes(szFileName2, FILE_ATTRIBUTE_HIDDEN);
                    }
                    SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
                    break;
                }
                case ID_COMPILENOW: {
                    SendMessage(hwnd, WM_COMMAND, IDM_COMPILERUN, 0);
                    break;
                }
                case ID_SAVENOW: {
                    SendMessage(hwnd, WM_COMMAND, IDM_FILE_SAVE, 0);
                    break;
                }
                case IDM_SETCURPOS: {
                    SendEditor(SCI_SETCURRENTPOS, HIWORD(wParam));
                }
                case IDM_SELECTEMPTY: {
                    SendEditor(SCI_GETSELECTIONEMPTY);
                }
                    /*
                    case IDM_CHECKINDENT: {
                        if (SendEditor(SCI_GETLINEINDENTATION, cursorpos-1) <= 0) {
                            break;
                        }
                        if (SendEditor(SCI_GETLINEINDENTATION, cursorpos) > 0) {
                            break;
                        }
                        char tmpspaces[1000];
                        strcpy(tmpspaces, "");
                        //Addinfo(i_to_str(SendEditor(SCI_GETLINEINDENTATION, cursorpos - 1)).c_str());
                        //SendEditor(SCI_SETLINEINDENTATION, cursorpos, SendEditor(SCI_GETLINEINDENTATION, cursorpos - 1));
                        inttmpnum = SendEditor(SCI_GETLINEINDENTATION, cursorpos - 1);
                        for (int i = 0; i < inttmpnum / 4; i++) {
                            strcat(tmpspaces, "\t");
                        }
                        for (int i = 0; i < inttmpnum % 4; i++) {
                            strcat(tmpspaces, " ");
                        }
                        SendEditor(SCI_GETLINE, cursorpos-1, (LPARAM)getallcodetmpstr);
                        if (strchr(getallcodetmpstr, '{') != NULL) {
                        for (int i = strlen(getallcodetmpstr)-1; i >= 0; i--) {
                            if (getallcodetmpstr[i] == '{') {
                                strcat(tmpspaces, "\t");
                                break;
                            } else if (getallcodetmpstr[i] == ' ' || getallcodetmpstr[i] == '\r' || getallcodetmpstr[i] == '\n') {
                                continue;
                            } else {
                                break;
                            }
                        }
                        }
                        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_ADDBRAEX, SendEditor(SCI_GETCURRENTPOS)-1), (LPARAM)tmpspaces);
                        break;
                    }*/
            }
            hMenu = GetMenu(hwnd);
            hFileMenu = GetSubMenu(hMenu, 0);
            hCompileMenu = GetSubMenu(hMenu, 1);
            EnableMenuItem(hFileMenu, IDM_FILE_SAVE, MF_BYCOMMAND | (!(!fsaved && !fopend) || strcmp(szFileName, "Untitled") ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(hCompileMenu, IDM_RUN, MF_BYCOMMAND | ((fcompiled) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(hCompileMenu, IDM_DEBUG, MF_BYCOMMAND | ((fcompiled) ? MF_ENABLED : MF_GRAYED));
            char tishitext[1024];
            linecount = 0;
            sprintf(tishitext, "Welcome\nto\nClickIDE!\n\nVersion:\n5.0.0\n\nWords:\n%d\nLines:\n%d\n\nFont size:%d", SendEditor(SCI_GETTEXTLENGTH), SendEditor(SCI_GETLINECOUNT), cf.iPointSize);
            SetDlgItemText(hwnd, IDC_LINE_NUM, tishitext);
            cursorpos = SendEditor(SCI_GETCURRENTPOS);
            if (SendEditor(SCI_GETCHARAT, cursorpos) == '(') {
                PostMessage(hwnd, WM_COMMAND, IDM_ADDBRA, (LPARAM)")");
            }
            if (SendEditor(SCI_GETCHARAT, cursorpos) == '[') {
                PostMessage(hwnd, WM_COMMAND, IDM_ADDBRA, (LPARAM)"]");
            }
            if (SendEditor(SCI_GETCHARAT, cursorpos) == '{') {
                PostMessage(hwnd, WM_COMMAND, IDM_ADDBRA, (LPARAM)"\n}");
            }
            if (!tabwidthset) {
                PostMessage(hwnd, WM_COMMAND, IDM_SETTABWIDTH, 0);
                tabwidthset = 1;
            }
            //if (SendEditor(SCI_GETCHARAT, cursorpos) == '{') {PostMessage(hwnd, WM_COMMAND, IDM_ADDBRA, (LPARAM)"\r\n\t\r\n}");}
            cursorpos = SendEditor(SCI_LINEFROMPOSITION, SendEditor(SCI_GETCURRENTPOS));

            break;
            /*
            case WM_CTLCOLOREDIT: {
                  HDC hdc = (HDC)wParam;
                   SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
                   SetBkColor((HDC)wParam,RGB(0x0, 0x0, 0x0));
                   return (long long int)((HBRUSH)GetStockObject(NULL_BRUSH));
                   break;
               }*/
        case WM_NOTIFY: {
            SCNotification* notify = (SCNotification*)lParam;
            static int LastProcessedChar = 0;
            switch (notify->nmhdr.code) {
                case SCN_CHARADDED: {
                    LastProcessedChar = notify->ch;
                    static const char* pCallTipNextWord = NULL;//下一个高亮位置
                    static const char* pCallTipCurDesc = NULL;//当前提示的函数信息
                    /*
                    if (notify->ch == '\n') {
                        SendEditor(SCI_SETLINEINDENTATION, , 8);
                    }
                     */
                    if(notify->ch == '(') //如果输入了左括号，显示函数提示
                    {
                        char word[1000]; //保存当前光标下的单词(函数名)
                        Sci_TextRange tr;    //用于SCI_GETTEXTRANGE命令
                        int pos = SendEditor(SCI_GETCURRENTPOS); //取得当前位置（括号的位置）
                        int startpos = SendEditor(SCI_WORDSTARTPOSITION,pos-1);//当前单词起始位置
                        int endpos = SendEditor(SCI_WORDENDPOSITION,pos-1);//当前单词终止位置
                        tr.chrg.cpMin = startpos;  //设定单词区间，取出单词
                        tr.chrg.cpMax = endpos;
                        tr.lpstrText = word;
                        SendEditor(SCI_GETTEXTRANGE,0, sptr_t(&tr));
                        for(size_t i=0; i<FUNCSIZE; i++) {
                            if(strcmp(g_szFuncList[i],word) == 0/*memcmp(g_szFuncList[i],word,sizeof(g_szFuncList[i])) == 0*/) {
                                pCallTipCurDesc = g_szFuncDesc[i];
                                SendEditor(SCI_CALLTIPSHOW,pos,sptr_t(pCallTipCurDesc));
                                const char *pStart = strchr(pCallTipCurDesc,'(')+1;
                                const char *pEnd = strchr(pStart,',');
                                if(pEnd == NULL) pEnd = strchr(pStart,')');
                                SendEditor(SCI_CALLTIPSETHLT,
                                           pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
                                pCallTipNextWord = pEnd+1;
                                break;
                            }
                        }
                    }
                    else if(notify->ch == ')') {
                        SendEditor(SCI_CALLTIPCANCEL);
                        pCallTipCurDesc = NULL;
                        pCallTipNextWord = NULL;
                    } else if(notify->ch == ',' && SendEditor(SCI_CALLTIPACTIVE) && pCallTipCurDesc) {
                        const char *pStart = pCallTipNextWord;
                        const char *pEnd = strchr(pStart,',');
                        if(pEnd == NULL) pEnd = strchr(pStart,')');
                        if(pEnd == NULL) SendEditor(SCI_CALLTIPCANCEL);
                        else {
                            SendEditor(SCI_CALLTIPSETHLT,pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
                            pCallTipNextWord = pEnd+1;
                        }
                    }
                    break;
                }
                case SCN_UPDATEUI: {
                    if (LastProcessedChar != 0) {
                        int pos = SendEditor(SCI_GETCURRENTPOS); //取得当前位置
                        int line = SendEditor(SCI_LINEFROMPOSITION,pos); //取得当前行
                        //如果最后输入的字符是右括号的话就自动让当前行缩进和它匹配的左括号所在行一致
                        if( strchr("})>]",LastProcessedChar) &&
                            isspace(SendEditor(SCI_GETCHARAT,pos-2)) && //要求右括号左边是空白字符
                            LastProcessedChar!=0)
                        {
                            //找前一个单词起始位置，这里用它来确定右括号左边是否全是空白字符
                            int startpos = SendEditor(SCI_WORDSTARTPOSITION,pos-1,false);
                            int linepos = SendEditor(SCI_POSITIONFROMLINE,line); //当前行起始位置
                            if(startpos == linepos) //这样相当于判断右括号左边是否全是空白字符
                            {
                                int othpos = SendEditor(SCI_BRACEMATCH,pos-1); //得到对应的左括号所在的位置
                                int othline = SendEditor(SCI_LINEFROMPOSITION,othpos);  //左括号所在行
                                int nIndent = SendEditor(SCI_GETLINEINDENTATION,othline);//左括号所在行的缩进值
                                // 替换右括号前面的空白字符，使之与左括号缩进一致
                                char space[1024];
                                memset(space,' ',1024);
                                SendEditor(SCI_SETTARGETSTART, startpos);
                                SendEditor(SCI_SETTARGETEND, pos-1);
                                SendEditor(SCI_REPLACETARGET,nIndent,(sptr_t)space);
                            }
                        }
                        // 如果输入的是回车，则保持与上一行缩进一致
                        // 如果上一行最后有效字符为左括号，就多缩进四个空格
                        if(LastProcessedChar == '\n')
                        {
                            if(line > 0)
                            {
                                // 得到上一行缩进设置
                                int nIndent = SendEditor(SCI_GETLINEINDENTATION,line-1);
                                // 查找上一行最后一个有效字符（非空白字符）
                                int nPrevLinePos = SendEditor(SCI_POSITIONFROMLINE,line-1);
                                int c = ' ';
                                for(int p = pos-2;
                                    p>=nPrevLinePos && isspace(c);
                                    p--, c=SendEditor(SCI_GETCHARAT,p));
                                // 如果是左括号，就多缩进四格
                                if(c && strchr("{([<",c)) nIndent+=4;
                                // 缩进...
                                char space[1024];
                                memset(space,' ',1024);
                                space[nIndent] = 0;
                                SendEditor(SCI_REPLACESEL, 0, (sptr_t)space);
                            }
                        }
                        LastProcessedChar = 0;
                    }
                }
                case SCN_MARGINCLICK: {
                    const int line_number = SendEditor(SCI_LINEFROMPOSITION,notify->position);
                    SendEditor(SCI_TOGGLEFOLD, line_number);
                }
            }
            break;
        }
        case WM_CLOSE:
            SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"Exitting...");
            /*settitle*/
            titlestr01="Click 5.0 [ Exiting... ]";
            sprintf (cmdbuf4, "%s_compile_tmp.log",szFileName);
            remove(cmdbuf4);
            SetWindowText (hwnd, titlestr01.c_str());
            /*end:settitle*/
            if (SendEditor(SCI_GETTEXTLENGTH) <= 0) {
                programmeexiterrorstatusflag = 0;
                DestroyWindow(hwnd);
            }
            inttmpnum = MessageBox (hwnd, "Do you want to save the changes before you quit?", "Exiting...", MB_YESNOCANCEL | MB_ICONQUESTION);
            if (inttmpnum == IDYES) {
                if ((!fsaved && !fopend) || strcmp(szFileName, "Untitled") == 0) {
                    if (!DoFileOpenSave(hwnd, TRUE)) {
                        SetWindowText (hwnd, "Click 5.0");
                        break;
                    } else {
                        remove(szFileName2);
                    }
                } else {
                    if(!SaveFile(GetDlgItem(hwnd, IDC_MAIN_TEXT), szFileName)) {
                        MessageBox(hwnd, "Save file failed.\n(Or this is an empty file.)", "Error",MB_OK|MB_ICONEXCLAMATION);
                        fsaved=0;
                    } else {
                        sprintf(cmdbuf4, "%s.clickide.autosave.txt",szFileName);
                        remove(cmdbuf4);
                    }
                }
                PostMessage(hwnd, WM_COMMAND, IDM_FILE_SAVEAS, 0);
                programmeexiterrorstatusflag = 0;
                DestroyWindow(hwnd);
                break;
            } else if (inttmpnum == IDNO) {
                programmeexiterrorstatusflag = 0;
                DestroyWindow(hwnd);
                break;
            }
            /*settitle*/
            titlestr01="Click 5.0 [ ";
            titlestr01+=szFileName;
            titlestr01+=" ]";
            SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)"...");
            SetWindowText (hwnd, titlestr01.c_str());
            /*end:settitle*/
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
    clock_t programstarttime = clock();
    SafeGetNativeSystemInfo(&si);
    HACCEL haccel;
    hDLL = LoadLibrary(TEXT("SciLexer"));
    if (hDLL == NULL) {
        MessageBox(NULL, "Error: SciLexer.dll Not found.\n\tPlease contact the software supporting team.\nFatal: Unable to register window class: \"Scintilla\".\nExecution failed.", "Click 5.0", MB_ICONHAND | MB_OK);
        return 0;
    }
    if (strcmp(lpCmdLine, "") != 0) {
        hasstartopenfile = 1;
        if (_access(lpCmdLine, W_OK) == -1) {
            MessageBox(NULL, "File does not exist or unable to read!", "Click 5.0", MB_OK);
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
    wc.hIcon         = LoadIcon(NULL, "A"/*IDI_APPLICATION*/);
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

    haccel = LoadAccelerators(hInstance, "MAINACCEL");
    if (haccel == NULL) {
        MessageBox(NULL, "Fail loading accelerators", "Click Error", MB_OK | MB_ICONHAND);
    }

    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        if ((clock() - programstarttime) % 60000 > 59500) {
            SendMessage(hwnd, WM_COMMAND, IDM_AUTOSAVE, 0);
        }
        variMsgCnt++;
        if (!TranslateAccelerator(
                hwnd,  // handle to receiving window
                haccel,    // handle to active accelerator table
                &Msg))         // message data
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    FreeLibrary(hDLL);
    return Msg.wParam;
}