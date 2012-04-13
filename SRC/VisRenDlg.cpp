/****************************************************************************
 * VisRenDlg.cpp
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

#pragma hdrstop
#include "VisRenDlg.hpp"

/****************************************************************************
 **************************** ShowDialog functions **************************
 ****************************************************************************/

/****************************************************************************
 * ID-константы диалога
 ****************************************************************************/
enum {
	DlgBORDER = 0,    // 0
	DlgSIZEICON,      // 1
	DlgLMASKNAME,     // 2
	DlgEMASKNAME,     // 3
	DlgLTEMPL,        // 4
	DlgETEMPLNAME,    // 5
	DlgBTEMPLNAME,    // 6

	DlgLMASKEXT,      // 7
	DlgEMASKEXT,      // 8
	DlgETEMPLEXT,     // 9
	DlgBTEMPLEXT,     //10

	DlgSEP1,          //11
	DlgLSEARCH,       //12
	DlgESEARCH,       //13
	DlgLREPLACE,      //14
	DlgEREPLACE,      //15
	DlgCASE,          //16
	DlgREGEX,         //17

	DlgSEP2,          //18
	DlgLIST,          //19
	DlgSEP3_LOG,      //20
	DlgREN,           //21
	DlgUNDO,          //22
	DlgEDIT,          //23
	DlgCANCEL         //24
};

/***************************************************************************
 * Узнаем и установим размеры для диалога
 ***************************************************************************/
void VisRenDlg::GetDlgSize()
{
	HANDLE hConOut = CreateFileW(L"CONOUT$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	CONSOLE_SCREEN_BUFFER_INFO csbiNfo;
	if (GetConsoleScreenBufferInfo(hConOut, &csbiNfo))
	{
		// необходимо после правки FAR: drkns 22.05.2010 20:00:00 +0200 - build 1564
		csbiNfo.dwSize.X=csbiNfo.srWindow.Right-csbiNfo.srWindow.Left+1;
		csbiNfo.dwSize.Y=csbiNfo.srWindow.Bottom-csbiNfo.srWindow.Top+1;

		if (csbiNfo.dwSize.X-2 != DlgSize.mW)
		{
			DlgSize.mW=csbiNfo.dwSize.X-2;
			DlgSize.mWS=DlgSize.mW-37;
			DlgSize.mW2=(DlgSize.mW)/2-2;
		}
		if (csbiNfo.dwSize.Y-2 != DlgSize.mH) DlgSize.mH=csbiNfo.dwSize.Y-2;

	}
	CloseHandle(hConOut);
}

/****************************************************************************
 * Узнаем максимальную длину файлов, длину имени и длину расширения
 ****************************************************************************/
void VisRenDlg::LenItems(const wchar_t *FileName, bool bDest)
{
	const wchar_t *start=FileName;
	while (*FileName++)
		;
	const wchar_t *end=FileName-1;

	if (bDest)
		Opt.lenDestFileName=max(Opt.lenDestFileName, end-start);
	else
		Opt.lenFileName=max(Opt.lenFileName, end-start);

	while (--FileName != start && *FileName != L'.')
		;
	if (*FileName == L'.')
		(bDest?Opt.lenDestExt:Opt.lenExt)=max((bDest?Opt.lenDestExt:Opt.lenExt), end-FileName-1);
	if (FileName != start)
		(bDest?Opt.lenDestName:Opt.lenName)=max((bDest?Opt.lenDestName:Opt.lenName), FileName-start);
}

/***************************************************************************
 * Изменение/обновление листа файлов в диалоге
 ***************************************************************************/
bool VisRenDlg::UpdateFarList(HANDLE hDlg, bool bFull, bool bUndoList)
{
	Opt.lenFileName=Opt.lenName=Opt.lenExt=0;
	Opt.lenDestFileName=Opt.lenDestName=Opt.lenDestExt=0;
	// ширина колонок
	int widthSrc, widthDest;
	widthSrc=widthDest=(bFull?DlgSize.mW2:DlgSize.W2)-2;
	if (Opt.CurBorder!=0) { widthSrc+=Opt.CurBorder; widthDest-=Opt.CurBorder; }
	// количество строк
	int ItemsNumber=(bUndoList?Undo.iCount:FileList.Count());
	// для корректного показа границы между колонками
	int AddNumber=((bFull?DlgSize.mH:DlgSize.H)-4)-9+1;
	if (ItemsNumber<AddNumber) AddNumber-=ItemsNumber;
	else AddNumber=0;
	// запросим информацию
	FarListInfo ListInfo;
	ListInfo.StructSize=sizeof(FarListInfo);
	Info.SendDlgMessage(hDlg,DM_LISTINFO,DlgLIST,&ListInfo);

	if (ListInfo.ItemsNumber)
		Info.SendDlgMessage(hDlg,DM_LISTDELETE,DlgLIST,0);

	wchar_t *buf=(wchar_t *)malloc(DlgSize.mW*sizeof(wchar_t));

	for (int i=0; i<ItemsNumber; i++)
	{
		File *cur=NULL; int index;
		for (cur=FileList.First(), index=0; cur && index<i; cur=FileList.Next(cur), index++)
			;
		const wchar_t *src=NULL,*dest=NULL;

		if (bUndoList)
		{
			src=Undo.CurFileName[i]; dest=Undo.OldFileName[i];
		}
		else
		{
			if (cur)
			{
				src=cur->strSrcFileName.get(); dest=cur->strDestFileName.get();
			}
/*
			else //на всякий случай
			{
				AddNumber+=ItemsNumber-(i+1);
				ItemsNumber=i+1;
				break;
			}
*/
		}
		// схитрим, тут же определим max длины :)
		LenItems(src, 0); LenItems(dest, 1);

		int lenSrc=wcslen(src), posSrc=Opt.srcCurCol;
		if (lenSrc<=widthSrc) posSrc=0;
		else if (posSrc>lenSrc-widthSrc) posSrc=lenSrc-widthSrc;

		int lenDest=wcslen(dest), posDest=Opt.destCurCol;
		if (lenDest<=widthDest) posDest=0;
		else if (posDest>lenDest-widthDest) posDest=lenDest-widthDest;

		FSF.sprintf( buf, L"%-*.*s%c%-*.*s", widthSrc, widthSrc, src+posSrc, 0x2502, widthDest, widthDest, (bError?GetMsg(MError):dest+posDest) );
		Info.SendDlgMessage(hDlg,DM_LISTADDSTR,DlgLIST,buf);
	}

	for (int i=ItemsNumber; i<ItemsNumber+AddNumber; i++)
	{
		FSF.sprintf( buf, L"%-*.*s%c%-*.*s", widthSrc, widthSrc, L"", 0x2502, widthDest, widthDest, L"" );
		Info.SendDlgMessage(hDlg,DM_LISTADDSTR,DlgLIST,buf);
	}

	if (buf) free(buf);
	// восстановим положение курсора
	FarListPos ListPos;
	ListPos.StructSize=sizeof(FarListPos);
	ListPos.SelectPos=ListInfo.SelectPos<ItemsNumber?ListInfo.SelectPos:(ListInfo.SelectPos-1<0?0:ListInfo.SelectPos-1);
	static bool bOldFull=bFull;
	if (bOldFull!=bFull) { ListPos.TopPos=-1; bOldFull=bFull; }
	else ListPos.TopPos=ListInfo.TopPos;
	Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,DlgLIST,&ListPos);

	return true;
}

