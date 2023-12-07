#ifndef _PLUGINMAIN_H
#define _PLUGINMAIN_H

// Include files required for plugin SDK
#include <TitanEngine.h>
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <intrin.h>
#include <_plugins.h>
#include "res\resource.h"

#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif //DLL_EXPORT

// Superglobal variables
extern int pluginHandle;
extern HWND hwndDlg;
extern HINSTANCE hInstDLL;
extern int hMenu;
extern int hMenuDisasm;
extern int hMenuDump;
extern int hMenuStack;

#define szMemoryDumpTo TEXT("MemoryDumpTo v%s")
#define szCopyright TEXT("©2023 OnLyOnE")
#define szCopyright2 TEXT(", aLL rIGHTS rEVERSED")
#define szMemoryDumpToAbout (szMemoryDumpTo TEXT(" \n") szCopyright)
#define plugin_name "MemoryDumpTo"
#define plugin_version 1
#define sz_plugin_version TEXT("0.1")

#define lengthof(s) { sizeof(s) / sizeof((s)[0]) }

typedef struct _INIT_PARAM
{
	BOOL   ProgLang;
	SIZE_T DataSize;
	LPVOID DataPtr;
}INIT_PARAM, * PINIT_PARAM;

enum _SizeData
{
    MD_Byte,
    MD_Word,
    MD_Dword
};

enum MenuAction
{
    MD_ABOUT,
    MD_FILE,
    MD_DELPHI,
    MD_CPP,
    MD_ASM,
    MD_VB
};

BOOL SaveToFile();
BOOL MemoryDumpToSource(BOOL ProgLang);
PVOID CreateTableText(PVOID InBuffer, SIZE_T InBuffSize, BOOL ProgLang, BOOL SizeSeg);
BOOL CopyToClipboard(HWND hWnd);
VOID ClearClipboard(HWND hWnd);

// menu identifiers
#define MENU_TEST 1

#ifdef __cplusplus
extern "C"
{
#endif

// Default plugin exports - required
DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct);
DLL_EXPORT bool plugstop();
DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct);

#ifdef __cplusplus
}
#endif

#endif //_PLUGINMAIN_H
