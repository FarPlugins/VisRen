/****************************************************************************
 * compare5_MSG.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 *************************** ShowMessage functions **************************
 ****************************************************************************/

/****************************************************************************
 * Усекает начало длинных имен файлов (или дополняет короткие имена)
 * для правильного показа в сообщении сравнения
 ****************************************************************************/
static void TrunCopy(char *cpDest, const char *cpSrc, bool bDir, const char *cpMsg)
{
  int iLen;
  if (bDir)
  {
    char Buf[NM];
    FSF.sprintf(Buf, cpMsg, FSF.TruncStr(lstrcpy(cpDest, cpSrc), iTruncLen - lstrlen(cpMsg)+2));
    iLen = lstrlen(lstrcpy(cpDest, Buf));
  }
  else
    iLen = lstrlen(FSF.TruncStr(lstrcpy(cpDest, cpSrc), iTruncLen));

  if (iLen < iTruncLen)
  {
    memset(&cpDest[iLen], ' ', iTruncLen - iLen);
    cpDest[iTruncLen] = '\0';
  }
}

/****************************************************************************
 * Преобразует int в char поразрядно: из 1234567890 в "1 234 567 890"
 ****************************************************************************/
static char *itoaa(int num, char *buf)
{
  if (!num)
    FSF.itoa(num, buf, 10);
  else
  {
    int digits_count = 1, m = 10;
    while (m <= num)
    {
      m *= 10;
      digits_count++;
    }
    char *p = buf + digits_count + (digits_count - 1) / 3;
    digits_count = 0;
    *(p--) = 0;
    while (num > 0)
    {
      *(p--) = num % 10 + '0';
      num /= 10;
      if ((++digits_count) % 3 == 0)
        *(p--) = ' ';
    }
  }
  return buf;
}

/****************************************************************************
 * Центрирование строки и заполнение символом заполнителем
 ****************************************************************************/
static void lstrcentr(char *cpDest, const char *cpSrc, int iTruncLen, char sym)
{
  int iLen, iLen2;
  iLen = lstrlen(lstrcpy(cpDest, cpSrc));
  if (iLen < iTruncLen)
  {
    iLen2 = (iTruncLen-iLen)/2;
    memmove(cpDest+iLen2, cpDest, iLen);
    memset(cpDest, sym, iLen2);
    memset(cpDest+iLen2+iLen, sym, iTruncLen-iLen2-iLen);
    cpDest[iTruncLen] = '\0';
  }
}

/****************************************************************************
 * Рисует строку-прогресс, возвращает процент
 ****************************************************************************/
static int GetProgressLine(char *cpBuf, DWORD64 nCurrent, DWORD64 nTotal)
{
  int n = 0, iPercent = 0, iLen = iTruncLen-4;
  if (nTotal > 0)
  {
    n = nCurrent*iLen / nTotal;
    iPercent = nCurrent*100 / nTotal;
  }
  if (n>iLen) n = iLen;

  memset(cpBuf, 0x000000DB, n);
  memset(&cpBuf[n], 0x000000B0, iLen-n);
  cpBuf[iLen] = '\0';

  return iPercent;
}

/****************************************************************************
 * Показывает сообщение о сравнении двух файлов
 ****************************************************************************/
static void ShowMessage(const char *Dir1, const char *Name1, const char *Dir2, const char *Name2, bool bRedraw)
{
  // Для перерисовки не чаще 3-х раз в 1 сек.
  if (bRedraw)
  {
    static DWORD dwTicks;
    DWORD dwNewTicks = GetTickCount();
    if (dwNewTicks - dwTicks < 350)
      return;
    dwTicks = dwNewTicks;
  }

  char TruncDir1[NM], TruncDir2[NM], TruncName1[NM], TruncName2[NM];
  TrunCopy(TruncDir1, Dir1, true, GetMsg(MComparing));
  TrunCopy(TruncName1, Name1, false, "");
  TrunCopy(TruncDir2, Dir2, true, GetMsg(MComparingWith));
  TrunCopy(TruncName2, Name2, false, "");

  char Items[20], ItemsWork[20], ItemsBuf[NM], ItemsOut[NM];
  FSF.sprintf(ItemsBuf, GetMsg(MComparingN), itoaa(Work.nProcess, ItemsWork), itoaa(Work.nCount, Items));
  lstrcentr(ItemsOut, ItemsBuf, iTruncLen, 0x000000C4);

  char ProgressLine[NM], ProgressLineOut[NM];
  if (Opt.CompareContents)
  {
    int Percent = GetProgressLine(ProgressLine, Work.n2SizeProcess, Work.n2Size);
    FSF.sprintf(ProgressLineOut, "%s%3d%%", ProgressLine, Percent);
  }
  else
    lstrcpy(ProgressLineOut, GetMsg(MWait));

  char ItemsDiff[20], ItemsDiffBuf[NM], ItemsDiffA[20], ItemsDiffP[20], ItemsDiffOut[NM];
  FSF.sprintf( ItemsDiffBuf, GetMsg(MComparingDiffN), itoaa(Diff.nDiffA + Diff.nDiffP, ItemsDiff),
               itoaa(Diff.nDiffA, ItemsDiffA), itoaa(Diff.nDiffP, ItemsDiffP) );
  lstrcentr(ItemsDiffOut, ItemsDiffBuf, iTruncLen, 0x000000C4);

  const char *MsgItems[] = {
    GetMsg(MCompareTitle),
    TruncDir1,
    TruncName1,
    ItemsOut,
    ProgressLineOut,
    ItemsDiffOut,
    TruncDir2,
    TruncName2
  };
  Info.Message( Info.ModuleNumber,
                bStartMsg ? FMSG_LEFTALIGN : FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,
                0,
                MsgItems,
                sizeof(MsgItems) / sizeof(MsgItems[0]),
                0 );
  bStartMsg = false;
}
