////////////////////////////////////////////////////////////////////////////////////////////////
// GbxMove.cpp - Gbx File Mover by Electron
//
#ifndef STRICT
#define STRICT 1
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <tchar.h>

#include "resource.h"

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x00000040
#endif
#ifndef BIF_UAHINT
#define BIF_UAHINT 0x00000100
#endif
#ifndef BFFM_SETOKTEXT
#define BFFM_SETOKTEXT (WM_USER + 105)
#endif
#ifndef BFFM_SETEXPANDED
#define BFFM_SETEXPANDED (WM_USER + 106)
#endif

#if !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

typedef struct _browseinit
{
	LPCITEMIDLIST pidlSelection;
	LPCWSTR lpszOKText;
} BROWSEINIT, *PBROWSEINIT, *LPBROWSEINIT;

#define LoadStr(Id, Str) LoadString(hInstance, Id, Str, _countof(Str))
#define TcsNCat(Dest, Src) _tcsncat(Dest, Src, _countof(Dest) - _tcslen(Dest) - 1)
#define MsgBox(Text) MessageBox(NULL, Text, TEXT("Gbx File Mover"), MB_OK | MB_ICONEXCLAMATION)

////////////////////////////////////////////////////////////////////////////////////////////////

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED && lpData != NULL)
	{
		LPBROWSEINIT lpBrowseInit = (PBROWSEINIT)lpData;

		if (lpBrowseInit->pidlSelection != NULL)
			SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)lpBrowseInit->pidlSelection);

		if (lpBrowseInit->lpszOKText != NULL && lpBrowseInit->lpszOKText[0] != TEXT('\0'))
			SendMessage(hwnd, BFFM_SETOKTEXT, FALSE, (LPARAM)lpBrowseInit->lpszOKText);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TCHAR szInputPath[MAX_PATH];
	TCHAR szOutputPath[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	szInputPath[0] = TEXT('\0');
	szOutputPath[0] = TEXT('\0');
	szFileName[0] = TEXT('\0');

	BOOL bOpenFile = FALSE;
	BOOL g_bGerUI = (PRIMARYLANGID(GetUserDefaultLangID()) == LANG_GERMAN);

	// Check command arguments
	if (lpCmdLine == NULL || lpCmdLine[0] == '\0')
	{
		if (LoadStr(g_bGerUI ? IDP_GER_NO_COMMAND : IDP_ENG_NO_COMMAND, szOutputPath) > 0)
			MsgBox(szOutputPath);

		return 0;
	}

	int argc;
	LPTSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if (argv != NULL && argc > 1)
	{
		if (_tcslen(argv[1]) > 1 &&
			(argv[1][0] == TEXT('/') || argv[1][0] == TEXT('-')) &&
			(argv[1][1] == TEXT('r') || argv[1][1] == TEXT('R')))
		{
			bOpenFile = TRUE;	// Open the file after moving
			if (argc > 2)
				lstrcpyn(szInputPath, argv[2], _countof(szInputPath));
		}
		else
			lstrcpyn(szInputPath, argv[1], _countof(szInputPath));
	}

	// Remove quotation marks
	TCHAR* pch = _tcschr(szInputPath, TEXT('\"'));
	if (pch != NULL)
	{
		TCHAR szTemp[2 * MAX_PATH];
		lstrcpyn(szTemp, pch + 1, _countof(szTemp));

		pch = _tcschr(szTemp, TEXT('\"'));
		if (pch != NULL)
			*pch = TEXT('\0');

		lstrcpyn(szInputPath, szTemp, _countof(szInputPath));
	}

	// Check whether the file exists
	if (GetFileAttributes(szInputPath) == 0xFFFFFFFF)
	{
		if (LoadStr(g_bGerUI ? IDP_GER_FILE_NOT_FOUND : IDP_ENG_FILE_NOT_FOUND, szOutputPath) > 0)
		{
			TCHAR szTemp[2 * MAX_PATH];
			wsprintf(szTemp, szOutputPath, szInputPath);
			MsgBox(szTemp);
		}

		return 0;
	}

	// Determine file name
	pch = _tcsrchr(szInputPath, TEXT('\\'));
	lstrcpyn(szFileName, pch != NULL ? pch + 1 : szInputPath, _countof(szFileName));

	BOOL bIsMap = FALSE;
	BOOL bIsChallenge = FALSE;
	BOOL bIsReplay = FALSE;

	if (_tcsstr((szFileName), TEXT(".Map.")) != NULL)
		bIsMap = TRUE;
	else if (_tcsstr((szFileName), TEXT(".Challenge.")) != NULL)
		bIsChallenge = TRUE;
	else if (_tcsstr((szFileName), TEXT(".Replay.")) != NULL)
		bIsReplay = TRUE;

	if (!SUCCEEDED(CoInitialize(NULL)))
		return 0;

	// Determine root folder for SHBrowseForFolder
	LPITEMIDLIST pidlRoot = NULL;
	LPITEMIDLIST pidlSelection = NULL;

	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidlRoot)))
	{
		if (SHGetPathFromIDList(pidlRoot, szOutputPath))
		{
			if (bIsMap)
				TcsNCat(szOutputPath, TEXT("\\ManiaPlanet"));
			else if (bIsChallenge)
				TcsNCat(szOutputPath, TEXT("\\TrackMania\\Tracks"));
			else if (bIsReplay)
			{
				TCHAR szTemp[MAX_PATH];
				lstrcpyn(szTemp, szOutputPath, _countof(szTemp));
				TcsNCat(szTemp, TEXT("\\ManiaPlanet"));
				if (GetFileAttributes(szTemp) != 0xFFFFFFFF)
					lstrcpyn(szOutputPath, szTemp, _countof(szOutputPath));
				else
					TcsNCat(szOutputPath, TEXT("\\TrackMania\\Tracks"));
			}

			if (GetFileAttributes(szOutputPath) != 0xFFFFFFFF)
			{
				LPSHELLFOLDER pDesktopFolder = NULL;
				LPITEMIDLIST pidlSubFolder = NULL;

				if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
				{
					if (SUCCEEDED(pDesktopFolder->ParseDisplayName(NULL, NULL, szOutputPath, NULL, &pidlSubFolder, NULL)))
					{
						if (pidlRoot != NULL)
							CoTaskMemFree(pidlRoot);
						pidlRoot = pidlSubFolder;
					}

					if (bIsMap)
					{
						TcsNCat(szOutputPath, TEXT("\\Maps\\Downloaded"));
						if (GetFileAttributes(szOutputPath) != 0xFFFFFFFF)
							pDesktopFolder->ParseDisplayName(NULL, NULL, szOutputPath, NULL, &pidlSelection, NULL);
					}
					else if (bIsChallenge)
					{
						TcsNCat(szOutputPath, TEXT("\\Challenges\\Downloaded"));
						if (GetFileAttributes(szOutputPath) != 0xFFFFFFFF)
							pDesktopFolder->ParseDisplayName(NULL, NULL, szOutputPath, NULL, &pidlSelection, NULL);
					}
					else if (bIsReplay)
					{
						TcsNCat(szOutputPath, TEXT("\\Replays\\Downloaded"));
						if (GetFileAttributes(szOutputPath) != 0xFFFFFFFF)
							pDesktopFolder->ParseDisplayName(NULL, NULL, szOutputPath, NULL, &pidlSelection, NULL);
					}

					pDesktopFolder->Release();
				}
			}
		}
	}

	int nRet = 0;
	TCHAR szOKText[128];
	TCHAR szInstructions[256];

	BROWSEINIT browseInit;
	memset(&browseInit, 0, sizeof(browseInit));
	browseInit.pidlSelection = pidlSelection;
	if (LoadStr(g_bGerUI ? IDS_GER_OKTEXT : IDS_ENG_OKTEXT, szOKText) > 0)
		browseInit.lpszOKText = szOKText;

	BROWSEINFO browseInfo;
	memset(&browseInfo, 0, sizeof(browseInfo));
	browseInfo.pidlRoot = pidlRoot;
	browseInfo.pszDisplayName = szOutputPath;
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_UAHINT;
	browseInfo.lpfn = BrowseCallbackProc;
	browseInfo.lParam = (LPARAM)&browseInit;
	if (LoadStr(g_bGerUI ? IDP_GER_INSTRUCTIONS : IDP_ENG_INSTRUCTIONS, szInstructions) > 0)
		browseInfo.lpszTitle = szInstructions;

	// Call SHBrowseForFolder
	LPITEMIDLIST pidlBrowse = SHBrowseForFolder(&browseInfo);
	if (pidlBrowse != NULL)
	{
		if (SHGetPathFromIDList(pidlBrowse, szOutputPath))
		{
			szInputPath[_tcslen(szInputPath) + 1] = TEXT('\0');
			szOutputPath[_tcslen(szOutputPath) + 1] = TEXT('\0');

			SHFILEOPSTRUCT fos;
			memset(&fos, 0, sizeof(fos));
			fos.wFunc = FO_MOVE;
			fos.pFrom = szInputPath;
			fos.pTo = szOutputPath;

			// Move the file
			nRet = SHFileOperation(&fos);

			if (bOpenFile && nRet == 0 && fos.fAnyOperationsAborted == FALSE)
			{
				TcsNCat(szOutputPath, TEXT("\\"));
				TcsNCat(szOutputPath, szFileName);

				// Open the file
				ShellExecute(NULL, NULL, szOutputPath, NULL, NULL, SW_SHOWNORMAL);
			}
		}

		CoTaskMemFree(pidlBrowse);
	}

	if (pidlSelection != NULL)
		CoTaskMemFree(pidlSelection);
	if (pidlRoot != NULL)
		CoTaskMemFree(pidlRoot);

	CoUninitialize();

	return nRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////
