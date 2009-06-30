/****************************************************************************
 * compare1_LNG.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 * Константы для извлечения строк из .lng файла
 ****************************************************************************/
enum {
  MCompareTitle = 0,

  MOK,
  MFromCache,
  MCancel,

  /**** основной диалог ****/

  MCompareMask,
  MCompareBox,
  MCompareName,
  MCompareIgnoreCase,
  MCompareSize,
  MCompareTime,
  MCompareLowPrecision,
  MCompareIgnoreTimeZone,
  MCompareContents,
  // ---
  MPercent,
  MPercent100,
  MPercent75,
  MPercent50,
  MPercent25,
  // ---
  MFilter,
  MFilterIgnoreAllWhitespace,
  MFilterIgnoreNewLines,
  MFilterIgnoreWhitespace,
  MFilterEditKey,
  // ---
  MTitleOptions,
  MProcessSubfolders,
  MMaxScanDepth,
  MProcessSelected,
  MProcessTillFirstDiff,
  MPanel,
  MHotKey,

  /**** параметры ****/

  MWithAsterisks,
  MCache,
  MShowMessage,
  MSound,
  MBufSize,
  MDiffProg,

  /**** процесс ****/

  MComparingFiles,
  MComparing,
  MComparingWith,
  MComparingN,
  MComparingDiffN,
  MWait,

  /**** панель ****/

  MPanelTitle,
  MName,
  MSize,
  MTime,

  /**** сообщения ****/

  MNoDiffTitle,
  MNoDiffBody,

  MUnderCursor,
  MNoDiff,
  MDiff,
  MRunDiffProgram,

  MCmpPathTitle,
  MCmpPathBody,

  MFirstDiffTitle,
  MFirstDiffBody,

  MFilePanelsRequired,

  MNoMemTitle,
  MNoMemBody,

  MOldFARTitle,
  MOldFARBody,

  MIncorrectMaskTitle,
  MIncorrectMaskBody,

  MClearCacheTitle,
  MClearCacheBody,

  MEscTitle,
  MEscBody,
};
