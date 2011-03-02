/****************************************************************************
 * VisRenFile_MP3.cpp
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

 /***************************************************************************
  * Используются исходники модуля "mp3" плагин "Anamorphosis"
  * WARP ItSelf <WARP_ItSelf@inbox.ru>
  ***************************************************************************/

// тэги преобразованные в строки
struct ID3TagInternal {
	int nEntryCount;             // кол-во
	wchar_t **pEntry;               // массив строк (содержимое тэга)
};

#define TAG_ENTRY_COUNT  6
// элементы
enum {
	TAG_TRACK,
	TAG_TITLE,
	TAG_ARTIST,
	TAG_ALBUM,
	TAG_YEAR,
	TAG_GENRE
};


/****************************************************************************
 ********************************   Tag v.1  ********************************
 ****************************************************************************/

#define ID3_V1_TAG_ID "TAG"
// структура
struct ID3v11TagReal {
	char cID[3];
	char cTitle[30];
	char cArtist[30];
	char cAlbum[30];
	char cYear[4];
	union {
		char cComment[30];
		struct {
			char cComment2[28];
			char cZero;
			BYTE btTrack;
		};
	};
	BYTE btGenre;
};

const char *Genres[148]={
  "Blues",       "Classic Rock",      "Country",           "Dance",            "Disco",
  "Funk",        "Grunge",            "Hip-Hop",           "Jazz",             "Metal",
  "New Age",     "Oldies",            "Other",             "Pop",              "R&B",
  "Rap",         "Reggae",            "Rock",              "Techno",           "Industrial",
  "Alternative", "Ska",               "Death Metal",       "Pranks",           "Soundtrack",
  "Euro-Techno", "Ambient",           "Trip-Hop",          "Vocal",            "Jazz & Funk",
  "Fusion",      "Trance",            "Classical",         "Instrumental",     "Acid",
  "House",       "Game",              "Sound Clip",        "Gospel",           "Noise",
  "Alt Rock",    "Bass",              "Soul",              "Punk",             "Space",
  "Meditative",  "Instrumental Pop",  "Instrumental Rock", "Ethnic",           "Gothic",
  "Darkwave",    "Techno-Industrial", "Electronic",        "Pop-Folk",         "Eurodance",
  "Dream",       "Southern Rock",     "Comedy",            "Cult",             "Gangsta Rap",
  "Top 40",      "Christian Rap",     "Pop-Funk",          "Jungle",           "Native American",
  "Cabaret",     "New Wave",          "Psychedelic",       "Rave",             "Showtunes",
  "Trailer",     "Lo-Fi",             "Tribal",            "Acid Punk",        "Acid Jazz",
  "Polka",       "Retro",             "Musical",           "Rock & Roll",      "Hard Rock",
  "Folk",        "Folk-Rock",         "National Folk",     "Swing",            "Fast-Fusion",
  "Bebob",       "Latin",             "Revival",           "Celtic",           "Bluegrass",
  "Avantgarde",  "Gothic Rock",       "Progressive Rock",  "Psychedelic Rock", "Symphonic Rock",
  "Slow Rock",   "Big Band",          "Chorus",            "Easy Listening",   "Acoustic",
  "Humour",      "Speech",            "Chanson",           "Opera",            "Chamber Music",
  "Sonata",      "Symphony",          "Booty Bass",        "Primus",           "Porn Groove",
  "Satire",      "Slow Jam",          "Club",              "Tango",            "Samba",
  "Folklore",    "Ballad",            "Power Ballad",      "Rhythmic Soul",    "Freestyle",
  "Duet",        "Punk Rock",         "Drum Solo",         "A Cappella",       "Euro-House",
  "Dance Hall",  "Goa",               "Drum & Bass",       "Club-House",       "Hardcore",
  "Terror",      "Indie",             "BritPop",           "Negerpunk",        "Polsk Punk",
  "Beat",        "Christian Gangsta Rap","Heavy Metal",    "Black Metal",      "Crossover",
  "Contemporary Christian","Christian Rock","Merengue",    "Salsa",            "Thrash Metal",
  "Anime",       "JPop",              "Synthpop"
};

/****************************************************************************
 * читаем оригинальный Tag v.1
 ****************************************************************************/
