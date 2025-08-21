
#include "MemoryDumpTo.h"

// GLOBAL Plugin SDK variables
int pluginHandle;
HWND hwndDlg;
HINSTANCE hInstDLL;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;

BOOL APIENTRY DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    hInstDLL = hinstDLL;
    return TRUE;
}

BOOL AboutDlg()
{
    MSGBOXPARAMS mbp;
    TCHAR Buffer[MAX_PATH / 2];

    __stosb((PBYTE)&mbp, 0, sizeof(mbp));
    __stosb((PBYTE)&Buffer, 0, sizeof(Buffer));

    mbp.cbSize = sizeof(mbp);
    mbp.hwndOwner = hwndDlg;
    mbp.hInstance = hInstDLL;
    mbp.lpszText = Buffer;
    mbp.lpszCaption = TEXT("About");
    mbp.dwStyle = MB_OK | MB_USERICON;
    mbp.lpszIcon = MAKEINTRESOURCE(IDI_ICON);


    wsprintf(Buffer, szMemoryDumpToAbout, sz_plugin_version);
    MessageBoxIndirect(&mbp);

    //MessageBox(hwndDlg, Buffer, TEXT("About"), MB_ICONINFORMATION);
    return TRUE;
}

DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, plugin_name);
    pluginHandle = initStruct->pluginHandle;

	// place any additional initialization code here
    return TRUE;
}

DLL_EXPORT bool plugstop()
{
    _plugin_menuclear(hMenu);

	// place any cleanup code here
	
    return TRUE;
}

DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    hwndDlg = setupStruct->hwndDlg;
    hMenu = setupStruct->hMenu;
    hMenuDisasm = setupStruct->hMenuDisasm;
    hMenuDump = setupStruct->hMenuDump;
    hMenuStack = setupStruct->hMenuStack;
    
	//GuiAddLogMessage, szMemoryDumpInfo;
	// place any additional setup code here
    _plugin_menuaddentry(hMenu, MD_ABOUT, "About");

    _plugin_menuaddentry(hMenuDump, MD_FILE, "File...");
    _plugin_menuaddentry(hMenuDump, MD_DELPHI, "Delphi/Pascal Table...");
    _plugin_menuaddentry(hMenuDump, MD_CPP, "C/C++ Table...");
    _plugin_menuaddentry(hMenuDump, MD_ASM, "Asm Table...");
    _plugin_menuaddentry(hMenuDump, MD_VB, "Visual Basic Table...");

    HRSRC hResource = FindResource(hInstDLL, MAKEINTRESOURCE(IDB_PNG), TEXT("PNG"));
    if (hResource)
    {
        HGLOBAL hMemory = LoadResource(hInstDLL, hResource);
        if (hMemory)
        {
            DWORD dwSize = SizeofResource(hInstDLL, hResource);
            LPVOID lpAddress = LockResource(hMemory);
            if (lpAddress)
            {
                ICONDATA IconData;
                IconData.data = lpAddress;
                IconData.size = dwSize;
                _plugin_menuseticon(hMenu, &IconData);
                _plugin_menuseticon(hMenuDump, &IconData);
            }
        }
    }
}

extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {
        case MD_ABOUT:
            AboutDlg();
            break;
        case MD_FILE:
            SaveToFile();
            break;
        case MD_DELPHI:
            MemoryDumpToSource(MD_DELPHI);
            break;
        case MD_CPP:
            MemoryDumpToSource(MD_CPP);
            break;
        case MD_ASM:
            MemoryDumpToSource(MD_ASM);
            break;
        case MD_VB:
            MemoryDumpToSource(MD_VB);
            break;
    }
}