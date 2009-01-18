/****************************************************************************
 * HD_UpMenu.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyrigth (c) 2007 Alexey Samlyukov
 ****************************************************************************/

class TUpDirMenu:public TMenu
{
  char Path[MAX_PATH];
  char **flist;
  int *fpos, fnum;

  void Init();
  void GetString(int i, char *Dest, int DestLen);
  LONG_PTR KeyPress(LONG_PTR Key, int Pos);
  bool MouseClick(int Bottom, int Pos);
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

void TUpDirMenu::Init()
{
  GetString(Param, Path, sizeof(Path));
  FSF.ExpandEnvironmentStr(Path, Path, sizeof(Path));
  flist = 0;
  fpos  = 0;
  fnum  = GetArgv(Path, &flist, &fpos);

  for (int i=fnum-1; Count<fnum; i--)
  {
    char Temp[MAX_PATH];
    if (i<=36)
      FSF.sprintf(Temp, "&%1.1c  %s",(i>=10 ? ('a'+(i-10)) : ('0'+i)), flist[Count]);
    else
      FSF.sprintf(Temp, "&%1.1s  %s", "", flist[Count]);
    InsItem(Count,Temp);
  }

  FarListPos flp={Count-1, -1};
  Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);

  TMenu::Init();
}

bool TUpDirMenu::MouseClick(int Bottom, int Pos)
{
  if (Pos < fnum-1)
    Path[fpos[Pos+1]] = 0;
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
      if (Pos < fnum-1)
        Path[fpos[Pos+1]] = 0;
      Info.Control(INVALID_HANDLE_VALUE, (Key==KEY_ENTER)?FCTL_SETPANELDIR:FCTL_SETANOTHERPANELDIR, (void *)Path);
      Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
      return true;
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
  return Index;
}
