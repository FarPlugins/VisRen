/****************************************************************************
 * HD_UpMenu.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/

class TUpDirMenu:public TMenu
{
  char Path[MAX_PATH], PlugPath[MAX_PATH];
  char **flist, **plist;
  int fnum, pnum, *fpos;
  bool bShortcutPath;           // брать путь из панели (=0) или из "быстрого каталога" (=1)?
  bool bPlug;
  void Init();
  void GetString(int i, char *Dest, int DestLen);
  LONG_PTR KeyPress(LONG_PTR Key, int Pos);
  bool MouseClick(int Bottom, int Pos);
  void SetPath(bool bPressCtrl, int i);
  int GetArgv(const char *cmd, char ***argv, int **position = 0);
  bool FreeArgv(int argc, char **argv, int *pos = 0);
  int Close(int Index);
};

/*
 *  Author GetArgv() Sergey Oblomov, hoopoepg@mail.ru
 */
int TUpDirMenu::GetArgv(const char *cmd, char ***argv, int **position)
{
  int l = lstrlen(cmd);
  // settings for arguments vectors
  int *pos = (int*) malloc(l * sizeof(*pos));
  int *len = (int*) malloc(l * sizeof(*pos));
  int  i, num = 0;
  for (i = 0; i < l;)
  {
    // skip white space
    while (cmd[i]=='\\' && i < l)
      i++;
    if (i >= l)
      break;
    // get argument
    pos[num] = i;
    while (cmd[i]!='\\' && i < l)
    {
      if (cmd[i] == '"' )
      {
        i++;
        while (cmd[i] != '"' && i < l)
          i++;
        i++;
      }
      else
        i++;
    }
    len[num] = i - pos[num];
    num++;
  }

  if (!num)
  {
    free(pos);
    free(len);
    return 0;
  }

  *argv = (char**)malloc(num * sizeof(**argv));
  for (i = 0; i < num; i++)
  {
    (*argv)[i] = (char*)malloc((len[i] + 2) * sizeof(***argv));
    lstrcpyn((*argv)[i], cmd + pos[i], len[i] + 1);
  }

  free(len);
  if (!position)
    free(pos);
  else
    *position = pos;

  return num;
}

bool TUpDirMenu::FreeArgv(int argc, char **argv, int *pos)
{
  if (!argv)
    return false;

  for (int i = 0; i < argc; i++)
    free(argv[i]);
  free(argv);

  if (pos)
    free(pos);

  return true;
}

void TUpDirMenu::GetString(int i, char *Dest, int DestLen)
{
  *Dest='\0';
  if (HKEY hKey=CreateOrOpenRegKey(false, PluginRootKey, Title))
  {
    char curPath[15]; FSF.sprintf(curPath, "Path%u", i);
    GetRegKey(hKey, curPath, Dest, DestLen);
    RegCloseKey(hKey);
  }
}

void TUpDirMenu::SetPath(bool bPressCtrl, int i)
{
  if (i<fnum)
  {
    if (i==fnum-1 && !bPressCtrl && !bPlug) return;
    if (i<fnum-1) Path[fpos[i+1]]=0;
    char name[MAX_PATH]={'\0'};
    PanelInfo pi;
    PanelRedrawInfo ri={0,0};

    if (i==fnum-1 && bPressCtrl && !bPlug)
    {
      Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi);
      lstrcpy(name, pi.PanelItems[pi.CurrentItem].FindData.cFileName);
    }
    else
      lstrcpy(name, flist[i+1]);

    Info.Control(INVALID_HANDLE_VALUE, !bPressCtrl ?
                 FCTL_SETPANELDIR:FCTL_SETANOTHERPANELDIR, Path);

    if ( !Info.Control(INVALID_HANDLE_VALUE, !bPressCtrl ?
                       FCTL_GETPANELINFO:FCTL_GETANOTHERPANELINFO, &pi)
         || pi.PanelType != PTYPE_FILEPANEL )
      return;

    for (int j=0; j<pi.ItemsNumber; j++)
    {
      if (!FSF.LStricmp(pi.PanelItems[j].FindData.cFileName, name))
      {
        ri.CurrentItem=j;
        if (i==fnum-1 && bPressCtrl && !bPlug) ri.TopPanelItem=j;
        Info.Control(INVALID_HANDLE_VALUE, !bPressCtrl ?
                     FCTL_REDRAWPANEL:FCTL_REDRAWANOTHERPANEL, &ri);
        break;
      }
    }
  }
  else if (bPlug && !bPressCtrl)
  {
    if (i==pnum+fnum+1) return;

    KeySequence keys={KSFLAGS_DISABLEOUTPUT, pnum+fnum+1-i};
    keys.Sequence = (DWORD*)malloc(keys.Count * sizeof(*keys.Sequence));
    for (int j=0; j<keys.Count; j++)
      keys.Sequence[j]=KEY_CTRL|KEY_PGUP;

    Info.AdvControl( Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &keys );
    free(keys.Sequence);
  }
  return;
}

