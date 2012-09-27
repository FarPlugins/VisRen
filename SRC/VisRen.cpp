/****************************************************************************
 * VisRen.cpp
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

#include "VisRen.hpp"
#include "VisRenDlg.hpp"

/****************************************************************************
 * ����� ����������� �������� FAR
 ****************************************************************************/
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

/****************************************************************************
 * ����� ����������
 ****************************************************************************/
struct Options Opt;         //������� ��������� �������
struct DlgSize DlgSize;     //������ �������
struct UndoFileName Undo;   //Undo ��������������


/****************************************************************************
 * ������� ��������� Undo ��������������
 ****************************************************************************/
void FreeUndo()
{
	for (int i=Undo.iCount-1; i>=0; i--)
	{
		free(Undo.CurFileName[i]);
		free(Undo.OldFileName[i]);
	}
	free(Undo.CurFileName); Undo.CurFileName=0;
	free(Undo.OldFileName); Undo.OldFileName=0;
	free(Undo.Dir); Undo.Dir=0;
	Undo.iCount=0;
}

/****************************************************************************
 * ������ ��������� ������� FAR: ��������� ������ �� .lng-�����
 ****************************************************************************/
const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid, MsgId);
}

/****************************************************************************
 * ����� ��������������-������ � ���������� � ����� ��������
 ****************************************************************************/
void ErrorMsg(DWORD Title, DWORD Body)
{
	const wchar_t *MsgItems[]={ GetMsg(Title), GetMsg(Body), GetMsg(MOK) };
	Info.Message(&MainGuid,&ErrorMsgGuid,FMSG_WARNING,0,MsgItems,3,1);
}

/****************************************************************************
 * ����� �������������� "Yes-No" � ���������� � ����� ��������
 ****************************************************************************/
bool YesNoMsg(DWORD Title, DWORD Body)
{
	const wchar_t *MsgItems[]={ GetMsg(Title), GetMsg(Body) };
	return (!Info.Message(&MainGuid,&YesNoMsgGuid,FMSG_WARNING|FMSG_MB_YESNO,0,MsgItems,2,0));
}

// ��������� ��� �������
int DebugMsg(wchar_t *msg, wchar_t *msg2, unsigned int i)
{
  wchar_t *MsgItems[] = {L"DebugMsg", L"", L"", L""};
  wchar_t buf[80]; FSF.itoa(i, buf,10);
  MsgItems[1] = msg2;
  MsgItems[2] = msg;
  MsgItems[3] = buf;
  return (!Info.Message(&MainGuid,&DebugMsgGuid,FMSG_WARNING|FMSG_MB_OKCANCEL,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2));
}

__int64 GetFarSetting(FARSETTINGS_SUBFOLDERS Root,const wchar_t* Name)
{
	__int64 result=0;
	FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
	HANDLE Settings=Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
	if (Settings)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_UNKNOWN,{0}};
		if(Info.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_QWORD==item.Type)
		{
			result=item.Number;
		}
		Info.SettingsControl(Settings,SCTL_FREE,0,0);
	}
	return result;
}


/****************************************************************************
 ***************************** Exported functions ***************************
 ****************************************************************************/

/****************************************************************************
 * ��� ������� ������� FAR �������� � ������ �������
 ****************************************************************************/
void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=MAKEFARVERSION(3,0,0,15,VS_RC);
	Info->Guid=MainGuid;
	Info->Title=L"VisRen";
	Info->Description=L"Visual renaming files plugin for Far Manager v3.0";
	Info->Author=L"Alexey Samlyukov";
}

// �������� ��������� PluginStartupInfo � ������� ��� �������� ��������...
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info = *Info;
	if (Info->StructSize >= sizeof(PluginStartupInfo))
	{
		FSF = *Info->FSF;
		::Info.FSF = &FSF;
		// ������� ��������� ��� �������� ������
		memset(&Undo, 0, sizeof(Undo));
	}
}

/****************************************************************************
 * ��� ������� ������� FAR �������� �� ������ ������� - �������� PluginInfo, �.�.
 * ������ FAR� ����� ������ �������� � "Plugin commands" � "Plugins configuration".
 ****************************************************************************/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MVRenTitle);

	Info->StructSize=sizeof(PluginInfo);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}

/****************************************************************************
 * �������� ������� �������. FAR � ��������, ����� ������������ ���� ������
 ****************************************************************************/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	HANDLE hPanel = NULL;
	struct PanelInfo PInfo;
	PInfo.StructSize=sizeof(PanelInfo);

	// ���� �� ������� ��������� ���������� � ������...
	if (!Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo))
		return hPanel;

	if (PInfo.PanelType!=PTYPE_FILEPANEL || (PInfo.Flags&PFLAGS_PLUGIN) || !PInfo.ItemsNumber || !PInfo.SelectedItemsNumber)
	{
		ErrorMsg(MVRenTitle, MFilePanelsRequired);
		return hPanel;
	}

	class VisRenDlg RenDlg;
	RenDlg.InitFileList(PInfo.SelectedItemsNumber);

	switch (RenDlg.ShowDialog())
	{
		case 0:
			RenDlg.RenameFile(PInfo.SelectedItemsNumber, PInfo.ItemsNumber); break;
		case 2:
			RenDlg.RenameInEditor(PInfo.SelectedItemsNumber, PInfo.ItemsNumber); break;
	}

	return hPanel;
}

/****************************************************************************
 * ��� ������� FAR �������� ����� ��������� �������
 ****************************************************************************/
void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	//��������� ������ � ������ �������� �������
	FreeUndo();
}
