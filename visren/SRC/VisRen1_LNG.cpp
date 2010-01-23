/****************************************************************************
 * VisRen1_LNG.cpp
 *
 * Plugin module for FAR Manager 1.75
 *
 * Copyright (c) 2007-2010 Alexey Samlyukov
 ****************************************************************************/


/****************************************************************************
 *  онстанты дл€ извлечени€ строк из .lng файла
 ****************************************************************************/
enum {
  MVRenTitle = 0,

  MOK,

  MRen,
  MUndo,
  MEdit,
  MCancel,

  /**** основной диалог ****/

  MMaskName,
  MMaskExt,
  MTempl,
  //-----
  MTempl_1,
  MTempl_2,
  MTempl_3,
  MTempl_4,  //< SEPARATOR
  MTempl_5,
  MTempl_6,
  MTempl_7,
  MTempl_8,
  MTempl_9,
  MTempl_10, //< SEPARATOR
  MTempl_11,
  MTempl_12,
  MTempl_13,
  MTempl_14,
  MTempl_15,
  MTempl_16,
  MTempl_17, //< SEPARATOR
  MTempl_18,
  MTempl_19,
  MTempl_20,
  MTempl_21,
  MTempl_22, //< SEPARATOR
  MTempl_23,
  MTempl_24,
  MTempl_25,
  MTempl_26,
//-----
  MTempl2_1,
  MTempl2_2,
  MTempl2_3,
  MTempl2_4, //< SEPARATOR
  MTempl2_5,
  MTempl2_6,
  MTempl2_7,
  MTempl2_8,
//-----
  MSet,
  MSearch,
  MReplace,
  MCase,
  MRegEx,
  MSep,
  MCreateLog,
  MNoCreateLog,

  /**** дополнительные диалоги ****/

  MWordDivTitle,
  MWordDivBody,

  MFullFileName,
  MOldName,
  MNewName,

  /**** сообщени€ ****/

  MOldFARTitle,
  MOldFARBody,

  MFilePanelsRequired,

  MErrorNoMem,

  MLoadFiles,

  MEscTitle,
  MEscBody,

  MError,

  MClearLogTitle,
  MClearLogBody,

  MEditorTitle,
  MShowOrgName,
  MShowOrgName2,

  MErrorCreateList,
  MErrorOpenList,
  MErrorReadList,
  MAborted,

  MEditorRename,

  MErrorCreateLog,

  MUndoTitle,
  MUndoBody,

  MRenameFail,
  MTo,
  MSkip,
  MSkipAll,
  MRetry,

  MProcessedFmt,
};