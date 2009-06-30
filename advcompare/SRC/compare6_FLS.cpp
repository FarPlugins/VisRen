/****************************************************************************
 * compare6_FLS.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/


/****************************************************************************
 ************************** CompareFiles functions **************************
 ****************************************************************************/

/****************************************************************************
 * Строит полное имя файла из пути и имени
 ****************************************************************************/
static char *BuildFullFilename(const char *cpDir, const char *cpFileName)
{
  static char cName[NM];
  if (!strpbrk(cpFileName, ":\\/") || bAPanelWithCRC)  //если не с Темп-панели
  {
    FSF.AddEndSlash(lstrcpy(cName, cpDir));
    return lstrcat(cName, cpFileName);
  }
  else
    return lstrcpy(cName, cpFileName);
}

/****************************************************************************
 * Замена сервисной функции Info.GetDirList(). В отличие от оной возвращает
 * список файлов только в каталоге Dir, без подкаталогов.
 ****************************************************************************/
static int GetDirList(const char *Dir, struct PluginPanelItem **pPanelItem, int *pItemsNumber)
{
  *pPanelItem = 0;
  *pItemsNumber = 0;
  char cPathMask[MAX_PATH];
  WIN32_FIND_DATA wfdFindData;
  HANDLE hFind;
  if ((hFind = FindFirstFile(lstrcat(lstrcpy(cPathMask, Dir), "\\*"), &wfdFindData)) == INVALID_HANDLE_VALUE)
    return true;

  bool bRet = true;
  do
  {
    if (!lstrcmp(wfdFindData.cFileName, ".") || !lstrcmp(wfdFindData.cFileName, ".."))
      continue;
    if ((wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !Opt.ProcessHidden)
      continue;
    struct PluginPanelItem *pPPI;
    if (!(pPPI = (struct PluginPanelItem *)realloc(*pPanelItem, (*pItemsNumber + 1) * sizeof(*pPPI))))
    {
      free(*pPanelItem);
      *pItemsNumber = 0;
      bRet = false;
      break;
    }
    *pPanelItem = pPPI;
    WFD2FFD(wfdFindData,(*pPanelItem)[(*pItemsNumber)++].FindData);
  } while (FindNextFile(hFind, &wfdFindData));
  FindClose(hFind);
  return bRet;
}

/****************************************************************************
 * Замена сервисной функции Info.FreeDirList().
 ****************************************************************************/
static void FreeDirList(struct PluginPanelItem *PanelItem)
{
  if (PanelItem)
    free(PanelItem);
}

/****************************************************************************
 * Формирование DWORD64
 ****************************************************************************/
inline DWORD64 BuildDWORD64(DWORD High, DWORD Low)
{
  return ((DWORD64)High << 32)|(DWORD64)Low;
}

/****************************************************************************
 * Проверка на Esc. Возвращает true, если пользователь нажал Esc
 ****************************************************************************/
static bool CheckForEsc(void)
{
  if (hConInp == INVALID_HANDLE_VALUE)
    return false;

  static DWORD dwTicks;
  DWORD dwNewTicks = GetTickCount();
  if (dwNewTicks - dwTicks < 500)
    return false;
  dwTicks = dwNewTicks;

  INPUT_RECORD rec;
  DWORD ReadCount;
  while (PeekConsoleInput(hConInp, &rec, 1, &ReadCount) && ReadCount)
  {
    ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
    if ( rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
         rec.Event.KeyEvent.bKeyDown )
    {
      // Опциональное подтверждение прерывания по Esc
      if (Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, 0) & FCS_INTERRUPTOPERATION)
      {
        if (YesNoMsg(MEscTitle, MEscBody))
          return bBrokenByEsc = true;
      }
      else
        return bBrokenByEsc = true;
    }
  }

  return false;
}

/****************************************************************************
 * Новая строка? и/или пробельный символ?
 * для нужд Opt.IgnoreWhitespace и Opt.IgnoreNewLines
 ****************************************************************************/
inline bool IsNewLine(int c) {return (c == '\r' || c == '\n');}
inline bool myIsSpace(int c) {return (c == ' ' || c == '\t' || c == '\v' || c == '\f');}
inline bool IsWhiteSpace(int c) {return (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == '\n');}