bool doReadID3v11Tag(HANDLE hFile, ID3v11TagReal *Tag, bool &NoTag)
{
	DWORD dwRead;

	if ( ((HANDLE)SetFilePointer(hFile, -128, 0, FILE_END) != INVALID_HANDLE_VALUE) &&
				ReadFile(hFile, Tag, sizeof(ID3v11TagReal), &dwRead, 0) )
	{
		NoTag = strncmp(Tag->cID, ID3_V1_TAG_ID, 3);
		return true;
	}
	return false;
}

/****************************************************************************
 * преобразовываем во внутреннюю форму
 ****************************************************************************/
ID3TagInternal *InitializeInternalTag1(ID3v11TagReal *pRealTag)
{
	ID3TagInternal *pInternalTag = new ID3TagInternal;

	pInternalTag->nEntryCount = TAG_ENTRY_COUNT;
	pInternalTag->pEntry      = (wchar_t**)malloc(TAG_ENTRY_COUNT * sizeof(wchar_t*));

	if ((pRealTag->cZero == 0) && pRealTag->btTrack)
	{
		pInternalTag->pEntry[TAG_TRACK] = (wchar_t*)malloc(4*sizeof(wchar_t));
		FSF.itoa(pRealTag->btTrack, pInternalTag->pEntry[TAG_TRACK], 10);
	}

	pInternalTag->pEntry[TAG_TITLE] = (wchar_t*)malloc(31*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP,0,pRealTag->cTitle,-1,pInternalTag->pEntry[TAG_TITLE],30);

	pInternalTag->pEntry[TAG_ARTIST] = (wchar_t*)malloc(31*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP,0,pRealTag->cArtist,-1,pInternalTag->pEntry[TAG_ARTIST],30);

	pInternalTag->pEntry[TAG_ALBUM] = (wchar_t*)malloc(31*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP,0,pRealTag->cAlbum,-1,pInternalTag->pEntry[TAG_ALBUM],30);

	pInternalTag->pEntry[TAG_YEAR] = (wchar_t*)malloc(5*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP,0,pRealTag->cYear,-1,pInternalTag->pEntry[TAG_YEAR],4);

	if (pRealTag->btGenre < 148)
	{
		int size = MultiByteToWideChar(CP_ACP,0,Genres[pRealTag->btGenre],-1,0,0);
		pInternalTag->pEntry[TAG_GENRE] = (wchar_t*)malloc(size*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,Genres[pRealTag->btGenre],-1,pInternalTag->pEntry[TAG_GENRE],size-1);
	}

	return pInternalTag;
}


#define TAG2
#ifdef TAG2
#pragma pack(push, 1)

/****************************************************************************
 ********************************  Tag v.2  *********************************
 ****************************************************************************/

#define ID3_V2_TAG_ID "ID3"

/****************************************************************************
 * читаем Tag.Header
 ****************************************************************************/
struct ID3v2TagHeader{
	char ID[3];
	WORD Version;
	char Flags;
	int  Size;
};

int DecodeHeaderSize(int X)
{
	char Size[4];
	memcpy(&Size,&X,4);
	X = Size[3]+((int)Size[2] << 7)+((int)Size[1] << 14)+((int)Size[0] << 21);
	return X+10;
}

bool doReadID3v2TagHeader(HANDLE hFile, ID3v2TagHeader *Header, bool &NoTag)
{
	DWORD dwRead;

	if ( ((HANDLE)SetFilePointer(hFile, 0, 0, FILE_BEGIN) != INVALID_HANDLE_VALUE) &&
				ReadFile(hFile, Header, sizeof(ID3v2TagHeader), &dwRead, 0) )
	{
		NoTag = true;
		if (!strncmp(Header->ID, ID3_V2_TAG_ID, 3))
		{
			Header->Size = DecodeHeaderSize(Header->Size);
			NoTag = false;
		}
		return true;
	}
	return false;
}


/****************************************************************************
 * читаем Tag.Frame
 ****************************************************************************/
struct ID3v2TagFrame {
	int  Length;
	union {
		char Filler[3];
		struct {
			char Filler2[2];
			char Encoding;
		};
	};
	union {
		char *Data;
		wchar_t *wData;
	};
};

int I4 (int X)
{
	__asm {
		mov eax,X
		rol ax,8
		rol eax,16
		rol ax,8
	}
}

