/****************************************************************************
 * HD_DMenu.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2003 Alexander Arefiev
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/

class TDirMenu:public TMenu
{
  struct FolderShortcuts {
    char Hotkey[2],
         Allias[512],
         Path[MAX_PATH],
         Macro[4096];
     int RunMacro;
    };
  int PathIsSet;
  FolderShortcuts *Shortcut;
  bool bPressAlt;
  int DlgPath(FolderShortcuts *Shortcut, int PressCtrl=0);
  void PostMacro(int Pos, int PressCtrl);
  void Init();
  LONG_PTR KeyPress(LONG_PTR Key, int Pos);
  bool MouseClick(int Bottom, int Pos);
  int Close(int Index);
};

LONG_PTR WINAPI DlgPathProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
  static char Path[MAX_PATH], CurPath[MAX_PATH];
  switch (Msg)
  {
    case DN_INITDIALOG:
      {
        int PressCtrl = Param2;
        GetCurrentDirectory(sizeof(Path), Path);

        PanelInfo pi;
        Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi);
        if (pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY &&
            FSF.LStricmp(pi.PanelItems[pi.CurrentItem].FindData.cFileName,".."))
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, 12, true);
          FSF.AddEndSlash(lstrcpy(CurPath, pi.CurDir));
          lstrcat(CurPath, pi.PanelItems[pi.CurrentItem].FindData.cFileName);
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, 12, false);
          lstrcpy(CurPath, Path);
        }

        if (PressCtrl==1)
          Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)Path);
        else if (PressCtrl==2)
          Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)CurPath);
        break;
      }
    case DN_BTNCLICK:
      if (Param1==11 && Param2==0)
      {
        Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)Path);
        Info.SendDlgMessage(hDlg, DM_SETFOCUS, 6, 0);
        return true;
      }
      else if (Param1==12 && Param2==0)
      {
        Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)CurPath);
        Info.SendDlgMessage(hDlg, DM_SETFOCUS, 6, 0);
        return true;
      }
  }

  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}


int TDirMenu::DlgPath(FolderShortcuts *Shortcut, int PressCtrl)
{

  if (lstrlen(Shortcut->Macro)>511)
  {
     const char *MsgItems[] = {
       GetMsg(MLength512Title),
       GetMsg(MLength512Body),
       GetMsg(MNo),
       GetMsg(MYes)
     };
     int Ret=Info.Message(Info.ModuleNumber, FMSG_WARNING, "EPath", MsgItems, COUNT(MsgItems), 2);
     if (Ret==-1 || !Ret) return false;
  }

  InitDialogItem InitItems[]={
/* 0*/{DI_DOUBLEBOX,   3, 1,72, 9, 0, 0, 0,               0,(char *)MShotrcutTitle},
/* 1*/{DI_TEXT,        5, 2,12, 0, 0, 0, 0,               0,(char *)MHotkey},
/* 2*/{DI_FIXEDIT,    13, 2,13, 0, 1, (int)"X", DIF_MASKEDIT, 0,""},
/* 3*/{DI_TEXT,       17, 2,27, 0, 0, 0, 0,               0,(char *)MAllias},
/* 4*/{DI_EDIT,       29, 2,70, 3, 0, 0, 0,               0,""},
/* 5*/{DI_TEXT,        5, 3, 0, 0, 0, 0, 0,               0,(char *)MPath},
/* 6*/{DI_EDIT,        5, 4,70, 6, 0, (int)"HotDirPath",DIF_HISTORY,0,""},
/* 7*/{DI_CHECKBOX,    5, 5,70, 6, 0, 0, DIF_3STATE,      0,(char *)MRunMacro},
/* 8*/{DI_EDIT,        5, 6,70, 6, 0, (int)"HotDirMacro",DIF_HISTORY,0,""},
/* 9*/{DI_TEXT,        0, 7, 0, 0, 0, 0, DIF_SEPARATOR,   0,""},
/*10*/{DI_BUTTON,      0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 1,(char *)MOK},
/*11*/{DI_BUTTON,      0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MCurrent},
/*12*/{DI_BUTTON,      0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MUnderCursor},
/*13*/{DI_BUTTON,      0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MCancel}
  };

  FarDialogItem Items[COUNT(InitItems)];
  memset(Items, 0, sizeof(Items));
  InitDialogItems(InitItems, Items, COUNT(InitItems));

  lstrcpy(Items[2].Data, Shortcut->Hotkey);
  lstrcpy(Items[4].Data, Shortcut->Allias);
  lstrcpy(Items[6].Data, Shortcut->Path);
  Items[7].Selected=Shortcut->RunMacro;
  lstrcpyn(Items[8].Data, Shortcut->Macro, 511);

  int Ret=Info.DialogEx(Info.ModuleNumber, -1,-1,76,11, "EPath", Items, COUNT(Items), 0, 0, DlgPathProc, (LONG_PTR)PressCtrl);
  if (Ret==-1 || Ret==13)  return false;

  FSF.Unquote(Items[6].Data);
  if (!*FSF.Trim(Items[6].Data)) return false;
  lstrcpyn(Shortcut->Path, Items[6].Data, MAX_PATH-1);
  lstrcpy(Shortcut->Hotkey, FSF.Trim(Items[2].Data));
  lstrcpy(Shortcut->Allias, FSF.Trim(Items[4].Data));
  if (*lstrcpy(Shortcut->Macro, FSF.Trim(Items[8].Data)))
    Shortcut->RunMacro=Items[7].Selected;
  else
    Shortcut->RunMacro=0;

  return true;
}