/***************************************************************************
 * Изменение размера диалога
 ***************************************************************************/
void VisRenDlg::DlgResize(HANDLE hDlg, bool bF5)
{
	COORD c;
	if (bF5) // нажали F5
	{
		if (DlgSize.Full==false)  // был нормальный размер
		{
			c.X=DlgSize.mW; c.Y=DlgSize.mH;
			DlgSize.Full=true;  // установили максимальный
		}
		else
		{
			c.X=DlgSize.W; c.Y=DlgSize.H;
			DlgSize.Full=false; // вернули нормальный
		}
	}
	else // иначе просто пересчитаем размеры диалога
	{
		if (DlgSize.Full) { c.X=DlgSize.mW; c.Y=DlgSize.mH; }
		else { c.X=DlgSize.W; c.Y=DlgSize.H; }
	}
	Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
	Opt.srcCurCol=Opt.destCurCol=Opt.CurBorder=0;

	FarGetDialogItem FGDI;
	FGDI.StructSize=sizeof(FarGetDialogItem);
	for (int i=DlgBORDER; i<=DlgCANCEL; i++)
	{
		FGDI.Item=(FarDialogItem *)malloc(FGDI.Size=Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0));
		if (!FGDI.Item) return;
		Info.SendDlgMessage(hDlg, DM_GETDLGITEM, i, &FGDI);
		switch (i)
		{
			case DlgBORDER:
				FGDI.Item->X2=(DlgSize.Full?DlgSize.mW:DlgSize.W)-1;
				FGDI.Item->Y2=(DlgSize.Full?DlgSize.mH:DlgSize.H)-1;
				break;
				case DlgSIZEICON:
				FGDI.Item->X1=(DlgSize.Full?DlgSize.mW:DlgSize.W)-4;
				break;
			case DlgEMASKNAME:
				FGDI.Item->X2=(DlgSize.Full?DlgSize.mWS:DlgSize.WS)-3;
				break;
			case DlgLMASKEXT:
			case DlgCASE:
			case DlgREGEX:
				FGDI.Item->X1=(DlgSize.Full?DlgSize.mWS:DlgSize.WS);
				break;
			case DlgEMASKEXT:
				FGDI.Item->X1=(DlgSize.Full?DlgSize.mWS:DlgSize.WS);
				FGDI.Item->X2=(DlgSize.Full?DlgSize.mW:DlgSize.W)-3;
				break;
			case DlgETEMPLEXT:
				FGDI.Item->X1=(DlgSize.Full?DlgSize.mWS:DlgSize.WS);
				FGDI.Item->X2=(DlgSize.Full?DlgSize.mW:DlgSize.W)-8;
				break;
			case DlgBTEMPLEXT:
				FGDI.Item->X1=(DlgSize.Full?DlgSize.mW:DlgSize.W)-5;
				break;
			case DlgESEARCH:
			case DlgEREPLACE:
				FGDI.Item->X2=(DlgSize.Full?DlgSize.mWS:DlgSize.WS)-3;
				break;
			case DlgLIST:
			{
				FGDI.Item->X2=(DlgSize.Full?DlgSize.mW:DlgSize.W)-2;
				FGDI.Item->Y2=(DlgSize.Full?DlgSize.mH:DlgSize.H)-4;

				// !!! Обходим некузявость FARa по возврату ListItems из FarDialogItem
				UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo);
				break;
			}
			case DlgSEP3_LOG:
				FGDI.Item->Y1=(DlgSize.Full?DlgSize.mH:DlgSize.H)-3;
				break;
			case DlgREN:
			case DlgUNDO:
			case DlgEDIT:
			case DlgCANCEL:
				FGDI.Item->Y1=(DlgSize.Full?DlgSize.mH:DlgSize.H)-2;
				break;
		}
		Info.SendDlgMessage(hDlg, DM_SETDLGITEM, i, FGDI.Item);
		free(FGDI.Item);
	}
	Info.SendDlgMessage(hDlg, DM_RESIZEDIALOG, 0, &c);
	c.X=c.Y=-1;
	Info.SendDlgMessage(hDlg, DM_MOVEDIALOG, true, &c);
	Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
	return;
}

/****************************************************************************
 * Формирование строки масок по выбранному шаблону
 ****************************************************************************/
bool VisRenDlg::SetMask(HANDLE hDlg, DWORD IdMask, DWORD IdTempl)
{
	wchar_t templ[15];
	FarListPos ListPos;
	ListPos.StructSize=sizeof(FarListPos);
	Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, IdTempl, &ListPos);

	if (IdTempl==DlgETEMPLNAME) // список с шаблонами имени
	{
		switch (ListPos.SelectPos)
		{
			case 0:  wcscpy(templ, L"[N]");      break;
			case 1:  FSF.sprintf(templ, L"[N1-%d]", Opt.lenName); break;
			case 2:  wcscpy(templ, L"[C1+1]");   break;
			//---
			case 4:  wcscpy(templ, L"[L]");      break;
			case 5:  wcscpy(templ, L"[U]");      break;
			case 6:  wcscpy(templ, L"[F]");      break;
			case 7:  wcscpy(templ, L"[T]");      break;
			case 8:  wcscpy(templ, L"[M]");      break;
			//---
			case 10: wcscpy(templ, L"[#]");      break;
			case 11: wcscpy(templ, L"[t]");      break;
			case 12: wcscpy(templ, L"[a]");      break;
			case 13: wcscpy(templ, L"[l]");      break;
			case 14: wcscpy(templ, L"[y]");      break;
			case 15: wcscpy(templ, L"[g]");      break;
			//---
			case 17: wcscpy(templ, L"[c]");      break;
			case 18: wcscpy(templ, L"[m]");      break;
			case 19: wcscpy(templ, L"[d]");      break;
			case 20: wcscpy(templ, L"[r]");      break;
			//---
			case 22: wcscpy(templ, L"[DM]");     break;
			case 23: wcscpy(templ, L"[TM]");     break;
			case 24: wcscpy(templ, L"[TL]");     break;
			case 25: wcscpy(templ, L"[TR]");     break;
		}
	}
	else   // список с шаблонами расширения
	{
		switch (ListPos.SelectPos)
		{
			case 0:  wcscpy(templ, L"[E]");      break;
			case 1:  FSF.sprintf(templ, L"[E1-%d]", Opt.lenExt); break;
			case 2:  wcscpy(templ, L"[C1+1]");   break;
			//---
			case 4:  wcscpy(templ, L"[L]");      break;
			case 5:  wcscpy(templ, L"[U]");      break;
			case 6:  wcscpy(templ, L"[F]");      break;
			case 7:  wcscpy(templ, L"[T]");      break;
		}
	}

	COORD Pos;
	Info.SendDlgMessage(hDlg, DM_GETCURSORPOS, IdMask, &Pos);
	EditorSelect es;
	Info.SendDlgMessage(hDlg, DM_GETSELECTION, IdMask, &es);
	string strBuf((const wchar_t*)Info.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, IdMask, 0));
	size_t length=strBuf.length();
	string strBuf2;

	if (es.BlockType!=BTYPE_NONE && es.BlockStartPos>=0)
	{
		Pos.X=es.BlockStartPos;
		strBuf2=(strBuf.get()+es.BlockStartPos+es.BlockWidth);  // обрывок за выделением
		strBuf(strBuf.get(),Pos.X);
		strBuf+=strBuf2.get();
	}

	if (Pos.X>=length)
		strBuf+=templ;
	else
	{
		strBuf2=(strBuf.get()+Pos.X);
		strBuf(strBuf.get(),Pos.X);
		strBuf+=templ;
		strBuf+=strBuf2.get();
	}
	Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, IdMask, FSF.Trim(strBuf.get()));
	Info.SendDlgMessage(hDlg, DM_SETFOCUS, IdMask, 0);
	Pos.X+=wcslen(templ);
	Info.SendDlgMessage(hDlg, DM_SETCURSORPOS, IdMask, &Pos);
	es.BlockType=BTYPE_NONE;
	Info.SendDlgMessage(hDlg, DM_SETSELECTION, IdMask, &es);
	return true;
}

