
#include "MemoryDumpTo.h"

#define szHeaderDELPHI   "const Table: array[0..%u] of %s = (\r\n"
#define szHeaderCPP      "const %s Table[%u] = {\r\n"
#define szHeaderASM      "Table \\\r\n"
#define szHeaderVB       "ReDim Table(%u)\r\n  Table = Array( "

#define Del_FormateByte  "$%.2X"
#define Cpp_FormateByte  "0x%.2X"
#define Asm_FormateByte  "%.3Xh"
#define Vbs_FormateByte  "&H%.2X"

#define Del_FormateWord  "$%.4X"
#define Cpp_FormateWord  "0x%.4X"
#define Asm_FormateWord  "%.5Xh"
#define Vbs_FormateWord  "&H%.4X"

#define Del_FormateDword "$%.8X"
#define Cpp_FormateDword "0x%.8X"
#define Asm_FormateDword "%.9Xh"
#define Vbs_FormateDword "&H%.8X"

#define Del_FormateQword "$%.16I64X"
#define Cpp_FormateQword "0x%.16I64X"
#define Asm_FormateQword "%.17I64Xh"
#define Vbs_FormateQword "&H%.16I64XUL" // VB.NET

#define Asm_Byte  "BYTE "
#define Asm_Word  "WORD "
#define Asm_Dword "DWORD "
#define Asm_Qword "QWORD "

#pragma warning(disable : 6387)

SIZE_T Alignment(SIZE_T InSize, int Align)
{
	SIZE_T OutSize;
	int _Align;

	_Align = Align;
	_Align--;
	OutSize = (InSize + _Align) & ~_Align;

	return OutSize;
}

PCHAR PrintTableHeader(PCHAR InBuffer, BOOL ProgLang, BOOL SizeSeg, SIZE_T NumbSeg)
{
	PCHAR CurrPtr;
	PCHAR szSize;

	CurrPtr = InBuffer;

	if (ProgLang == MD_DELPHI)
	{
		if (SizeSeg == MD_Byte)
		{
			szSize = "Byte";
		}
		else if (SizeSeg == MD_Word)
		{
			szSize = "Word";
		}
		else if (SizeSeg == MD_Dword)
		{
			szSize = "Longword";
		}
		else if (SizeSeg == MD_Qword)
		{
			szSize = "UInt64";
		}
		CurrPtr += wsprintfA(CurrPtr, szHeaderDELPHI, (int)(NumbSeg - 1), szSize);
	}
	else if (ProgLang == MD_CPP)
	{		
		if (SizeSeg == MD_Byte)
		{
			szSize = "UCHAR";
		}
		else if (SizeSeg == MD_Word)
		{
			szSize = "USHORT";
		}
		else if (SizeSeg == MD_Dword)
		{
			szSize = "ULONG";
		}
		else if (SizeSeg == MD_Qword)
		{
			szSize = "ULONG64";
		}
		CurrPtr += wsprintfA(CurrPtr, szHeaderCPP, szSize, (int)NumbSeg);
	}
	else if (ProgLang == MD_ASM)
	{
		lstrcpyA(CurrPtr, szHeaderASM);
		CurrPtr += lstrlenA(CurrPtr);
	}
	else // MD_VB
	{
		CurrPtr += wsprintfA(CurrPtr, szHeaderVB, (int)(NumbSeg - 1));
	}

	return CurrPtr;
}