void TDirMenu::PostMacro(int Pos, int PressCtrl)
{
  if (Shortcut[Pos].RunMacro)
  {
    if (Shortcut[Pos].RunMacro==2)
    {
      const char *MsgItems[] = {
        GetMsg(MShotrcutTitle),
        GetMsg(MRunMacroBody),
        GetMsg(MYes),
        GetMsg(MNo)
      };
      int Ret=Info.Message(Info.ModuleNumber, 0, "EPath", MsgItems, COUNT(MsgItems), 2);
      if (Ret==-1 || Ret) return;
    }

    ActlKeyMacro command;
    command.Command=MCMD_POSTMACROSTRING;

    command.Param.PlainText.SequenceText=(char *)malloc(lstrlen(Shortcut[Pos].Macro)+10);
    if (command.Param.PlainText.SequenceText)
    {
      command.Param.PlainText.Flags=KSFLAGS_DISABLEOUTPUT;
      if (PressCtrl==2)
        FSF.sprintf(command.Param.PlainText.SequenceText, "%s %s %s", "Tab", Shortcut[Pos].Macro, "Tab");
      else
        lstrcpy(command.Param.PlainText.SequenceText, Shortcut[Pos].Macro);
      if (!Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &command))
        MessageBeep(MB_ICONHAND);
      free(command.Param.PlainText.SequenceText);
    }
  }
}

