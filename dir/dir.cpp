#include "..\plugin.hpp"
#include "..\fmt.hpp"

/******************************************************
 Том в устройстве D имеет метку DIST_ALX
 Серийный номер тома: 356D-B9B3

 Содержимое папки D:\!Develop!\Dir

15.11.2008  17:36    <DIR>          .
15.11.2008  17:36    <DIR>          ..
15.11.2008  17:39    <DIR>          dir010327
15.11.2008  17:46    <DIR>          dirparse
15.11.2008  18:11    <DIR>          [2008.11.15]
15.11.2008  18:36                 0 !!
               1 файлов              0 байт

 Содержимое папки D:\!Develop!\Dir\dir010327

15.11.2008  17:39    <DIR>          .
15.11.2008  17:39    <DIR>          ..
27.03.2001  08:47             7 581 dir.cpp
27.03.2001  09:03             6 144 dir.fmt
29.03.2001  18:11             2 600 dir.txt
               3 файлов         16 325 байт

     Всего файлов:
               9 файлов         26 795 байт
              11 папок     396 525 568 байт свободно
 Volume in drive C is w2k3
 Volume Serial Number is F0AC-A340

 Directory of C:\Programs\Develop\mingw32\mingw

31.10.2008  12:30    <DIR>          .
31.10.2008  12:30    <DIR>          ..
31.10.2008  12:30    <DIR>          bin
04.02.2008  04:19            35 821 COPYING-gcc-tdm.txt
09.08.2007  17:18            26 930 COPYING.lib-gcc-tdm.txt
31.10.2008  12:30    <DIR>          doc
31.10.2008  13:06    <DIR>          include
31.10.2008  12:30    <DIR>          info
31.10.2008  13:06    <DIR>          lib
28.08.2008  16:39    <DIR>          libexec
31.10.2008  12:30    <DIR>          man
31.10.2008  12:30    <DIR>          mingw32
26.05.2005  10:15            21 927 pthreads-win32-README
28.08.2008  16:58            14 042 README-gcc-tdm.txt
               4 File(s)         98 720 bytes

******************************************************/

enum {  // смещение для WIN XP/2003/VISTA:
  DATA_DISP = 0,
  TIME_DISP = 12,
  SIZE_DISP = 19,
  NAME_DISP = 36,

  ID_DIR_DISP = 21,
};

//static struct PluginStartupInfo Info;
HANDLE Handle, MapHandle;
char *Data, *Pos, *Edge, Prefix[MAX_PATH], Descrip[80];
unsigned int nStrLen, nSkipLen;
bool isBtanch;

bool Compare(char *Str, char *Mask, int Len=-1)
{
  return CompareString(LOCALE_USER_DEFAULT, 0, Str, Len==-1?lstrlen(Mask):Len, Mask, Len)==CSTR_EQUAL;
}

__int64 AtoI(char *Str, int Len)
{
  __int64 Ret=0;
  for (; Len; Len--, Str++)
    if (*Str>='0' && *Str<='9')
      (Ret*=10)+=*Str-'0';
  return Ret;
}

bool GetS(char *Buf)
{
  const char *Start=Buf;
  if (Pos>=Edge) return nStrLen=0;
  while (Pos<Edge && *Pos!='\r' && *Pos!='\n')
    *Buf++=*Pos++;
  *Buf=0;
  for (; Pos<Edge && (*Pos=='\r' || *Pos=='\n'); Pos++)
    ;
  nStrLen=Buf-Start;
  return true;
}

void CatS(char *Buf, char *Str)
{
  while (*Buf) Buf++;
  const char *Start=Buf;
  while (*Str && *Str!=':')
    *Buf++=*Str++;
  if (Buf-Start>3) *(Buf-3)=0;
}