VOID PrintTable(PCHAR InBuffer, PVOID InData, BOOL ProgLang, BOOL SizeSeg, SIZE_T NumbSeg, int SegNumb)
{
	BOOL Vb_First_Block = 0;
	int SegCount = 0;
	PCHAR CurrPtr = nullptr;
	PBYTE pByte = nullptr;
	PWORD pWord = nullptr;
	PDWORD pDword = nullptr;
	PDWORD64 pQword = nullptr;
	LPSTR Formate = nullptr;
	LPSTR AsmPefix = nullptr;

	CurrPtr = PrintTableHeader(InBuffer, ProgLang, SizeSeg, NumbSeg);

	if (SizeSeg == MD_Byte)
	{
		pByte = (PBYTE)InData;
	}
	else if (SizeSeg == MD_Word)
	{
		pWord = (PWORD)InData;
	}
	else if (SizeSeg == MD_Dword)
	{
		pDword = (PDWORD)InData;
	}
	else if (SizeSeg == MD_Qword)
	{
		pQword = (PDWORD64)InData;
	}
	
	if (ProgLang == MD_DELPHI)
	{
		if (pByte) Formate = Del_FormateByte;
		else if (pWord) Formate = Del_FormateWord;
		else if (pDword) Formate = Del_FormateDword;
		else if (pQword) Formate = Del_FormateQword;
	}
	else if (ProgLang == MD_CPP)
	{
		if (pByte) Formate = Cpp_FormateByte;
		else if (pWord) Formate = Cpp_FormateWord;
		else if (pDword) Formate = Cpp_FormateDword;
		else if (pQword) Formate = Cpp_FormateQword;
	}
	else if (ProgLang == MD_ASM)
	{
		if (pByte) 
		{
			Formate = Asm_FormateByte;
			AsmPefix = Asm_Byte;
		}
		else if (pWord)
		{
			Formate = Asm_FormateWord;
			AsmPefix = Asm_Word;
		}
		else if (pDword)
		{
			AsmPefix = Asm_Dword;
			Formate = Asm_FormateDword;
		}
		else if (pQword)
		{
			AsmPefix = Asm_Qword;
			Formate = Asm_FormateQword;
		}
	}
	else if (ProgLang == MD_VB)
	{
		if (pByte) Formate = Vbs_FormateByte;
		else if (pWord) Formate = Vbs_FormateWord;
		else Formate = Vbs_FormateDword;
	}

	for (;;)
	{
		if (!SegCount)
		{
			if (ProgLang == MD_ASM)
			{
				lstrcpyA(CurrPtr, AsmPefix);
				CurrPtr += lstrlenA(AsmPefix);

			}
			else if (ProgLang == MD_VB)
			{
				if (!Vb_First_Block)
				{
					Vb_First_Block++;
				}
				else
				{
					lstrcpyA(CurrPtr, "                 ");
					CurrPtr += 17;
				}
			}
			else
			{
				lstrcpyA(CurrPtr, "    ");
				CurrPtr += 4;
			}
		}
		else if (SegCount == SegNumb)
		{
			SegCount = 0;
			continue;
		}
		if (pByte) CurrPtr += wsprintfA(CurrPtr, Formate, *pByte);
		if (pWord) CurrPtr += wsprintfA(CurrPtr, Formate, *pWord);
		if (pDword) CurrPtr += wsprintfA(CurrPtr, Formate, *pDword);
		if (pQword) CurrPtr += wsprintfA(CurrPtr, Formate, *pQword);
		NumbSeg--;
		if (!NumbSeg)
			break;
			
		if (SegCount == SegNumb - 1)
		{
			if (ProgLang != MD_ASM)
			{
				lstrcpyA(CurrPtr, ",");
				CurrPtr++;
			}
		}
		else
		{
			lstrcpyA(CurrPtr, ",");
			CurrPtr++;
		}
		if (SegCount == SegNumb - 1)
		{
			if (ProgLang == MD_VB)
			{
				lstrcpyA(CurrPtr, " _");
				CurrPtr += 2;
			}
			lstrcpyA(CurrPtr, "\r\n");
			CurrPtr += 2;
		}
		else
		{
			lstrcpyA(CurrPtr, " ");
			CurrPtr++;
		}
		if (pByte) pByte++;
		if (pWord) pWord++;
		if (pDword) pDword++;
		if (pQword) pQword++;
		SegCount++;
	};

	if (ProgLang == MD_DELPHI)
		lstrcpyA(CurrPtr, " );\r\n");
	else if (ProgLang == MD_CPP)
		lstrcpyA(CurrPtr, " };\r\n");
	else if (ProgLang == MD_ASM)
		lstrcpyA(CurrPtr, "\r\n");
	else if (ProgLang == MD_VB)
		lstrcpyA(CurrPtr, " )\r\n");
}

PVOID CreateTableText(PVOID InBuffer, SIZE_T InBuffSize, BOOL ProgLang, BOOL SizeSeg, int SegNumb)
{
	SIZE_T OutBuffSize = 0;
	SIZE_T NumbSeg = 0;
	int Align = 0;
	PVOID OutBuffer = 0;

	NumbSeg = InBuffSize;
	if (SizeSeg == MD_Word || SizeSeg == MD_Dword || SizeSeg == MD_Qword)
	{
		if (SizeSeg == MD_Word)
			Align = sizeof(WORD);
		else if (SizeSeg == MD_Dword)
			Align = sizeof(DWORD);
		else if (SizeSeg == MD_Qword)
			Align = sizeof(DWORD64);

		NumbSeg = Alignment(NumbSeg, Align);
		NumbSeg = NumbSeg / Align;
	}

	OutBuffSize = InBuffSize * 8;
	OutBuffer = VirtualAlloc(0, OutBuffSize, MEM_COMMIT, PAGE_READWRITE);
	if (OutBuffer)
	{
		PrintTable((PCHAR)OutBuffer, InBuffer, ProgLang, SizeSeg, NumbSeg, SegNumb);
	}
	return OutBuffer;
}