void TDirMenu::Init()
{
  Shortcut=0;
  PathIsSet=0;
  bPressAlt=false;
  unsigned i,j;

  // Загрузим элементы меню из реестра
  for (i=0, j=2; ; i++, j++)
  {
    Shortcut = (struct FolderShortcuts *) realloc(Shortcut, (j+1)*sizeof(*Shortcut));
    memset(&Shortcut[j], 0, sizeof(Shortcut[j]));
    if (HKEY hKey=CreateOrOpenRegKey(false, PluginRootKey, Title))
    {
      char curHotkey[15], curAllias[15], curPath[15], curMacro[15], curRunMacro[15];
      FSF.sprintf(curHotkey, "Hotkey%u", i);
      GetRegKey(hKey, curHotkey, Shortcut[j].Hotkey, sizeof(Shortcut[j].Hotkey));
      FSF.sprintf(curAllias, "Allias%u", i);
      GetRegKey(hKey, curAllias, Shortcut[j].Allias, sizeof(Shortcut[j].Allias));
      FSF.sprintf(curRunMacro, "RunMacro%u", i);
      GetRegKey(hKey, curRunMacro, (DWORD &)Shortcut[j].RunMacro);
      FSF.sprintf(curMacro, "Macro%u", i);
      GetRegKey(hKey, curMacro, Shortcut[j].Macro, sizeof(Shortcut[j].Macro));
      FSF.sprintf(curPath, "Path%u", i);
      if ( GetRegKey(hKey, curPath, Shortcut[j].Path, sizeof(Shortcut[j].Path))
           || Shortcut[j].Hotkey[0]=='-' && !*Shortcut[j].Allias)

      {
        RegCloseKey(hKey);
        continue;
      }
      RegCloseKey(hKey);
    }
    break;
  }

  InsItem(Count, GetMsg(MRootFolder));
  InsItem(Count, "", true);

  do
  {
    char Temp[MAX_PATH+10];
    if (Shortcut[Count].Hotkey[0]=='-' && !*Shortcut[Count].Allias)
      InsItem(Count, "", true);
    else
    {
      FSF.sprintf(Temp, "&%1.1s  %s", Shortcut[Count].Hotkey,
           (*Shortcut[Count].Allias ? Shortcut[Count].Allias : Shortcut[Count].Path));
      InsItem(Count, Temp);
    }
  } while (Count<=j);

  FarListPos flp={2, -1};
  Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);

  TMenu::Init();
}

bool TDirMenu::MouseClick(int Bottom, int Pos)
{
  bool bRet=false;
  char Path[MAX_PATH];
  if (Pos>1)
  {
    if (Pos<Count-1)
    {
      FSF.ExpandEnvironmentStr(Shortcut[Pos].Path, Path, sizeof(Path));
      if (Bottom==RIGHTMOST_BUTTON_PRESSED)
      {
        PathIsSet=2; bRet=true;
        Info.Control(INVALID_HANDLE_VALUE, FCTL_SETANOTHERPANELDIR, (void *)Path);
      }
      else if (Bottom==FROM_LEFT_1ST_BUTTON_PRESSED)
      {
        PathIsSet=1; bRet=true;
        Info.Control(INVALID_HANDLE_VALUE, FCTL_SETPANELDIR, (void *)Path);
      }
      else if (Bottom==FROM_LEFT_2ND_BUTTON_PRESSED)
      {
        *((LONG_PTR *)Param)=(KEY_CTRL|KEY_BACKSLASH);  bRet=true;
      }
    }
    else
      bRet=true;
  }
  else
  {
    if (Bottom==FROM_LEFT_1ST_BUTTON_PRESSED || Bottom==FROM_LEFT_2ND_BUTTON_PRESSED)
    {
      *((LONG_PTR *)Param)=(KEY_CTRL|KEY_BACKSLASH);  bRet=true;
    }
  }

  if (bRet)
  {
    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    return true;
  }

  return false;
}