/****************************************************************************
 * Поддержка Drag&Drop мышью в листе файлов
 ****************************************************************************/
void VisRenDlg::MouseDragDrop(HANDLE hDlg, DWORD dwMousePosY)
{
	FarListPos ListPos;
	ListPos.StructSize=sizeof(FarListPos);
	Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgLIST, &ListPos);
	if (ListPos.SelectPos>=FileList.Count()) return;

	SMALL_RECT dlgRect;
	Info.SendDlgMessage(hDlg, DM_GETDLGRECT, 0, &dlgRect);
	int CurPos=ListPos.TopPos+(dwMousePosY-(dlgRect.Top+9));

	if (CurPos<0) CurPos=0;
	else if (CurPos>=FileList.Count()) CurPos=FileList.Count()-1;

	if (CurPos!=ListPos.SelectPos)
	{
		bool bUp=CurPos<ListPos.SelectPos;
		if (bUp?ListPos.SelectPos>0:ListPos.SelectPos<FileList.Count()-1)
		{
			Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
			File add, *cur=NULL; unsigned index;
			for (cur=FileList.First(), index=0; cur && index<ListPos.SelectPos; cur=FileList.Next(cur), index++)
				;
			if (cur)
			{
				add=*cur;
				cur=FileList.Delete(cur);
				if (bUp)
					FileList.InsertBefore(cur,&add);
				else
				{
					cur=FileList.Next(cur);
					FileList.InsertAfter(cur,&add);
				}
			}
			for (int i=DlgEMASKNAME; i<=DlgEREPLACE; i++)
				switch(i)
				{
					case DlgEMASKNAME: case DlgEMASKEXT:
					case DlgESEARCH:   case DlgEREPLACE:
					{
						FarGetDialogItem FGDI;
						FGDI.StructSize=sizeof(FarGetDialogItem);
						FGDI.Item=(FarDialogItem *)malloc(FGDI.Size=Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0));
						if (FGDI.Item)
						{
							Info.SendDlgMessage(hDlg, DM_GETDLGITEM, i, &FGDI);
							Info.SendDlgMessage(hDlg, DN_EDITCHANGE, i, FGDI.Item);
							free(FGDI.Item);
							break;
						}
					}
				}
			bUp?ListPos.SelectPos--:ListPos.SelectPos++;
			Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, DlgLIST, &ListPos);
			Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
		}
	}
	return;
}

/****************************************************************************
 * Обработка клика правой клавишей мыши в листе файлов
 ****************************************************************************/
int VisRenDlg::ListMouseRightClick(HANDLE hDlg, DWORD dwMousePosX, DWORD dwMousePosY)
{
	SMALL_RECT dlgRect;
	Info.SendDlgMessage(hDlg, DM_GETDLGRECT, 0, &dlgRect);
	FarListPos ListPos;
	ListPos.StructSize=sizeof(FarListPos);
	Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgLIST, &ListPos);
	int Pos=ListPos.TopPos+(dwMousePosY-(dlgRect.Top+9));
	if ( Pos>=(Opt.LoadUndo?Undo.iCount:FileList.Count())
				// щелкнули за пределами границ DlgLIST
				|| dwMousePosX<=dlgRect.Left  || dwMousePosX>=dlgRect.Right
				|| dwMousePosY<=dlgRect.Top+8 || dwMousePosY>=dlgRect.Bottom-2
		)
		return -1;
	ListPos.SelectPos=Pos;
	Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, DlgLIST, &ListPos);
	return Pos;
}

/****************************************************************************
 * Ф-ция отображает текущее имя из листа файлов на несколько строк
 ****************************************************************************/
