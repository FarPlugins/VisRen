/****************************************************************************
 * VisRen7_EDT.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/


/****************************************************************************
 * Создание файла с листом имен файлов для переименования
 ****************************************************************************/
static int CreateList(TCHAR *TempFileName)
{
  static SECURITY_ATTRIBUTES sa;
  memset(&sa, 0, sizeof(sa));
  sa.nLength=sizeof(sa);

  HANDLE hFile=CreateFile( TempFileName, GENERIC_WRITE,
                           FILE_SHARE_READ, &sa, CREATE_ALWAYS,
                           FILE_FLAG_SEQUENTIAL_SCAN, 0 );
  if (hFile==INVALID_HANDLE_VALUE) return -1;

  const TCHAR *MsgItems[]={ GetMsg(MVRenTitle), GetMsg(MShowOrgName), GetMsg(MShowOrgName2) };
  Opt.ShowOrgName=!Info.Message(Info.ModuleNumber, FMSG_MB_YESNO, 0, MsgItems, 3, 0);

  int width=0;
  if (Opt.ShowOrgName) width=Opt.lenFileName;
  width+=2;

  for (int i=0; i<sFI.iCount; i++)
  {
    TCHAR buf[MAX_PATH*3];
    TCHAR *p=sFI.ppi[i].FindData.cFileName;
    DWORD BytesWritten;
    if (Opt.ShowOrgName)
      FSF.sprintf(buf, _T("\"%s\"%*c\"%s\"\n"), p, width-lstrlen(p), _T(' '), p);
    else
      FSF.sprintf(buf, _T("\"%s\"\n"), p);
    if (!WriteFile(hFile, buf, lstrlen(buf), &BytesWritten, 0))
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
static int ReadList(TCHAR *TempFileName)
{
  HANDLE hFile=CreateFile( TempFileName, GENERIC_READ,
                           FILE_SHARE_READ, 0, OPEN_EXISTING,
                           FILE_FLAG_SEQUENTIAL_SCAN, 0 );
  if (hFile==INVALID_HANDLE_VALUE) return 1;
  HANDLE hMap=CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
  CloseHandle(hFile);
  if (!hMap) return 1;
  TCHAR *StrStart=0, *buf=(TCHAR *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
  CloseHandle(hMap);
  if (!buf) return 1;
  TCHAR *p=buf;
  int quote=0, i=0;
  while (*p)
  {
    quote=0;
    while (*p && isspace(*p)) p++;
    while (*p && *p != _T('\n') && *p != _T('\r'))
    {
      if (*p==_T('"')) quote++;
      p++;
      if (Opt.ShowOrgName?quote==3:quote==1)
      {
        for (StrStart=p; *p; p++)
        {
          if (*p==_T('"')) break;
          if (*p==_T('\n') || *p==_T('\r')) { UnmapViewOfFile(buf); return 2; }
        }
        if (!*p) continue;
        lstrcpyn(sFI.DestFileName[i++], StrStart, min(p-StrStart+1,MAX_PATH));
        // проверим, что имя не пусто и/или не содержит недопустимых символов:
        if (!*(FSF.Trim(sFI.DestFileName[i-1])) || !CheckFileName(sFI.DestFileName[i-1]))
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
static void RenameInEditor(PanelInfo *PInfo)
{
  TCHAR TempFileName[MAX_PATH];
  int  width;
  if ( !FSF.MkTemp(TempFileName, _T("FRen")) ||
       (width=CreateList(TempFileName)) <0 )
  {
    ErrorMsg(MVRenTitle, MErrorCreateList);
    return;
  }

 EDIT:
  switch (Info.Editor(TempFileName, GetMsg(MEditorTitle), 0,0,-1,-1, EF_DISABLEHISTORY, 0, width))
  {
    case 3:       // Загрузка файла прервана пользователем
      ErrorMsg(MVRenTitle, MAborted);
    case 2:       // Файл не был изменен
      goto END;
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

  RenameFile(PInfo);

 END:
  if (TempFileName[0]) DeleteFile(TempFileName);
  return;
}
