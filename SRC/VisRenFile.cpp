/****************************************************************************
 * VisRenFile.cpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2007-2012 Alexey Samlyukov
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

#pragma hdrstop
#include "VisRenFile.hpp"
#include "VisRenFile_MP3.cpp"
#include "VisRenFile_JPG.cpp"

enum {
	NONE								=0xFFFFFFFF,

	NAME_CASE_LOWER			=0x00000001,
	NAME_CASE_UPPER			=0x00000002,
	NAME_CASE_FIRST			=0x00000004,
	NAME_CASE_TITLE			=0x00000008,
	NAME_CASE_MUSIC			=0x00000010, // музыкальный файл - обрабатывается особо
	NAME_TRANSLIT_ENG		=0x00000020,
	NAME_TRANSLIT_RUS		=0x00000040,

	EXT_CASE_LOWER			=0x00001000,
	EXT_CASE_UPPER			=0x00002000,
	EXT_CASE_FIRST			=0x00004000,
	EXT_CASE_TITLE			=0x00008000,
	EXT_TRANSLIT_ENG		=0x00010000,
	EXT_TRANSLIT_RUS		=0x00020000
};

/****************************************************************************
 * Инициализация элементами для переименования
 ****************************************************************************/
bool RenFile::InitFileList(int SelectedItemsNumber)
{
	for (int i=0; i<SelectedItemsNumber; i++)
	{
		File add;
		size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,i,0);
		PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
		if (PPI)
		{
			FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,i,&FGPPI);

			if (FGPPI.Item->FileName)
			{
				add.strSrcFileName=FGPPI.Item->FileName;
				add.strDestFileName=FGPPI.Item->FileName;
				add.ftLastWriteTime.dwLowDateTime=FGPPI.Item->LastWriteTime.dwLowDateTime;
				add.ftLastWriteTime.dwHighDateTime=FGPPI.Item->LastWriteTime.dwHighDateTime;
				FileList.Push(&add);
			}
			free(PPI);
		}
		else
			return false;
	}

	// получим strPanelDir
	size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,0);
	if (size)
	{
		FarPanelDirectory *dirbuf=(FarPanelDirectory*)malloc(size);
		if (dirbuf)
		{
			dirbuf->StructSize=sizeof(FarPanelDirectory);
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,size,dirbuf);
			strPanelDir=dirbuf->Name;
			free(dirbuf);
		}
	}
	// получим strNativePanelDir - "\\?\dir\"
	size=FSF.ConvertPath(CPM_NATIVE,strPanelDir.get(),0,0);
	wchar_t *buf=strNativePanelDir.get(size+1); //+1 для FSF.AddEndSlash()
	FSF.ConvertPath(CPM_NATIVE,strPanelDir.get(),buf,size);
	strNativePanelDir.updsize();
	FSF.AddEndSlash(buf);
	strNativePanelDir.updsize();

	return true;
}

/****************************************************************************
 * Заполнение структуры элементами для отката
 ****************************************************************************/
bool RenFile::InitUndoItem(const wchar_t *CurFileName, const wchar_t *OldFileName, int Count)
{
	if (Count>=Undo.iCount)
	{
		Undo.CurFileName=(wchar_t**)realloc(Undo.CurFileName,(Count+1)*sizeof(wchar_t*));
		Undo.OldFileName=(wchar_t**)realloc(Undo.OldFileName,(Count+1)*sizeof(wchar_t*));
		if (!Undo.CurFileName || !Undo.OldFileName)
			return false;
	}
	Undo.CurFileName[Count]=(wchar_t*)realloc(Undo.CurFileName[Count],(wcslen(CurFileName)+1)*sizeof(wchar_t));
	Undo.OldFileName[Count]=(wchar_t*)realloc(Undo.OldFileName[Count],(wcslen(OldFileName)+1)*sizeof(wchar_t));
	if (!Undo.CurFileName[Count] || !Undo.OldFileName[Count])
		return false;
	wcscpy(Undo.CurFileName[Count],CurFileName);
	wcscpy(Undo.OldFileName[Count],OldFileName);
	return true;
}

/****************************************************************************
 * Проверка имени файла на допустимые символы
 ****************************************************************************/
bool RenFile::CheckFileName(const wchar_t *str)
{
	static wchar_t Denied[]=L"<>:\"*?/\\|";

	for (wchar_t *s=(wchar_t *)str; *s; s++)
		for (wchar_t *p=Denied; *p; p++)
			if (*s==*p) return false;
	return true;
}

/****************************************************************************
 * Проверим, что содержимое тега не пусто
 ****************************************************************************/
bool RenFile::IsEmpty(const wchar_t *Str)
{
	if (Str)
	{
	for (int i=0; i<wcslen(Str); i++)
		if ( (Str[i] != L'\n') && (Str[i] != L'\r') && (Str[i] != L'\t')
			&& (Str[i] != L'\0') && (Str[i] != L' ') )
			return false;
	}
	return true;
}


/****************************************************************************
 * Преобразование по маске имени и расширения файлов
 ****************************************************************************/