LONG_PTR TDirMenu::KeyPress(LONG_PTR Key, int Pos)
{
  char Path[MAX_PATH], Temp[MAX_PATH+10];
  FolderShortcuts TempShortcut;
  DWORD k=Key&0xFFFFFF;
  if (k<244 && ((k>='!' && k<='п')||(k>='р' && k<='э')))
  {
    char h[2]={k,'\0'};
    for (int i=0; i<Count-1; i++)
    {
      if (!FSF.LStricmp(Shortcut[i].Hotkey, h))
      {
        Pos=i;
        Key=(Key&0xFF000000)|KEY_ENTER;
        break;
      }
    }
  }

  Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, false, 0);
  switch (Key)
  {
    /*case KEY_ALT:
      {
        FarListTitles flt;
        flt.Title=Title;
        if (!bPressAlt)
        {
          bPressAlt=true;
          flt.Bottom=0; // сбросить нижний заголовок
        }
        else
        {
          bPressAlt=false;
          flt.Bottom=Bottom;
        }
        Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,(LONG_PTR)&flt);
      }
      break; */
    case KEY_CTRL|KEY_INS:
    case KEY_RCTRL|KEY_INS:
    case KEY_INS:
    {
      memset(&TempShortcut, 0, sizeof(TempShortcut));
      if (Pos>1 && DlgPath(&TempShortcut, (Key==KEY_INS ? 1:2) ))
      {
        Shortcut = (struct FolderShortcuts *) realloc(Shortcut, (Count+1)*sizeof(*Shortcut));
        memset(&Shortcut[Count], 0, sizeof(Shortcut[Count]));
        for (int i=Count-1; i>Pos; i--)
          Shortcut[i]=Shortcut[i-1];
        Shortcut[Pos]=TempShortcut;
        if (Shortcut[Pos].Hotkey[0]=='-' && !*Shortcut[Pos].Allias)
        {
          InsItem(Pos, "", true);
          FarListPos flp={Pos+1, -1};
          Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
        }
        else
        {
          FSF.sprintf(Temp, "&%1.1s  %s", Shortcut[Pos].Hotkey,
               (*Shortcut[Pos].Allias ? Shortcut[Pos].Allias : Shortcut[Pos].Path));
          InsItem(Pos, Temp);
        }
      }
      break;
    }
    //-------------------
    case KEY_F4:
      if (Pos>1 && Pos<Count-1 && DlgPath(&Shortcut[Pos]))
      {
        DelItem(Pos);
        if (Shortcut[Pos].Hotkey[0]=='-' && !*Shortcut[Pos].Allias)
        {
          InsItem(Pos, "", true);
          FarListPos flp={Pos+1, -1};
          Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
        }
        else
        {
          FSF.sprintf(Temp, "&%1.1s  %s", Shortcut[Pos].Hotkey,
               (*Shortcut[Pos].Allias ? Shortcut[Pos].Allias : Shortcut[Pos].Path));
          InsItem(Pos, Temp);
        }
      }
      break;
    //-------------------
    case KEY_DEL:
      if (Pos>1 && Pos<Count-1)
      {
        const char *MsgItems[] = {
          GetMsg(MShotrcutTitle),
          GetMsg(MDelItem),
          GetMsg(MYes),
          GetMsg(MNo)
        };
        int Ret=Info.Message(Info.ModuleNumber, FMSG_WARNING, 0, MsgItems, COUNT(MsgItems), 2);
        if (Ret==-1 || Ret) break;
        DelItem(Pos);
        do
        {
          Shortcut[Pos]=Shortcut[Pos+1];
          Pos++;
        } while(Pos<Count);
        free(&Shortcut[Count]);
      }
      break;
    //-------------------
    case KEY_CTRL|KEY_DEL:
    case KEY_RCTRL|KEY_DEL:
      if (Pos>2 && Shortcut[Pos-1].Hotkey[0]=='-' && !*Shortcut[Pos-1].Allias)
      {
        const char *MsgItems[] = {
          GetMsg(MShotrcutTitle),
          GetMsg(MDelItem),
          GetMsg(MYes),
          GetMsg(MNo)
        };
        int Ret=Info.Message(Info.ModuleNumber, FMSG_WARNING, 0, MsgItems, COUNT(MsgItems), 2);
        if (Ret==-1 || Ret) break;
        DelItem(Pos-1);
        FarListPos flp={Pos-1, -1};
        Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
        do
        {
          Shortcut[Pos-1]=Shortcut[Pos];
          Pos++;
        } while(Pos<=Count);
        free(&Shortcut[Count]);
      }
      break;
    //-------------------
    case KEY_CTRL|KEY_UP:
    case KEY_RCTRL|KEY_UP:
      if (Pos>2 && Pos<Count-1)
      {
        TempShortcut=Shortcut[Pos-1];
        Shortcut[Pos-1]=Shortcut[Pos];
        Shortcut[Pos]=TempShortcut;
        DelItem(Pos-1);
        if (Shortcut[Pos].Hotkey[0]=='-' && !*Shortcut[Pos].Allias)
          InsItem(Pos, "", true);
        else
        {
          FSF.sprintf(Temp, "&%1.1s  %s", Shortcut[Pos].Hotkey,
               (*Shortcut[Pos].Allias ? Shortcut[Pos].Allias : Shortcut[Pos].Path));
          InsItem(Pos, Temp);
        }
        FarListPos flp={Pos-1, -1};
        Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
      }
      break;
    //-------------------
    case KEY_CTRL|KEY_DOWN:
    case KEY_RCTRL|KEY_DOWN:
      if (Pos>1 && Pos<Count-2)
      {
        TempShortcut=Shortcut[Pos+1];
        Shortcut[Pos+1]=Shortcut[Pos];
        Shortcut[Pos]=TempShortcut;
        DelItem(Pos+1);
        if (Shortcut[Pos].Hotkey[0]=='-' && !*Shortcut[Pos].Allias)
          InsItem(Pos, "", true);
        else
        {
          FSF.sprintf(Temp, "&%1.1s  %s", Shortcut[Pos].Hotkey,
               (*Shortcut[Pos].Allias ? Shortcut[Pos].Allias : Shortcut[Pos].Path));
          InsItem(Pos, Temp);
        }
        FarListPos flp={Pos+1, -1};
        Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
      }
      break;
    //-------------------
    case KEY_ENTER:
    case KEY_CTRL|KEY_ENTER:
    case KEY_RCTRL|KEY_ENTER:
    {
      FarListPos flp={Pos, -1};
      Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
      if (Pos>1)
      {
        if (Pos<Count-1)
        {
          FSF.ExpandEnvironmentStr(Shortcut[Pos].Path, Path, sizeof(Path));
          if (Info.Control(INVALID_HANDLE_VALUE, (Key==KEY_ENTER)?FCTL_SETPANELDIR:FCTL_SETANOTHERPANELDIR, (void *)Path))
            (Key==KEY_ENTER) ? PathIsSet=1 : PathIsSet=2;
        }
        Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
        return true;
      }
      else
        Key=KEY_BACKSLASH;
    }
    //-------------------
    case KEY_BACKSLASH:
    {
      FarListPos flp={0, -1};
      Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
    }
    case KEY_CTRL|KEY_BACKSLASH:
    case KEY_RCTRL|KEY_BACKSLASH:
      if (Pos>=Count-1) break;
    case KEY_LEFT:
    case KEY_RIGHT:
    case KEY_BS:
      *((LONG_PTR *)Param)=Key;
      Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
      return true;
    //-------------------
    case KEY_ESC:
      fexit=-1;
      Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
      return true;
    //-------------------
    default:
      Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, TRUE, 0);
      return false;
  }
  ResizeConsole();
  Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, TRUE, 0);
  Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
  return true;
}

