/****************************************************************************
 * HD_HkMenu.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2003 Alexander Arefiev
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/

class THotKeyMenu:public TMenu
{
  char SectionMenu[10][15];
  void Init();
  LONG_PTR KeyPress(LONG_PTR Key, int Pos);
public:
  int Close(int Index);
};

void THotKeyMenu::Init()
{
  for ( ; Count<10; Count++)
  {
    FSF.sprintf(SectionMenu[Count], "RightCtrl-&%u", Count);
    Info.SendDlgMessage(hDlg, DM_LISTADDSTR, 0, (LONG_PTR)SectionMenu[Count]);
  }

  TMenu::Init();
}

LONG_PTR THotKeyMenu::KeyPress(LONG_PTR Key, int Pos)
{
  DWORD k=Key&0xFFFFFF;
  if (k>='0' && k<='9')
  {
    Pos=k-'0';
    FarListPos flp={Pos, -1};
    Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&flp);
    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    return true;
  }
  if (Key == KEY_ESC)
  {
    Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
    fexit=-1;
    return true;
  }
  return false;
}

int THotKeyMenu::Close(int Index)
{
  if (Index<0)       // KEY_LEFT
    Index=Count-1;
  if (Index>=Count)  // KEY_RIGHT
    Index=0;
  lstrcpy((char *)Param, SectionMenu[Index]);

  if (fexit==-1)
    Index=fexit;
  return Index;
}