bool doReadFrame(HANDLE hFile, ID3v2TagFrame *pFrame, bool bSkip)
{
	DWORD dwRead;

	if (ReadFile(hFile, pFrame, 7, &dwRead, 0))
	{
		pFrame->Length = I4(pFrame->Length);

		if (bSkip)
			return SetFilePointer(hFile, pFrame->Length-1, 0, FILE_CURRENT);
		else
		{
			pFrame->Data = (char*)malloc(pFrame->Length+1); //+1 in case of unicode

			if (ReadFile(hFile, pFrame->Data, pFrame->Length-1, &dwRead, 0))
				return true;

			return false;
		}
	}
	return false;
}

#define KNOWN_FRAMES  6
/****************************************************************************
 * преобразуем Tag.Frame
 ****************************************************************************/
enum {
	FRAME_TRACK,
	FRAME_TITLE,
	FRAME_ARTIST,
	FRAME_ALBUM,
	FRAME_YEAR,
	FRAME_GENRE
};

enum {
	FRAMEID_TRACK           = 0x4B435254,  // TRCK
	FRAMEID_TITLE           = 0x32544954,  // TIT2
	FRAMEID_ARTIST          = 0x31455054,  // TPE1
	FRAMEID_ALBUM           = 0x424C4154,  // TALB
	FRAMEID_YEAR            = 0x52455954,  // TYER
	FRAMEID_GENRE           = 0x4E4F4354   // TCON
};

struct FramToIndexTranstaltion {
	int nID;
	int nIndex;
};

const FramToIndexTranstaltion FrameTranslationTable[KNOWN_FRAMES] = {
	{FRAMEID_TRACK,  FRAME_TRACK},
	{FRAMEID_TITLE,  FRAME_TITLE},
	{FRAMEID_ARTIST, FRAME_ARTIST},
	{FRAMEID_ALBUM,  FRAME_ALBUM},
	{FRAMEID_YEAR,   FRAME_YEAR},
	{FRAMEID_GENRE,  FRAME_GENRE}
};

/****************************************************************************
 * читаем Tag v.2
 ****************************************************************************/
#define FRAME_READ false
#define FRAME_SKIP true

// структура
typedef struct {
	ID3v2TagHeader Header;
	ID3v2TagFrame Frames[KNOWN_FRAMES];
} ID3v2TagReal;


bool doReadID3v2Tag(HANDLE hFile, ID3v2TagReal *Tag, bool &NoTag)
{
	int ID;
	bool Result;
	DWORD dwRead;

	memset(Tag, 0, sizeof(ID3v2TagReal));

	if (doReadID3v2TagHeader(hFile, &Tag->Header, NoTag))
	{
		if (NoTag) return true;

		do {
			Result = true;

			if (ReadFile(hFile, &ID, 4, &dwRead, 0))
			{
				bool bFound = false;

				for (int i=0; i<KNOWN_FRAMES; i++)
				{
					if (FrameTranslationTable[i].nID == ID)
					{
						bFound = true;
						Result = doReadFrame(hFile, &Tag->Frames[FrameTranslationTable[i].nIndex], FRAME_READ);
						break;
					}
				}

				if (!bFound)
				{
					ID3v2TagFrame Frame;
					Result = doReadFrame(hFile, &Frame, FRAME_SKIP);
				}
			}

		} while ((Result) &&  (SetFilePointer(hFile, 0, 0, FILE_CURRENT) <= Tag->Header.Size));

		if (Result)
			return true;
		else
		{
			for (int i=0; i<KNOWN_FRAMES; i++)
			{
				if (Tag->Frames[i].Data)
					free(Tag->Frames[i].Data);
			}
			memset(Tag, 0, sizeof(ID3v2TagReal));
		}
	}

	return false;
}

#define ENCODING_ISO_8859_1     0
#define ENCODING_UTF_16_BOM     1
#define ENCODING_UTF_16_NO_BOM  2
#define ENCODING_UTF_8          3