int TDirMenu::Close(int Index)
{

  if (PathIsSet) PostMacro(Index, PathIsSet);
  DeleteRegKey(PluginRootKey, Title);
  for (int i=0, j=2; j<Count && (Shortcut[j].Hotkey[0]=='-' || *Shortcut[j].Path); i++, j++)
  {
    char curHotkey[15], curAllias[15], curPath[15], curMacro[15], curRunMacro[15];
    if (*Shortcut[j].Hotkey)
    {
      FSF.sprintf(curHotkey, "Hotkey%u", i);
      SetRegKey(PluginRootKey, Title, curHotkey, Shortcut[j].Hotkey);
      if (Shortcut[j].Hotkey[0]=='-' && !*Shortcut[j].Allias)
        continue;
    }
    if (*Shortcut[j].Allias)
    {
      FSF.sprintf(curAllias, "Allias%u", i);
      SetRegKey(PluginRootKey, Title, curAllias, Shortcut[j].Allias);
    }
    FSF.sprintf(curPath, "Path%u", i);
    SetRegKey(PluginRootKey, Title, curPath, Shortcut[j].Path);
    FSF.sprintf(curRunMacro, "RunMacro%u", i);
    SetRegKey(PluginRootKey, Title, curRunMacro, (DWORD)Shortcut[j].RunMacro);
    if (*Shortcut[j].Macro)
    {
      FSF.sprintf(curMacro, "Macro%u", i);
      SetRegKey(PluginRootKey, Title, curMacro, Shortcut[j].Macro);
    }
  }
  if (Shortcut) free(Shortcut);

  if (fexit==-1) Index=fexit;

  return Index;
}