void TUpDirMenu::Init()
{
  flist=plist=0;
  fpos=0;
  pnum=0;
  char Temp[MAX_PATH];
  if (bShortcutPath=Param)
  {
    GetString(Param-2, Path, sizeof(Path));
    FSF.ExpandEnvironmentStr(Path, Path, sizeof(Path));
    if (fnum=GetArgv(Path, &flist, &fpos))
      for (int i=fnum-1; Count<fnum; i--)
      {
        if (i<=36)
          FSF.sprintf(Temp, "&%1.1c  %s",(i>=10 ? ('a'+(i-10)) : ('0'+i)), flist[Count]);
        else
          FSF.sprintf(Temp, "&%1.1s  %s", "", flist[Count]);
        InsItem(Count,Temp);
      }
    else
      InsItem(Count,"");
  }
  else
  {
    Path[0]=PlugPath[0]='\0';
    PanelInfo pi;
    Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELSHORTINFO, &pi);
    if (bPlug=pi.Plugin)
    { //  узнаем путь на ФАР-панели и плагиновой панели
      GetCurrentDirectory(sizeof(Path), Path);
      if (!FSF.LStrnicmp(pi.CurDir, "plugin manager:\\", 16))
        lstrcpy(PlugPath, pi.CurDir+16);
      else
        lstrcpy(PlugPath, pi.CurDir);
      pnum=GetArgv(PlugPath, &plist);
      //DebugMsg(PlugPath, "PlugPath",pnum);
    }
    else
      lstrcpy(Path, pi.CurDir);
    fnum=GetArgv(Path, &flist, &fpos);
    //DebugMsg(Path,"Path",fnum);
    // check for network folder
    if (Path[0]=='\\' && Path[1]=='\\')
    {
      FSF.sprintf(Temp, "\\\\%s\\%s", flist[0], flist[1]);
      flist[0]=(char*)realloc(flist[0], (lstrlen(Temp)+1) * sizeof(**flist));
      lstrcpy(flist[0], Temp);
      free(flist[1]);
      fnum--;
      for (int i=1; i<fnum; i++)
      {
        flist[i]=flist[i+1];
        fpos[i]=fpos[i+1];
      }
    }

    for (int i=(bPlug?pnum+fnum:fnum-1); Count<fnum; i--)
    {
      if (i<=36)
        FSF.sprintf(Temp, "&%1.1c  %s",(i>=10 ? ('a'+(i-10)) : ('0'+i)), flist[Count]);
      else
        FSF.sprintf(Temp, "&%1.1s  %s", "", flist[Count]);
      InsItem(Count,Temp);
    }
    if (bPlug)
    {
      InsItem(Count,"",true);
      if (pnum<=36)
        FSF.sprintf(Temp, "&%1.1c  %s",(pnum>=10 ? ('a'+(pnum-10)) : ('0'+pnum)), "\\");
      else
        FSF.sprintf(Temp, "&%1.1s  %s", "", "\\");
      InsItem(Count,Temp);

      for (int i=pnum-1, j=0; Count<pnum+fnum+2; i--, j++)
      {
        if (i<=36)
          FSF.sprintf(Temp, "&%1.1c  %s",(i>=10 ? ('a'+(i-10)) : ('0'+i)), plist[j]);
        else
          FSF.sprintf(Temp, "&%1.1s  %s", "", plist[j]);
        InsItem(Count,Temp);
      }
    }
  }

  FarListPos flp={Count-1, -1};
  Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);

  TMenu::Init();
}

bool TUpDirMenu::MouseClick(int Bottom, int Pos)
{
  if (Bottom==RIGHTMOST_BUTTON_PRESSED || Bottom==FROM_LEFT_1ST_BUTTON_PRESSED)
  {
    if (bShortcutPath)
    {
      if (Pos<fnum-1) Path[fpos[Pos+1]]=0;
      Info.Control(INVALID_HANDLE_VALUE, (Bottom==RIGHTMOST_BUTTON_PRESSED ?
                   FCTL_SETANOTHERPANELDIR : FCTL_SETPANELDIR), (void *)Path);
    }
    else
      SetPath(Bottom==RIGHTMOST_BUTTON_PRESSED, Pos);

    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    return true;
  }
  return false;
}

LONG_PTR TUpDirMenu::KeyPress(LONG_PTR Key, int Pos)
{
  DWORD k=Key&0xFFFFFF;
  if (k>='A' && k<='Z') k+=32;
  if (k<123 && ((k>='0' && k<='9')||(k>='a' && k<='z')))
  {
    char h[3]={k,'*','\0'};
    FarListFind flf={0, h, 0, 0};
    int Index=Info.SendDlgMessage(hDlg, DM_LISTFINDSTRING, 0, (LONG_PTR)&flf);
    if (Index!=-1)
    {
      Pos=Index;
      Key=(Key&0xFF000000)|KEY_ENTER;
    }
  }

  switch (Key)
  {
    case KEY_ENTER:
    case KEY_CTRL|KEY_ENTER:
    case KEY_RCTRL|KEY_ENTER:
    {
      if (bShortcutPath)
      {
        if (Pos<fnum-1)
          Path[fpos[Pos+1]] = 0;
        Info.Control(INVALID_HANDLE_VALUE, (Key==KEY_ENTER)?FCTL_SETPANELDIR:FCTL_SETANOTHERPANELDIR, (void *)Path);
        Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
        return true;
      }
      else
      {
        SetPath(Key!=KEY_ENTER, Pos);
        Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
        return true;
      }
      break;
    }
    //-------------------
    case KEY_ESC:
    case KEY_BS:
      fexit = -1;
      Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
      return true;
  }
  return false;
}

int TUpDirMenu::Close(int Index)
{
  if (fexit==-1)
    Index=fexit;
  FreeArgv(fnum, flist, fpos);
  FreeArgv(pnum, plist);

  return Index;
}
