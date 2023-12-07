
#include <MemoryDumpTo.h>

#define szHeaderDELPHI   "const Table: array[0..%u] of %s = (\r\n"
#define szHeaderCPP      "const %s Table[%u] = {\r\n"
#define szHeaderASM      "Table \\\r\n"
#define szHeaderVB       "ReDim Table(%u)\r\n  Table = Array( "

#define Del_FormateByte  "$%02X"
#define Cpp_FormateByte  "0x%02X"
#define Asm_FormateByte  "%03Xh"
#define Vbs_FormateByte  "&H%02X"

#define Del_FormateWord  "$%04X"
#define Cpp_FormateWord  "0x%04X"
#define Asm_FormateWord  "%05Xh"
#define Vbs_FormateWord  "&H%04X"

#define Del_FormateDword "$%08X"
#define Cpp_FormateDword "0x%08X"
#define Asm_FormateDword "%09Xh"
#define Vbs_FormateDword "&H%08X"

#define Asm_Byte  "BYTE "
#define Asm_Word  "WORD "
#define Asm_Dword "DWORD "

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
		else // MD_Dword
		{
			szSize = "Longword";
		}
		CurrPtr += wsprintfA(CurrPtr, szHeaderDELPHI, (NumbSeg - 1), szSize);
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
		else // MD_Dword
		{
			szSize = "ULONG";
		}
		CurrPtr += wsprintfA(CurrPtr, szHeaderCPP, szSize, NumbSeg);
	}
	else if (ProgLang == MD_ASM)
	{
		lstrcpyA(CurrPtr, szHeaderASM);
		CurrPtr += lstrlenA(CurrPtr);
	}
	else // MD_VB
	{
		CurrPtr += wsprintfA(CurrPtr, szHeaderVB, (NumbSeg - 1));
	}

	return CurrPtr;
}

VOID PrintTable(PCHAR InBuffer, PVOID InData, BOOL ProgLang, BOOL SizeSeg, SIZE_T NumbSeg)
{
	BOOL Vb_First_Block = 0;
	int SegCount = 0;
	int SegNumb;
	PCHAR CurrPtr;
	PBYTE pByte = 0;
	PWORD pWord = 0;
	PDWORD pDword = 0;
	LPSTR Formate;
	LPSTR AsmPefix = 0;

	CurrPtr = PrintTableHeader(InBuffer, ProgLang, SizeSeg, NumbSeg);

	if (SizeSeg == MD_Byte)
	{
		pByte = (PBYTE)InData;
		SegNumb = 16;
	}
	else if (SizeSeg == MD_Word)
	{
		pWord = (PWORD)InData;
		SegNumb = 8;
	}
	else // MD_Dword
	{
		pDword = (PDWORD)InData;
		SegNumb = 8;
	}
	
	if (ProgLang == MD_DELPHI)
	{
		if (pByte) Formate = Del_FormateByte;
		else if (pWord) Formate = Del_FormateWord;
		else Formate = Del_FormateDword;
	}
	else if (ProgLang == MD_CPP)
	{
		if (pByte) Formate = Cpp_FormateByte;
		else if (pWord) Formate = Cpp_FormateWord;
		else Formate = Cpp_FormateDword;
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
		else
		{
			AsmPefix = Asm_Dword;
			Formate = Asm_FormateDword;
		}
	}
	else // MD_VB
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
			SegCount++;
		};

	if (ProgLang == MD_DELPHI)
		lstrcpyA(CurrPtr, " );\r\n");
	else if (ProgLang == MD_CPP)
		lstrcpyA(CurrPtr, " };\r\n");
	else if (ProgLang == MD_ASM)
		lstrcpyA(CurrPtr, "\r\n");
	else // MD_VB
		lstrcpyA(CurrPtr, " )\r\n");
}

PVOID CreateTableText(PVOID InBuffer, SIZE_T InBuffSize, BOOL ProgLang, BOOL SizeSeg)
{
	SIZE_T OutBuffSize = 0;
	SIZE_T NumbSeg;
	int Align;
	PVOID OutBuffer = 0;

	NumbSeg = InBuffSize;
	if (SizeSeg == MD_Word || SizeSeg == MD_Dword)
	{
		if (SizeSeg == MD_Word)
			Align = sizeof(WORD);
		else
			Align = sizeof(DWORD);

		NumbSeg = Alignment(NumbSeg, Align);
		NumbSeg = NumbSeg / Align;
	}

	OutBuffSize = InBuffSize * 6;
	OutBuffer = VirtualAlloc(0, OutBuffSize, MEM_COMMIT, PAGE_READWRITE);
	if (OutBuffer)
	{
		PrintTable((PCHAR)OutBuffer, InBuffer, ProgLang, SizeSeg, NumbSeg);
	}
	return OutBuffer;
}