void EncodeData(char *pData, DWORD dwDataSize, wchar_t **pResult, int nEncoding)
{
	wchar_t *wData = (wchar_t*)pData;
	int nCount;
	*pResult = (wchar_t*)malloc((dwDataSize+1)*sizeof(wchar_t));

	switch (nEncoding)
	{
		case ENCODING_ISO_8859_1:
			MultiByteToWideChar(CP_ACP,0,pData,-1,*pResult,dwDataSize);
			break;

		// они разные, но вот писалки глючные...
		case ENCODING_UTF_16_BOM:
		case ENCODING_UTF_16_NO_BOM:
		{
			nCount = dwDataSize/2;
			// запроверим здесь, что это за юникод
			if (wData[0] == 0xFFFE)
				for (int i=0; i<nCount; i++)
					wData[i] = MAKEWORD(HIBYTE(wData[i]), LOBYTE(wData[i]));
			lstrcpyn(*pResult, wData+1,dwDataSize-1);
			break;
		}
		case ENCODING_UTF_8:
			MultiByteToWideChar(CP_UTF8,0,pData,-1,*pResult,dwDataSize);
			break;
	}
}

void ConvertFrameData(ID3v2TagFrame *pFrame, wchar_t **lpResult, int nOffset=0)
{
	char *pData = pFrame->Data+nOffset;
	EncodeData(pData, pFrame->Length, lpResult, pFrame->Encoding);
}


/****************************************************************************
 * преобразовываем во внутреннюю форму
 ****************************************************************************/
ID3TagInternal *InitializeInternalTag2(ID3v2TagReal *pRealTag)
{
	ID3v2TagFrame *pFrame;
	ID3TagInternal *pInternalTag = new ID3TagInternal;

	pInternalTag->nEntryCount = TAG_ENTRY_COUNT;
	pInternalTag->pEntry      = (wchar_t**)malloc(TAG_ENTRY_COUNT * sizeof(wchar_t*));

	ConvertFrameData(&pRealTag->Frames[FRAME_TRACK],  &pInternalTag->pEntry[TAG_TRACK]);
	ConvertFrameData(&pRealTag->Frames[FRAME_TITLE],  &pInternalTag->pEntry[TAG_TITLE]);
	ConvertFrameData(&pRealTag->Frames[FRAME_ARTIST], &pInternalTag->pEntry[TAG_ARTIST]);
	ConvertFrameData(&pRealTag->Frames[FRAME_ALBUM],  &pInternalTag->pEntry[TAG_ALBUM]);
	ConvertFrameData(&pRealTag->Frames[FRAME_YEAR],   &pInternalTag->pEntry[TAG_YEAR]);

	if (pRealTag->Frames[FRAME_GENRE].Data)
	{
		int nCount = 0;
		if (pRealTag->Frames[FRAME_GENRE].Data[0] == '(')
		{
			while (pRealTag->Frames[FRAME_GENRE].Data[nCount] != ')') nCount++;
			nCount++;
		}
		int size = MultiByteToWideChar(CP_ACP,0,pRealTag->Frames[FRAME_GENRE].Data+nCount,-1,0,0);
		pInternalTag->pEntry[TAG_GENRE]=(wchar_t*)malloc(size*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,pRealTag->Frames[FRAME_GENRE].Data+nCount,-1,pInternalTag->pEntry[TAG_GENRE],size-1);
	}
	return pInternalTag;
}

#pragma pack(pop)
#endif


ID3TagInternal *AnalyseMP3File(const wchar_t *FileName)
{
	// оригинальные тэги
	ID3v11TagReal Tag1;
#ifdef TAG2
	ID3v2TagReal  Tag2;
#endif

	// преобразованный
	ID3TagInternal *pInternalTag=0;

	bool bTag1Found = false;
	bool bTag2Found = false;

	HANDLE hFile=CreateFileW( FileName, GENERIC_READ, FILE_SHARE_READ,
                           0, OPEN_EXISTING, 0, 0 );

	if (hFile != INVALID_HANDLE_VALUE)
	{
		bool bNoTag;

		if (doReadID3v11Tag(hFile, &Tag1, bNoTag) && !bNoTag)
			bTag1Found = true;
#ifdef TAG2
		if (doReadID3v2Tag(hFile, &Tag2, bNoTag) && !bNoTag)
			bTag2Found = true;
#endif
		CloseHandle (hFile);
	}

#ifdef TAG2
	if (bTag2Found)
		pInternalTag = InitializeInternalTag2(&Tag2);
	else
#endif
		if (bTag1Found)
			pInternalTag = InitializeInternalTag1(&Tag1);

	return pInternalTag;
}