/****************************************************************************
 * Перемещение указателя в файле для нужд Opt.ContentsPercent
 ****************************************************************************/
static bool mySetFilePointer(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
  bool bSet = true;
  LARGE_INTEGER li;
  li.QuadPart = distance;
  li.LowPart = SetFilePointer(hf, li.LowPart, &li.HighPart, MoveMethod);
  if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
    bSet = false;
  return bSet;
}

/****************************************************************************
 * CRC32 со стандартным полиномом 0xEDB88320.
 ****************************************************************************/
static DWORD ProcessCRC(void *pData, register int iLen, DWORD FileCRC)
{
  register unsigned char *pdata = (unsigned char *)pData;
  register DWORD crc = FileCRC;
  static unsigned TableCRC[256];
  if (!TableCRC[1])
  { // Инициализация CRC32 таблицы
    unsigned i, j, r;
    for (i = 0; i < 256; i++)
    {
      for (r = i, j = 8; j; j--)
        r = r & 1 ? (r >> 1) ^ 0xEDB88320 : r >> 1;
      TableCRC[i] = r;
    }
  }
  while (iLen--)
  {
    crc = TableCRC[(unsigned char)crc ^ *pdata++] ^ crc >> 8;
    crc ^= 0xD202EF8D;
  }
  return crc;
}

/****************************************************************************
 * Результат предыдущего сравнения "по содержимому".
 ****************************************************************************/
static int GetResult( struct ResultCmpContents *pRCC, DWORD FileName1, DWORD FileName2,
                      DWORD64 WriteTime1, DWORD64 WriteTime2 )
{
  DWORD dwResult = 0;
  for (int i=0; i<pRCC->iCount; i++)
  {
    if ( ((FileName1 == pRCC->rci[i].dwFileName[0] && FileName2 == pRCC->rci[i].dwFileName[1]) &&
         (WriteTime1 == pRCC->rci[i].dwWriteTime[0] && WriteTime2 == pRCC->rci[i].dwWriteTime[1]))
       || ((FileName1 == pRCC->rci[i].dwFileName[1] && FileName2 == pRCC->rci[i].dwFileName[0]) &&
          (WriteTime1 == pRCC->rci[i].dwWriteTime[1] && WriteTime2 == pRCC->rci[i].dwWriteTime[0])) )
    {
      dwResult = pRCC->rci[i].dwFlags;

      if (dwResult & Flag0)
      {
        if (dwResult & Flag1) return 1;        // true
        else return -1;                        // false
      }
      break;
    }
  }
  return 0;  // 0 - результат не определен, т.к. элемент не найден
}

/****************************************************************************
 * Сохранение результата сравнения "по содержимому".
 ****************************************************************************/
static bool SetResult( struct ResultCmpContents *pRCC, DWORD FileName1, DWORD FileName2,
                       DWORD64 WriteTime1, DWORD64 WriteTime2,
                       DWORD dwFlag0, DWORD dwFlag1, bool bDisable )
{
  bool bFoundItem = false;
  for (int i=0; i<pRCC->iCount; i++)
  {
    if ( ((FileName1 == pRCC->rci[i].dwFileName[0] && FileName2 == pRCC->rci[i].dwFileName[1]) &&
         (WriteTime1 == pRCC->rci[i].dwWriteTime[0] && WriteTime2 == pRCC->rci[i].dwWriteTime[1]))
       || ((FileName1 == pRCC->rci[i].dwFileName[1] && FileName2 == pRCC->rci[i].dwFileName[0]) &&
          (WriteTime1 == pRCC->rci[i].dwWriteTime[1] && WriteTime2 == pRCC->rci[i].dwWriteTime[0])) )
    {
      bFoundItem = true;
      pRCC->rci[i].dwFlags |= dwFlag1;
      if (bDisable) pRCC->rci[i].dwFlags &= ~dwFlag0;
      break;
    }
  }

  if (!bFoundItem)
  {
    if ( struct ResultCmpItem *NewItem =
         (struct ResultCmpItem *)realloc(pRCC->rci, (pRCC->iCount + 1) * sizeof(*NewItem)) )
    {
      struct ResultCmpItem *CurItem;
      pRCC->rci             = NewItem;
      CurItem = &pRCC->rci[pRCC->iCount++];
      CurItem->dwFileName[0]  = FileName1;
      CurItem->dwFileName[1]  = FileName2;
      CurItem->dwWriteTime[0] = WriteTime1;
      CurItem->dwWriteTime[1] = WriteTime2;
      CurItem->dwFlags     |= dwFlag1;
      if (bDisable) CurItem->dwFlags &= ~dwFlag0;
      return true;
    }
    else
    {
      ErrorMsg(MNoMemTitle, MNoMemBody);
      free(pRCC->rci); pRCC->rci = 0; pRCC->iCount = 0;
      return false;
    }
  }
  return true;
}

