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

/*
 * Changes:
 *
 * rrrr-mm-dd - author
 * 2007-06-18 - ReWolf - corrected some problems with LoadLibrary and GetProcAddress
 *                       searching (reported by Gynvael Coldwind)
 *
 */

#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include "resource.h"
#include <vector>
#include <string>

#include "defines.h"
#include "loader.h"

DWORD rva2raw(WORD NumOfSections, IMAGE_SECTION_HEADER* FSH, DWORD rva)
{
	for (int i = NumOfSections-1; i >= 0; i--)
		if (FSH[i].VirtualAddress <= rva)
			return FSH[i].PointerToRawData + rva - FSH[i].VirtualAddress;
	return 0xFFFFFFFF;
}

LPVOID VirtualReAlloc(LPVOID hOldMem, unsigned int OldSize, unsigned int NewSize)
{
	LPVOID tmp = VirtualAlloc(0, NewSize, MEM_COMMIT, PAGE_READWRITE);
	memmove(tmp, hOldMem, OldSize);
	VirtualFree(hOldMem, 0, MEM_RELEASE);
	return tmp;
}


BYTE* genDllStruct(std::vector<std::string>& paths, std::vector<std::string>& names, DWORD baseOffset, DWORD* size)
{
	DWORD structSize = 0;
	DWORD dllsSize = 0;
	std::vector<DWORD> fSizes;
	//counting DLLs sizes
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		structSize += 16 + (DWORD)names[i].length() + 1;
		HANDLE tmp = CreateFile(paths[i].c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		fSizes.push_back(GetFileSize(tmp, 0));
		CloseHandle(tmp);
		dllsSize += fSizes[i];
	}

	//memory allocation for DLL structure
	*size = structSize + 4 + dllsSize;
	BYTE* retVal = (BYTE*)GlobalAlloc(GMEM_FIXED, *size);

	if (retVal)
	{
		int current = 0;
		int curDlls = 0;
		for (unsigned int i = 0; i < paths.size(); i++)
		{
			//setting values in DLL structure for each DLL
			((DWORD*)(retVal + current))[0] = fSizes[i];
			((DWORD*)(retVal + current))[1] = 0;
			((DWORD*)(retVal + current))[2] = baseOffset + structSize + 4 + curDlls;
			((DWORD*)(retVal + current))[3] = (DWORD)names[i].length();
			memmove(retVal + current + 16, names[i].c_str(), names[i].length() + 1);
			current += 16 + (DWORD)names[i].length() + 1;

			//reading DLL
			HANDLE tmp = CreateFile(paths[i].c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			DWORD tmp2;
			ReadFile(tmp, retVal + structSize + 4 + curDlls, fSizes[i], &tmp2, 0);
			CloseHandle(tmp);
			curDlls += fSizes[i];
		}
	}

	return retVal;
}

void Bundle(char* exeName, HWND dllListBoxHandle, HWND logListBoxHandle)
{
#define LOG(a) { int _u_u = (int)SendMessage(logListBoxHandle, LB_ADDSTRING, 0, (LPARAM)a); SendMessage(logListBoxHandle, LB_SETCURSEL, _u_u, 0); }
#define LOGRET(a) { SendMessage(logListBoxHandle, LB_ADDSTRING, 0, (LPARAM)a); return; }
#define LOGRET2(a) { SendMessage(logListBoxHandle, LB_ADDSTRING, 0, (LPARAM)a); VirtualFree(hExeMem, 0, MEM_RELEASE); return; }
#define EXEMEMCHECK(a, b) if (((BYTE*)a < (BYTE*)hExeMem) || ((BYTE*)a > (BYTE*)hExeMem + fileSize)) LOGRET2(b)

	//open and read executable to memory
	HANDLE hExeFile = CreateFile(exeName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hExeFile == INVALID_HANDLE_VALUE) LOGRET("Cannot open executable file.");
	DWORD fileSize = GetFileSize(hExeFile, 0);
	LPVOID hExeMem = VirtualAlloc(0, fileSize, MEM_COMMIT, PAGE_READWRITE);
	if (!hExeMem)
	{
		CloseHandle(hExeFile);
		LOG("Memory allocation error.");
	}
	DWORD tmp;
	ReadFile(hExeFile, hExeMem, fileSize, &tmp, 0);
	CloseHandle(hExeFile);

	//setting pointers to standard PE structures
	//IMAGE_DOS_HEADER* idh = (IMAGE_DOS_HEADER*)hExeMem;
	EXEMEMCHECK(&((IMAGE_DOS_HEADER*)hExeMem)->e_lfanew, "Memory error (e_lfanew).");
	IMAGE_NT_HEADERS* inh = (IMAGE_NT_HEADERS*)((BYTE*)hExeMem + ((IMAGE_DOS_HEADER*)hExeMem)->e_lfanew);
	EXEMEMCHECK(&inh->FileHeader.SizeOfOptionalHeader, "Memory error (SizeOfOptionalHeader).");
	IMAGE_SECTION_HEADER* ish = (IMAGE_SECTION_HEADER*)((BYTE*)hExeMem + ((IMAGE_DOS_HEADER*)hExeMem)->e_lfanew + inh->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	//setting writeable attribute to all sections
	for (int i = 0; i < inh->FileHeader.NumberOfSections; i++)
	{
		ish[i].Characteristics |= 0x80000000;
	}

	//searching for DLLs in executable imports table
	DWORD importsRaw = rva2raw(inh->FileHeader.NumberOfSections, ish, inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	IMAGE_IMPORT_DESCRIPTOR* iidPtr = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)hExeMem + importsRaw);
	EXEMEMCHECK(iidPtr, "Memory error (importsRaw).");

	unsigned int libCnt = (unsigned int)SendMessage(dllListBoxHandle, LB_GETCOUNT, 0, 0);
	std::vector<IMAGE_IMPORT_DESCRIPTOR> remIIDVector;
	std::vector<IMAGE_IMPORT_DESCRIPTOR> newIIDVector;
	std::vector<std::string> libsToBundle;
	std::vector<std::string> pathsToLibs;
	int curLib = 0;

	//
	DWORD thunkLoadLibraryA = 0;
	DWORD thunkGetProcAddress = 0;
	DWORD oldLLANamePtr = 0;
	DWORD oldGPANamePtr = 0;
	char xchgLLABuf[13] = {0};
	char xchgGPABuf[15] = {0};

	while (iidPtr[curLib].Name)
	{
		DWORD nameRaw = rva2raw(inh->FileHeader.NumberOfSections, ish, iidPtr[curLib].Name);
		EXEMEMCHECK((BYTE*)hExeMem + nameRaw, "Memory error (nameRaw).");
		strupr((char*)hExeMem + nameRaw);

		//checking if in imports from kernel32 occuring LoadLibraryA and GetProcAddress
		//if not it will change any suitable import name to LoadLibraryA or GetProcAddress
		DWORD probLLA = 0;		//12
		char* xchgLLA;
		DWORD probGPA = 0;		//14
		char* xchgGPA;
		if (!strcmp((char*)hExeMem + nameRaw, "KERNEL32.DLL") && !thunkGetProcAddress)
		{
			DWORD* thunks = (DWORD*)((char*)hExeMem + rva2raw(inh->FileHeader.NumberOfSections, ish, iidPtr[curLib].FirstThunk));
			EXEMEMCHECK(thunks, "Memory error (thunks).");
			int i = 0;
			while (thunks[i])
			{
				char* curImpName = (char*)hExeMem + rva2raw(inh->FileHeader.NumberOfSections, ish, thunks[i]) + 2;
				EXEMEMCHECK(curImpName, "Memory error (curImpName).");
				if (!probGPA && (strlen(curImpName) >= 14))
				{
					//if (strlen(curImpName) >= 14)
					{
						probGPA = iidPtr[curLib].FirstThunk + i*4;
						xchgGPA = curImpName;
						oldGPANamePtr = thunks[i] + 2;
					}
				}
				else if (!probLLA)
				{
					if (strlen(curImpName) >= 12)
					{
						probLLA = iidPtr[curLib].FirstThunk + i*4;
						xchgLLA = curImpName;
						oldLLANamePtr = thunks[i] + 2;
					}
				}

				if (!thunkGetProcAddress)
				{
					if (!strcmp(curImpName, "GetProcAddress"))
					{
						thunkGetProcAddress = iidPtr[curLib].FirstThunk + i*4;
						LOG("GetProcAddress found.");
					}
				}
				if (!thunkLoadLibraryA)
				{
					if (!strcmp(curImpName, "LoadLibraryA"))
					{
						thunkLoadLibraryA = iidPtr[curLib].FirstThunk + i*4;
						LOG("LoadLibraryA found.");
					}
				}
				i++;
			}
			char logTmp[256];
			if (!thunkLoadLibraryA)
			{
				if (!probLLA) LOGRET2("Problem with adding LoadLibraryA.");
				memmove(xchgLLABuf, xchgLLA, 13);
				sprintf(logTmp, "Changing %s to LoadLibraryA.", xchgLLA);
				memmove(xchgLLA, "LoadLibraryA", 13);
				thunkLoadLibraryA = probLLA;
				LOG(logTmp);
			}
			if (!thunkGetProcAddress)
			{
				if (!probGPA) LOGRET2("Problem with adding GetProcAddress.");
				memmove(xchgGPABuf, xchgGPA, 15);
				sprintf(logTmp, "Changing %s to GetProcAddress.", xchgGPA);
				memmove(xchgGPA, "GetProcAddress", 15);
				thunkGetProcAddress = probGPA;
				LOG(logTmp);
			}
		}

		bool ifExist = false;

		for (unsigned int i = 0; i < libCnt; i++)
		{
			char libName[256];
			SendMessage(dllListBoxHandle, LB_GETTEXT, i, (LPARAM)libName);

			int j = (int)strlen(libName) - 1;
			while ((libName[j] != '\\') && (j)) j--;
			if (j) j++;
			strupr(libName + j);

			if (libsToBundle.size() < libCnt)
			{
				if (i >= libsToBundle.size())
				{
					libsToBundle.push_back(libName + j);
					LOG(libName + j);
					pathsToLibs.push_back(libName);
				}
			}


			if (!strcmp(libName + j, (char*)hExeMem + nameRaw))
			{
				char logTmp[100];
				sprintf(logTmp, "Removing %s from original import table.", libName + j);
				LOG(logTmp);
				remIIDVector.push_back(iidPtr[curLib]);
				ifExist = true;
				break;
			}
		}
		if (!ifExist) newIIDVector.push_back(iidPtr[curLib]);
		curLib++;
	}

	for (unsigned int i = 0; i < newIIDVector.size(); i++) memmove(&iidPtr[i], &newIIDVector[i], sizeof(IMAGE_IMPORT_DESCRIPTOR));
	memset(&iidPtr[newIIDVector.size()], 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));

	char logTmp[256];

	//calculating address (virtual and raw) of new section
	DWORD newSectionRawOff = ish[inh->FileHeader.NumberOfSections - 1].PointerToRawData + TRUNC(ish[inh->FileHeader.NumberOfSections - 1].SizeOfRawData, inh->OptionalHeader.FileAlignment);
	DWORD newSectionRVA = inh->OptionalHeader.SizeOfImage;

	DWORD ldrCodeSize = (DWORD)((BYTE*)dummy - (BYTE*)ldrMemMove);
	sprintf(logTmp, "Size of appended code: %d", ldrCodeSize);
	LOG(logTmp);
	#define LDRDATASIZE 76
	/*
		DWORD LoadLibraryA;			//
		DWORD GetProcAddress;		//
		DWORD VirtualAlloc;			//
		DWORD emuGetProcAddress;	//
		DWORD emuLoadLibraryA;		//
		DWORD emuGetModuleHandleA;	//     look in loader.txt for details
		DWORD GetModuleHandleA;		//
		DWORD thunkLoadLibraryA;	//
		DWORD thunkGetProcAddress;	//
		DWORD oldLLANamePtr;		//
		DWORD oldGPANamePtr;		//
		DWORD originalEntryPoint
		char xchgLLABuf[13];		//
		char xchgGPABuf[15];		//
	*/
	DWORD ldrDllStructAddressToCorrect = (DWORD)(LDRDATASIZE + (BYTE*)ldrGetDllStructAddress - (BYTE*)ldrMemMove + 1);
	DWORD ldrFunctionsTableToCorrect = (DWORD)(LDRDATASIZE + (BYTE*)ldrGetFunctionsTable - (BYTE*)ldrMemMove + 1);
	DWORD ldrRemImportsAddrToCorrect = (DWORD)(LDRDATASIZE + (BYTE*)ldrGetRemovedImportsAddress - (BYTE*)ldrMemMove + 1);

	DWORD dllsSize;
	BYTE* dlls = genDllStruct(pathsToLibs, libsToBundle, inh->OptionalHeader.ImageBase + LDRDATASIZE + ldrCodeSize + (remIIDVector.size() + 1)*sizeof(IMAGE_IMPORT_DESCRIPTOR) + newSectionRVA, &dllsSize);

	IMAGE_SECTION_HEADER newSection;
	memset(&newSection, 0, sizeof(IMAGE_SECTION_HEADER));
	char secName[8] = ".DLL   ";
	memmove(newSection.Name, secName, 8);
	newSection.Characteristics = 0xE0000060;
	newSection.PointerToRawData = newSectionRawOff;
	newSection.SizeOfRawData = TRUNC((LDRDATASIZE + ldrCodeSize + dllsSize + (remIIDVector.size() + 1)*sizeof(IMAGE_IMPORT_DESCRIPTOR)), inh->OptionalHeader.FileAlignment);
	newSection.VirtualAddress = newSectionRVA;
	newSection.Misc.VirtualSize = TRUNC((LDRDATASIZE + ldrCodeSize + dllsSize + (remIIDVector.size() + 1)*sizeof(IMAGE_IMPORT_DESCRIPTOR)), inh->OptionalHeader.SectionAlignment);

	sprintf(logTmp, "Size of new section: %d", newSection.SizeOfRawData);
	LOG(logTmp);

	LPVOID oldMem = hExeMem;
	hExeMem = VirtualReAlloc(hExeMem, fileSize, fileSize + newSection.SizeOfRawData);
	//correction of inh and ish pointers
	IMAGE_NT_HEADERS* inh2 = (IMAGE_NT_HEADERS*)((BYTE*)hExeMem + ((BYTE*)inh - (BYTE*)oldMem));
	IMAGE_SECTION_HEADER* ish2 = (IMAGE_SECTION_HEADER*)((BYTE*)hExeMem + ((BYTE*)ish - (BYTE*)oldMem));

	//generating functions table
	#define LDRITEMCNT 12
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[0] = 0;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[1] = 0;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[2] = 0;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[3] = inh2->OptionalHeader.ImageBase + newSection.VirtualAddress + LDRDATASIZE + ((BYTE*)emuGetProcAddress - (BYTE*)ldrMemMove);
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[4] = inh2->OptionalHeader.ImageBase + newSection.VirtualAddress + LDRDATASIZE + ((BYTE*)emuLoadLibraryA - (BYTE*)ldrMemMove);
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[5] = inh2->OptionalHeader.ImageBase + newSection.VirtualAddress + LDRDATASIZE + ((BYTE*)emuGetModuleHandleA - (BYTE*)ldrMemMove);
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[6] = 0;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[7] = inh2->OptionalHeader.ImageBase + thunkLoadLibraryA;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[8] = inh2->OptionalHeader.ImageBase + thunkGetProcAddress;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[9] = inh2->OptionalHeader.ImageBase + oldLLANamePtr;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[10] = inh2->OptionalHeader.ImageBase + oldGPANamePtr;
	((DWORD*)((BYTE*)hExeMem + newSectionRawOff))[11] = inh2->OptionalHeader.ImageBase + inh2->OptionalHeader.AddressOfEntryPoint;
	memmove((BYTE*)hExeMem + newSectionRawOff + LDRITEMCNT*4, xchgLLABuf, 13);
	memmove((BYTE*)hExeMem + newSectionRawOff + LDRITEMCNT*4 + 13, xchgGPABuf, 15);
	//copying loader code to new memory
	memmove((BYTE*)hExeMem + newSectionRawOff + LDRDATASIZE, ldrMemMove, ldrCodeSize);

	//correction of addressess returned by: ldrGetRemovedImportsAddress, ldrGetFunctionsTable, ldrGetDllStructAddress
	*((DWORD*)((BYTE*)hExeMem + newSectionRawOff + ldrDllStructAddressToCorrect)) = inh2->OptionalHeader.ImageBase + newSectionRVA + LDRDATASIZE + ldrCodeSize + (remIIDVector.size() + 1)*sizeof(IMAGE_IMPORT_DESCRIPTOR);
	*((DWORD*)((BYTE*)hExeMem + newSectionRawOff + ldrFunctionsTableToCorrect)) = inh2->OptionalHeader.ImageBase + newSectionRVA;
	*((DWORD*)((BYTE*)hExeMem + newSectionRawOff + ldrRemImportsAddrToCorrect)) = inh2->OptionalHeader.ImageBase + newSectionRVA + LDRDATASIZE + ldrCodeSize;

	//copying to loader import descriptors removed from original import table
	unsigned int i = 0;
	for (i = 0; i < remIIDVector.size(); i++)
	{
		memmove((BYTE*)hExeMem + newSectionRawOff + LDRDATASIZE + ldrCodeSize + i*sizeof(IMAGE_IMPORT_DESCRIPTOR), &remIIDVector[i], sizeof(IMAGE_IMPORT_DESCRIPTOR));
	}
	memset((BYTE*)hExeMem + newSectionRawOff + LDRDATASIZE + ldrCodeSize + i*sizeof(IMAGE_IMPORT_DESCRIPTOR), 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));
	i++;

	//copying DLL structure
	memmove((BYTE*)hExeMem + newSectionRawOff + LDRDATASIZE + ldrCodeSize + i*sizeof(IMAGE_IMPORT_DESCRIPTOR), dlls, dllsSize);
	GlobalFree(dlls);

	//correcting PE header
	memmove(&ish2[inh2->FileHeader.NumberOfSections], &newSection, sizeof(IMAGE_SECTION_HEADER));
	inh2->FileHeader.NumberOfSections++;
	inh2->OptionalHeader.AddressOfEntryPoint = newSection.VirtualAddress + LDRDATASIZE + ((BYTE*)ldrMain - (BYTE*)ldrMemMove);
	inh2->OptionalHeader.SizeOfImage += newSection.Misc.VirtualSize;

	//saving new executable
	char outFileName[256];
	sprintf(outFileName, "%s.bundled.exe", exeName);
	HANDLE outFile = CreateFile(outFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(outFile, hExeMem, fileSize + newSection.SizeOfRawData, &tmp, 0);
	CloseHandle(outFile);
	sprintf(logTmp, "OK: %s", outFileName);
	LOG(logTmp);

	VirtualFree(hExeMem, 0, MEM_RELEASE);

#undef LDRITEMCNT
#undef LDRDATASIZE
#undef EXEMEMCHECK
#undef LOGRET2
#undef LOGRET
#undef LOG
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			break;
		case WM_COMMAND:
			{
				switch (wParam)
				{
					case BTN_EXIT:
						EndDialog(hwndDlg, 0);
						break;
					case BTN_OPEN:
						{
							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hwndOwner = hwndDlg;
							BYTE fileName[256];
							fileName[0] = 0;
							ofn.lpstrFile = (LPSTR)fileName;
							ofn.nMaxFile = 256;
							ofn.Flags = OFN_FILEMUSTEXIST;
							GetOpenFileName(&ofn);
							SetDlgItemText(hwndDlg, EDIT_FILENAME, ofn.lpstrFile);
						}
						break;
					case BTN_ADDLIB:
						{
							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hwndOwner = hwndDlg;
							BYTE fileName[256];
							fileName[0] = 0;
							ofn.lpstrFile = (LPSTR)fileName;
							ofn.nMaxFile = 256;
							ofn.Flags = OFN_FILEMUSTEXIST;
							GetOpenFileName(&ofn);
							if (fileName[0]) SendDlgItemMessage(hwndDlg, LB_LIBS, LB_ADDSTRING, 0, (LPARAM)fileName);
						}
						break;
					case BTN_DELLIB:
						{
							int sel = (int)SendDlgItemMessage(hwndDlg, LB_LIBS, LB_GETCURSEL, 0, 0);
							if (sel != LB_ERR) SendDlgItemMessage(hwndDlg, LB_LIBS, LB_DELETESTRING, sel, 0);
						}
						break;
					case BTN_BUNDLE:
						{
							char fileName[256];
							fileName[0] = 0;
							GetDlgItemText(hwndDlg, EDIT_FILENAME, (LPSTR)fileName, 256);
							int libCnt = (int)SendDlgItemMessage(hwndDlg, LB_LIBS, LB_GETCOUNT, 0, 0);
							if (fileName[0] && libCnt) Bundle(fileName, GetDlgItem(hwndDlg, LB_LIBS), GetDlgItem(hwndDlg, LB_LOG));
							else SendDlgItemMessage(hwndDlg, LB_LOG, LB_ADDSTRING, 0, (LPARAM)"Select executable and at least one DLL.");
						}
						break;
					case BTN_ABT:
						{
							MessageBox(hwndDlg, "DLLPackager v1.0\n\nAuthor\t: ReWolf\ne-mail\t: rewolf@poczta.onet.pl\nwww\t: http://rewolf.pl", "About", MB_ICONINFORMATION);
						}
				}
			}
			break;
	}
	return 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	DialogBoxParam(hInstance, (LPCSTR)IDD_DIALOG1, 0, DialogProc, 0);
	return 0;
}