void VisRenDlg::ShowName(int Pos)
{
	int i;
	wchar_t srcName[4][66], destName[4][66];

	for (i=0; i<4; i++)
	{
		srcName[i][0]=0; destName[i][0]=0;
	}

	File *cur=NULL; int index;
	for (cur=FileList.First(), index=0; cur && index<Pos; cur=FileList.Next(cur), index++)
			;
	// старое имя
	wchar_t *src=Opt.LoadUndo?Undo.CurFileName[Pos]:(cur?cur->strSrcFileName.get():NULL);
	int len=wcslen(src);
	for (i=0; i<4; i++, len-=65)
	{
		if (len<65)
		{
			lstrcpyn(srcName[i], (src+i*65)?(src+i*65):L"", len+1);
			break;
		}
		else
			lstrcpyn(srcName[i], (src+i*65)?(src+i*65):L"", 66);
	}
	// новое имя
	wchar_t *dest=Opt.LoadUndo?Undo.OldFileName[Pos]:(cur?cur->strDestFileName.get():NULL);
	len=wcslen(dest);
	for (i=0; i<4; i++, len-=65)
	{
		if (len<65)
		{
			lstrcpyn(destName[i], (dest+i*65)?(dest+i*65):L"", len+1);
			break;
		}
		else
			lstrcpyn(destName[i], (dest+i*65)?(dest+i*65):L"", 66);
	}

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2	Selected	History	Mask	Flags	Data	MaxLen	UserParam	
		/* 0*/{DI_DOUBLEBOX,0, 0,70,14, 0, 0, 0,                  0, GetMsg(MFullFileName), 0,0},
		/* 1*/{DI_SINGLEBOX,2, 1,68, 6, 0, 0, 0,       DIF_LEFTTEXT, GetMsg(MOldName), 0,0},
		/* 2*/{DI_TEXT,     3, 2, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, srcName[0],0,0},
		/* 3*/{DI_TEXT,     3, 3, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, srcName[1],0,0},
		/* 4*/{DI_TEXT,     3, 4, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, srcName[2],0,0},
		/* 5*/{DI_TEXT,     3, 5, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, srcName[3],0,0},
		/* 6*/{DI_SINGLEBOX,2, 7,68,12, 0, 0, 0,       DIF_LEFTTEXT, GetMsg(MNewName), 0,0},
		/* 7*/{DI_TEXT,     3, 8, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, destName[0],0,0},
		/* 8*/{DI_TEXT,     3, 9, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, destName[1],0,0},
		/* 9*/{DI_TEXT,     3,10, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, destName[2],0,0},
		/*10*/{DI_TEXT,     3,11, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, destName[3],0,0},
		/*11*/{DI_BUTTON,   0,13, 0, 0, 0, 0, 0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsg(MOK),0,0},
	};

	HANDLE hDlg=Info.DialogInit(&MainGuid,&DlgNameGuid,-1,-1,71,15,L"Contents", DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG,0,0);
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}
	return;
}