bool RenFile::GetNewNameExt(const wchar_t *src, string &strDest,unsigned ItemIndex, DWORD *dwFlags, SYSTEMTIME ModificTime, bool bProcessName)
{
	bool bCorrectJPG=false, bCorrectBMP=false, bCorrectGIF=false, bCorrectPNG=false;
	string FullFilename=strNativePanelDir.get();
	FullFilename+=src;

	ID3TagInternal *pInternalTag=0;
	if (FSF.ProcessName(L"*.mp3",(wchar_t *)src,0,PN_CMPNAME))
		pInternalTag=AnalyseMP3File(FullFilename.get());

	else if (FSF.ProcessName(L"*.jpg",(wchar_t *)src,0,PN_CMPNAME))
		bCorrectJPG=AnalyseImageFile(FullFilename, isJPG);
	else if (FSF.ProcessName(L"*.bmp",(wchar_t *)src,0,PN_CMPNAME))
		bCorrectBMP=AnalyseImageFile(FullFilename, isBMP);
	else if (FSF.ProcessName(L"*.gif",(wchar_t *)src,0,PN_CMPNAME))
		bCorrectGIF=AnalyseImageFile(FullFilename, isGIF);
	else if (FSF.ProcessName(L"*.png",(wchar_t *)src,0,PN_CMPNAME))
		bCorrectPNG=AnalyseImageFile(FullFilename, isPNG);

	wchar_t *pMask= bProcessName?StrOpt.MaskName.get():StrOpt.MaskExt.get();
	string buf;
	while (*pMask)
	{
		if (!Strncmp(pMask, L"[[]", 3))
		{
			buf+=*pMask++; pMask+=2;
		}
		else if (!Strncmp(pMask, L"[]]", 3))
		{
			pMask+=2; buf+=*pMask++;
		}
		else if (!Strncmp(pMask, L"[N", 2) || !Strncmp(pMask, L"[E", 2))
		{
			string Name(src);
			string Ext;
			wchar_t *ptr=wcsrchr((wchar_t *)src,L'.');
			if (ptr && *(ptr+1)) { Ext=ptr+1; Name(src, (size_t)(ptr-src)); }

			bool bName=!Strncmp(pMask, L"[N", 2);
			int len=bName?Name.length():Ext.length();

			if (*(pMask+2)==L']')  // [N] или [E]
			{
				buf+=(bName?Name.get():Ext.get()); pMask+=3;
			}
			else // [N#-#] или [E#-#]
			{
				string strStart, strEnd;
				int iStart, iEnd;
				bool bFromEnd_Start=false, bFromEnd_End=false;
				pMask+=2;
				// отсчитывать будем с конца имени
				if (*pMask==L'-') { bFromEnd_Start=true; pMask++; }
				// число
				for (int i=0; *pMask>=L'0' && *pMask<=L'9'; i++, pMask++) { strStart+=*pMask; }
				iStart=FSF.atoi(strStart.get());
				if (iStart==0)
					return false;
				if (bFromEnd_Start)
					iStart=len-iStart;
				else
				{
					iStart--;
					if (iStart>len) iStart=len;
				}
				// копируем символ...
				if (*pMask==L']')
				{
					pMask++;
					if (iStart<0) iStart=len;   //< bFromEnd_Start==true
					buf+=(bName?Name[(size_t)iStart]:Ext[(size_t)iStart]);
				}
				// иначе, разбираем дальше...
				else
				{
					if (iStart<0) iStart=0;     //< bFromEnd_Start==true
					// копируем несколько символов
					if (*pMask==L',')
					{
						pMask++;
						for (int i=0; *pMask>=L'0' && *pMask<=L'9'; i++, pMask++) { strEnd+=*pMask; }
						iEnd=FSF.atoi(strEnd.get());
						if (!iEnd || *pMask!=L']')
							return false;
						wchar_t *ptrcur=(wchar_t *)malloc((iEnd+2)*sizeof(wchar_t));
						if (ptrcur)
						{
							lstrcpyn(ptrcur, (bName?Name.get():Ext.get())+iStart, iEnd+1);
							buf+=ptrcur;  free(ptrcur);  pMask++;
						}
					}
					// копируем диапазон символов...
					else if (*pMask==L'-')
					{
						pMask++;
						// просто до конца строки
						if (*pMask==L']')
						{
							buf+=(bName?Name.get():Ext.get())+iStart;
							pMask++;
						}
						// сам диапазон
						else
						{
							if (bFromEnd_Start) bFromEnd_End=true;
							if (*pMask==L'-')
							{
								if (bFromEnd_End) return false;
								else { bFromEnd_End=true; pMask++; }
							}
							for (int i=0; *pMask>=L'0' && *pMask<=L'9'; i++, pMask++) { strEnd+=*pMask; }
							iEnd=FSF.atoi(strEnd.get());
							if (!iEnd || *pMask!=L']')
								return false;
							if (bFromEnd_End)
							{
								iEnd=len-(iEnd-1);
								if (iEnd<iStart) iEnd=iStart;
							}
							else
								if (iEnd<=iStart) return false;
							wchar_t *ptrcur=(wchar_t *)malloc((iEnd+2)*sizeof(wchar_t));
							if (ptrcur)
							{
								lstrcpyn(ptrcur, (bName?Name.get():Ext.get()), iEnd+1);
								buf+=(ptrcur+iStart);  free(ptrcur);  pMask++;
							}
						}
					}
					else return false;
				}
			}
		}
		else if (!Strncmp(pMask, L"[C", 2))
		{
			string strStart, strWidth, strStep;
			unsigned i, iStart, iWidth, iStep;
			pMask+=2;
			for (i=0; *pMask>=L'0' && *pMask<=L'9'; i++, pMask++) { strStart+=*pMask; }
			if (!i || i>9 || *pMask!=L'+')
				return false;
			iStart=FSF.atoi(strStart.get());
			iWidth=i; pMask++;
			for (i=0; *pMask>=L'0' && *pMask<=L'9'; i++, pMask++) { strStep+=*pMask; }
			if (!i || *pMask!=L']')
				return false;
			iStep=FSF.atoi(strStep.get());
			wchar_t ptr[15];
			FSF.sprintf(ptr, L"%0*d", iWidth, iStart+ItemIndex*iStep);
			buf+=ptr;  pMask++;
		}
		else if (!Strncmp(pMask, L"[L]", 3))
		{
			*dwFlags|=(bProcessName?NAME_CASE_LOWER:EXT_CASE_LOWER);
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[U]", 3))
		{
			*dwFlags|=(bProcessName?NAME_CASE_UPPER:EXT_CASE_UPPER);
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[F]", 3))
		{
			*dwFlags|=(bProcessName?NAME_CASE_FIRST:EXT_CASE_FIRST);
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[T]", 3))
		{
			*dwFlags|=(bProcessName?NAME_CASE_TITLE:EXT_CASE_TITLE);
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[M]", 3))
		{
			*dwFlags|=(bProcessName?NAME_CASE_MUSIC:NONE);
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[#]", 3))
		{
			if (pInternalTag && !IsEmpty(pInternalTag->pEntry[TAG_TRACK]))
				buf+=pInternalTag->pEntry[TAG_TRACK];
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[t]", 3))
		{
			if ( pInternalTag &&
					!IsEmpty(pInternalTag->pEntry[TAG_TITLE]) &&
					CheckFileName(pInternalTag->pEntry[TAG_TITLE]) )
			{
				FSF.Trim(pInternalTag->pEntry[TAG_TITLE]);
				buf+=pInternalTag->pEntry[TAG_TITLE];
			}
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[a]", 3))
		{
			if ( pInternalTag &&
					!IsEmpty(pInternalTag->pEntry[TAG_ARTIST]) &&
					CheckFileName(pInternalTag->pEntry[TAG_ARTIST]) )
			{
				FSF.Trim(pInternalTag->pEntry[TAG_ARTIST]);
				buf+=pInternalTag->pEntry[TAG_ARTIST];
			}
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[l]", 3))
		{
			if ( pInternalTag &&
					!IsEmpty(pInternalTag->pEntry[TAG_ALBUM]) &&
					CheckFileName(pInternalTag->pEntry[TAG_ALBUM]) )
			{
				FSF.Trim(pInternalTag->pEntry[TAG_ALBUM]);
				buf+=pInternalTag->pEntry[TAG_ALBUM];
			}
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[y]", 3))
		{
			if (pInternalTag && !IsEmpty(pInternalTag->pEntry[TAG_YEAR]))
				buf+=pInternalTag->pEntry[TAG_YEAR];
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[g]", 3))
		{
			if ( pInternalTag &&
					!IsEmpty(pInternalTag->pEntry[TAG_GENRE]) &&
					CheckFileName(pInternalTag->pEntry[TAG_GENRE]) )
			{
				buf+=pInternalTag->pEntry[TAG_GENRE];
			}
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[c]", 3))
		{
			if (bCorrectJPG && CheckFileName(ImageInfo.CameraMake))
				buf+=ImageInfo.CameraMake;
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[m]", 3))
		{
			if (bCorrectJPG && CheckFileName(ImageInfo.CameraModel))
				buf+=ImageInfo.CameraModel;
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[d]", 3))
		{
			if (bCorrectJPG || bCorrectBMP || bCorrectGIF || bCorrectPNG)
			{
				if (ImageInfo.DateTime[0])
					buf+=ImageInfo.DateTime;
				else
				{
					wchar_t modific[20];
					FSF.sprintf(modific, L"%04d.%02d.%02d %02d-%02d-%02d",
											ModificTime.wYear, ModificTime.wMonth, ModificTime.wDay,
											ModificTime.wHour, ModificTime.wMinute, ModificTime.wSecond);
					buf+=modific;
				}
			}
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[r]", 3))
		{
			if (bCorrectJPG || bCorrectBMP || bCorrectGIF || bCorrectPNG)
			{
				wchar_t ptr[80];
				FSF.sprintf(ptr, L"%dx%d", ImageInfo.Width, ImageInfo.Height);
				buf+=ptr;
			}
			pMask+=3;
		}
		else if (!Strncmp(pMask, L"[DM]", 4))
		{
			wchar_t ptr[11];
			FSF.sprintf(ptr, L"%04d.%02d.%02d", ModificTime.wYear, ModificTime.wMonth, ModificTime.wDay);
			buf+=ptr;  pMask+=4;
		}
		else if (!Strncmp(pMask, L"[TM]", 4))
		{
			wchar_t ptr[9];
			FSF.sprintf(ptr, L"%02d-%02d-%02d", ModificTime.wHour, ModificTime.wMinute, ModificTime.wSecond);
			buf+=ptr;  pMask+=4;
		}
		else if (!Strncmp(pMask, L"[TL]", 4))
		{
			*dwFlags|=(bProcessName?NAME_TRANSLIT_ENG:EXT_TRANSLIT_ENG);
			pMask+=4;
		}
		else if (!Strncmp(pMask, L"[TR]", 4))
		{
			*dwFlags|=(bProcessName?NAME_TRANSLIT_RUS:EXT_TRANSLIT_RUS);
			pMask+=4;
		}
		else if (*pMask==L'[' || *pMask==L']') return false;
		else buf+=*pMask++;
	}

	strDest=buf.get();

	if (pInternalTag)
	{
		for (int i=0; i<pInternalTag->nEntryCount; i++)
			free(pInternalTag->pEntry[i]);
		delete pInternalTag;
	}

	return true;
}


/****************************************************************************
 * Поиск и замена в именах файлов (регистрозависимая, опционально).
 ****************************************************************************/
bool RenFile::Replase(string &strSrc)
{
	int lenSearch=StrOpt.Search.length();
	if (!lenSearch) return true;

	int lenSrc=strSrc.length();
	int lenReplace=StrOpt.Replace.length();
	int j=0;
	const wchar_t *src=strSrc.get();
	string strBuf;
	// делаем замену
	if (!Opt.RegEx)
	{
		for (int i=0; i<lenSrc; )
		{
			if (!(Opt.CaseSensitive?Strncmp(src+i, StrOpt.Search.get(), lenSearch)
														:FSF.LStrnicmp(src+i, StrOpt.Search.get(), lenSearch) ))
			{
				strBuf+=StrOpt.Replace.get();
				i+=lenSearch;
			}
			else
				strBuf+=src[i++];
		}
		if (!CheckFileName(strBuf.get())) return false;
		strSrc=strBuf.get();
	}
	else
	{
		HANDLE re;
		int start_offset=0;
		if (!Info.RegExpControl(0,RECTL_CREATE,0,&re)) return false;

		string Search=L"/";
		if (StrOpt.Search.length()>0 && StrOpt.Search[(size_t)0]==L'/') 
			Search+=(StrOpt.Search.get()+1);
		else Search+=StrOpt.Search.get();
		if (StrOpt.Search.length()>0 && StrOpt.Search[(size_t)(StrOpt.Search.length()-1)]!=L'/') 
			Search+=L"/";
		if (Search.length()>0 && Search[(size_t)(Search.length()-1)]==L'/' && !Opt.CaseSensitive)
			Search+=L"i";
		if (Info.RegExpControl(re,RECTL_COMPILE,0,Search.get()))
		{
			int brackets=Info.RegExpControl(re,RECTL_BRACKETSCOUNT,0,0);
			if (!brackets) { Info.RegExpControl(re,RECTL_FREE,0,0); return false; }
			RegExpMatch *match=(RegExpMatch*)malloc(brackets*sizeof(RegExpMatch));

			for (;;)
			{
				RegExpSearch search= { src,start_offset,lenSrc,match,brackets,0 };

				if (Info.RegExpControl(re,RECTL_SEARCHEX,0,&search))
				{
					// копируем ДО паттерна
					for (int i=start_offset; i<match[0].start; i++)
						strBuf+=src[i];

					// нашли паттерн. подменяем содержимым StrOpt.Replace
					for (int i=0; i<lenReplace; i++)
					{
						// после '\' вставляем без изменений
						if (StrOpt.Replace[(size_t)i]==L'\\' && i+1<lenReplace)
						{
							strBuf+=(StrOpt.Replace.get())[++i]; continue;
						}

						// подменяем на найденные подвыражения
						if (StrOpt.Replace[(size_t)i]==L'$' && i+1<lenReplace)
						{
							wchar_t *p=StrOpt.Replace.get()+(i+1), Digit[10];
							unsigned int k, iDigit;
							for (k=0; *p>=L'0' && *p<=L'9' && k<9; k++, p++)
								Digit[k]=*p;
							if (k)
							{
								Digit[k]=L'\0';
								iDigit=FSF.atoi(Digit);
								if (brackets && iDigit<=(brackets-1))
								{
									i+=k;
									for (k=match[iDigit].start; k<match[iDigit].end; k++)
										strBuf+=src[k];
								}
								else i+=k;
							}
							else strBuf+=StrOpt.Replace[(size_t)i];
						}
						else strBuf+=StrOpt.Replace[(size_t)i];
					}
					start_offset=match[0].end;

					if (match[0].start==match[0].end || start_offset>=lenSrc)
						break;
				}
				else
					break;
			}
			free(match);
			Info.RegExpControl(re,RECTL_FREE,0,0);
		}
		// копируем всё то что не вошло в паттерн
		for (int i=start_offset; i<lenSrc; i++)
			strBuf+=src[i];
		if (!CheckFileName(strBuf.get())) return false;
		strSrc=strBuf.get();
	}
	return true;
}

/****************************************************************************
 * Транслитерация имен файлов: русский <-> russkij
 ****************************************************************************/
void RenFile::Translit(string &strName, string &strExt, DWORD dwTranslit)
{
	if (dwTranslit&NAME_TRANSLIT_ENG || dwTranslit&NAME_TRANSLIT_RUS ||
			dwTranslit&EXT_TRANSLIT_ENG || dwTranslit&EXT_TRANSLIT_RUS   )
	{
		const wchar_t rus[][33]={
///			 "ж"         "з"         "ы"         "в"         "у"
			{0x436,0x0},{0x437,0x0},{0x44b,0x0},{0x432,0x0},{0x443,0x0},
///			 "т"         "щ"         "ш"         "с"         "р"
			{0x442,0x0},{0x449,0x0},{0x448,0x0},{0x441,0x0},{0x440,0x0},
///			 "п"         "о"         "н"         "м"         "л"
			{0x43f,0x0},{0x43e,0x0},{0x43d,0x0},{0x43c,0x0},{0x43b,0x0},
///			 "х"         "к"         "ю"         "ё"         "я"
			{0x445,0x0},{0x43a,0x0},{0x44e,0x0},{0x451,0x0},{0x44f,0x0},
///			 "й"         "и"         "г"         "ф"         "э"
			{0x439,0x0},{0x438,0x0},{0x433,0x0},{0x444,0x0},{0x44d,0x0},
///			 "е"         "д"         "ч"         "ц"         "б"
			{0x435,0x0},{0x434,0x0},{0x447,0x0},{0x446,0x0},{0x431,0x0},
///			 "а"         "ъ"         "ь"
			{0x430,0x0},{0x44a,0x0},{0x44c,0x0}
		};

		const wchar_t eng[][33]={
			L"zh",L"z",L"y",L"v",L"u",
			L"t",L"shh",L"sh",L"s",L"r",
			L"p",L"o",L"n",L"m",L"l",
			L"kh",L"k",L"ju",L"jo",L"ja",
			L"j",L"i",L"g",L"f",L"eh",
			L"e",L"d",L"ch",L"c",L"b",
			L"a",L"`",L"'"
		};

		string out;

		if (dwTranslit&NAME_TRANSLIT_ENG || dwTranslit&NAME_TRANSLIT_RUS)
		{
			int i=0, lenIn=0;
			wchar_t *in=strName.get();
			out.clear();
			bool bEng=dwTranslit&NAME_TRANSLIT_ENG;
			while (*in)
			{
				for (i=0; i<33; i++)
					if (!FSF.LStrnicmp(in, (bEng?rus[i]:eng[i]), lenIn=wcslen(bEng?rus[i]:eng[i])))
					{
						wchar_t buf[4];
						wcscpy(buf,(bEng?eng[i]:rus[i]));
						if (FSF.LIsUpper(*in)) FSF.LStrupr(buf);
						out+=buf;
						in+=lenIn;
						break;
					}
				if (i==33) out+=*in++;
			}
			strName=out.get();
		}

		if (dwTranslit&EXT_TRANSLIT_ENG || dwTranslit&EXT_TRANSLIT_RUS)
		{
			int i=0, lenIn=0;
			wchar_t *in=strExt.get();
			out.clear();
			bool bEng=dwTranslit&EXT_TRANSLIT_ENG;
			while (*in)
			{
				for (i=0; i<33; i++)
					if (!FSF.LStrnicmp(in, (bEng?rus[i]:eng[i]), lenIn=wcslen(bEng?rus[i]:eng[i])))
					{
						wchar_t buf[4];
						wcscpy(buf,(bEng?eng[i]:rus[i]));
						if (FSF.LIsUpper(*in)) FSF.LStrupr(buf);
						out+=buf;
						in+=lenIn;
						break;
					}
				if (i==33) out+=*in++;
			}
			strExt=out.get();
		}
	}
}

/****************************************************************************
 * Изменение регистра имен файлов.
 ****************************************************************************/
void RenFile::Case(wchar_t *Name, wchar_t *Ext, DWORD dwCase)
{
	if (!dwCase) return;
	// регистр имени
	if (dwCase&NAME_CASE_LOWER)
		FSF.LStrlwr(Name);
	else if (dwCase&NAME_CASE_UPPER)
		FSF.LStrupr(Name);
	else if (dwCase&NAME_CASE_FIRST)
	{
		*Name = (wchar_t)FSF.LUpper((wchar_t)*Name);
		FSF.LStrlwr(Name+1);
	}
	else if (dwCase&NAME_CASE_TITLE)
	{
		for (int i=0; Name[i]; i++)
		{
			if (!i || wmemchr(StrOpt.WordDiv.get(), Name[i-1], StrOpt.WordDiv.length()))
				Name[i]=(wchar_t)FSF.LUpper((wchar_t)Name[i]);
			else
				Name[i]=(wchar_t)FSF.LLower((wchar_t)Name[i]);
		}
	}
	// музыкальный файл обрабатывается особо:
	// все слова до " - " или "_-_" будут в CASE_NAME_TITLE, а после в - CASE_NAME_FIRST
	else if (dwCase&NAME_CASE_MUSIC)
	{
		int lenName=wcslen(Name);
		int Ptr=lenName;
		for (int i=0; i<lenName; i++)
		{
			if (!Strncmp(Name+i, L" - ", 3) || !Strncmp(Name+i, L"_-_", 3))
			{
				if (i>0 && FSF.LIsAlpha((wchar_t)Name[i-1]))
				{
					Ptr=i;
					break;
				}
			}
		}
		for (int i=0; Name[i] && i<Ptr; i++)
		{
			if (!i || wmemchr(StrOpt.WordDiv.get(), Name[i-1], StrOpt.WordDiv.length()))
				Name[i]=(wchar_t)FSF.LUpper((wchar_t)Name[i]);
			else
				Name[i]=(wchar_t)FSF.LLower((wchar_t)Name[i]);
			if (i>0 && Name[i+1] && wmemchr(StrOpt.WordDiv.get(), Name[i-1], StrOpt.WordDiv.length())
					&& ((wchar_t)Name[i])==0x418 && wmemchr(StrOpt.WordDiv.get(), Name[i+1], StrOpt.WordDiv.length()))
				Name[i]=(wchar_t)FSF.LLower((wchar_t)Name[i]);
		}
		if (Ptr!=lenName)
		{
			*(Name+Ptr+3) = (wchar_t)FSF.LUpper((wchar_t)*(Name+Ptr+3));
			FSF.LStrlwr(Name+Ptr+3+1);
		}
	}

	// регистр расширения
	if (dwCase&EXT_CASE_LOWER)
		FSF.LStrlwr(Ext);
	else if (dwCase&EXT_CASE_UPPER)
		FSF.LStrupr(Ext);
	else if (dwCase&EXT_CASE_FIRST)
	{
		*Ext = (wchar_t)FSF.LUpper((wchar_t)*Ext);
		FSF.LStrlwr(Ext+1);
	}
	else if (dwCase&EXT_CASE_TITLE)
		for (int i=0; Ext[i]; i++)
		{
			if (!i || wmemchr(StrOpt.WordDiv.get(), Ext[i-1], StrOpt.WordDiv.length()))
				Ext[i]=(wchar_t)FSF.LUpper((wchar_t)Ext[i]);
			else
				Ext[i]=(wchar_t)FSF.LLower((wchar_t)Ext[i]);
		}
}


/****************************************************************************
 * Проверка на Esc. Возвращает true, если пользователь нажал Esc
 ****************************************************************************/
bool RenFile::CheckForEsc(HANDLE hConInp)
{
	if (hConInp==INVALID_HANDLE_VALUE) return false;

	static DWORD dwTicks;
	DWORD dwNewTicks=GetTickCount();
	if (dwNewTicks-dwTicks<500) return false;
	dwTicks=dwNewTicks;

	INPUT_RECORD rec;
	DWORD ReadCount;
	while (PeekConsoleInput(hConInp, &rec, 1, &ReadCount) && ReadCount)
	{
		ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
		if ( rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
					rec.Event.KeyEvent.bKeyDown )
			// Опциональное подтверждение прерывания по Esc
			if (GetFarSetting(FSSF_CONFIRMATIONS,L"Esc"))
			{
				if (YesNoMsg(MEscTitle, MEscBody)) return true;
			}
			else return true;
	}
	return false;
}


/****************************************************************************
 * Основная функция по обработке и созданию новых имен файлов.
 ****************************************************************************/
bool RenFile::ProcessFileName()
{
	if (!CheckFileName(StrOpt.MaskName.get()) || !CheckFileName(StrOpt.MaskExt.get()))
		return false;

	HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
	HANDLE hConInp=CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	DWORD dwTicks=GetTickCount();
	bool bRet=true;

	File *Item=NULL; unsigned Index;
	for (Item=FileList.First(), Index=0; Item != NULL; Item=FileList.Next(Item), Index++)
	{
		if (GetTickCount()-dwTicks>1000)
		{
			wchar_t buf[15];
			FSF.itoa(Index, buf, 10);
			static wchar_t *MsgItems[]={(wchar_t *)GetMsg(MVRenTitle), (wchar_t *)GetMsg(MLoadFiles), buf};
			Info.Message(&MainGuid,&LoadFilesMsgGuid, 0, 0, MsgItems, 3, 0);

			if (CheckForEsc(hConInp))
			{
				for ( ; Item != NULL; Item=FileList.Next(Item))
					Item=FileList.Delete(Item);
				break;
			}
		}

		SYSTEMTIME ModificTime; FILETIME local;
		FileTimeToLocalFileTime(&(Item->ftLastWriteTime), &local);
		FileTimeToSystemTime(&local, &ModificTime);
		DWORD dwFlags=0;

		string NewName(Item->strSrcFileName.get());
		string NewExt;
		wchar_t *src=Item->strSrcFileName.get();
		wchar_t *ptr=wcsrchr(src,L'.');
		if (ptr && *(ptr+1))
		{
			NewExt=ptr+1;
			NewName(src, (size_t)(ptr-src));
		}

		if ( !GetNewNameExt(Item->strSrcFileName.get(), NewName, Index, &dwFlags, ModificTime, true) ||   // имя файла
				 !GetNewNameExt(Item->strSrcFileName.get(), NewExt, Index, &dwFlags, ModificTime, false)  )   // расширение
		{
			bRet=false; break;
		}

		Item->strDestFileName=NewName;

		if (NewExt.length())
		{
			Item->strDestFileName+=L'.';
			Item->strDestFileName+=NewExt.get();
		}

		NewName = Item->strDestFileName;

		if (!Replase(NewName))
		{
			bRet=false; break;
		}

		src=NewName.get();
		ptr=wcsrchr(src,L'.');
		if (ptr && *(ptr+1))
		{
			NewExt=ptr+1;
			NewName(src, (size_t)(ptr-src));
		}
		else
			NewExt.clear();

		Translit(NewName, NewExt, dwFlags);

		Case(NewName.get(), NewExt.get(), dwFlags);

		Item->strDestFileName=NewName;
		if (NewExt.length())
		{
			Item->strDestFileName+=L'.';
			Item->strDestFileName+=NewExt.get();
		}
	}

	CloseHandle(hConInp);
	Info.RestoreScreen(hScreen);
	return bRet;
}


/****************************************************************************
 * Функция по переименованию файлов. Делает протокол переименования.
 * Показывает предупреждение, если произошла ошибка при переименовании файла.
 * Показывает кол-во обработанных файлов из общего числа переданных.
 ****************************************************************************/
bool RenFile::RenameFile(int SelectedItemsNumber, int ItemsNumber)
{
	bool bSkipAll=false;
	int i, Count, iRen=0, iUndo=0;
	PanelRedrawInfo RInfo={0,0};
	SetLastError(ERROR_SUCCESS);

	Info.PanelControl(PANEL_ACTIVE,FCTL_BEGINSELECTION,0,0);

	// вначале снимем выделение на панели
	for (int j=0; j<SelectedItemsNumber; j++)
		Info.PanelControl(PANEL_ACTIVE,FCTL_CLEARSELECTION,j,0);

	if (!Opt.Undo)
	{
		Count=FileList.Count();


		for (File *Item=FileList.First(); Item != NULL; Item=FileList.Next(Item))
		{
			wchar_t *src =Item->strSrcFileName.get(),
							*dest=Item->strDestFileName.get();

			// Имена совпадают - пропустим переименование
			if (!Strncmp(src, dest)) continue;

			string srcFull=strNativePanelDir.get();
			srcFull+=src;
			string destFull=strNativePanelDir.get();
			destFull+=dest;

 RETRY_1:
			// Переименовываем
			if (MoveFileW(srcFull.get(), destFull.get()))
			{
				iRen++;
				// поместим в Undo
				if (Opt.LogRen && !InitUndoItem(dest, src, iUndo++))
				{
					Opt.LogRen=0;
					ErrorMsg(MErrorCreateLog, MErrorNoMem);
				}
				continue;
			}
			// файл был удален сторонним процессом - продолжим без него
			if (GetLastError()==ERROR_FILE_NOT_FOUND) continue;
			// не переименовали - отметим
			if (bSkipAll)
			{
				for (int j=0; j<ItemsNumber; j++)
				{
					size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,0);
					PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
					if (PPI)
					{
						FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
						Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,&FGPPI);
						if (!FSF.LStricmp(FGPPI.Item->FileName, src))
						{
							Info.PanelControl(PANEL_ACTIVE,FCTL_SETSELECTION,j,(void*)true);
							free(PPI);
							break;
						}
						free(PPI);
					}
				}
				continue;
			}
			// Запрос с сообщением-ошибкой переименования
			const wchar_t *MsgItems[]={
				GetMsg(MVRenTitle),
				GetMsg(MRenameFail),
				src,
				GetMsg(MTo),
				dest,
				GetMsg(MSkip), GetMsg(MSkipAll), GetMsg(MRetry), GetMsg(MCancel)
			};
			int Ret=Info.Message( &MainGuid, &RenameFailMsgGuid, FMSG_WARNING|FMSG_ERRORTYPE, 0,
														MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 4 );
			switch (Ret)
			{
				case 2:     // Повторить
					goto RETRY_1;
				case 1:     // Пропустить все
					bSkipAll=true;
				case 0:     // Пропустить
				{
					// не переименовали - отметим
					for (int j=0; j<ItemsNumber; j++)
					{
						size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,0);
						PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
						if (PPI)
						{
							FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
							Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,&FGPPI);
							if (!FSF.LStricmp(FGPPI.Item->FileName, src))
							{
								Info.PanelControl(PANEL_ACTIVE,FCTL_SETSELECTION,j,(void*)true);
								free(PPI);
								break;
							}
							free(PPI);
						}
					}
					break;
				}
				default:    // Отменить
				{
					// не переименовали - отметим
					for ( ; Item != NULL; Item=FileList.Next(Item))
					{
						for (int j=0; j<ItemsNumber; j++)
						{
							size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,0);
							PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
							if (PPI)
							{
								FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
								Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,&FGPPI);
								if (!FSF.LStricmp(FGPPI.Item->FileName, Item->strSrcFileName.get()))
								{
									Info.PanelControl(PANEL_ACTIVE,FCTL_SETSELECTION,j,(void*)true);
									free(PPI);
									break;
								}
								free(PPI);
							}
						}
					}
					goto NEXT;
				}
			}
		}
	}
	else
	{
		Count=Undo.iCount;

		size_t n=FSF.ConvertPath(CPM_NATIVE,Undo.Dir,0,0);
		string strNativeDir;
		wchar_t *p=strNativeDir.get(n+1); //+1 для FSF.AddEndSlash()
		FSF.ConvertPath(CPM_NATIVE,Undo.Dir,p,n);
		strNativeDir.updsize();
		FSF.AddEndSlash(p);
		strNativeDir.updsize();

		for (i=Count-1; i>=0; i--)
		{
			string srcFull=strNativeDir.get();
			srcFull+=Undo.CurFileName[i];
			string destFull=strNativeDir.get();
			destFull+=Undo.OldFileName[i];

 RETRY_2:
			if (MoveFileW(srcFull.get(), destFull.get()))  { iRen++; continue; }

			if (GetLastError()==ERROR_FILE_NOT_FOUND || bSkipAll) continue;

			const wchar_t *MsgItems[]={
				GetMsg(MVRenTitle),
				GetMsg(MRenameFail),
				srcFull.get(),
				GetMsg(MTo),
				destFull.get(),
				GetMsg(MSkip), GetMsg(MSkipAll), GetMsg(MRetry), GetMsg(MCancel)
			};
			int Ret=Info.Message( &MainGuid, &RenameFailMsgGuid, FMSG_WARNING|FMSG_ERRORTYPE, 0,
														MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 4 );
			switch (Ret)
			{
				case 2:     // Повторить
					goto RETRY_2;
				case 1:     // Пропустить все
					bSkipAll=true;
				case 0:     // Пропустить
					break;
				default:    // Отменить
					goto BREAK;
			}
		}
 BREAK:
		if (i<0) i=0;
		// установим каталог
		FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),Undo.Dir,NULL,{0},NULL};
		Info.PanelControl(PANEL_ACTIVE,FCTL_SETPANELDIRECTORY,0,&dirInfo);
		// отметим файлы
		struct PanelInfo PInfo;
		PInfo.StructSize=sizeof(PanelInfo);
		Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);
		for (int k=i; k<Count; k++)
		{
			for (int j=0; j<PInfo.ItemsNumber; j++)
			{
				size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,0);
				PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
				if (PPI)
				{
					FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
					Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,j,&FGPPI);
					if (!FSF.LStricmp(FGPPI.Item->FileName, Undo.OldFileName[k]))
					{
						if (k==i) RInfo.TopPanelItem=RInfo.CurrentItem=j;
						Info.PanelControl(PANEL_ACTIVE,FCTL_SETSELECTION,j,(void*)true);
						free(PPI);
						break;
					}
					free(PPI);
				}
			}
		}
	}

 NEXT:
	// удалим из структуры лишние элементы (от старого Undo)
	for (i=Undo.iCount-1; i>iUndo-1; i--)
	{
		free(Undo.CurFileName[i]); Undo.CurFileName[i]=0;
		free(Undo.OldFileName[i]); Undo.OldFileName[i]=0;
	}

	// что-то поместили в Undo...
	if (Undo.iCount=iUndo)
	{
		// ...запомним тогда и каталог
		if (!(Undo.Dir=(wchar_t*)realloc(Undo.Dir, (strPanelDir.length()+1)*sizeof(wchar_t))) )
		{
			FreeUndo();
			ErrorMsg(MVRenTitle, MErrorCreateLog);
		}
		else
			wcscpy(Undo.Dir, strPanelDir.get());
	}

	if (iRen)
	{
		wchar_t tmp[80];
		FSF.sprintf(tmp, GetMsg(MProcessedFmt), iRen, Count);
		const wchar_t *MsgItems[]={GetMsg(MVRenTitle), tmp, GetMsg(MOK)};
		Info.Message( &MainGuid, &RenMsgGuid, 0, 0, MsgItems,
									sizeof(MsgItems) / sizeof(MsgItems[0]), 1 );
	}

	Info.PanelControl(PANEL_ACTIVE,FCTL_ENDSELECTION,0,0);
	Info.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0, Opt.Undo?&RInfo:0);
	Info.PanelControl(PANEL_ACTIVE,FCTL_UPDATEPANEL,1,0);

	return true;
}


/****************************************************************************
 * Создание файла с листом имен файлов для переименования
 ****************************************************************************/
int RenFile::CreateList(const wchar_t *TempFileName)
{
	static SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(sa));
	sa.nLength=sizeof(sa);

	HANDLE hFile=CreateFileW( TempFileName, GENERIC_WRITE,
														FILE_SHARE_READ, &sa, CREATE_ALWAYS,
														FILE_FLAG_SEQUENTIAL_SCAN, 0 );
	if (hFile==INVALID_HANDLE_VALUE)
		return -1;

	const wchar_t *MsgItems[]={ GetMsg(MVRenTitle), GetMsg(MShowOrgName), GetMsg(MShowOrgName2) };
	Opt.ShowOrgName=!Info.Message(&MainGuid, &ShowOrgNameMsgGuid, FMSG_MB_YESNO, 0, MsgItems, 3, 0);

	int width=0;
	if (Opt.ShowOrgName) width=Opt.lenFileName;
	width+=2;

	for (File *Item=FileList.First(); Item != NULL; Item=FileList.Next(Item))
	{
		string strCur;
		wchar_t *buf=strCur.get(Item->strSrcFileName.length()+Item->strDestFileName.length()+(width-Item->strSrcFileName.length())+10);
		if (Opt.ShowOrgName)
			FSF.sprintf(buf, L"\"%s\"%*c\"%s\"\n", Item->strSrcFileName.get(), width-Item->strSrcFileName.length(), L' ',
										bError?Item->strSrcFileName.get():Item->strDestFileName.get());
		else
			FSF.sprintf(buf, L"\"%s\"\n", bError?Item->strSrcFileName.get():Item->strDestFileName.get());
		strCur.updsize();
		DWORD BytesWritten;
		if (!WriteFile(hFile,(LPCVOID)buf,(DWORD)(strCur.length()*sizeof(wchar_t)), &BytesWritten, 0))
		{
			CloseHandle(hFile); return -1;
		}
	}
	CloseHandle(hFile);
	return (Opt.ShowOrgName?width+4:width);
}

/****************************************************************************
 * Чтение листа
 ****************************************************************************/
int RenFile::ReadList(const wchar_t *TempFileName)
{
	HANDLE hFile=CreateFileW( TempFileName, GENERIC_READ,
														FILE_SHARE_READ, 0, OPEN_EXISTING,
														FILE_FLAG_SEQUENTIAL_SCAN, 0 );
	if (hFile==INVALID_HANDLE_VALUE)
		return 1;
	HANDLE hMap=CreateFileMappingW(hFile, 0, PAGE_READONLY, 0, 0, 0);
	CloseHandle(hFile);
	if (!hMap)
		return 1;
	wchar_t *StrStart=0, *buf=(wchar_t *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(hMap);
	if (!buf)
		return 1;
	wchar_t *p=buf;
	int quote=0, i=0;
	while (*p)
	{
		quote=0;
		while (*p && isspace(*p)) p++;
		while (*p && *p != L'\n' && *p != L'\r')
		{
			if (*p==L'"') quote++;
			p++;
			if (Opt.ShowOrgName?quote==3:quote==1)
			{
				for (StrStart=p; *p; p++)
				{
					if (*p==L'"') break;
					if (*p==L'\n' || *p==L'\r') { UnmapViewOfFile(buf); return 2; }
				}
				if (!*p) continue;
				// найдем по индексу нужный элемент
				File *cur=NULL; unsigned index;
				for (cur=FileList.First(), index=0; cur && index<i; cur=FileList.Next(cur), index++)
					;
				i++;
				if (cur) cur->strDestFileName(StrStart, p-StrStart);
				else { UnmapViewOfFile(buf); return 1; }
				// проверим, что имя не пусто и/или не содержит недопустимых символов:
				FSF.Trim(cur->strDestFileName.get()); cur->strDestFileName.updsize();
				if (cur->strDestFileName.length()==0 || !CheckFileName(cur->strDestFileName.get()))
				{
					UnmapViewOfFile(buf); return 2;
				}
			}
		}
	}
	UnmapViewOfFile(buf);
	return 0;
}

/****************************************************************************
 * Основная ф-ция по переименованию файлов в редакторе
 ****************************************************************************/
void RenFile::RenameInEditor(int SelectedItemsNumber, int ItemsNumber)
{
	wchar_t TempFileName[MAX_PATH];
	FSF.MkTemp(TempFileName,MAX_PATH,L"FRen");
	int width;
	if ((width=CreateList(TempFileName)) <0 )
	{
		ErrorMsg(MVRenTitle, MErrorCreateList);
		return;
	}

 EDIT:
	switch (Info.Editor(TempFileName, GetMsg(MEditorTitle), 0,0,-1,-1, EF_DISABLEHISTORY, 0, width, CP_UNICODE))
	{
		case 3:       // Загрузка файла прервана пользователем
			ErrorMsg(MVRenTitle, MAborted);
			goto END;
		case 2:       // Файл не был изменен
			{
				bool ren=false;
				for (File *Item=FileList.First(); Item != NULL; Item=FileList.Next(Item))
				{
					if (Strncmp(Item->strSrcFileName.get(),Item->strDestFileName.get()))
					{
						if (YesNoMsg(MVRenTitle,MEditorRename)) ren=true;
						break;
					}
				}
				if (!ren) goto END;
				break;
			}
		case 0:       // Ошибка открытия файла
			ErrorMsg(MVRenTitle, MErrorOpenList);
			goto END;
		default:      // Файл был изменен
			break;
	}

	switch (ReadList(TempFileName))
	{
		case 2:       // Ошибки в листе имен файлов
			ErrorMsg(MVRenTitle, MErrorReadList);
			goto EDIT;
		case 1:       // Ошибка открытия файла
			ErrorMsg(MVRenTitle, MErrorOpenList);
			goto END;
		default:      // Удачно прочитали лист новых имен
			break;
	}

	RenameFile(SelectedItemsNumber, ItemsNumber);

 END:
	if (TempFileName[0]) DeleteFileW(TempFileName);
	return;
}
