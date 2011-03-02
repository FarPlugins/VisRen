/****************************************************************************
 * VisRen.hpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2007-2011 Alexey Samlyukov
 ****************************************************************************/
/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <wchar.h>
#include <initguid.h>
#include "plugin.hpp"
#include "farkeys.hpp"
#include "farcolor.hpp"
#include "string.hpp"
#include "VisRenLng.hpp"        // ����� �������� ��� ���������� ����� �� .lng �����

/// �����! ���������� ������ �������, ���� ������������� �� �������� ������
#define malloc(size) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define free(ptr) ((ptr)?HeapFree(GetProcessHeap(),0,ptr):0)
#define realloc(ptr,size) ((size)?((ptr)?HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,ptr,size):HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)):(HeapFree(GetProcessHeap(),0,ptr),(void *)0))
#ifdef __cplusplus
inline void * __cdecl operator new(size_t size) { return malloc(size); }
inline void __cdecl operator delete(void *block) { free(block); }
#endif

/// ������� strncmp() (��� strcmp() ��� n=-1)
inline int __cdecl Strncmp(const wchar_t *s1, const wchar_t *s2, int n=-1) { return CompareString(0,SORT_STRINGSORT,s1,n,s2,n)-2; }


/****************************************************************************
 * GUID
 ****************************************************************************/
// {E80B8002-EED3-4563-9C78-2E3C3246F8D3}
DEFINE_GUID(MainGuid, 0xe80b8002, 0xeed3, 0x4563, 0x9c, 0x78, 0x2e, 0x3c, 0x32, 0x46, 0xf8, 0xd3);

// {F8A515F3-A57C-4c91-9C07-9A196143550A}
DEFINE_GUID(MenuGuid, 0xf8a515f3, 0xa57c, 0x4c91, 0x9c, 0x7, 0x9a, 0x19, 0x61, 0x43, 0x55, 0xa);

// {AD50A5CA-1E33-4e24-9A51-97A481D41F56}
DEFINE_GUID(DlgGuid, 0xad50a5ca, 0x1e33, 0x4e24, 0x9a, 0x51, 0x97, 0xa4, 0x81, 0xd4, 0x1f, 0x56);

// {B6334FF1-57A0-47ca-A924-007B6F3F770F}
DEFINE_GUID(DlgNameGuid, 0xb6334ff1, 0x57a0, 0x47ca, 0xa9, 0x24, 0x0, 0x7b, 0x6f, 0x3f, 0x77, 0xf);


/****************************************************************************
 * ����� ����������� �������� FAR
 ****************************************************************************/
extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

/****************************************************************************
 * ������� ��� ��������������
 ****************************************************************************/
struct File
{
	string strSrcFileName;                         //   ����� ������������ ����� � ������
	string strDestFileName;                        //   �������� �����
	FILETIME ftLastWriteTime;                      //   ����� ��������� �����������

	File()
	{
		strSrcFileName.clear();
		strDestFileName.clear();
		ftLastWriteTime.dwLowDateTime=0;
		ftLastWriteTime.dwHighDateTime=0;
	}

	const File& operator=(const File &f)
	{
		if (this != &f)
		{
			strSrcFileName=f.strSrcFileName;
			strDestFileName=f.strDestFileName;
			ftLastWriteTime.dwLowDateTime=f.ftLastWriteTime.dwLowDateTime;
			ftLastWriteTime.dwHighDateTime=f.ftLastWriteTime.dwHighDateTime;
		}
		return *this;
	}
};

/****************************************************************************
 * ������� ��������� �������
 ****************************************************************************/
extern struct Options {
	int UseLastHistory,
			lenFileName,
			lenName,
			lenExt,
			lenDestFileName,
			lenDestName,
			lenDestExt,
			CurBorder,
			srcCurCol,
			destCurCol,
			ShowOrgName,
			CaseSensitive,
			RegEx,
			LogRen,                             // �������� �� ���������� ������
			LoadUndo,                           // ���� ���� ����� - ��������/���������
			Undo;                               // ���� ���� ����� - �������� ���
	Options()
	{
		UseLastHistory=0;
		lenFileName=0;
		lenName=0;
		lenExt=0;
		lenDestFileName=0;
		lenDestName=0;
		lenDestExt=0;
		CurBorder=0;
		srcCurCol=0;
		destCurCol=0;
		ShowOrgName=0;
		CaseSensitive=1;
		RegEx=0;
		LogRen=1;
		LoadUndo=0;
		Undo=0;
	}
} Opt;

struct StrOptions {
	string MaskName;
	string MaskExt;
	string Search;
	string Replace;
	string WordDiv;
};

/****************************************************************************
 * Undo ��������������
 ****************************************************************************/
extern struct UndoFileName                       // �������� ��� ������ ��������������
{
	wchar_t *Dir;                                  //   ����� � ������� ���������������
	wchar_t **CurFileName;                         //   �����, � ������� ������������� �����
	wchar_t **OldFileName;                         //   �����, ������� ���� � ������ �� ��������������
	int iCount;                                    //   ���-��
} Undo;

/****************************************************************************
 * ������ �������.
 ****************************************************************************/
extern struct DlgSize
{
	// ��������� �������
	bool Full;
	// ����������
	DWORD W;
	DWORD W2;
	DWORD WS;
	DWORD H;
	// �����������������
	DWORD mW;
	DWORD mW2;
	DWORD mWS;
	DWORD mH;

	DlgSize()
	{
		Full=false;
		W=mW=80-2;
		WS=mWS=(80-2)-37;
		W2=mW2=(80-2)/2-2;
		H=mH=25-2;
	}
} DlgSize;

const wchar_t *GetMsg(int MsgId);
void ErrorMsg(DWORD Title, DWORD Body);
bool YesNoMsg(DWORD Title, DWORD Body);
void FreeUndo();
int DebugMsg(wchar_t *msg, wchar_t *msg2 = L" ", unsigned int i = 1000);
