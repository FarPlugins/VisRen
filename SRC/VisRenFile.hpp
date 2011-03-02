/****************************************************************************
 * VisRenFile.hpp
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
#include "DList.hpp"
#include "VisRen.hpp"

class RenFile
{
	public:
		DList<File> FileList;
		string strPanelDir, strNativePanelDir;
		struct StrOptions StrOpt;
		bool bError;

	private:
		bool InitUndoItem(const wchar_t *CurFileName, const wchar_t *OldFileName, int Count);
		bool CheckFileName(const wchar_t *str);
		bool IsEmpty(const wchar_t *str);
		bool GetNewNameExt(const wchar_t *src, string &strDest,unsigned ItemIndex, DWORD *dwFlags, SYSTEMTIME ModificTime, bool bProcessName);
		bool Replase(string &strSrc);
		void Translit(string &strName,string &strExt,DWORD dwTranslit);
		void Case(wchar_t *Name,wchar_t *Ext,DWORD dwCase);
		bool CheckForEsc(HANDLE hConInp);
		int CreateList(const wchar_t *TempFileName);
		int ReadList(const wchar_t *TempFileName);

	public:
		RenFile() { }
		~RenFile() { }
		bool InitFileList(int SelectedItemsNumber);
		bool ProcessFileName();
		bool RenameFile(int SelectedItemsNumber, int ItemsNumber);
		void RenameInEditor(int SelectedItemsNumber, int ItemsNumber);
};
