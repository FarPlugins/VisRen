/****************************************************************************
 * compare7_DIR.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 *************************** CompareDirs functions **************************
 ****************************************************************************/


/****************************************************************************
 * Функция сравнения имён файлов в двух структурах PluginPanelItem
 * для нужд qsort()
 ****************************************************************************/
static int __cdecl PICompare(const void *el1, const void *el2)
{
  const PluginPanelItem *ppi1 = *(const PluginPanelItem **)el1, *ppi2 = *(const PluginPanelItem **)el2;

  if (ppi1->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (!(ppi2->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      return 1;
  }
  else
  {
    if (ppi2->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      return -1;
  }

  return FSF.LStricmp(FSF.PointToName(ppi1->FindData.cFileName), FSF.PointToName(ppi2->FindData.cFileName));
}

/****************************************************************************
 * Функция удаляет начало строки, до нужного слеша
 ****************************************************************************/
static void SlashTrim(char *str, int i)
{
  char buf[NM];
  char *p = lstrcpy(buf, str);

  while (*p && i)
  {
    if (*p == '\\') i--;
    p++;
  }
  while (*str++ = *p++)
   ;
}

/****************************************************************************
 * Функция проверяет, входит ли файл из архива в заданную глубину вложенности
 ****************************************************************************/
static bool CheckScanDepth(const char *cFileName, int ScanDepth)
{
  int i = 0;
  while (*cFileName++)
    if (*cFileName == '\\') i++;
  return  i < ScanDepth;
}

/****************************************************************************
 * Построение сортированного списка файлов для быстрого сравнения
 ****************************************************************************/
static bool BuildPanelIndex(const struct PanelInfo *pInfo, struct FileIndex *pIndex, int ScanDepth)
{
  bool bProcessSelected;
  pIndex->ppi = 0;
  pIndex->iCount = ( bProcessSelected = (Opt.ProcessSelected && pInfo->SelectedItemsNumber &&
                     (pInfo->SelectedItems[0].Flags & PPIF_SELECTED)) ) ? pInfo->SelectedItemsNumber :
                     pInfo->ItemsNumber;
  if (!Opt.CompareName) pIndex->iCount = 1;

  if (!pIndex->iCount) return true;
  if (!(pIndex->ppi = (PluginPanelItem **)malloc(pIndex->iCount * sizeof(pIndex->ppi[0]))))
    return false;

  int j = 0;
  for (int i = (Opt.CompareName ? pInfo->ItemsNumber-1 : pInfo->CurrentItem); i >= 0 && j < pIndex->iCount; i--)
  {
    if ( (Opt.ProcessSubfolders || !(pInfo->PanelItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
         && (!bProcessSelected || (pInfo->PanelItems[i].Flags & PPIF_SELECTED)) &&
         lstrcmp(pInfo->PanelItems[i].FindData.cFileName, "..") &&
         lstrcmp(pInfo->PanelItems[i].FindData.cFileName, ".") )
    {
      if (ScanDepth && (bAPanelWithCRC && !bPPanelWithCRC))
      {
        SlashTrim(pInfo->PanelItems[i].FindData.cFileName, 1);
        if (Opt.ProcessSubfolders==2 && !CheckScanDepth(pInfo->PanelItems[i].FindData.cFileName, Opt.MaxScanDepth))
          continue;
      }

      if ( FSF.ProcessName(FSF.Trim(Opt.MasksExclude), pInfo->PanelItems[i].FindData.cFileName, PN_CMPNAMELIST) )
        continue;
      else if (FSF.ProcessName(FSF.Trim(Opt.MasksInclude), pInfo->PanelItems[i].FindData.cFileName, PN_CMPNAMELIST) )
      {
        //DebugMsg(pInfo->PanelItems[i].FindData.cFileName, "PanelItems");
        pIndex->ppi[j++] = &pInfo->PanelItems[i];
      }
    }
  }

  if (pIndex->iCount = j)
  {
    // элементы из подкаталогов архива уже отсортированы...
    if (!ScanDepth || !(bAPanelWithCRC && !bPPanelWithCRC))
      // иначе, отсортируем их...
      FSF.qsort(pIndex->ppi, j, sizeof(pIndex->ppi[0]), PICompare);
  }
  else
  {
    free(pIndex->ppi);
    pIndex->ppi = 0;
  }

  return true;
}

/****************************************************************************
 * Освобождение памяти
 ****************************************************************************/
static void FreePanelIndex(struct FileIndex *pIndex)
{
  if (pIndex->ppi)
    free(pIndex->ppi);
  pIndex->ppi = 0;
  pIndex->iCount = 0;
}

/****************************************************************************
 * Сравнение двух каталогов, описанных структурами AInfo и PInfo.
 * Возвращает true, если они совпадают.
 * Параметр bCompareAll определяет,
 * надо ли сравнивать все файлы и взводить PPIF_SELECTED (bCompareAll == true)
 * или просто вернуть false при первом несовпадении (bCompareAll == false).
 ****************************************************************************/
static bool CompareDirs(const struct PanelInfo *AInfo, const struct PanelInfo *PInfo, bool bCompareAll, int ScanDepth )
{
  // Стартуем с сообщением о сравнении
  ShowMessage(AInfo->CurDir, "*", PInfo->CurDir, "*", false);

  // Проверим совпадают ли пути сравнения
  if (ScanDepth==0 && !(bAPluginPanels || bPPluginPanels) && Opt.CompareName)
  {
    char DestA[MAX_PATH], DestP[MAX_PATH];
    if ( FSF.ConvertNameToReal(AInfo->CurDir, DestA, sizeof(DestA)) &&
         FSF.ConvertNameToReal(PInfo->CurDir, DestP, sizeof(DestP)) &&
         !FSF.LStricmp(DestA, DestP) &&
         YesNoMsg(MCmpPathTitle, MCmpPathBody) )
    {
        bBrokenByEsc = true;
        return true;
    }
  }

  // Строим индексы файлов для быстрого сравнения
  struct FileIndex sfiA, sfiP;
  if (!BuildPanelIndex(AInfo, &sfiA, ScanDepth) || !BuildPanelIndex(PInfo, &sfiP, ScanDepth))
  {
    ErrorMsg(MNoMemTitle, MNoMemBody);
    bBrokenByEsc = true;
    FreePanelIndex(&sfiA);
    FreePanelIndex(&sfiP);
    return true;
  }
/*  char buf1[10], buf2[10];
  FSF.itoa(sfiA.iCount, buf1,10);
  FSF.itoa(sfiP.iCount, buf2,10);
  DebugMsg(buf2,buf1);   */

  // Экспресс-сравнение вложенного каталога
  if (ScanDepth && !Opt.Panel && sfiA.iCount != sfiP.iCount)
    return false;

  int i, j;
  // вначале снимем выделение на панелях
  for (i = AInfo->ItemsNumber-1; i>=0; i--)
    AInfo->PanelItems[i].Flags &= ~PPIF_SELECTED;
  for (j = PInfo->ItemsNumber-1; j>=0; j--)
    PInfo->PanelItems[j].Flags &= ~PPIF_SELECTED;

  // начинаем сравнивать "наши" элементы...
  Work.nCount = sfiA.iCount + sfiP.iCount;
  Work.nProcess = 0;
  bool bDifferenceNotFound = true;
  FAR_FIND_DATA ffdEmpty;
  memset(&ffdEmpty, 0, sizeof(ffdEmpty));
  i=0; j=0;
  while (i<sfiA.iCount && j<sfiP.iCount && (bDifferenceNotFound || bCompareAll) && !bBrokenByEsc)
  {
    const int iMaxCounter = 256;
    static int iCounter = iMaxCounter;
    if (!--iCounter)
    {
      iCounter = iMaxCounter;
      if (CheckForEsc())
        break;
    }
    //DebugMsg((char*)sfiA.ppi[i]->FindData.cFileName, (char*)AInfo->CurDir);
    //DebugMsg((char*)sfiP.ppi[j]->FindData.cFileName, (char*)PInfo->CurDir);

    if (Opt.CompareName)
    // Если игнорирование имен отключено.
    {
      bool bNextItem;
      switch (PICompare(&sfiA.ppi[i], &sfiP.ppi[j]))
      {
        case 0: // Имена совпали - проверяем всё остальное
          if ( CompareFiles( &sfiA.ppi[i]->FindData, &sfiP.ppi[j]->FindData, AInfo->CurDir, PInfo->CurDir,
                             ScanDepth, sfiA.ppi[i]->CRC32, sfiP.ppi[j]->CRC32 ) )
          {       // И остальное совпало
            i++; j++;
            Diff.nDiffNo += 2;
          }
          else
          {
            bDifferenceNotFound = false;
            sfiA.ppi[i]->Flags |= PPIF_SELECTED;
            sfiP.ppi[j]->Flags |= PPIF_SELECTED;
            if (Opt.Panel)
            {     // если нужно создать список отличий
              Panel->BuildItem(AInfo->CurDir, PInfo->CurDir, &sfiA.ppi[i]->FindData, &sfiP.ppi[j]->FindData);
            }
            i++; j++;
            Diff.nDiffA++; Diff.nDiffP++;
            if (Opt.ProcessTillFirstDiff && !bBrokenByEsc)
            {     // нужно ли продолжать сравнивать
              bCompareAll=(Opt.ShowMessage && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
              Opt.ProcessTillFirstDiff = 0;
            }
          }
          Work.nProcess += 2;
          break;
        case -1: // Элемент sfiA.ppi[i] не имеет одноимённых в sfiP.ppi
          CmpContinueA:
          if (!bAPanelTmp)
          {
            bNextItem = true;
            goto FoundDiffA;
          }
          else
          { // ...но если с Темп-панели, то проверим с элементом sfiP.ppi
            bNextItem = false;
            for (int k=0; k<sfiP.iCount; k++)
            {
              if (!PICompare(&sfiA.ppi[i], &sfiP.ppi[k]))
              {
                bNextItem = true;
                if ( CompareFiles( &sfiA.ppi[i]->FindData, &sfiP.ppi[k]->FindData, AInfo->CurDir, PInfo->CurDir,
                                   ScanDepth, sfiA.ppi[i]->CRC32, sfiP.ppi[k]->CRC32 ) )
                {
                  i++; Diff.nDiffNo++;
                  break;
                }
                else
                FoundDiffA:
                {
                  bDifferenceNotFound = false;
                  sfiA.ppi[i]->Flags |= PPIF_SELECTED;
                  if (Opt.Panel)
                    Panel->BuildItem(AInfo->CurDir, PInfo->CurDir, &sfiA.ppi[i]->FindData, &ffdEmpty);
                  i++; Diff.nDiffA++;
                  if (bAPanelTmp && k<sfiP.iCount && !(sfiP.ppi[k]->Flags & PPIF_SELECTED))
                  {
                    sfiP.ppi[k]->Flags |= PPIF_SELECTED;
                    Diff.nDiffP++; Diff.nDiffNo--;
                  }
                  if (Opt.ProcessTillFirstDiff && !bBrokenByEsc)
                  {
                    bCompareAll=(Opt.ShowMessage && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
                    Opt.ProcessTillFirstDiff = 0;
                  }
                  break;
                }
              }
            }
            if (!bNextItem)
            {
              bNextItem = true;
              goto FoundDiffA;
            }
          }
          Work.nProcess++;
          break;
        case 1: // Элемент sfiP.ppi[j] не имеет одноимённых в sfiA.ppi
          CmpContinueP:
          if (!bPPanelTmp)
          {
            bNextItem = true;
            goto FoundDiffP;
          }
          else
          { // ...но если с Темп-панели, то проверим с элементом sfiA.ppi
            bNextItem = false;
            for (int k=0; k<sfiA.iCount; k++)
            {
              if (!PICompare(&sfiA.ppi[k], &sfiP.ppi[j]))
              {
                bNextItem = true;
                if ( CompareFiles( &sfiA.ppi[k]->FindData, &sfiP.ppi[j]->FindData, AInfo->CurDir, PInfo->CurDir,
                                   ScanDepth, sfiA.ppi[k]->CRC32, sfiP.ppi[j]->CRC32 ) )
                {
                  j++; Diff.nDiffNo++;
                  break;
                }
                else
                FoundDiffP:
                {
                  bDifferenceNotFound = false;
                  sfiP.ppi[j]->Flags |= PPIF_SELECTED;
                  if (Opt.Panel)
                    Panel->BuildItem(AInfo->CurDir, PInfo->CurDir, &ffdEmpty, &sfiP.ppi[j]->FindData);
                  j++; Diff.nDiffP++;
                  if (bPPanelTmp && k<sfiA.iCount && !(sfiA.ppi[k]->Flags & PPIF_SELECTED))
                  {
                    sfiA.ppi[k]->Flags |= PPIF_SELECTED;
                    Diff.nDiffA++; Diff.nDiffNo--;
                  }
                  if (Opt.ProcessTillFirstDiff && !bBrokenByEsc)
                  {
                    bCompareAll=(Opt.ShowMessage && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
                    Opt.ProcessTillFirstDiff = 0;
                  }
                  break;
                }
              }
            }
            if (!bNextItem)
            {
              bNextItem = true;
              goto FoundDiffP;
            }
          }
          Work.nProcess++;
          break;
      }
    }
    else
    {
      if ( CompareFiles( &sfiA.ppi[i]->FindData, &sfiP.ppi[j]->FindData, AInfo->CurDir, PInfo->CurDir,
                         ScanDepth, sfiA.ppi[i]->CRC32, sfiP.ppi[j]->CRC32 ) )
      {
        i++; j++;
        Diff.nDiffNo += 2;
        Work.nProcess += 2;
      }
      else
      {
        bDifferenceNotFound = false;
        sfiA.ppi[i++]->Flags |= PPIF_SELECTED;
        sfiP.ppi[j++]->Flags |= PPIF_SELECTED;
        Diff.nDiffA++; Diff.nDiffP++;
        Work.nProcess += 2;
      }
    }
  }

  if (!bBrokenByEsc)
  {
    // Собственно сравнение окончено. Пометим то, что осталось необработанным в массивах
    if (i<sfiA.iCount)
    {
      if (!bAPanelTmp)
        bDifferenceNotFound = false;
      if (bCompareAll)
        goto CmpContinueA;
    }
    if (j<sfiP.iCount)
    {
      if (!bPPanelTmp)
        bDifferenceNotFound = false;
      if (bCompareAll)
        goto CmpContinueP;
    }
  }

  FreePanelIndex(&sfiA);
  FreePanelIndex(&sfiP);

  return bDifferenceNotFound;
}
