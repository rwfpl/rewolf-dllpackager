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

void CALLBACK ldrMemMove(BYTE* dest, BYTE* src, int size);
DWORD CALLBACK ldrStrLen(char* str);
char* CALLBACK ldrUpperCase(char* str);
int CALLBACK ldrStrCmp(char* str1, char* str2);
BYTE* ldrGetDllStructAddress();
IMAGE_IMPORT_DESCRIPTOR* ldrGetRemovedImportsAddress();
DWORD* ldrGetFunctionsTable();
char* ldrGetGetProcAddressText();
char* ldrGetLoadLibraryAText();
char* ldrGetGetModuleHandleText();
char* ldrGetKernel32Text();
char* ldrGetVirtualAllocText();
LPVOID CALLBACK ldrVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
HMODULE CALLBACK ldrLoadLibraryA(LPCTSTR lpFileName);
HMODULE CALLBACK ldrGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HMODULE CALLBACK ldrGetModuleHandleA(LPCSTR lpModuleName);
LPVOID CALLBACK emuGetProcAddress(HMODULE hModule, LPCSTR lpProcname);
HMODULE CALLBACK emuGetModuleHandleA(LPCTSTR lpModuleName);
HMODULE CALLBACK emuLoadLibraryA(char* fileName);
void CALLBACK ldrMain();
void dummy();