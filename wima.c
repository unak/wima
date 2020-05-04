#include "wima.h"


enum {
    ERR_UNKNOWN_OPTION = 1,
    ERR_NO_TARGET,
    ERR_BOTH_MIN_MAX,
    ERR_WITHOUT_PARAM,
    ERR_NOT_NUMBER,
    ERR_CREATE_PROCESS,
    ERR_MEMORY,
};


void ShowHelp()
{
    MessageBoxW(NULL, L"Usage: wima <option...> 実行プログラム <実行プログラムへの引数...>\n"
        L"\n"
        L"option:\n"
        L"  -x --maximize     実行プログラムを最大化する\n"
        L"  -n --minimize     実行プログラムを最小化する\n"
        L"  -cNUM --close=NUM NUM秒後に実行プログラムを閉じる(0:閉じない)\n"
        L"  -h --help         このヘルプを表示する",
        L"wima", MB_ICONINFORMATION|MB_OK);
}

void ShowError(int err, LPWSTR str)
{
    WCHAR msg[4096];

    switch (err) {
    case ERR_UNKNOWN_OPTION:
        StringCbPrintfW(msg, sizeof(msg), L"不明なオプション %s が指定されました", str);
        break;
    case ERR_NO_TARGET:
        lstrcpyW(msg, L"実行ファイルを指定してください");
        break;
    case ERR_BOTH_MIN_MAX:
        lstrcpyW(msg, L"最大化と最小化の両方が指定されています");
        break;
    case ERR_WITHOUT_PARAM:
        StringCbPrintfW(msg, sizeof(msg), L"%s の引数がありません", str);
        break;
    case ERR_NOT_NUMBER:
        StringCbPrintfW(msg, sizeof(msg), L"%s は有効な数値ではありません", str);
        break;
    case ERR_CREATE_PROCESS:
        StringCbPrintfW(msg, sizeof(msg), L"実行ファイル %s が起動できません", str);
        break;
    case ERR_MEMORY:
        lstrcpyW(msg, L"メモリ確保に失敗しました");
        break;
    default:
        StringCbPrintfW(msg, sizeof(msg), L"エラーが発生しました: %d", err);
        break;
    }

    MessageBoxW(NULL, msg, L"wima", MB_ICONSTOP|MB_OK);
}

int AtoI(LPWSTR str)
{
    const LPWSTR table = L"0123456789";
    int num = 0;
    while (*str) {
        num *= 10;
        const LPWSTR p = StrChrW(table, *str++);
        if (!p) {
            return -1;
        }
        num += (int)(p - table);
    }
    return num;
}

int ExecuteProcess(int argc, LPWSTR *wargv, BOOL minimize, BOOL maximize, int close)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    LPWSTR cmdline = NULL;
    int len = 0;
    int i;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (minimize || maximize) {
        si.dwFlags = STARTF_USESHOWWINDOW;
        if (minimize) {
            si.wShowWindow = SW_MINIMIZE;
        } else if (maximize) {
            si.wShowWindow = SW_MAXIMIZE;
        }
    }

    for (i = 0; i < argc; i++) {
        len += lstrlenW(wargv[i])+1;
    }
    if (len > 0) {
        cmdline = VirtualAlloc(NULL, sizeof(WCHAR)*len, MEM_COMMIT, PAGE_READWRITE);
        if (!cmdline) {
            ShowError(ERR_MEMORY, NULL);
            return 1;
        }
        ZeroMemory(cmdline, sizeof(WCHAR)*len);
        for (i = 0; i < argc; i++) {
            if (i != 0) {
                lstrcatW(cmdline, L" ");
            }
            lstrcatW(cmdline, wargv[i]);
        }
    }

    if (!CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        ShowError(ERR_CREATE_PROCESS, cmdline);
        if (cmdline) {
            VirtualFree(cmdline, sizeof(WCHAR)*len, MEM_DECOMMIT);
        }
        return 1;
    }
    WaitForInputIdle(pi.hProcess, 1000); // wait max 1 sec
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (cmdline) {
        VirtualFree(cmdline, sizeof(WCHAR)*len, MEM_DECOMMIT);
    }

    if (close > 0) {
        Sleep(close * 1000);
        PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0);
    }

    return 0;
}

int MyMain(int argc, LPWSTR *wargv)
{
    BOOL maximize = FALSE;
    BOOL minimize = FALSE;
    int close = 0;

    int i;
    for (i = 1; i < argc; i++) {
        if (wargv[i][0] != L'-' && wargv[i][0] != L'/') {
            break;
        }
        if (wargv[i][1] == wargv[i][0]) {
            if (StrCmpW(&wargv[i][2], L"help") == 0) {
                ShowHelp();
                return 0;
            } else if (StrCmpW(&wargv[i][2], L"minimize") == 0) {
                minimize = TRUE;
            } else if (StrCmpW(&wargv[i][2], L"maximize") == 0) {
                maximize = TRUE;
            } else if (StrCmpW(&wargv[i][2], L"close") == 0) {
                if (++i >= argc) {
                    ShowError(ERR_WITHOUT_PARAM, wargv[i-1]);
                    return 1;
                }
                close = AtoI(wargv[i]);
                if (close < 0) {
                    ShowError(ERR_NOT_NUMBER, wargv[i]);
                    return 1;
                }
            } else if (StrCmpNW(&wargv[i][2], L"close=", 6) == 0) {
                close = AtoI(&wargv[i][8]);
                if (close < 0) {
                    ShowError(ERR_NOT_NUMBER, &wargv[i][8]);
                    return 1;
                }
            } else {
                ShowError(ERR_UNKNOWN_OPTION, wargv[i]);
                return 1;
            }
        } else {
            int pos;
            BOOL cont;
            if (!wargv[i][1]) {
                ShowError(ERR_UNKNOWN_OPTION, wargv[i]);
                return 1;
            }
            for (pos = 1, cont = TRUE; cont && wargv[i][pos]; pos++) {
                switch (wargv[i][pos]) {
                case L'h':
                    ShowHelp();
                    return 0;
                case L'n':
                    minimize = TRUE;
                    break;
                case L'x':
                    maximize = TRUE;
                    break;
                case L'c':
                    cont = FALSE;
                    if (wargv[i][pos+1]) {
                        close = AtoI(&wargv[i][pos+1]);
                        if (close < 0) {
                            ShowError(ERR_NOT_NUMBER, &wargv[i][pos+1]);
                            return 1;
                        }
                    } else {
                        if (++i >= argc) {
                            ShowError(ERR_WITHOUT_PARAM, wargv[i-1]);
                            return 1;
                        }
                        close = AtoI(wargv[i]);
                        if (close < 0) {
                            ShowError(ERR_NOT_NUMBER, wargv[i]);
                            return 1;
                        }
                    }
                    break;
                default:
                    ShowError(ERR_UNKNOWN_OPTION, wargv[i]);
                    return 1;
                }
            }
        }
    }
    if (i >= argc) {
        ShowError(ERR_NO_TARGET, NULL);
        return 1;
    }
    if (minimize && maximize) {
        ShowError(ERR_BOTH_MIN_MAX, NULL);
        return 1;
    }

    return ExecuteProcess(argc-i, &wargv[i], minimize, maximize, close);
}
