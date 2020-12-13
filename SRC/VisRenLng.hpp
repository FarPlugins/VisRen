/****************************************************************************
 * VisRenLng.hpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2007 Alexey Samlyukov
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

#pragma once


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
	MTempl_27,
	MTempl_28,
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

	/**** дополнительные диалоги ****/

	MWordDivTitle,
	MWordDivBody,

	MFullFileName,
	MOldName,
	MNewName,

	/**** сообщени€ ****/

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