static bool CompareDirs(const struct PanelInfo *AInfo, const struct PanelInfo *PInfo, bool bCompareAll, int ScanDepth);
/****************************************************************************
 * Сравнение атрибутов и прочего для двух одноимённых элементов (файлов или
 * подкаталогов).
 * Возвращает true, если они совпадают.
 ****************************************************************************/
static bool CompareFiles( const FAR_FIND_DATA *AData, const FAR_FIND_DATA *PData,
                          const char *ACurDir, const char *PCurDir, int ScanDepth,
                          const DWORD ACRC32, const DWORD PCRC32 )
{
  if (AData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    // Здесь сравниваем два подкаталога
    if (Opt.ProcessSubfolders)
    {
      if (Opt.ProcessSubfolders==2 && Opt.MaxScanDepth<ScanDepth+1)
        return true;
      if (bAPanelWithCRC && !bPPanelWithCRC && ScanDepth>0)
        return true;

      // Составим списки файлов в подкаталогах
      struct PanelInfo AInfo, PInfo;
      memset(&AInfo, 0, sizeof(AInfo));
      memset(&PInfo, 0, sizeof(PInfo));

      char cpDirA[MAX_PATH], cpDirP[MAX_PATH];
      HANDLE hDirA;
      lstrcpy(cpDirA, BuildFullFilename(ACurDir, AData->cFileName));
      lstrcpy(cpDirP, BuildFullFilename(PCurDir, PData->cFileName));
      if ( bPlatformNT && !bAPanelWithCRC && !bPPanelWithCRC &&
           (hDirA = CreateFile(cpDirA, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE )
      {
        lstrcpy(cpDirA, BuildFullFilename(ACurDir, AData->cAlternateFileName));
        lstrcpy(cpDirP, BuildFullFilename(PCurDir, PData->cAlternateFileName));
      }
      if (hDirA) CloseHandle(hDirA);

      bool bEqual = true;
      if (bAPanelWithCRC && !bPPanelWithCRC)
      {
        lstrcpy(AInfo.CurDir, cpDirA);
        if (!Info.GetPluginDirList(1, INVALID_HANDLE_VALUE, AData->cFileName, &AInfo.PanelItems, &AInfo.ItemsNumber)
            || !Info.GetDirList(lstrcpy(PInfo.CurDir, cpDirP), &PInfo.PanelItems, &PInfo.ItemsNumber))
        {
          bBrokenByEsc = true; // То ли юзер прервал, то ли ошибка чтения
          bEqual = false; // Остановим сравнение
        }
      }
      else
      {
        if (!GetDirList(lstrcpy(AInfo.CurDir, cpDirA), &AInfo.PanelItems, &AInfo.ItemsNumber)
            || !GetDirList(lstrcpy(PInfo.CurDir, cpDirP), &PInfo.PanelItems, &PInfo.ItemsNumber))
        {
          bBrokenByEsc = true; // То ли юзер прервал, то ли ошибка чтения
          bEqual = false; // Остановим сравнение
        }
      }
      if (bEqual)
        bEqual = CompareDirs(&AInfo, &PInfo, Opt.Panel, ScanDepth+1);  // Opt.Panel==1 то всё сравним в подкаталоге

      if (bAPanelWithCRC && !bPPanelWithCRC)
      {
        Info.FreeDirList(AInfo.PanelItems);
        Info.FreeDirList(PInfo.PanelItems);
      }
      else
      {
        FreeDirList(AInfo.PanelItems);
        FreeDirList(PInfo.PanelItems);
      }
      return bEqual;
    }
  }
  else
  // Здесь сравниваем два файла
  {
    DWORD64 nFileSizeA = BuildDWORD64(AData->nFileSizeHigh, AData->nFileSizeLow),
            nFileSizeP = BuildDWORD64(PData->nFileSizeHigh, PData->nFileSizeLow),
            nFileTimeA = BuildDWORD64(AData->ftLastWriteTime.dwHighDateTime,AData->ftLastWriteTime.dwLowDateTime),
            nFileTimeP = BuildDWORD64(PData->ftLastWriteTime.dwHighDateTime,PData->ftLastWriteTime.dwLowDateTime);
    //===========================================================================
    // регистр имен
    if ( Opt.CompareName && !Opt.IgnoreCase &&
        (lstrcmp( bAPanelTmp ? FSF.PointToName(AData->cFileName) : AData->cFileName,
                  bPPanelTmp ? FSF.PointToName(PData->cFileName) : PData->cFileName )) )
    {
      return false;
    }
    //===========================================================================
    // размер
    if (Opt.CompareSize && (nFileSizeA != nFileSizeP))
    {
      return false;
    }
    //===========================================================================
    // время
    if (Opt.CompareTime)
    {
      if (Opt.LowPrecisionTime || Opt.IgnoreTimeZone)
      {
        union {
          __int64 num;
          struct {
            DWORD lo;
            DWORD hi;
          } hilo;
        } Precision, Difference, TimeDelta, temp;

        Precision.hilo.hi = 0;
        Precision.hilo.lo = Opt.LowPrecisionTime ? 20000000 : 0; //2s or 0s
        Difference.num = _i64(9000000000); //15m

        if (AData->ftLastWriteTime.dwHighDateTime > PData->ftLastWriteTime.dwHighDateTime)
        {
          TimeDelta.hilo.hi = AData->ftLastWriteTime.dwHighDateTime - PData->ftLastWriteTime.dwHighDateTime;
          TimeDelta.hilo.lo = AData->ftLastWriteTime.dwLowDateTime - PData->ftLastWriteTime.dwLowDateTime;
          if (TimeDelta.hilo.lo > AData->ftLastWriteTime.dwLowDateTime)
            --TimeDelta.hilo.hi;
        }
        else
        {
          if (AData->ftLastWriteTime.dwHighDateTime == PData->ftLastWriteTime.dwHighDateTime)
          {
            TimeDelta.hilo.hi = 0;
            TimeDelta.hilo.lo = max(PData->ftLastWriteTime.dwLowDateTime,AData->ftLastWriteTime.dwLowDateTime)-
                                min(PData->ftLastWriteTime.dwLowDateTime,AData->ftLastWriteTime.dwLowDateTime);
          }
          else
          {
            TimeDelta.hilo.hi = PData->ftLastWriteTime.dwHighDateTime - AData->ftLastWriteTime.dwHighDateTime;
            TimeDelta.hilo.lo = PData->ftLastWriteTime.dwLowDateTime - AData->ftLastWriteTime.dwLowDateTime;
            if (TimeDelta.hilo.lo > PData->ftLastWriteTime.dwLowDateTime)
              --TimeDelta.hilo.hi;
          }
        }

        //игнорировать различия не больше чем 26 часов.
        if (Opt.IgnoreTimeZone)
        {
          int counter = 0;
          while (TimeDelta.hilo.hi > Difference.hilo.hi && counter<=26*4)
          {
            temp.hilo.lo = TimeDelta.hilo.lo - Difference.hilo.lo;
            temp.hilo.hi = TimeDelta.hilo.hi - Difference.hilo.hi;
            if (temp.hilo.lo > TimeDelta.hilo.lo)
              --temp.hilo.hi;
            TimeDelta.hilo.lo = temp.hilo.lo;
            TimeDelta.hilo.hi = temp.hilo.hi;
            ++counter;
          }
          if (counter<=26*4 && TimeDelta.hilo.hi == Difference.hilo.hi)
          {
            TimeDelta.hilo.hi = 0;
            TimeDelta.hilo.lo = max(TimeDelta.hilo.lo,Difference.hilo.lo) - min(TimeDelta.hilo.lo,Difference.hilo.lo);
          }
        }

        if ( Precision.hilo.hi < TimeDelta.hilo.hi ||
            (Precision.hilo.hi == TimeDelta.hilo.hi && Precision.hilo.lo < TimeDelta.hilo.lo))
        {
          return false;
        }
      }
      else if (nFileTimeA != nFileTimeP)
      {
        return false;
      }
    }
    //===========================================================================
    // содержимое
    if (Opt.CompareContents)
    {
      // сравним размер файлов
      if (!Opt.Filter && !Opt.CompareSize && (nFileSizeA != nFileSizeP))
      {
        return false;
      }

      // сравним 2-е архивные панели
      if (bAPanelWithCRC && bPPanelWithCRC)
      {
        if (ACRC32 == PCRC32) return true;
        else return false;
      }

      char cpFileA[MAX_PATH], cpFileP[MAX_PATH];
      DWORD FileNameA, FileNameP;

      if (!bAPanelWithCRC)
      {
        lstrcpy(cpFileA, BuildFullFilename(ACurDir, AData->cFileName));
        FileNameA = ProcessCRC((void *)cpFileA, lstrlen(cpFileA), 0);
      }
      if (!bPPanelWithCRC)
      {
        lstrcpy(cpFileP, BuildFullFilename(PCurDir, PData->cFileName));
        FileNameP = ProcessCRC((void *)cpFileP, lstrlen(cpFileP), 0);
      }
      // Используем кэшированные данные
      if (Opt.Cache && Opt.UseCacheResult && !(bAPanelWithCRC || bPPanelWithCRC))
      {
        int Result = GetResult(&CacheResult, FileNameA, FileNameP, nFileTimeA, nFileTimeP);
        if (Result == 1) return true;
        else if (Result == -1) return false;
      }

      HANDLE hFileA, hFileP;
      FILETIME AAccess, PAccess;

      if (!bAPanelWithCRC)
      {
        if ((hFileA = CreateFile(cpFileA, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                 OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
        {
          // Попробуем тогда через короткое имя
          lstrcpy(cpFileA, BuildFullFilename(ACurDir, AData->cAlternateFileName));
          if ((hFileA = CreateFile(cpFileA, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                   OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
          {
            return false;
          }
        }
        // Сохраним время последнего доступа к файлу
        AAccess = AData->ftLastAccessTime;
      }

      if (!bPPanelWithCRC)
      {
        if ((hFileP = CreateFile(cpFileP, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                 OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
        {
          lstrcpy(cpFileP, BuildFullFilename(PCurDir, PData->cAlternateFileName));
          if ((hFileP = CreateFile(cpFileP, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                   OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
          {
            if (hFileA) CloseHandle(hFileA);
            return false;
          }
        }
        PAccess = PData->ftLastAccessTime;
      }

    //---------------------------------------------------------------------------

      ShowMessage(ACurDir, AData->cFileName, PCurDir, PData->cFileName, false);

      Work.n2Size = (nFileSizeA + nFileSizeP), Work.n2SizeProcess = 0;
      DWORD ReadSizeA = 1, ReadSizeP = 1;
      DWORD BufPosA   = 1, BufPosP   = 1;
      const DWORD ReadBlock = 65536;
      bool bEqual     = true;

      // если больше 0,5 Гб, то узнаем выставленный процент сравнения содержимого
      if (!Opt.Filter && nFileSizeA >= _i64(512000000) && !(bAPanelWithCRC || bPPanelWithCRC))
      {
        DWORD64 nUpToSize;
        __int64 FilePointer, FilePointerStart;
        int BlockStart, Block = 160;
        DWORD bufSizeStart;
        bool bFromEnd = false;
        bool bStart = false;

        switch (Opt.ContentsPercent)
        {
          case 1:  // 75%
            nUpToSize = (nFileSizeA >> 1) + (nFileSizeA >> 2);
            if (nUpToSize < Opt.BufSize)
            {
              bFromEnd = true;
              FilePointer = nUpToSize;
            }
            else
            {
              bStart = true;
              FilePointer = (Opt.BufSize * Block) >> 2;
              FilePointerStart = nUpToSize % (Opt.BufSize * Block) >> 2;
              bufSizeStart = nUpToSize % (Opt.BufSize * Block) % Opt.BufSize;
              BlockStart = (nUpToSize % (Opt.BufSize * Block) - bufSizeStart) / Opt.BufSize;
            }
            break;
          case 2:  // 50%
            nUpToSize = nFileSizeA >> 1;
            if (nUpToSize < Opt.BufSize)
            {
              bFromEnd = true;
              FilePointer = nUpToSize;
            }
            else
            {
              bStart = true;
              FilePointer = Opt.BufSize * Block;
              FilePointerStart = nUpToSize % (Opt.BufSize * Block);
              bufSizeStart = nUpToSize % (Opt.BufSize * Block) % Opt.BufSize;
              BlockStart = (nUpToSize % (Opt.BufSize * Block) - bufSizeStart) / Opt.BufSize;
            }
            break;
          case 3:  // 25%
            nUpToSize = nFileSizeA >> 2;
            if (nUpToSize < Opt.BufSize)
            {
              bFromEnd = true;
              FilePointer = nUpToSize;
            }
            else
            {
              bStart = true;
              FilePointer =  Opt.BufSize * Block * 3;
              FilePointerStart = nUpToSize % (Opt.BufSize * Block) * 3;
              bufSizeStart = nUpToSize % (Opt.BufSize * Block) % Opt.BufSize;
              BlockStart = (nUpToSize % (Opt.BufSize * Block) - bufSizeStart) / Opt.BufSize;
            }
            break;
          default: // 100%
            goto UpToSizeFull;
            break;
        }

        // сравниваем до указанного процента
        // читать будем блоками по Block
        if (bFromEnd)
        {
          if ( !mySetFilePointer(hFileA, -FilePointer, FILE_END) ||
               !mySetFilePointer(hFileP, -FilePointer, FILE_END) )
            bEqual = false;
        }

        while (1)
        {
          DWORD myReadBlock = ReadBlock;
          DWORD myBufSize = Opt.BufSize;
          BufPosA = 0; BufPosP = 0;
          if (bStart)
          {
            myReadBlock = min(ReadBlock, bufSizeStart);
            myBufSize = min(Opt.BufSize, bufSizeStart);
          }

          while (BufPosA < myBufSize)
          {
            if (CheckForEsc() || !ReadFile(hFileA, ABuf+BufPosA, myReadBlock, &ReadSizeA, 0))
            {
               bEqual = false;
               break;
            }
            BufPosA += ReadSizeA;
            Work.n2SizeProcess += ReadSizeA;
            if (ReadSizeA < ReadBlock) break;
          }
          if (!bEqual)
            break;

          while (BufPosP < myBufSize)
          {
            if (CheckForEsc() || !ReadFile(hFileP, PBuf+BufPosP, myReadBlock, &ReadSizeP, 0))
            {
               bEqual = false;
               break;
            }
            BufPosP += ReadSizeP;
            Work.n2SizeProcess += ReadSizeP;
            if (ReadSizeP < ReadBlock) break;
          }
          if (!bEqual)
            break;

          ShowMessage(ACurDir, AData->cFileName, PCurDir, PData->cFileName, true);

          if (memcmp(ABuf, PBuf, BufPosA))
          {
            bEqual = false;
            break;
          }

          if (!ReadSizeA && !ReadSizeP)
            break;

          if (bStart)
          {
            if ( !mySetFilePointer(hFileA, FilePointerStart, FILE_CURRENT) ||
                 !mySetFilePointer(hFileP, FilePointerStart, FILE_CURRENT) )
            {
              bEqual = false;
              break;
            }
            bStart = false;
            Work.n2SizeProcess += (FilePointerStart<<1);
          }
          else
          {
            if (BlockStart-- <= 0)
            {
              if (!Block--)
              {
                if ( !mySetFilePointer(hFileA, FilePointer, FILE_CURRENT) ||
                     !mySetFilePointer(hFileP, FilePointer, FILE_CURRENT) )
                {
                  bEqual = false;
                  break;
                }
                Block = 160;
                Work.n2SizeProcess += (FilePointer<<1);
              }
            }
          }
        }
      }
      // иначе сравниваем полностью 100%
      else
      {
        UpToSizeFull:

        char *PtrA = ABuf+BufPosA, *PtrP = PBuf+BufPosP;
        bool bExpectNewLineA = false, bExpectNewLineP = false;
        SHFILEINFO shinfo;
        DWORD FileCRC = 0;
        DWORD dwFlag0 = 0, dwFlag1 = 0;

        while (1)
        {
          // читаем файл с активной панели
          if (!bAPanelWithCRC && PtrA >= ABuf+BufPosA)
          {
            BufPosA = 0;
            PtrA = ABuf;

            while (BufPosA < Opt.BufSize)
            {
              if (CheckForEsc() || !ReadFile(hFileA, ABuf+BufPosA, ReadBlock, &ReadSizeA, 0))
              {
                bEqual = false;
                break;
              }
              BufPosA += ReadSizeA;
              Work.n2SizeProcess += ReadSizeA;
              if (ReadSizeA < ReadBlock) break;
            }
          }
          if (!bEqual)
            break;

          // читаем файл с пассивной панели
          if (!bPPanelWithCRC && PtrP >= PBuf+BufPosP)
          {
            BufPosP = 0;
            PtrP = PBuf;

            while (BufPosP < Opt.BufSize)
            {
              if (CheckForEsc() || !ReadFile(hFileP, PBuf+BufPosP, ReadBlock, &ReadSizeP, 0))
              {
                bEqual = false;
                break;
              }
              BufPosP += ReadSizeP;
              Work.n2SizeProcess += ReadSizeP;
              if (ReadSizeP < ReadBlock) break;
            }
          }
          if (!bEqual)
            break;

          ShowMessage(ACurDir, AData->cFileName, PCurDir, PData->cFileName, true);

          // сравниваем с архивом
          if (bPPanelWithCRC)
          {
            FileCRC = ProcessCRC(ABuf, BufPosA, FileCRC);
            PtrA += BufPosA;
            Work.n2SizeProcess += BufPosA;
          }
          else if (bAPanelWithCRC)
          {
            FileCRC = ProcessCRC(PBuf, BufPosP, FileCRC);
            PtrP += BufPosP;
            Work.n2SizeProcess += BufPosP;
          }

          if (bAPanelWithCRC || bPPanelWithCRC)
          {
            if ((bPPanelWithCRC && BufPosA != Opt.BufSize) || (bAPanelWithCRC && BufPosP != Opt.BufSize))
            {
              if (!bAPanelWithCRC && bPPanelWithCRC && FileCRC != PCRC32)
                bEqual = false;
              else if (bAPanelWithCRC && !bPPanelWithCRC && FileCRC != ACRC32)
                bEqual = false;
              //char buf[40];
              //FSF.sprintf(buf, "A - %X P- %X", ACRC32, FileCRC);
              //DebugMsg(buf, "");
              break;
            }
            else
              continue;
          }

          // обычное сравнение (фильтр отключен или файлы исполнимые)
          if ( !Opt.Filter ||
               ( SHGetFileInfo(AData->cFileName, 0, &shinfo, sizeof(shinfo), SHGFI_EXETYPE) ||
                 SHGetFileInfo(PData->cFileName, 0, &shinfo, sizeof(shinfo), SHGFI_EXETYPE) ) )
          {
            if (!(bAPanelWithCRC || bPPanelWithCRC))
            {
              dwFlag0 = Flag0;
              dwFlag1 = Flag1;
            }

            if (memcmp(ABuf, PBuf, BufPosA))
            {
              bEqual = false;
              break;
            }
            PtrA += BufPosA;
            PtrP += BufPosP;

            if (BufPosA != Opt.BufSize || BufPosP != Opt.BufSize)
            {
              if (Opt.Cache && bBuildCacheResult && Opt.CompareName && dwFlag0 && dwFlag1)
                bBuildCacheResult = SetResult(&CacheResult, FileNameA, FileNameP, nFileTimeA, nFileTimeP,
                                              dwFlag0, dwFlag1, !bEqual);
              break;
            }
          }
          else
          // фильтр включен
          {
            if (Opt.FilterTemplatesN == 0)      // '\n' & ' '
            {
              while (PtrA < ABuf+BufPosA && PtrP < PBuf+BufPosP && !IsWhiteSpace(*PtrA) && !IsWhiteSpace(*PtrP))
              {
                if (*PtrA != *PtrP)
                {
                  bEqual = false;
                  break;
                }
                ++PtrA;
                ++PtrP;
              }
              if (!bEqual)
                break;

              while (PtrA < ABuf+BufPosA && IsWhiteSpace(*PtrA))
                ++PtrA;

              while (PtrP < PBuf+BufPosP && IsWhiteSpace(*PtrP))
                ++PtrP;
            }
            else if (Opt.FilterTemplatesN == 1)  // '\n'
            {
              if (bExpectNewLineA)
              {
                bExpectNewLineA = false;
                if (PtrA < ABuf+BufPosA && *PtrA == '\n')
                  ++PtrA;
              }

              if (bExpectNewLineP)
              {
                bExpectNewLineP = false;
                if (PtrP < PBuf+BufPosP && *PtrP == '\n')
                  ++PtrP;
              }

              while (PtrA < ABuf+BufPosA && PtrP < PBuf+BufPosP && !IsNewLine(*PtrA) && !IsNewLine(*PtrP))
              {
                if (*PtrA != *PtrP)
                {
                  bEqual = false;
                  break;
                }
                ++PtrA;
                ++PtrP;
              }
              if (!bEqual)
                break;

              if (PtrA < ABuf+BufPosA && PtrP < PBuf+BufPosP && (!IsNewLine(*PtrA) || !IsNewLine(*PtrP)))
              {
                bEqual = false;
                break;
              }

              if (PtrA < ABuf+BufPosA && PtrP < PBuf+BufPosP)
              {
                if (*PtrA == '\r')
                  bExpectNewLineA = true;

                if (*PtrP == '\r')
                  bExpectNewLineP = true;

                ++PtrA;
                ++PtrP;
              }
            }
            else if (Opt.FilterTemplatesN == 2)  // ' '
            {
              while (PtrA < ABuf+BufPosA && PtrP < PBuf+BufPosP && !myIsSpace(*PtrA) && !myIsSpace(*PtrP))
              {
                if (*PtrA != *PtrP)
                {
                  bEqual = false;
                  break;
                }
                ++PtrA;
                ++PtrP;
              }
              if (!bEqual)
                break;

              while (PtrA < ABuf+BufPosA && myIsSpace(*PtrA))
                ++PtrA;

              while (PtrP < PBuf+BufPosP && myIsSpace(*PtrP))
                ++PtrP;
            }
            // наблюдаем баги при сравнении файлов самих с собой, пока грохнем...
            /*
            if (PtrA < ABuf+BufPosA && !ReadSizeP)
            {
              bEqual = false;
              break;
            }
            if (PtrP < PBuf+BufPosP && !ReadSizeA)
            {
              bEqual = false;
              break;
            } */
            // и сделаем так...
            if (!ReadSizeA && ReadSizeP || ReadSizeA && !ReadSizeP)
            {
              bEqual = false;
              break;
            }
          }
          if (!ReadSizeA && !ReadSizeP)
            break;
        }
      }
      CloseHandle(hFileA);
      CloseHandle(hFileP);

      if ((hFileA = CreateFile(cpFileA, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) != INVALID_HANDLE_VALUE)
      {
        SetFileTime(hFileA, 0, &AAccess, 0);
        CloseHandle(hFileA);
      }

      if ((hFileP = CreateFile(cpFileP, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) != INVALID_HANDLE_VALUE)
      {
        SetFileTime(hFileP, 0, &PAccess, 0);
        CloseHandle(hFileP);
      }

      if (!bEqual)
        return false;
    }
  }
  return true;
}
