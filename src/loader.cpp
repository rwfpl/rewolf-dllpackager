/* DLLPackager v1.0
 * Copyright 2007 ReWolf
 * Contact:
 * rewolf@poczta.onet.pl
 * http://rewolf.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <windows.h>
#include "defines.h"

void CALLBACK ldrMemMove(BYTE* dest, BYTE* src, int size)
{
	for (int i = 0; i < size; i++) dest[i] = src[i];
}

DWORD CALLBACK ldrStrLen(char* str)
{
	int len = 0;
	while (str[len]) len++;
	return len;
}

char* CALLBACK ldrUpperCase(char* str)
{
	int i = 0;
	while (str[i])
	{
		if ((str[i] >= 'a') && (str[i] <= 'z')) str[i] -= 0x20;
		i++;
	}
	return str;
}

int CALLBACK ldrStrCmp(char* str1, char* str2)
{
	DWORD len1 = ldrStrLen(str1);
	DWORD len2 = ldrStrLen(str2);

	//if (len1 != len2) return -1;

	char* strMin = (len1 < len2) ? str1 : str2;

	int i = 0;
	while (strMin[i])
	{
		if (str1[i] < str2[i]) return -1;
		if (str1[i] > str2[i]) return 1;
		i++;
	}

	if (len1 < len2) return -1;
	else if (len1 == len2) return 0;
	else return 1;
}

__declspec(naked) BYTE* ldrGetDllStructAddress()
{
	__asm
	{
		mov eax, 0x12345678
			ret
	}
}

__declspec(naked) IMAGE_IMPORT_DESCRIPTOR* ldrGetRemovedImportsAddress()
{
	__asm
	{
		mov eax, 0x90ABCDEF
			ret
	}
}

__declspec(naked) DWORD* ldrGetFunctionsTable()
{
	__asm
	{
		mov eax, 0x87654321
			ret
	}
}

__declspec(naked) char* ldrGetGetProcAddressText()
{
	__asm
	{
		call    _n
			_emit   'G'
			_emit   'e'
			_emit   't'
			_emit   'P'
			_emit   'r'
			_emit   'o'
			_emit   'c'
			_emit   'A'
			_emit   'd'
			_emit   'd'
			_emit   'r'
			_emit   'e'
			_emit   's'
			_emit   's'
			_emit   0
_n:
		pop     eax
			ret
	}
}

__declspec(naked) char* ldrGetLoadLibraryAText()
{
	__asm
	{
		call    _n
			_emit   'L'
			_emit   'o'
			_emit   'a'
			_emit   'd'
			_emit   'L'
			_emit   'i'
			_emit   'b'
			_emit   'r'
			_emit   'a'
			_emit   'r'
			_emit   'y'
			_emit   'A'
			_emit   0
_n:
		pop     eax
			ret
	}
}

__declspec(naked) char* ldrGetGetModuleHandleText()
{
	__asm
	{
		call    _n
			_emit   'G'
			_emit   'e'
			_emit   't'
			_emit   'M'
			_emit   'o'
			_emit   'd'
			_emit   'u'
			_emit   'l'
			_emit   'e'
			_emit   'H'
			_emit   'a'
			_emit   'n'
			_emit   'd'
			_emit   'l'
			_emit   'e'
			_emit   'A'
			_emit   0
_n:
		pop     eax
			ret
	}
}

__declspec(naked) char* ldrGetKernel32Text()
{
	__asm
	{
		call    _n
			_emit   'K'
			_emit   'E'
			_emit   'R'
			_emit   'N'
			_emit   'E'
			_emit   'L'
			_emit   '3'
			_emit   '2'
			_emit   '.'
			_emit   'D'
			_emit   'L'
			_emit   'L'
			_emit   0
_n:
		pop     eax
			ret
	}
}

__declspec(naked) char* ldrGetVirtualAllocText()
{
	__asm
	{
		call    _n
			_emit   'V'
			_emit   'i'
			_emit   'r'
			_emit   't'
			_emit   'u'
			_emit   'a'
			_emit   'l'
			_emit   'A'
			_emit   'l'
			_emit   'l'
			_emit   'o'
			_emit   'c'
			_emit   0
_n:
		pop     eax
			ret
	}
}

#ifdef TEST
#define ldrVirtualAlloc VirtualAlloc
#define ldrLoadLibraryA LoadLibraryA
#define ldrGetProcAddress GetProcAddress
#define ldrGetModuleHandleA GetModuleHandleA
#else
__declspec(naked) LPVOID CALLBACK ldrVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	__asm
	{
		call    $+5
			pop     edx
			call    ldrGetFunctionsTable
			add     eax, 8
			sub     edx, 5
			mov     word ptr [edx], 0x25FF
			mov     dword ptr [edx+2], eax
			jmp     edx
	}
}
__declspec(naked) HMODULE CALLBACK ldrLoadLibraryA(LPCTSTR lpFileName)
{
	__asm
	{
		call    $+5
			pop     edx
			call    ldrGetFunctionsTable
			sub     edx, 5
			mov     word ptr [edx], 0x25FF
			mov     dword ptr [edx+2], eax
			jmp     edx
	}
}

__declspec(naked) HMODULE CALLBACK ldrGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	__asm
	{
		call    $+5
			pop     edx
			call    ldrGetFunctionsTable
			add     eax, 4
			sub     edx, 5
			mov     word ptr [edx], 0x25FF
			mov     dword ptr [edx+2], eax
			jmp     edx
	}
}

__declspec(naked) HMODULE CALLBACK ldrGetModuleHandleA(LPCSTR lpModuleName)
{
	__asm
	{
		call    $+5
			pop     edx
			call    ldrGetFunctionsTable
			add     eax, 6*4
			sub     edx, 5
			mov     word ptr [edx], 0x25FF
			mov     dword ptr [edx+2], eax
			jmp     edx
	}
}
#endif

HMODULE CALLBACK emuLoadLibraryA(char* fileName);

LPVOID CALLBACK emuGetProcAddress(HMODULE hModule, LPCSTR lpProcname)
{
	//check if lpProcname is emulated
	DWORD* ldrFuncTab = ldrGetFunctionsTable();

	if ((DWORD)lpProcname & 0xFFFF0000)
	{
		char* funcName = ldrGetGetProcAddressText();
		if (!ldrStrCmp((char*)lpProcname, funcName)) return (LPVOID)ldrFuncTab[3];

		funcName = ldrGetLoadLibraryAText();
		if (!ldrStrCmp((char*)lpProcname, funcName)) return (LPVOID)ldrFuncTab[4];

		funcName = ldrGetGetModuleHandleText();
		if (!ldrStrCmp((char*)lpProcname, funcName)) return (LPVOID)ldrFuncTab[5];
	}

	//check if hModule is base of bundled DLL
	BYTE* dllStructAddr = ldrGetDllStructAddress();

	bool found = false;
	while ((*(DWORD*)dllStructAddr) && !found)
	{
		if (((DWORD*)dllStructAddr)[1] == (DWORD)hModule) found = true;
		else dllStructAddr += ((DWORD*)dllStructAddr)[3] + 0x10 + 1;
	}

	//if not call original GetProcAddress
	if (!found) return ldrGetProcAddress(hModule, lpProcname);

	//else dllStructAddr points to strcuture that describe bundled DLL

	//setting pointers to standard PE structures
	IMAGE_DOS_HEADER* idh = (IMAGE_DOS_HEADER*)(((DWORD*)dllStructAddr)[2]);
	IMAGE_NT_HEADERS* inh = (IMAGE_NT_HEADERS*)((BYTE*)((DWORD*)dllStructAddr)[2] + idh->e_lfanew);
	//IMAGE_SECTION_HEADER* ish = (IMAGE_SECTION_HEADER*)((BYTE*)((DWORD*)dllStructAddr)[2] + idh->e_lfanew + inh->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	//
	IMAGE_EXPORT_DIRECTORY* ied = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hModule + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	//searching for lpProcname in export table
	LPVOID retVal = 0;
	DWORD* funcTab = (DWORD*)((BYTE*)hModule + ied->AddressOfFunctions);
	if (!((DWORD)lpProcname & 0xFFFF0000))
	{
		//ordinal
		if (((DWORD)lpProcname) > ied->NumberOfFunctions + ied->Base) return 0;
		retVal = (BYTE*)hModule + funcTab[(DWORD)lpProcname - ied->Base];
	}
	else
	{
		//name
		DWORD* namesTab = (DWORD*)((BYTE*)hModule + ied->AddressOfNames);
		WORD* ordinalsTab = (WORD*)((BYTE*)hModule + ied->AddressOfNameOrdinals);

		int left = 0;
		int right = ied->NumberOfNames - 1;
		int current;
		while ((left <= right) && !retVal)
		{
			current = (left + right) / 2;
			int cmpVal = ldrStrCmp((char*)hModule + namesTab[current], (char*)lpProcname);
			if (cmpVal == 1) right = current;
			else if (cmpVal == -1) left = current + 1;
			else retVal = (BYTE*)hModule + funcTab[ordinalsTab[current]];
		}
	}

	//forwarded exports handling
	if (((DWORD)retVal > (DWORD)hModule + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) &&
		(DWORD)retVal < (DWORD)hModule + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
	{
		int i = 0;
		while (((char*)retVal)[i] != '.') i++;
		i++;
		DWORD tmp = *(DWORD*)((BYTE*)retVal + i);
		*(DWORD*)((BYTE*)retVal + i) = 0x4c4c44;	//'DLL',0

		HMODULE tmpLib = emuLoadLibraryA((char*)retVal);

		*(DWORD*)((BYTE*)retVal + i) = tmp;
		retVal = emuGetProcAddress(tmpLib, (char*)retVal + i);
	}

	return retVal;
}

HMODULE CALLBACK emuGetModuleHandleA(LPCTSTR lpModuleName)
{
	BYTE* dllStructAddr = ldrGetDllStructAddress();

	if ((DWORD)lpModuleName == 0) return ldrGetModuleHandleA(0);

	int j = ldrStrLen((char*)lpModuleName) - 1;
	while ((lpModuleName[j] != '\\') && (j)) j--;
	if (j) j++;
	ldrUpperCase((char*)lpModuleName + j);
	bool found = false;
	while ((*(DWORD*)dllStructAddr) && !found)
	{
		if (!ldrStrCmp((char*)lpModuleName + j, (char*)dllStructAddr + 0x10)) found = true;
		else dllStructAddr += ((DWORD*)dllStructAddr)[3] + 0x10 + 1;
	}

	if(!found) return ldrGetModuleHandleA((LPCTSTR)lpModuleName);
	else return (HMODULE)((DWORD*)dllStructAddr)[1];
}

HMODULE CALLBACK emuLoadLibraryA(char* fileName)
{

	BYTE* dllStructAddr = ldrGetDllStructAddress();

	//check if library is bundled
	int j = ldrStrLen(fileName) - 1;
	while ((fileName[j] != '\\') && (j)) j--;
	if (j) j++;
	ldrUpperCase(fileName + j);
	bool found = false;
	while ((*(DWORD*)dllStructAddr) && !found)
	{
		if (!ldrStrCmp(fileName + j, (char*)dllStructAddr + 0x10)) found = true;
		else dllStructAddr += ((DWORD*)dllStructAddr)[3] + 0x10 + 1;
	}

	//if not, call original LoadLibraryA
	if(!found) return ldrLoadLibraryA((LPCTSTR)fileName);

	//check if library already loaded
	if (((DWORD*)dllStructAddr)[1]) return (HMODULE)((DWORD*)dllStructAddr)[1];

	//setting pointers to standard PE structures
	IMAGE_DOS_HEADER* idh = (IMAGE_DOS_HEADER*)(((DWORD*)dllStructAddr)[2]);
	IMAGE_NT_HEADERS* inh = (IMAGE_NT_HEADERS*)((BYTE*)((DWORD*)dllStructAddr)[2] + idh->e_lfanew);
	IMAGE_SECTION_HEADER* ish = (IMAGE_SECTION_HEADER*)((BYTE*)((DWORD*)dllStructAddr)[2] + idh->e_lfanew + inh->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	//mapping sections to memory
	BYTE* newImageBase = (BYTE*)ldrVirtualAlloc((LPVOID)inh->OptionalHeader.ImageBase, inh->OptionalHeader.SizeOfImage, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!newImageBase) newImageBase = (BYTE*)ldrVirtualAlloc(0, inh->OptionalHeader.SizeOfImage, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!newImageBase) return 0;
	ldrVirtualAlloc(newImageBase, (BYTE*)ish - (BYTE*)idh + inh->FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	ldrMemMove(newImageBase, (BYTE*)((DWORD*)dllStructAddr)[2], TRUNC(((BYTE*)ish - (BYTE*)idh + inh->FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER)), inh->OptionalHeader.FileAlignment));
	for (int i = 0; i < inh->FileHeader.NumberOfSections; i++)
	{
		ldrVirtualAlloc(newImageBase + ish[i].VirtualAddress, ish[i].Misc.VirtualSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		ldrMemMove(newImageBase + ish[i].VirtualAddress, (BYTE*)((DWORD*)dllStructAddr)[2] + ish[i].PointerToRawData, TRUNC(ish[i].SizeOfRawData, inh->OptionalHeader.FileAlignment));
	}

	//imports
	IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)(newImageBase + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	while (iid->Name)
	{
		HMODULE hLib = emuLoadLibraryA((char*)newImageBase + iid->Name);
		int thunkIndex = 0;
		while (((DWORD*)(newImageBase + iid->FirstThunk))[thunkIndex])
		{
			if (((DWORD*)(newImageBase + iid->FirstThunk))[thunkIndex] & 0x80000000)
			{
				//ordinals
				((DWORD*)(newImageBase + iid->FirstThunk))[thunkIndex] = (DWORD)emuGetProcAddress(hLib, (LPCSTR)(((DWORD*)(newImageBase + iid->FirstThunk))[thunkIndex] & 0xFFFF));
			}
			else
			{
				//names
				((DWORD*)(newImageBase + iid->FirstThunk))[thunkIndex] = (DWORD)emuGetProcAddress(hLib, (LPCSTR)newImageBase + ((DWORD*)(newImageBase + iid->FirstThunk))[thunkIndex] + 2);
			}
			thunkIndex++;
		}
		iid++;
	}

	//relocations
	if ((DWORD)newImageBase != inh->OptionalHeader.ImageBase)
	{
		if (!inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) return 0;

		BYTE* relocPtr = newImageBase + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		DWORD delta = (DWORD)newImageBase - inh->OptionalHeader.ImageBase;

		while (*(DWORD*)relocPtr)
		{
			DWORD relocRVA = ((DWORD*)relocPtr)[0];
			DWORD blockSize = ((DWORD*)relocPtr)[1];

			for (int i = 0; i < (blockSize - 8) / 2; i++)
			{
				if (((WORD*)(relocPtr + 8))[i] & 0xF000)
				{
					*(DWORD*)(newImageBase + relocRVA + (((WORD*)(relocPtr + 8))[i] & 0xFFF)) += delta;
				}
			}
			relocPtr += blockSize;
		}
	}

	((DWORD*)dllStructAddr)[1] = (DWORD)newImageBase;

	//calling DllMain
	if (inh->OptionalHeader.AddressOfEntryPoint)
	{
		typedef bool (__stdcall *_dllMain)(HANDLE hInstDLL, DWORD dwReason, LPVOID lpvReserved);
		_dllMain dllMain = (_dllMain)(newImageBase + inh->OptionalHeader.AddressOfEntryPoint);
		if (!dllMain(newImageBase, DLL_PROCESS_ATTACH, 0))
		{
			((DWORD*)dllStructAddr)[1] = 0;
			return 0;
		}
	}

	return (HMODULE)newImageBase;
}

void CALLBACK ldrMain()
{
	DWORD* funcTable = ldrGetFunctionsTable();

	funcTable[0] = *(DWORD*)funcTable[7];
	funcTable[1] = *(DWORD*)funcTable[8];
	HMODULE kernelHandle = ldrLoadLibraryA((LPCTSTR)ldrGetKernel32Text());
	funcTable[2] = (DWORD)ldrGetProcAddress(kernelHandle, (LPCSTR)ldrGetVirtualAllocText());
	funcTable[6] = (DWORD)ldrGetProcAddress(kernelHandle, (LPCSTR)ldrGetGetModuleHandleText());

	if (((BYTE*)funcTable + 12*4)[0])
	{
		//restore function name exchenged with LoadLibraryA during packaging
		ldrMemMove((BYTE*)funcTable[9], (BYTE*)funcTable + 12*4, 13);
		*(DWORD*)funcTable[7] = (DWORD)ldrGetProcAddress(kernelHandle, (LPCSTR)funcTable[9]);
	}

	if (((BYTE*)funcTable + 12*4 + 13)[0])
	{
		//restore function name exchenged with GetProcAddress during packaging
		ldrMemMove((BYTE*)funcTable[10], (BYTE*)funcTable + 12*4 + 13, 15);
		*(DWORD*)funcTable[8] = (DWORD)ldrGetProcAddress(kernelHandle, (LPCSTR)funcTable[10]);
	}

	//filling import table of bundled DLLs
	IMAGE_IMPORT_DESCRIPTOR* remImps = ldrGetRemovedImportsAddress();
	HMODULE thisModule = ldrGetModuleHandleA(0);
	while (remImps->Name)
	{
		HMODULE curLib = emuLoadLibraryA((char*)((BYTE*)thisModule + remImps->Name));
		int i = 0;
		DWORD* thunks = (DWORD*)((BYTE*)thisModule + remImps->FirstThunk);
		while (thunks[i])
		{
			if (thunks[i] & 0x80000000)
			{
				//ordinal
				thunks[i] = (DWORD)emuGetProcAddress(curLib, (LPCSTR)(thunks[i] & 0xFFFF));
			}
			else
			{
				//name
				thunks[i] = (DWORD)emuGetProcAddress(curLib, (LPCSTR)((BYTE*)thisModule + thunks[i] + 2));
			}
			i++;
		}
		remImps++;
	}

	//setting addressess of emulated functions in IAT
	IMAGE_DOS_HEADER* idh = (IMAGE_DOS_HEADER*)thisModule;
	IMAGE_NT_HEADERS* inh = (IMAGE_NT_HEADERS*)((BYTE*)thisModule + idh->e_lfanew);
	IMAGE_IMPORT_DESCRIPTOR* origImports = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)thisModule + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	while (origImports->Name)
	{
		if (!ldrStrCmp((char*)thisModule + origImports->Name, (char*)ldrGetKernel32Text()))
		{
			DWORD* thunks = (DWORD*)((BYTE*)thisModule + origImports->FirstThunk);
			while (*thunks)
			{
				if (*thunks == funcTable[0]) *thunks = funcTable[4];		//LoadLibraryA
				else if (*thunks == funcTable[1]) *thunks = funcTable[3];	//GetProcAddress
				else if (*thunks == funcTable[6]) *thunks = funcTable[5];	//GetModulehandleA
				thunks++;
			}
		}
		origImports++;
	}

	//jump to original entry point
	__asm
	{
		mov     eax, funcTable
		add     eax, 44
		jmp     dword ptr [eax]
	}
}

__declspec(naked) void dummy()
{
	__asm
	{
		_emit 'D'
		_emit 'L'
		_emit 'L'
		_emit 'P'
		_emit 'a'
		_emit 'c'
		_emit 'k'
		_emit 'a'
		_emit 'g'
		_emit 'e'
		_emit 'r'
	}
}
