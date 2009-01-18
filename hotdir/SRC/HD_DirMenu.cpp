/****************************************************************************
 * HD_DMenu.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2003 Alexander Arefiev
 * Copyrigth (c) 2007 Alexey Samlyukov
 ****************************************************************************/

class TDirMenu:public TMenu
{
  struct FolderShortcuts {
    char Hotkey[2],
         Allias[512],
         Path[MAX_PATH];
    };
  FolderShortcuts *Shortcut;

  int DlgPath(char *Hotkey, char *Allias, char *Path, int Ins=0);
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
        int Ins = Param2;
        GetCurrentDirectory(sizeof(Path), Path);

        PanelInfo pi;
        Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi);
        if (pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY
            && pi.CurrentItem)
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, 10, true);
          FSF.AddEndSlash(lstrcpy(CurPath, pi.CurDir));
          lstrcat(CurPath, pi.PanelItems[pi.CurrentItem].FindData.cFileName);
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, 10, false);
          lstrcpy(CurPath, Path);
        }

        if (Ins==1)
          Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)Path);
        else if (Ins==2)
          Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)CurPath);
        break;
      }
    case DN_BTNCLICK:
      if (Param1 == 9 && Param2 == 0)
      {
        Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)Path);
        Info.SendDlgMessage(hDlg, DM_SETFOCUS, 6, 0);
        return true;
      }
      if (Param1 == 10 && Param2 == 0)
      {
        Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, 6, (LONG_PTR)CurPath);
        Info.SendDlgMessage(hDlg, DM_SETFOCUS, 6, 0);
        return true;
      }
  }

  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}


int TDirMenu::DlgPath(char *Hotkey, char *Allias, char *Path, int Ins)
{
  InitDialogItem InitItems[]={
/* 0*/{DI_DOUBLEBOX,   3, 1,72, 7, 0, 0, 0,               0,(char *)MShotrcutTitle},
/* 1*/{DI_TEXT,        5, 2,12, 0, 0, 0, 0,               0,(char *)MHotkey},
/* 2*/{DI_FIXEDIT,    13, 2,13, 0, 1, (int)"X", DIF_MASKEDIT, 0,""},
/* 3*/{DI_TEXT,       17, 2,27, 0, 0, 0, 0,               0,(char *)MAllias},
/* 4*/{DI_EDIT,       29, 2,70, 3, 0, 0, 0,               0,""},
/* 5*/{DI_TEXT,        5, 3, 0, 0, 0, 0, 0,               0,(char *)MPath},
/* 6*/{DI_EDIT,        5, 4,70, 6, 0, 0, 0,               0,""},
/* 7*/{DI_TEXT,        5, 5, 0, 0, 0, 0, DIF_SEPARATOR,   0,""},
/* 8*/{DI_BUTTON,      0, 6, 0, 0, 0, 0, DIF_CENTERGROUP, 1,(char *)MOK},
/* 9*/{DI_BUTTON,      0, 6, 0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MCurrent},
/*10*/{DI_BUTTON,      0, 6, 0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MUnderCursor},
/*11*/{DI_BUTTON,      0, 6, 0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MCancel}
  };

  FarDialogItem Items[COUNT(InitItems)];
  InitDialogItems(InitItems, Items, COUNT(InitItems));

  lstrcpy(Items[2].Data, Hotkey);
  lstrcpy(Items[4].Data, Allias);
  lstrcpy(Items[6].Data, Path);

  int Ret=Info.DialogEx(Info.ModuleNumber, -1,-1,76,9, "EPath", Items, COUNT(Items), 0, 0, DlgPathProc, (LONG_PTR)Ins);
  if (Ret==-1 || Ret==11)  return false;

  lstrcpy(Hotkey, Items[2].Data);
  lstrcpy(Allias, Items[4].Data);
  lstrcpy(Path, Items[6].Data);

  return true;
}

void TDirMenu::Init()
{
  Shortcut=0;
  unsigned i;

  // Загрузим элементы меню из реестра
  for (i=0; ; i++)
  {
    Shortcut = (struct FolderShortcuts *) realloc(Shortcut, (i+1)*sizeof(*Shortcut));
    memset(&Shortcut[i], 0, sizeof(Shortcut[i]));
    bool Ret=true;
    if (HKEY hKey=CreateOrOpenRegKey(false, PluginRootKey, Title))
    {
      char curHotkey[15], curAllias[15], curPath[15];
      FSF.sprintf(curHotkey, "Hotkey%u", i);
      GetRegKey(hKey, curHotkey, Shortcut[i].Hotkey, sizeof(Shortcut[i].Hotkey));
      FSF.sprintf(curAllias, "Allias%u", i);
      GetRegKey(hKey, curAllias, Shortcut[i].Allias, sizeof(Shortcut[i].Allias));
      FSF.sprintf(curPath, "Path%u", i);
      if (GetRegKey(hKey, curPath, Shortcut[i].Path, sizeof(Shortcut[i].Path)))
        Ret=false;
      RegCloseKey(hKey);
    }
    if (Ret) break;
  }

  do
  {
    char Temp[MAX_PATH+10];
    FSF.sprintf(Temp, "&%1.1s  %s", Shortcut[Count].Hotkey,
         (*Shortcut[Count].Allias ? Shortcut[Count].Allias : Shortcut[Count].Path));
    InsItem(Count, Temp);
  } while (Count<=i);
  TMenu::Init();
}

