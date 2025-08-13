
#include "MemoryDumpTo.h"

SIZE_T ReadMemoryDump(PVOID* MemoryDump)
{
	SIZE_T SizeDump = 0;
	PVOID MemDump = 0;
	SELECTIONDATA sel;

	if (DbgIsDebugging())
	{
		if (GuiSelectionGet(GUI_DUMP, &sel))
		{
			SizeDump = sel.end - sel.start;
			SizeDump++;
			MemDump = VirtualAlloc(0, SizeDump, MEM_COMMIT, PAGE_READWRITE);
			if (MemDump)
			{
				if (DbgMemRead(sel.start, MemDump, SizeDump))
				{
					*MemoryDump = MemDump;
				}
				else
				{
					VirtualFree(MemDump, 0, MEM_RELEASE);
					SizeDump = 0;
				}
			}
		}
	}
	return SizeDump;
}

BOOL SaveToFile()
{
	SIZE_T SizeDump = 0;
	PVOID StartDump;
	SELECTIONDATA SelectionData;
	OPENFILENAME ofn{};
	TCHAR FilePath[MAX_PATH]{};
	
	if (!GuiSelectionGet(GUI_DUMP, &SelectionData))
		return FALSE;

	SizeDump = SelectionData.end - SelectionData.start;
	if (SizeDump)
	{
		SizeDump++;
		StartDump = (PVOID)SelectionData.start;

#if defined(_WIN64)
		wsprintf(FilePath, TEXT("dump_0x%016I64X-0x%016I64X"), SelectionData.start, SelectionData.end);
#else
		wsprintf(FilePath, TEXT("dump_0x%08lX-0x%08lX"), SelectionData.start, SelectionData.end);
#endif

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndDlg;
		ofn.lpstrFile = FilePath;
		ofn.nMaxFile = lengthof(FilePath);
		ofn.lpstrFilter = TEXT("dmp files (*.dmp)\0*.dmp\0");
		ofn.lpstrDefExt = TEXT("dmp");
		ofn.lpstrTitle = TEXT("Save Memory Dump File");
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_EXTENSIONDIFFERENT | OFN_OVERWRITEPROMPT;
		if (GetSaveFileName(&ofn))
		{
#ifdef _UNICODE
			DumpMemoryW(DbgGetProcessHandle(), StartDump, SizeDump, FilePath);
#else
			DumpMemory(DbgGetProcessHandle(), StartDump, SizeDump, FilePath);
#endif
			return TRUE;
		}
	}
	return FALSE;
}

static INIT_PARAM InitParam;
static PVOID TableText;

INT_PTR CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR FormatBuffer[MAX_PATH / 2]{};
	
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstDLL, MAKEINTRESOURCE(IDI_ICON)));
			wsprintf(FormatBuffer, szMemoryDumpTo, sz_plugin_version);
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)FormatBuffer);
			lstrcpy(FormatBuffer, szCopyright);
			lstrcat(FormatBuffer, szCopyright2);			
			SendMessage(GetDlgItem(hWnd, COPYR_STC), WM_SETTEXT, 0, (LPARAM)FormatBuffer);
			CheckDlgButton(hWnd, BYTE_RBN, TRUE);
			SendMessage(GetDlgItem(hWnd, BYTE_RBN), BM_CLICK, 0, 0);
		}
		return (INT_PTR)TRUE;

		case WM_CLOSE:
		{
			ClearClipboard(hWnd);
			EndDialog(hWnd, NULL);
		}
		return (INT_PTR)TRUE;

		case WM_COMMAND:
		{
			switch (wParam)
			{
				case BYTE_RBN:
				{
					TableText = CreateTableText(InitParam.DataPtr, InitParam.DataSize, InitParam.ProgLang, MD_Byte);
					if (TableText)
					{
						SendMessage(GetDlgItem(hWnd, EXPORT_EDT), WM_CLEAR, 0, 0);
						SendMessageA(GetDlgItem(hWnd, EXPORT_EDT), WM_SETTEXT, 0, (LPARAM)TableText);
						VirtualFree(TableText, 0, MEM_RELEASE);
					}
				}
				return (INT_PTR)TRUE;

				case WORD_RBN:
				{
					TableText = CreateTableText(InitParam.DataPtr, InitParam.DataSize, InitParam.ProgLang, MD_Word);
					if (TableText)
					{
						SendMessage(GetDlgItem(hWnd, EXPORT_EDT), WM_CLEAR, 0, 0);
						SendMessageA(GetDlgItem(hWnd, EXPORT_EDT), WM_SETTEXT, 0, (LPARAM)TableText);
						VirtualFree(TableText, 0, MEM_RELEASE);
					}
				}
				return (INT_PTR)TRUE;

				case DWORD_RBN:
				{
					TableText = CreateTableText(InitParam.DataPtr, InitParam.DataSize, InitParam.ProgLang, MD_Dword);
					if (TableText)
					{
						SendMessage(GetDlgItem(hWnd, EXPORT_EDT), WM_CLEAR, 0, 0);
						SendMessageA(GetDlgItem(hWnd, EXPORT_EDT), WM_SETTEXT, 0, (LPARAM)TableText);
						VirtualFree(TableText, 0, MEM_RELEASE);
					}
				}
				return (INT_PTR)TRUE;

				case COPY_BTN:
				{
					CopyToClipboard(GetDlgItem(hWnd, EXPORT_EDT));
				}
				return (INT_PTR)TRUE;
			}
		}
	}
	return (INT_PTR)FALSE;
}

BOOL MemoryDumpToSource(BOOL ProgLang)
{
	InitParam = {0};

	InitParam.ProgLang = ProgLang;
	InitParam.DataSize = ReadMemoryDump(&InitParam.DataPtr);
	if (InitParam.DataSize)
	{
		DialogBoxParam(hInstDLL, MAKEINTRESOURCE(EXPORT_DLG), hwndDlg, &DialogProc, NULL);
		VirtualFree(InitParam.DataPtr, 0, MEM_RELEASE);
	}
	return TRUE;
}

BOOL CopyToClipboard(HWND hWnd)
{
	BOOL Result = FALSE;
	int TextLen;
	HGLOBAL hGlob;
	PCHAR pOut;

	if (OpenClipboard(hWnd))
	{
		EmptyClipboard();
		if (TextLen = (int)SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0))
		{
			if (hGlob = GlobalAlloc(GMEM_MOVEABLE, TextLen + 1))
			{
				pOut = reinterpret_cast<char*>(GlobalLock(hGlob));
				if (pOut)
				{
					if (SendMessageA(hWnd, WM_GETTEXT, TextLen + 1, (LPARAM)pOut))
					{
						SetClipboardData(CF_TEXT, pOut);
						GlobalUnlock(hGlob);
						Result = TRUE;
					}
					else
					{
						GlobalUnlock(hGlob);
						GlobalFree(hGlob);
					}
				}
				else
				{
					GlobalFree(hGlob);
				}
			}
		}
		CloseClipboard();
	}
	return Result;
}

VOID ClearClipboard(HWND hWnd)
{
	if (OpenClipboard(hWnd))
	{
		EmptyClipboard();
		CloseClipboard();
	}
}