INT_PTR WINAPI VisRenDlg::ShowDialogProcThunk(HANDLE hDlg, int Msg, int Param1, void *Param2)
{
	VisRenDlg* Class=(VisRenDlg*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	return Class->ShowDialogProc(hDlg,Msg,Param1,Param2);
}

/****************************************************************************
 * Обработчик диалога для ShowDialog
 ****************************************************************************/
INT_PTR WINAPI VisRenDlg::ShowDialogProc(HANDLE hDlg, int Msg, int Param1, void *Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
			{
				StrOpt.MaskName.clear();
				StrOpt.MaskExt.clear();
				StrOpt.Search.clear();
				StrOpt.Replace.clear();
				StrOpt.WordDiv=L"-. _&";

				FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
				if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
				{
					int Root=0; // корень ключа
					FarSettingsItem item={Root,L"WordDiv",FST_STRING};
					if (Info.SettingsControl(settings.Handle,SCTL_GET,0,&item))
						StrOpt.WordDiv=item.String;
					Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
				}

				Opt.CurBorder=Opt.srcCurCol=Opt.destCurCol=0;
				Opt.CaseSensitive=Opt.LogRen=1;
				Opt.RegEx=Opt.LoadUndo=Opt.Undo=0;
				StartPosX=Focus=-1;
				bError=false;
				SaveItemFocus=DlgEMASKNAME;

				Info.SendDlgMessage(hDlg, DM_SETCOMBOBOXEVENT, DlgETEMPLNAME, (void*)CBET_KEY);
				Info.SendDlgMessage(hDlg, DM_SETCOMBOBOXEVENT, DlgETEMPLEXT, (void*)CBET_KEY);

				if (!Opt.UseLastHistory)
				{
					Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgEMASKNAME, L"[N]");
					Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgEMASKEXT, L"[E]");
				}
				// установим предыдущий размер диалога
				DlgResize(hDlg);

				// для корректного использования масок из истории
				FarGetDialogItem FGDI;
				FGDI.StructSize=sizeof(FarGetDialogItem);
				FGDI.Item=(FarDialogItem *)malloc(FGDI.Size=Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgEMASKNAME,0));
				if (FGDI.Item)
				{
					Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgEMASKNAME,&FGDI);
					Info.SendDlgMessage(hDlg,DN_EDITCHANGE,DlgEMASKNAME,FGDI.Item);
					free(FGDI.Item);
				}
				FGDI.Item=(FarDialogItem *)malloc(FGDI.Size=Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgEMASKEXT,0));
				if (FGDI.Item)
				{
					Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgEMASKEXT,&FGDI);
					Info.SendDlgMessage(hDlg,DN_EDITCHANGE,DlgEMASKEXT,FGDI.Item);
					free(FGDI.Item);
				}
				break;
			}

	/************************************************************************/

		case DN_RESIZECONSOLE:
			GetDlgSize();
			DlgResize(hDlg);
			return true;

	/************************************************************************/

		case DN_DRAWDIALOG:
			{
				Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgSIZEICON, (DlgSize.Full?L"[]":L"[]"));
				string buf;
				if (Opt.LoadUndo && Undo.Dir)
				{
					buf=Undo.Dir;
					FSF.TruncStr(buf.get(), DlgSize.Full?DlgSize.mW-6:DlgSize.W-6);
					buf.updsize();
				}
				string sep=L" ";
				sep+=Opt.LoadUndo?buf.get():GetMsg(MSep);
				sep+=L" ";
				Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgSEP2, sep.get());

				sep=L" ";
				sep+=Opt.LogRen?0x221a:L' ';
				sep+=L' ';
				sep+=GetMsg(MCreateLog);
				sep+=Undo.iCount?L"* ":L" ";
				Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgSEP3_LOG, sep.get());

				for (int i=DlgEMASKNAME; i<=DlgEDIT; i++)
				{
					switch (i)
					{
						case DlgEMASKNAME: case DlgETEMPLNAME: case DlgBTEMPLNAME:
						case DlgEMASKEXT:  case DlgETEMPLEXT:  case DlgBTEMPLEXT:
						case DlgESEARCH:   case DlgEREPLACE:   case DlgCASE:
						case DlgREGEX:
							Info.SendDlgMessage(hDlg, DM_ENABLE, i, (void*)!(Opt.LoadUndo || !FileList.Count()));
							break;
						case DlgREN:
							Info.SendDlgMessage(hDlg, DM_ENABLE, i, (void*)!(!FileList.Count() || bError || (Opt.LoadUndo && !Undo.iCount)));
							break;
						case DlgUNDO:
							Info.SendDlgMessage(hDlg, DM_ENABLE, i, (void*)!(!Undo.iCount || Opt.LoadUndo || bError));
							break;
						case DlgEDIT:
							Info.SendDlgMessage(hDlg, DM_ENABLE, i, (void*)!(Opt.LoadUndo || bError || !FileList.Count()));
							break;
					}
				}
				return true;
			}

	/************************************************************************/

		case DN_BTNCLICK:
			if (Param1==DlgBTEMPLNAME)
			{
				if (!SetMask(hDlg, DlgEMASKNAME, DlgETEMPLNAME)) return false;
			}
			else if (Param1==DlgBTEMPLEXT)
			{
				if (!SetMask(hDlg, DlgEMASKEXT, DlgETEMPLEXT)) return false;
			}
			else if (Param1==DlgCASE || Param1==DlgREGEX)
			{
				Param1==DlgCASE?Opt.CaseSensitive=(int)Param2:Opt.RegEx=(int)Param2;
				FarGetDialogItem FGDI;
				FGDI.StructSize=sizeof(FarGetDialogItem);
				FGDI.Item=(FarDialogItem *)malloc(FGDI.Size=Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgESEARCH,0));
				if (FGDI.Item)
				{
					Info.SendDlgMessage(hDlg, DM_GETDLGITEM, DlgESEARCH,&FGDI);
					Info.SendDlgMessage(hDlg, DN_EDITCHANGE, DlgESEARCH,FGDI.Item);
					free(FGDI.Item);
				}
			}
			else if (Param1==DlgUNDO)
			{
				Opt.srcCurCol=Opt.destCurCol=Opt.CurBorder=0;
				UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo=1);
				Info.SendDlgMessage(hDlg, DM_SETFOCUS, DlgREN, 0);
			}
			break;

	/************************************************************************/

		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==MOUSE_EVENT)
			{
				if (Param1==DlgSIZEICON && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED)
					goto DLGRESIZE;
				else if (Param1==DlgSEP3_LOG && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED)
					goto LOGREN;
				else if (Param1==DlgSEP3_LOG && record->Event.MouseEvent.dwButtonState==RIGHTMOST_BUTTON_PRESSED)
					goto CLEARLOGREN;
				else if (Param1==DlgLIST && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED &&
									record->Event.MouseEvent.dwEventFlags==DOUBLE_CLICK )
					goto GOTOFILE;
				else if (Param1==DlgLIST && record->Event.MouseEvent.dwButtonState==RIGHTMOST_BUTTON_PRESSED)
				{
					Info.SendDlgMessage(hDlg, DM_SETFOCUS, DlgLIST, 0);
					goto RIGHTCLICK;
				}
				return false;
			}
			if (record->EventType==KEY_EVENT && record->Event.KeyEvent.bKeyDown)
			{
				WORD Key=record->Event.KeyEvent.wVirtualKeyCode;

				if (IsNone(record) && Key==VK_F1 && (Param1==DlgETEMPLNAME || Param1==DlgETEMPLEXT))
				{
					Info.ShowHelp(Info.ModuleName, 0, 0);
					return true;
				}
				else if (IsNone(record) && Key==VK_F2 && !Info.SendDlgMessage(hDlg, DM_GETDROPDOWNOPENED, 0, 0))
				{
 LOGREN:
					if (Opt.LogRen) Opt.LogRen=0;
					else Opt.LogRen=1;
					Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
					return true;
				}
				//----
				else if (IsNone(record) && Key==VK_F3)
				{
					int Pos=Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgLIST, 0);
					if (Pos<(Opt.LoadUndo?Undo.iCount:FileList.Count())) ShowName(Pos);
					return true;
				}
				//----
				else if (IsNone(record) && Key==VK_F4)
				{
					if ( (Param1==DlgETEMPLNAME || Param1==DlgETEMPLEXT) && Info.SendDlgMessage(hDlg, DM_GETDROPDOWNOPENED, 0, 0) )
					{
						if (  Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgETEMPLNAME, 0)==7 ||
									Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgETEMPLEXT, 0)==7)
						{
							wchar_t WordDiv[20];
							if (Info.InputBox(&MainGuid,&InputBoxGuid, GetMsg(MWordDivTitle), GetMsg(MWordDivBody), L"VisRenWordDiv", StrOpt.WordDiv.get(),WordDiv,19, 0, FIB_BUTTONS ))
							{
								StrOpt.WordDiv=WordDiv;
								FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
								if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
								{
									int Root=0; // корень ключа
									FarSettingsItem item={Root,L"WordDiv",FST_STRING};
									item.String=WordDiv;
									Info.SettingsControl(settings.Handle,SCTL_SET,0,&item);
									Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
								}
							}
						}
					}
					else if (!Opt.LoadUndo)
						Info.SendDlgMessage(hDlg, DM_CLOSE, DlgEDIT, 0);
					return true;
				}
				//----
				else if (IsNone(record) && Key==VK_F5 && !Info.SendDlgMessage(hDlg, DM_GETDROPDOWNOPENED, 0, 0))
				{
 DLGRESIZE:
					DlgResize(hDlg, true);  //true - т.к. нажали F5
					return true;
				}
				//----
				else if (IsNone(record) && Key==VK_F6 && Undo.iCount && !bError && !Info.SendDlgMessage(hDlg, DM_GETDROPDOWNOPENED, 0, 0))
				{
					Opt.srcCurCol=Opt.destCurCol=Opt.CurBorder=0;
					UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo=1);
					Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
					Info.SendDlgMessage(hDlg, DM_SETFOCUS, DlgREN, 0);
					return true;
				}
				//----
				else if (IsNone(record) && Key==VK_F8 && !Info.SendDlgMessage(hDlg, DM_GETDROPDOWNOPENED, 0, 0))
				{
 CLEARLOGREN:
					if (Undo.iCount && !Opt.LoadUndo && YesNoMsg(MClearLogTitle, MClearLogBody))
					{
						FreeUndo();
						int ItemFocus=Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);
						Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
						Info.SendDlgMessage(hDlg, DM_SETFOCUS, (ItemFocus!=DlgUNDO?ItemFocus:DlgREN), 0);
						return true;
					}
					return false;
				}
				//----
				else if (IsNone(record) && Key==VK_F12 && !Info.SendDlgMessage(hDlg, DM_GETDROPDOWNOPENED, 0, 0))
				{
					int ItemFocus=Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);
					if (ItemFocus!=DlgLIST) SaveItemFocus=ItemFocus;
					Info.SendDlgMessage(hDlg, DM_SETFOCUS, ItemFocus!=DlgLIST?DlgLIST:SaveItemFocus, 0);
					return true;
				}
				//----
				else if (IsNone(record) && Key==VK_INSERT)
				{
					if (Param1==DlgETEMPLNAME || Param1==DlgBTEMPLNAME)
					{
						if (SetMask(hDlg, DlgEMASKNAME, DlgETEMPLNAME))
						{
							if (Param1==DlgETEMPLNAME)
								Info.SendDlgMessage(hDlg, DM_SETFOCUS, DlgETEMPLNAME, 0);
							return true;
						}
					}
					else if (Param1==DlgETEMPLEXT || Param1==DlgBTEMPLEXT)
					{
						if (SetMask(hDlg, DlgEMASKEXT, DlgETEMPLEXT))
						{
							if (Param1==DlgETEMPLEXT)
								Info.SendDlgMessage(hDlg, DM_SETFOCUS, DlgETEMPLEXT, 0);
							return true;
						}
					}
				}
				//----
				else if (IsNone(record) && Key==VK_DELETE && Param1==DlgLIST)
				{
					int Pos=Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgLIST, 0);
					if (Pos>=(Opt.LoadUndo?Undo.iCount:FileList.Count())) return false;
					Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
					FarListPos ListPos={sizeof(FarListPos), Pos, -1};
					FarListDelete ListDel={sizeof(FarListDelete), Pos, 1};
					Info.SendDlgMessage(hDlg, DM_LISTDELETE, DlgLIST, &ListDel);

					if (Opt.LoadUndo)
					{
						do
						{
							Undo.CurFileName[Pos]=(wchar_t*)realloc(Undo.CurFileName[Pos],(wcslen(Undo.CurFileName[Pos+1])+1)*sizeof(wchar_t));
							Undo.OldFileName[Pos]=(wchar_t*)realloc(Undo.OldFileName[Pos],(wcslen(Undo.OldFileName[Pos+1])+1)*sizeof(wchar_t));
							wcscpy(Undo.CurFileName[Pos], Undo.CurFileName[Pos+1]);
							wcscpy(Undo.OldFileName[Pos], Undo.OldFileName[Pos+1]);
						} while(++Pos<Undo.iCount-1);
						free(Undo.CurFileName[Pos]); Undo.CurFileName[Pos]=0;
						free(Undo.OldFileName[Pos]); Undo.OldFileName[Pos]=0;
						--Undo.iCount;
					}
					else
					{
						File *cur=NULL; unsigned index;
						for (cur=FileList.First(), index=0; cur && index<Pos; cur=FileList.Next(cur), index++)
							;
						if (cur) FileList.Delete(cur);
					}

					Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, DlgLIST, &ListPos);
					UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo);
					Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
					return true;
				}
				//----
				else if ( Param1==DlgLIST && !Opt.LoadUndo && IsCtrl(record) && (Key==VK_UP || Key==VK_DOWN) )
				{
					FarListPos ListPos;
					ListPos.StructSize=sizeof(FarListPos);
					Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgLIST, &ListPos);
					if (ListPos.SelectPos>=FileList.Count()) 
						return false;
					bool bUp=(Key==VK_UP);
					if (bUp?ListPos.SelectPos>0:ListPos.SelectPos<FileList.Count()-1)
					{
						Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
						File add, *cur=NULL; unsigned index;
						for (cur=FileList.First(), index=0; cur && index<ListPos.SelectPos; cur=FileList.Next(cur), index++)
							;
						if (cur)
						{
							add=*cur;
							cur=FileList.Delete(cur);
							if (bUp)
								FileList.InsertBefore(cur,&add);
							else
							{
								cur=FileList.Next(cur);
								FileList.InsertAfter(cur,&add);
							}
						}
						for (int i=DlgEMASKNAME; i<=DlgEREPLACE; i++)
							switch(i)
							{
								case DlgEMASKNAME: case DlgEMASKEXT:
								case DlgESEARCH:   case DlgEREPLACE:
								{
									FarGetDialogItem FGDI;
									FGDI.StructSize=sizeof(FarGetDialogItem);
									FGDI.Item=(FarDialogItem *)malloc(FGDI.Size=Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0));
									if (FGDI.Item)
									{
										Info.SendDlgMessage(hDlg, DM_GETDLGITEM, i, &FGDI);
										Info.SendDlgMessage(hDlg, DN_EDITCHANGE, i, FGDI.Item);
										free(FGDI.Item);
										break;
									}
								}
							}
						bUp?ListPos.SelectPos--:ListPos.SelectPos++;
						Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, DlgLIST, &ListPos);
						Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
						return true;
					}
				}
				//----
				else if ( Param1==DlgLIST && IsCtrl(record) && (Key==VK_LEFT || Key==VK_RIGHT || Key==VK_NUMPAD5 || Key==VK_CLEAR) )
				{
					bool bLeft=(Key==VK_LEFT);
					int maxBorder=(DlgSize.Full?DlgSize.mW2:DlgSize.W2)-2-5;
					bool Ret=false;
					if (Key==VK_NUMPAD5 || Key==VK_CLEAR)
					{
						Opt.srcCurCol=Opt.destCurCol=Opt.CurBorder=0; Ret=true;
					}
					else if (bLeft?Opt.CurBorder>-maxBorder:Opt.CurBorder<maxBorder)
					{
						if (bLeft)
						{
							Opt.CurBorder-=1;
						}
						else
						{
							Opt.CurBorder+=1;
						}
						Ret=true;
					}
					if (Ret)
					{
						Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
						Ret=UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo);
						Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
						if (Ret) return true;
						else return false;
					}
				}
				//----
				else if ( Param1==DlgLIST && ((IsNone(record) && (Key==VK_LEFT || Key==VK_RIGHT)) || (IsAlt(record) && (Key==VK_LEFT || Key==VK_RIGHT))) )
				{
					bool Ret=false;
					if (IsNone(record))
					{
						int maxDestCol=Opt.lenDestFileName-((DlgSize.Full?DlgSize.mW2:DlgSize.W2)-2)+Opt.CurBorder;
						if (Opt.destCurCol>maxDestCol)
						{
							if (maxDestCol>0) Opt.destCurCol=maxDestCol;
							else Opt.destCurCol=0;
						}
						if (Key==VK_LEFT && Opt.destCurCol>0)
						{
							Opt.destCurCol-=1; Ret=true;
						}
						else if (Key==VK_RIGHT && Opt.destCurCol<maxDestCol)
						{
							Opt.destCurCol+=1; Ret=true;
						}
					}
					else
					{
						int maxSrcCol=Opt.lenFileName-((DlgSize.Full?DlgSize.mW2:DlgSize.W2)-2)-Opt.CurBorder;
						if (Opt.srcCurCol>maxSrcCol)
						{
							if (maxSrcCol>0) Opt.srcCurCol=maxSrcCol;
							else Opt.srcCurCol=0;
						}
						if (Key==VK_LEFT && Opt.srcCurCol>0)
						{
							Opt.srcCurCol-=1; Ret=true;
						}
						else if (Key==VK_RIGHT && Opt.srcCurCol<maxSrcCol)
						{
							Opt.srcCurCol+=1; Ret=true;
						}
					}
					if (Ret)
					{
						Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
						Ret=UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo);
						Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
					}
					return true;
				}
				//----
				else if ( Param1==DlgLIST && IsCtrl(record) && Key==VK_PRIOR )
				{
 GOTOFILE:
					int Pos=Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, DlgLIST, 0);
					if (Pos>=(Opt.LoadUndo?Undo.iCount:FileList.Count())) 
						return true;
					string name;
					size_t size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,0);
					if (size)
					{
						FarPanelDirectory *buf=(FarPanelDirectory*)malloc(size);
						if (buf)
						{
							buf->StructSize=sizeof(FarPanelDirectory);
							Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,size,buf);
							name=buf->Name;
							free(buf);
						}
					}
					if (Opt.LoadUndo && FSF.LStricmp(name.get(),Undo.Dir))
					{
						FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),Undo.Dir,NULL,{0},NULL};
						Info.PanelControl(PANEL_ACTIVE,FCTL_SETPANELDIRECTORY,0,&dirInfo);
					}

					PanelInfo PInfo;
					PInfo.StructSize=sizeof(PanelInfo);
					Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);
					PanelRedrawInfo RInfo;
					RInfo.CurrentItem=PInfo.CurrentItem;
					RInfo.TopPanelItem=PInfo.TopPanelItem;

					name.clear();
					if (Opt.LoadUndo)
						name=Undo.CurFileName[Pos];
					else
					{
						File *cur=NULL; unsigned index;
						for (cur=FileList.First(), index=0; cur && index<Pos; cur=FileList.Next(cur), index++)
							;
						if (cur) name=cur->strSrcFileName;
					}

					FarGetPluginPanelItem FGPPI;
					for (int i=0; i<PInfo.ItemsNumber; i++)
					{
						FGPPI.Size=0; FGPPI.Item=0;
						FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,i,&FGPPI));
						if (FGPPI.Item)
						{
							Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,i,&FGPPI);
							if (!FSF.LStricmp(name.get(),FGPPI.Item->FileName))
							{
								RInfo.CurrentItem=i;
								free(FGPPI.Item);
								break;
							}
							else
								free(FGPPI.Item);
						}
					}
					Info.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,&RInfo);
					Info.SendDlgMessage(hDlg,DM_CLOSE,DlgCANCEL,0);
					return true;
				}
				else if ( Param1==DlgLIST && (record->Event.KeyEvent.dwControlKeyState&ControlKeyCtrlMask) &&
									(record->Event.KeyEvent.dwControlKeyState&ControlKeyAltMask) && Key==0x46 /*VK_F*/ )
				{
					return true;
				}
			}
		}
		break;

	/************************************************************************/

		case DN_GOTFOCUS:
			{
				bool Ret=true;
				switch (Param1)
				{
					case DlgLIST:
						Focus=DlgLIST; break;
					default:
						Ret=false; break;
				}
				/* $ !!  skirda, 10.07.2007 1:27:03:
				*       в обработчике мыши после смены фокуса идет ShowDialog(-1)
				*       а в обработчике клавиатуры - ShowDialog(OldPos); ShowDialog(FocusPos);
				*
				*       AS,  т.е. для мыши Фар дает команду "DN_DRAWDIALOG"
				*            а для клавы "DN_DRAWDLGITEM"
				*       учтем этот нюанс для DN_CTLCOLORDLGITEM:
				*/
				Info.SendDlgMessage(hDlg, DM_SHOWITEM, DlgSEP2, (void*)1);
					/* $ */
				if (Ret) Info.SendDlgMessage(hDlg, DM_SETMOUSEEVENTNOTIFY, 1, 0);
			}
			break;

	/************************************************************************/

		case DN_INPUT:
			{
 RIGHTCLICK:
				const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
				if (record->EventType==MOUSE_EVENT)
				{
					if ( record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED &&
							 record->Event.MouseEvent.dwEventFlags==MOUSE_MOVED )
					{
						switch (Focus)
						{
							case DlgLIST:
								if (!Opt.LoadUndo)
									MouseDragDrop(hDlg,record->Event.MouseEvent.dwMousePosition.Y);
								return false;
						}
					}
					// обработка клика правой клавишей мыши в листе файлов
					else if (record->Event.MouseEvent.dwButtonState==RIGHTMOST_BUTTON_PRESSED && Focus==DlgLIST)
					{
						int Pos=ListMouseRightClick(hDlg, record->Event.MouseEvent.dwMousePosition.X, record->Event.MouseEvent.dwMousePosition.Y);
						if (Pos>=0)
						{
							ShowName(Pos);
							return false;
						}
					}
					return true;
				}
			}

	/************************************************************************/

		case DN_KILLFOCUS:
			{
				switch (Param1)
					case DlgLIST:
						Focus=-1;
						Info.SendDlgMessage(hDlg, DM_SETMOUSEEVENTNOTIFY, 0, 0);
						break;
			}
			break;

	/************************************************************************/

		case DN_EDITCHANGE:
			if (Param1==DlgEMASKNAME || Param1==DlgEMASKEXT ||
					Param1==DlgESEARCH || Param1==DlgEREPLACE )
			{
				if (Param1==DlgEMASKNAME)
					StrOpt.MaskName=((FarDialogItem *)Param2)->Data;
				else if (Param1==DlgEMASKEXT)
					StrOpt.MaskExt=((FarDialogItem *)Param2)->Data;
				else if (Param1==DlgESEARCH)
					StrOpt.Search=((FarDialogItem *)Param2)->Data;
				else if (Param1==DlgEREPLACE)
					StrOpt.Replace=((FarDialogItem *)Param2)->Data;

				if (ProcessFileName())
				{
					bError=false;
					Opt.destCurCol=0;
				}
				else bError=true;
				Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
				bool Ret=UpdateFarList(hDlg, DlgSize.Full, false);
				Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, true, 0);
				if (Ret) return true;
				else return false;
			}
			break;

	/************************************************************************/

		case DN_CTLCOLORDLGITEM:
			// !!! См. комментарии в DN_GOTFOCUS.
			// красим сепаратор над листбоксом...
			if (Param1==DlgSEP2)
			{
				FarColor Color;
				struct FarDialogItemColors *Colors=(FarDialogItemColors*)Param2;

				// ... если листбокс в фокусе, то красим в выделенный цвет
				if (DlgLIST==Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0))
					Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGHIGHLIGHTBOXTITLE,&Color);
				// ... иначе - в обычный цвет
				else
					Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGBOX,&Color);
				Colors->Colors[0]=Color;
			}
			break;

	/************************************************************************/

		case DN_CLOSE:
			if (Param1==DlgREN && Opt.LoadUndo)
			{
				const wchar_t *MsgItems[]={ GetMsg(MUndoTitle), GetMsg(MUndoBody) };
				switch (Info.Message(&MainGuid,&LoadUndoMsgGuid,FMSG_WARNING|FMSG_MB_YESNOCANCEL,0,MsgItems,2,0))
				{
					case 0:
						Opt.Undo=1; return true;
					case 1:
						UpdateFarList(hDlg, DlgSize.Full, Opt.LoadUndo=Opt.Undo=0);
						Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
						return false;
					default: return false;
				}
			}
			break;
	}

	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}