BOOL WINAPI _export IsArchive(char *Name, const unsigned char *Data, int DataSize)
{
  static char *ID[]={ " Том в устройстве ", " Volume in drive " };
  return Compare(ID[0], (char *)Data, min(lstrlen(ID[0]), DataSize)) ||
         Compare(ID[1], (char *)Data, min(lstrlen(ID[1]), DataSize))  ;
}
/*
void WINAPI SetFarInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
}
*/
BOOL WINAPI _export OpenArchive(char *Name, int *Type)
{
  Handle=CreateFile(Name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (Handle!=INVALID_HANDLE_VALUE)
  {
    MapHandle=CreateFileMapping(Handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (MapHandle)
    {
      Pos=Data=(char *)MapViewOfFile(MapHandle, FILE_MAP_READ, 0, 0, 0);
      if (Data)
      {
        *Type=0;
        *Prefix=0;
        *Descrip=0;
        nSkipLen=0;
        Edge=Data+GetFileSize(Handle, NULL);
        //static const char *MsgItems[]={ "", "In mode branch?"};
        //isBtanch=!Info.Message(Info.ModuleNumber, FMSG_MB_YESNO, 0, MsgItems, 2, 0);
        return true;
      }
      CloseHandle(MapHandle);
    }
    CloseHandle(Handle);
  }
  return false;
}

int WINAPI _export GetArcItem(struct PluginPanelItem *Item, struct ArcItemInfo *Info)
{
  static char *LabelHeader[]={ " Том", " Volume in" };
  static char *SerialHeader[]={ " Серийный", " Volume Serial" };
  static char *DirHeader[]={ " Содержимое папки", " Directory of" };
  static char *DirEndHeader[]={ "     Всего файлов:", "     Total Files" };
  char Buf[4096];
  bool isRus=true;

  //if (isBtanch) nSkipLen=0;
  while (GetS(Buf))
  {
    if (Compare(Buf, LabelHeader[0]) || Compare(Buf, LabelHeader[1]))
    {
      isRus=Compare(Buf, LabelHeader[0]);
      lstrcpy(Descrip, "  Label:");
      if (isRus?*(Buf+20)=='и':*(Buf+19)=='i' ) lstrcat(Descrip, Buf+(isRus?31:21));
      continue;
    }
    if (Compare(Buf, SerialHeader[0]) || Compare(Buf, SerialHeader[1]))
    {
      char temp[80];
      lstrcpy(temp, Descrip);
      lstrcpy(Descrip, Buf+(isRus?21:24));
      lstrcat(Descrip, temp);
      continue;
    }
    if (Compare(Buf, DirHeader[0]) || Compare(Buf, DirHeader[1]))
    {
      if (!nSkipLen) nSkipLen=nStrLen;
      else lstrcpy(Prefix, Buf+nSkipLen+(Buf[nSkipLen]=='\\'?1:0));
      continue;
    }
    if ( (nStrLen==NAME_DISP+1 && *(Buf+NAME_DISP)=='.') ||
         (nStrLen==NAME_DISP+2 && *(Buf+NAME_DISP+1)=='.') )
      continue;
    if (Compare(Buf, DirEndHeader[0]) || Compare(Buf, DirEndHeader[1]))
    {
      nSkipLen=0; *Prefix=0;
      continue;
    }
    if (nStrLen>1 && *Buf!=' ' && *(Buf+1)!=' ')
    {
      memset(Item, 0, sizeof(PluginPanelItem));

      if (*Prefix)
      {
        lstrcpy(Item->FindData.cFileName, Prefix);
        lstrcat(Item->FindData.cFileName, "\\");
      }

      //<DIR>
      if (*(Buf+ID_DIR_DISP)=='<' && *(Buf+ID_DIR_DISP+4)=='>')
      {
        Item->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
        lstrcat(Item->FindData.cFileName, Buf+NAME_DISP);
      }
      //<JUNCTION> or <SYMLINKD>
      else if (*(Buf+ID_DIR_DISP)=='<' && *(Buf+ID_DIR_DISP+9)=='>')
      {
        Item->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
        CatS(Item->FindData.cFileName, Buf+NAME_DISP);
      }
      //<SYMLINK>
      else if (*(Buf+ID_DIR_DISP)=='<' && *(Buf+ID_DIR_DISP+8)=='>')
      {
        CatS(Item->FindData.cFileName, Buf+NAME_DISP);
      }
      else
      {
        lstrcat(Item->FindData.cFileName, Buf+NAME_DISP);
        FARINT64 nFileSize;
        nFileSize.i64=AtoI(Buf+SIZE_DISP, 16);
        Item->FindData.nFileSizeLow=nFileSize.Part.LowPart;
        Item->FindData.nFileSizeHigh=nFileSize.Part.HighPart;
      }

      SYSTEMTIME st;
      st.wDayOfWeek=st.wSecond=st.wMilliseconds=0;
      st.wDay=(WORD)AtoI(Buf+DATA_DISP, 2);
      st.wMonth=(WORD)AtoI(Buf+DATA_DISP+3, 2);
      st.wYear=(WORD)AtoI(Buf+DATA_DISP+6, 4);
      st.wHour=(WORD)AtoI(Buf+TIME_DISP, 2);
      st.wMinute=(WORD)AtoI(Buf+TIME_DISP+3, 2);
      //st.wYear+=st.wYear<50?2000:1900;
      FILETIME ft;
      SystemTimeToFileTime(&st,&ft);
      LocalFileTimeToFileTime(&ft,&Item->FindData.ftLastWriteTime);

      lstrcpy(Info->Description, Descrip);

      return GETARC_SUCCESS;
    }
  }
  return GETARC_EOF;
}

BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  UnmapViewOfFile(Data);
  CloseHandle(MapHandle);
  CloseHandle(Handle);
  return true;
}

//??эту функцию надо бы выбросить, чтобы модуль не появлялся в меню MultiArc,
//но тогда нарушается работа самого MultiArc :(
BOOL WINAPI _export GetFormatName(int Type, char *FormatName, char *DefaultExt)
{
  if (Type==0)
  {
    lstrcpy(FormatName, "DIR");
    lstrcpy(DefaultExt,"dir");
    return true;
  }
  return false;
}
