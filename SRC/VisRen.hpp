/****************************************************************************
 * VisRen.hpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2007 Alexey Samlyukov
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
#include "farcolor.hpp"
#include "string.hpp"
#include "VisRenLng.hpp"        // набор констант для извлечения строк из .lng файла

/// ВАЖНО! используем данные функции, чтоб дополнительно не обнулять память
#define malloc(size) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define free(ptr) ((ptr)?HeapFree(GetProcessHeap(),0,ptr):0)
#define realloc(ptr,size) ((size)?((ptr)?HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,ptr,size):HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)):(HeapFree(GetProcessHeap(),0,ptr),(void *)0))
#ifdef __cplusplus
inline void * __cdecl operator new(size_t size) { return malloc(size); }
inline void __cdecl operator delete(void *block) { free(block); }
#endif

/// Подмена strncmp() (или strcmp() при n=-1)
inline int __cdecl Strncmp(const wchar_t *s1, const wchar_t *s2, int n=-1) { return CompareString(0,SORT_STRINGSORT,s1,n,s2,n)-2; }

///
#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyAltMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)
#define ControlKeyNonAltMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyCtrlMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED)
#define ControlKeyNonCtrlMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED)
#define IsShift(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==SHIFT_PRESSED)
#define IsAlt(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAltMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonAltMask))
#define IsCtrl(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyCtrlMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonCtrlMask))
#define IsNone(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)


/****************************************************************************
 * GUID
 ****************************************************************************/

// {00000000-0000-0000-0000-000000000000}
DEFINE_GUID(FarGuid,0,0,0,0,0,0,0,0,0,0,0);
// {E80B8002-EED3-4563-9C78-2E3C3246F8D3}
DEFINE_GUID(MainGuid, 0xe80b8002, 0xeed3, 0x4563, 0x9c, 0x78, 0x2e, 0x3c, 0x32, 0x46, 0xf8, 0xd3);
// {F8A515F3-A57C-4c91-9C07-9A196143550A}
DEFINE_GUID(MenuGuid, 0xf8a515f3, 0xa57c, 0x4c91, 0x9c, 0x7, 0x9a, 0x19, 0x61, 0x43, 0x55, 0xa);
// {AD50A5CA-1E33-4e24-9A51-97A481D41F56}
DEFINE_GUID(DlgGuid, 0xad50a5ca, 0x1e33, 0x4e24, 0x9a, 0x51, 0x97, 0xa4, 0x81, 0xd4, 0x1f, 0x56);
// {B6334FF1-57A0-47ca-A924-007B6F3F770F}
DEFINE_GUID(DlgNameGuid, 0xb6334ff1, 0x57a0, 0x47ca, 0xa9, 0x24, 0x0, 0x7b, 0x6f, 0x3f, 0x77, 0xf);
// {116F1641-ACB2-45FA-A227-B846B848C731}
DEFINE_GUID(ErrorMsgGuid,0x116f1641, 0xacb2, 0x45fa, 0xa2, 0x27, 0xb8, 0x46, 0xb8, 0x48, 0xc7, 0x31);
// {E9201D9D-BB06-4436-8B73-A58EE40FD7F4}
DEFINE_GUID(YesNoMsgGuid,0xe9201d9d, 0xbb06, 0x4436, 0x8b, 0x73, 0xa5, 0x8e, 0xe4, 0xf, 0xd7, 0xf4);
// {F9F5198A-127F-453B-AB5D-AA499F85D8AC}
DEFINE_GUID(DebugMsgGuid,0xf9f5198a, 0x127f, 0x453b, 0xab, 0x5d, 0xaa, 0x49, 0x9f, 0x85, 0xd8, 0xac);
// {3F7C0285-33A9-4B82-A02A-885A32EECC7E}
DEFINE_GUID(LoadUndoMsgGuid,0x3f7c0285, 0x33a9, 0x4b82, 0xa0, 0x2a, 0x88, 0x5a, 0x32, 0xee, 0xcc, 0x7e);
// {18D8B418-AEC6-48B2-AAF2-C527D04AF5C4}
DEFINE_GUID(LoadFilesMsgGuid,0x18d8b418, 0xaec6, 0x48b2, 0xaa, 0xf2, 0xc5, 0x27, 0xd0, 0x4a, 0xf5, 0xc4);
// {5E96F905-E19B-4BB4-AD71-95D74BEB55F6}
DEFINE_GUID(RenameFailMsgGuid,0x5e96f905, 0xe19b, 0x4bb4, 0xad, 0x71, 0x95, 0xd7, 0x4b, 0xeb, 0x55, 0xf6);
// {4AE35A38-CB3F-4B6D-A3B8-180F8C3AE35C}
DEFINE_GUID(RenMsgGuid,0x4ae35a38, 0xcb3f, 0x4b6d, 0xa3, 0xb8, 0x18, 0xf, 0x8c, 0x3a, 0xe3, 0x5c);
// {CCAF7A6F-7078-46BC-9092-9769C2665329}
DEFINE_GUID(ShowOrgNameMsgGuid,0xccaf7a6f, 0x7078, 0x46bc, 0x90, 0x92, 0x97, 0x69, 0xc2, 0x66, 0x53, 0x29);
// {B60068E2-AC3B-4B4A-8F5D-991190A28183}
DEFINE_GUID(InputBoxGuid,0xb60068e2, 0xac3b, 0x4b4a, 0x8f, 0x5d, 0x99, 0x11, 0x90, 0xa2, 0x81, 0x83);


/****************************************************************************
 * Копии стандартных структур FAR
 ****************************************************************************/
extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

/****************************************************************************
 * Элемент для переименования
 ****************************************************************************/
struct File
{
	string strSrcFileName;                         //   здесь оригинальные имена с панели
	string strDestFileName;                        //   конечные имена
	FILETIME ftLastWriteTime;                      //   время последней модификации

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
 * Текущие настройки плагина
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
			LogRen,                             // отвечает за заполнение отката
			LoadUndo,                           // если есть откат - загрузим/отобразим
			Undo;                               // если есть откат - выполним его
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
 * Undo переименования
 ****************************************************************************/
extern struct UndoFileName                       // элементы для отката переименования
{
	wchar_t *Dir;                                  //   папка в которой переименовывали
	wchar_t **CurFileName;                         //   имена, в которые переименовали файлы
	wchar_t **OldFileName;                         //   имена, которые были у файлов до переименования
	int iCount;                                    //   кол-во
} Undo;

/****************************************************************************
 * Размер диалога.
 ****************************************************************************/
extern struct DlgSize
{
	// состояние диалога
	bool Full;
	// нормальный
	DWORD W;
	DWORD W2;
	DWORD WS;
	DWORD H;
	// максимизированный
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
__int64 GetFarSetting(FARSETTINGS_SUBFOLDERS Root,const wchar_t* Name);