/****************************************************************************
 * Читает настройки из реестра, показывает диалог с опциями переименования,
 * заполняет структуру Opt, сохраняет (если надо) новые настройки в реестре.
 ****************************************************************************/
int VisRenDlg::ShowDialog()
{
	GetDlgSize();

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2	Selected	History	Mask	Flags	Data	MaxLen	UserParam
		/* 0*/{DI_DOUBLEBOX,0,          0,DlgSize.W-1,DlgSize.H-1, 0, 0,                0, 0, GetMsg(MVRenTitle), 0,0},
		/* 1*/{DI_TEXT,     DlgSize.W-4,0,DlgSize.W-2,          0, 0, 0,                0, 0, L"", 0,0},

		/* 2*/{DI_TEXT,     2,          1,          0,          0, 0, 0,                0, 0, GetMsg(MMaskName), 0,0},
		/* 3*/{DI_EDIT,     2,          2,DlgSize.WS-3,         0, 0, L"VisRenMaskName",0, DIF_USELASTHISTORY|DIF_HISTORY|DIF_FOCUS, L"", 0,0},
		/* 4*/{DI_TEXT,     2,          3,          0,          0, 0, 0,                0, 0, GetMsg(MTempl), 0,0},
		/* 5*/{DI_COMBOBOX, 2,          4,         31,          0, 0, 0, 0, DIF_LISTAUTOHIGHLIGHT|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE, GetMsg(MTempl_1), 0,0},
		/* 6*/{DI_BUTTON,  34,          4,          0,          0, 0, 0,                0, DIF_NOBRACKETS|DIF_BTNNOCLOSE, GetMsg(MSet), 0,0},

		/* 7*/{DI_TEXT,     DlgSize.WS, 1,          0,          0, 0, 0,                0, 0, GetMsg(MMaskExt), 0,0},
		/* 8*/{DI_EDIT,     DlgSize.WS, 2,DlgSize.W-3,          0, 0, L"VisRenMaskExt", 0, DIF_USELASTHISTORY|DIF_HISTORY, L"", 0,0},
		/* 9*/{DI_COMBOBOX, DlgSize.WS, 4,DlgSize.W-8,          0, 0, 0,  0,  DIF_LISTAUTOHIGHLIGHT|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE, GetMsg(MTempl2_1), 0,0},
		/*10*/{DI_BUTTON,   DlgSize.W-5,4,          0,          0, 0, 0,                0, DIF_NOBRACKETS|DIF_BTNNOCLOSE, GetMsg(MSet), 0,0},


		/*11*/{DI_TEXT,     0,          5,          0,          0, 0, 0,                0, DIF_SEPARATOR, L"", 0,0},
		/*12*/{DI_TEXT,     2,          6,         14,          0, 0, 0,                0, 0, GetMsg(MSearch), 0,0},
		/*13*/{DI_EDIT,    15,          6,DlgSize.WS-3,         0, 0, L"VisRenSearch",  0, DIF_HISTORY, L"", 0,0},
		/*14*/{DI_TEXT,     2,          7,         14,          0, 0, 0,                0, 0, GetMsg(MReplace), 0,0},
		/*15*/{DI_EDIT,    15,          7,DlgSize.WS-3,         0, 0, L"VisRenReplace", 0, DIF_HISTORY, L"", 0,0},
		/*16*/{DI_CHECKBOX, DlgSize.WS, 6,         19,          0, 1, 0,                0, 0, GetMsg(MCase), 0,0},
		/*17*/{DI_CHECKBOX, DlgSize.WS, 7,         19,          0, 0, 0,                0, 0, GetMsg(MRegEx), 0,0},

		/*18*/{DI_TEXT,     0,          8,          0,          0, 0, 0,                0, DIF_SEPARATOR, L"", 0,0},
		/*19*/{DI_LISTBOX,  2,          9,DlgSize.W-2,DlgSize.H-4, 0, 0,                0, DIF_LISTNOCLOSE|DIF_LISTNOBOX, L"", 0,0},
		/*20*/{DI_TEXT,     0,DlgSize.H-3,          0,          0, 0, 0,                0, DIF_SEPARATOR, L"", 0,0},

		/*21*/{DI_BUTTON,   0,DlgSize.H-2,          0,          0, 0, 0,                0, DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsg(MRen), 0,0},
		/*22*/{DI_BUTTON,   0,DlgSize.H-2,          0,          0, 0, 0,                0, DIF_BTNNOCLOSE|DIF_CENTERGROUP, GetMsg(MUndo), 0,0},
		/*23*/{DI_BUTTON,   0,DlgSize.H-2,          0,          0, 0, 0,                0, DIF_CENTERGROUP, GetMsg(MEdit), 0,0},
		/*24*/{DI_BUTTON,   0,DlgSize.H-2,          0,          0, 0, 0,                0, DIF_CENTERGROUP, GetMsg(MCancel), 0,0}
	};

	// комбинированный список с шаблонами
	FarListItem itemTempl1[26];
	int n = sizeof(itemTempl1) / sizeof(itemTempl1[0]);
	for (int i = 0; i < n; i++)
	{
		itemTempl1[i].Flags=((i==3 || i==9 || i==16 || i==21)?LIF_SEPARATOR:0);
		itemTempl1[i].Text=GetMsg(MTempl_1+i);
		itemTempl1[i].Reserved[0]=itemTempl1[i].Reserved[1]=itemTempl1[i].Reserved[2]=0;
	}
	FarList Templates1 = {n, itemTempl1};
	DialogItems[DlgETEMPLNAME].ListItems = &Templates1;

	FarListItem itemTempl2[8];
	n = sizeof(itemTempl2) / sizeof(itemTempl2[0]);
	for (int i = 0; i < n; i++)
	{
		itemTempl2[i].Flags=(i==3?LIF_SEPARATOR:0);
		itemTempl2[i].Text=GetMsg(MTempl2_1+i);
		itemTempl2[i].Reserved[0]=itemTempl2[i].Reserved[1]=itemTempl2[i].Reserved[2]=0;
	}
	FarList Templates2 = {n, itemTempl2};
	DialogItems[DlgETEMPLEXT].ListItems = &Templates2;

	HANDLE hDlg=Info.DialogInit( &MainGuid, &DlgGuid,
                               -1, -1, DlgSize.W, DlgSize.H,
                               L"Contents",
                               DialogItems,
                               sizeof(DialogItems) / sizeof(DialogItems[0]),
                               0, FDLG_SMALLDIALOG,
                               ShowDialogProcThunk,
                               this );

	int ExitCode=3;

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ExitCode=Info.DialogRun(hDlg);
		switch (ExitCode)
		{
			case DlgREN:
				Opt.UseLastHistory=1;
				ExitCode=0;
				break;
			case DlgEDIT:
				ExitCode=2;
				break;
			default:
				ExitCode=3;
				break;
		}
		Info.DialogFree(hDlg);
	}
	return ExitCode;
}