bool TDirMenu::MouseClick(int Bottom, int Pos)
{
  char Path[MAX_PATH];
  FSF.ExpandEnvironmentStr(Shortcut[Pos].Path, Path, sizeof(Path));
  if (Bottom==RIGHTMOST_BUTTON_PRESSED)
  {
    Info.Control(INVALID_HANDLE_VALUE, FCTL_SETANOTHERPANELDIR, (void *)Path);
    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    return true;
  }
  else if (Bottom==FROM_LEFT_1ST_BUTTON_PRESSED)
  {
    Info.Control(INVALID_HANDLE_VALUE, FCTL_SETPANELDIR, (void *)Path);
    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    return true;
  }
  else if (Bottom==FROM_LEFT_2ND_BUTTON_PRESSED && Pos<Count-1)
  {
    *((LONG_PTR *)Param)=(KEY_CTRL|KEY_BACKSLASH);
    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    return true;
  }
  return false;
}

LONG_PTR TDirMenu::KeyPress(LONG_PTR Key, int Pos)
{
  char Path[MAX_PATH], Temp[MAX_PATH+10], Allias[512], Hotkey[2];
  DWORD k=Key&0xFFFFFF;
  if (k<244 && (k>='!' && k<='п') || (k>='р' && k<='э'))
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
    case KEY_CTRL|KEY_INS:
    case KEY_RCTRL|KEY_INS:
    case KEY_INS:
    {
      int Ins=2;
      if (Key==KEY_INS) Ins=1;
      lstrcpy(Hotkey, "");
      lstrcpy(Allias, "");
      if (DlgPath(Hotkey, Allias, Path, Ins))
      {
        Shortcut = (struct FolderShortcuts *) realloc(Shortcut, (Count+1)*sizeof(*Shortcut));
        memset(&Shortcut[Count], 0, sizeof(Shortcut[Count]));
        for (int i=Count-1; i>Pos; i--)
          Shortcut[i]=Shortcut[i-1];
        lstrcpy(Shortcut[Pos].Hotkey, Hotkey);
        lstrcpy(Shortcut[Pos].Allias, FSF.Trim(Allias));
        FSF.Unquote(Path);
        lstrcpy(Shortcut[Pos].Path, FSF.Trim(Path));
        FSF.sprintf(Temp, "&%1.1s  %s", Hotkey, (*Allias ? Allias : Path));
        InsItem(Pos, Temp);
      }
      break;
    }
    //-------------------
    case KEY_F4:
    {
      if (Pos<Count-1)
      {
        lstrcpy(Hotkey, Shortcut[Pos].Hotkey);
        lstrcpy(Allias, Shortcut[Pos].Allias);
        lstrcpy(Path, Shortcut[Pos].Path);
        if (DlgPath(Hotkey, Allias, Path))
        {
          DelItem(Pos);
          lstrcpy(Shortcut[Pos].Hotkey, Hotkey);
          lstrcpy(Shortcut[Pos].Allias, FSF.Trim(Allias));
          lstrcpy(Shortcut[Pos].Path, FSF.Trim(Path));
          FSF.sprintf(Temp, "&%1.1s  %s", Hotkey, (*Allias ? Allias : Path));
          InsItem(Pos, Temp);
        }
      }
      break;
    }
    //-------------------
    case KEY_DEL:
      if (Pos<Count-1)
      {
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
    case KEY_CTRL|KEY_UP:
    case KEY_RCTRL|KEY_UP:
      if (Pos<Count-1 && Pos)
      {
        FolderShortcuts TempShortcut=Shortcut[Pos-1];
        Shortcut[Pos-1]=Shortcut[Pos];
        Shortcut[Pos]=TempShortcut;
        DelItem(Pos-1);
        FSF.sprintf(Temp, "&%1.1s  %s", TempShortcut.Hotkey,
              (*TempShortcut.Allias ? TempShortcut.Allias : TempShortcut.Path));
        InsItem(Pos, Temp);
        FarListPos flp={Pos-1, -1};
        Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
      }
      break;
    //-------------------
    case KEY_CTRL|KEY_DOWN:
    case KEY_RCTRL|KEY_DOWN:
      if (Pos<Count-2)
      {
        FolderShortcuts TempShortcut=Shortcut[Pos+1];
        Shortcut[Pos+1]=Shortcut[Pos];
        Shortcut[Pos]=TempShortcut;
        DelItem(Pos+1);
        FSF.sprintf(Temp, "&%1.1s  %s", TempShortcut.Hotkey,
              (*TempShortcut.Allias ? TempShortcut.Allias : TempShortcut.Path));
        InsItem(Pos, Temp);
        FarListPos flp={Pos+1, -1};
        Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
      }
      break;
    //-------------------
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
    case KEY_ENTER:
    case KEY_CTRL|KEY_ENTER:
    case KEY_RCTRL|KEY_ENTER:
    {
      FSF.ExpandEnvironmentStr(Shortcut[Pos].Path, Path, sizeof(Path));
      Info.Control(INVALID_HANDLE_VALUE, (Key==KEY_ENTER)?FCTL_SETPANELDIR:FCTL_SETANOTHERPANELDIR, (void *)Path);
      Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
      return true;
    }
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
  DeleteRegKey(PluginRootKey, Title);
  for (int i=0; i<Count && *Shortcut[i].Path; i++)
  {
    char curHotkey[15], curAllias[15], curPath[15];
    FSF.sprintf(curHotkey, "Hotkey%u", i);
    SetRegKey(PluginRootKey, Title, curHotkey, Shortcut[i].Hotkey);
    FSF.sprintf(curAllias, "Allias%u", i);
    SetRegKey(PluginRootKey, Title, curAllias, Shortcut[i].Allias);
    FSF.sprintf(curPath, "Path%u", i);
    SetRegKey(PluginRootKey, Title, curPath, Shortcut[i].Path);
  }
  if (Shortcut) free(Shortcut);

  if (fexit==-1) Index=fexit;

  return Index;
}
