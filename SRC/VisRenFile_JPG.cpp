/****************************************************************************
 * VisRenFile_JPG.cpp
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

enum {
	isNonImage=0,
	isJPG,
	isBMP,
	isGIF,
	isPNG
};

// работа с EXIF
//--------------------------------------------------------------------------
// Program to pull the information out of various types of EXIF digital
// camera files and show it in a reasonably consistent way
//
// This module parses the very complicated exif structures.
//
// Matthias Wandel,  Dec 1999 - Dec 2002
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// JPEG markers consist of one or more 0xFF bytes, followed by a marker
// code byte (which is not an FF).  Here are the marker codes of interest
// in this program.  (See jdmarker.c for a more complete list.)
//--------------------------------------------------------------------------
#define MAX_SECTIONS 20

#define M_SOF0  0xC0            // Start Of Frame N
#define M_SOF1  0xC1            // N indicates which compression process
#define M_SOF2  0xC2            // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5            // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8            // Start Of Image (beginning of datastream)
#define M_EOI   0xD9            // End Of Image (end of datastream)
#define M_SOS   0xDA            // Start Of Scan (begins compressed data)
#define M_JFIF  0xE0            // Jfif marker
#define M_EXIF  0xE1            // Exif marker
#define M_COM   0xFE            // COMment

//--------------------------------------------------------------------------
// This structure stores Exif header image elements in a simple manner
// Used to store camera data as extracted from the various ways that it can be
// stored in an exif header
typedef struct {
	wchar_t  CameraMake[32];
	wchar_t  CameraModel[40];
	wchar_t  DateTime[20];
	DWORD Height, Width;
}ImageInfo_t;

// Storage for simplified info extracted from file.
ImageInfo_t ImageInfo;
int MotorolaOrder=0;

//--------------------------------------------------------------------------
// Describes tag values
#define TAG_EXIF_OFFSET       0x8769
#define TAG_INTEROP_OFFSET    0xa005
#define TAG_MAKE              0x010F
#define TAG_MODEL             0x0110
#define TAG_DATETIME_ORIGINAL 0x9003

//--------------------------------------------------------------------------
// Get 16 bits motorola order (always) for jpeg header stuff.
//--------------------------------------------------------------------------
int Get16m(const void * Short)
{
	return (((UCHAR *)Short)[0] << 8) | ((UCHAR *)Short)[1];
}

//--------------------------------------------------------------------------
// Convert a 16 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
int Get16u(void * Short)
{
	if (MotorolaOrder)
		return (((UCHAR *)Short)[0] << 8) | ((UCHAR *)Short)[1];
	else
		return (((UCHAR *)Short)[1] << 8) | ((UCHAR *)Short)[0];
}

//--------------------------------------------------------------------------
// Convert a 32 bit signed value from file's native byte order
//--------------------------------------------------------------------------
int Get32s(void * Long)
{
	if (MotorolaOrder)
		return  ((( char *)Long)[0] << 24) | (((UCHAR *)Long)[1] << 16)
						| (((UCHAR *)Long)[2] << 8 ) | (((UCHAR *)Long)[3] << 0 );
	else
		return  ((( char *)Long)[3] << 24) | (((UCHAR *)Long)[2] << 16)
						| (((UCHAR *)Long)[1] << 8 ) | (((UCHAR *)Long)[0] << 0 );
}

//--------------------------------------------------------------------------
// Convert a 32 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
unsigned Get32u(void * Long)
{
	return (unsigned)Get32s(Long) & 0xffffffff;
}

int FileGetC(HANDLE infile)
{
	unsigned char read;
	DWORD Readed;
	if(!ReadFile(infile,&read,sizeof(read),&Readed,NULL)) return -1;
	else return read;
}

//--------------------------------------------------------------------------
// Process one of the nested EXIF directories.
//--------------------------------------------------------------------------
void ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, unsigned ExifLength, int NestingLevel)
{
#define DIR_ENTRY_ADDR(Start, Entry) (Start+2+12*(Entry))
#define NUM_FORMATS 12
	if (NestingLevel>4) return;

	int BytesPerFormat[]={0,1,1,2,4,8,1,1,2,4,8,4,8};
	int NumDirEntries = Get16u(DirStart);
	unsigned char *DirEnd = DIR_ENTRY_ADDR(DirStart, NumDirEntries);
	if (DirEnd+4 > (OffsetBase+ExifLength))
	{
		if (DirEnd+2 == OffsetBase+ExifLength || DirEnd == OffsetBase+ExifLength) ;
		else return;
	}

	for (int de=0; de<NumDirEntries; de++)
	{
		unsigned char *DirEntry = DIR_ENTRY_ADDR(DirStart, de);
		int Tag = Get16u(DirEntry);
		int Format = Get16u(DirEntry+2);
		int Components = Get32u(DirEntry+4);

		if ((Format-1) >= NUM_FORMATS) continue;
		int ByteCount = Components * BytesPerFormat[Format];
		unsigned char *ValuePtr;
		if (ByteCount>4)
		{
			unsigned OffsetVal = Get32u(DirEntry+8);
			// If its bigger than 4 bytes, the dir entry contains an offset.
			if (OffsetVal+ByteCount > ExifLength) continue;
			ValuePtr = OffsetBase+OffsetVal;
		}
		else
		{
			// 4 bytes or less and value is in the dir entry itself
			ValuePtr = DirEntry+8;
		}

		// Extract useful components of tag
		switch (Tag)
		{
			case TAG_MAKE:
				MultiByteToWideChar(CP_ACP,0,(LPCSTR)ValuePtr,-1,ImageInfo.CameraMake,32);
				break;
			case TAG_MODEL:
				MultiByteToWideChar(CP_ACP,0,(LPCSTR)ValuePtr,-1,ImageInfo.CameraModel,40);
				break;
			case TAG_DATETIME_ORIGINAL:
			{
				char DateTime[20];
				for (int i=0; i<19; i++)
				{
					if (ValuePtr[i]==':')
					{
						if (i<10) DateTime[i]='.';
						else DateTime[i]='-';
					}
					else DateTime[i]=ValuePtr[i];
				}
				DateTime[19]='\0';
				MultiByteToWideChar(CP_ACP,0,DateTime,-1,ImageInfo.DateTime,20);
				break;
			}
			case TAG_EXIF_OFFSET:
			case TAG_INTEROP_OFFSET:
			{
				unsigned char *SubdirStart = OffsetBase + Get32u(ValuePtr);
				if (SubdirStart < OffsetBase || SubdirStart > OffsetBase+ExifLength) ;
				else
					ProcessExifDir(SubdirStart, OffsetBase, ExifLength, NestingLevel+1);
				continue;
			}
		}
	}

	// In addition to linking to subdirectories via exif tags,
	// there's also a potential link to another directory at the end of each
	// directory.  this has got to be the result of a comitee!
	if (DIR_ENTRY_ADDR(DirStart, NumDirEntries) + 4 <= OffsetBase+ExifLength)
	{
		unsigned Offset = Get32u(DirStart+2+12*NumDirEntries);
		if (Offset)
		{
			unsigned char *SubdirStart = OffsetBase + Offset;
			if (SubdirStart <= OffsetBase+ExifLength)
				ProcessExifDir(SubdirStart, OffsetBase, ExifLength, NestingLevel+1);
		}
	}
}

//--------------------------------------------------------------------------
// Process a EXIF marker
// Describes all the drivel that most digital cameras include...
//--------------------------------------------------------------------------
void process_EXIF(unsigned char * ExifSection, unsigned int length)
{
	// Check the EXIF header component
	static UCHAR ExifHeader[] = "Exif\0\0";
	if (memcmp(ExifSection+2, ExifHeader,6)) return;

	if (memcmp(ExifSection+8,"II",2) == 0) MotorolaOrder=0;
	else if (memcmp(ExifSection+8,"MM",2) == 0) MotorolaOrder=1;
	else return;

	// Check the next value for correctness.
	if (Get16u(ExifSection+10) != 0x2a) return;

	int FirstOffset = Get32u(ExifSection+12);

	// First directory starts 16 bytes in.  All offset are relative to 8 bytes in.
	ProcessExifDir(ExifSection+8+FirstOffset, ExifSection+8, length-6, 0);
}

//--------------------------------------------------------------------------
// Parse the marker stream until SOS or EOI is seen;
//--------------------------------------------------------------------------
bool ReadJpegSections(HANDLE infile)
{
	DWORD Readed;
	int a=FileGetC(infile);
	if (a != 0xff || FileGetC(infile) != M_SOI) return false;
	bool ret=true;

	for (int SectionsRead=0; SectionsRead<20; SectionsRead++)
	{
		int itemlen;
		int marker=0;
		int ll,lh, got;

		for (a=0; a<7; a++)
		{
			marker=FileGetC(infile);
			if (marker != 0xff) break;
			if (a >= 6) return false;
		}
		if (marker == 0xff) return false;

		// Read the length of the section.
		lh = FileGetC(infile);
		ll = FileGetC(infile);
		itemlen = (lh << 8) | ll;
		if (itemlen < 2) return false; //!!!

		UCHAR *Data = (UCHAR *)malloc(itemlen);
		if (!Data) return false;
		// Store first two pre-read bytes.
		Data[0] = (UCHAR)lh;
		Data[1] = (UCHAR)ll;

		// Read the whole section.
		if (!ReadFile(infile,Data+2,itemlen-2,&Readed,NULL)) got=0;
		else got=Readed;
		if (got != itemlen-2)
		{
			if (Data) free(Data);
			return true; //!!!
		}

		switch (marker)
		{
			case M_EOI:   // in case it's a tables-only JPEG stream
				if (Data) free(Data);
				return false;
			case M_EXIF:
				// Seen files from some 'U-lead' software with Vivitar scanner
				// that uses marker 31 for non exif stuff.  Thus make sure
				// it says 'Exif' in the section before treating it as exif.
				if (memcmp(Data+2, "Exif", 4) == 0)
					process_EXIF((UCHAR *)Data, itemlen);
				break;
			case M_SOF0:
			case M_SOF1:
			case M_SOF2:
			case M_SOF3:
			case M_SOF5:
			case M_SOF6:
			case M_SOF7:
			case M_SOF9:
			case M_SOF10:
			case M_SOF11:
			case M_SOF13:
			case M_SOF14:
			case M_SOF15:
				if (ret)
				{
					ImageInfo.Height = Get16m(Data+3);
					ImageInfo.Width  = Get16m(Data+5);
					ret=false;
				}
			default:
				// Skip any other sections.
				break;
		}
		if (Data) free(Data);
	}

	return true;
}

//--------------------------------------------------------------------------
// Read image data.
//--------------------------------------------------------------------------
bool AnalyseImageFile(const wchar_t *FileName, int ImageFormat)
{
	memset(&ImageInfo, 0, sizeof(ImageInfo));
	HANDLE infile=CreateFileW(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (infile==INVALID_HANDLE_VALUE) return false;
	int ret=0;
	DWORD Readed=0;

	// Scan the JPEG headers.
	if (ImageFormat==isJPG)
		ret=ReadJpegSections(infile);

	// !!NEW!! Scan the BMP :-)
	else if (ImageFormat==isBMP)
	{
		ret= (    FileGetC(infile) == 0x42
					 && FileGetC(infile) == 0x4D
					 && ((HANDLE)SetFilePointer(infile, 18, 0, FILE_BEGIN) != INVALID_HANDLE_VALUE)
					 && ReadFile(infile, &(ImageInfo.Width), sizeof(DWORD), &Readed, 0)
					 && ReadFile(infile, &(ImageInfo.Height), sizeof(DWORD), &Readed, 0)
				 );
	}

	// !!NEW!! Scan the GIF :-)
	else if (ImageFormat==isGIF)
	{
		ret= (    FileGetC(infile) == 0x47
					 && FileGetC(infile) == 0x49
					 && FileGetC(infile) == 0x46
					 && FileGetC(infile) == 0x38
					 && ((Readed=FileGetC(infile)) == 0x39 || Readed==0x37)
					 && FileGetC(infile) == 0x61
					 && ReadFile(infile, &(ImageInfo.Width), sizeof(WORD), &Readed, 0)
					 && ReadFile(infile, &(ImageInfo.Height), sizeof(WORD), &Readed, 0)
				 );
	}

	// !!NEW!! Scan the PNG :-)
	else if (ImageFormat==isPNG)
	{
		if ((ret=
					(   FileGetC(infile) == 0x89
					 && FileGetC(infile) == 0x50
					 && FileGetC(infile) == 0x4E
					 && FileGetC(infile) == 0x47
					 && FileGetC(infile) == 0x0D
					 && FileGetC(infile) == 0x0A
					 && FileGetC(infile) == 0x1A
					 && FileGetC(infile) == 0x0A
					 && ((HANDLE)SetFilePointer(infile, 18, 0, FILE_BEGIN) != INVALID_HANDLE_VALUE)
					 && ReadFile(infile, &(ImageInfo.Width), sizeof(WORD), &Readed, 0)
					 && ((HANDLE)SetFilePointer(infile, 22, 0, FILE_BEGIN) != INVALID_HANDLE_VALUE)
					 && ReadFile(infile, &(ImageInfo.Height), sizeof(WORD), &Readed, 0)
					)
			 ))
			{
				ImageInfo.Width=Get16m(&ImageInfo.Width);
				ImageInfo.Height=Get16m(&ImageInfo.Height);
			}
	}

	CloseHandle(infile);
	return (ret && ImageInfo.Width && ImageInfo.Height);
}
