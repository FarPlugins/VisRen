/****************************************************************************
 * HD_Menu.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2003 Alexander Arefiev
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/

class TMenu
{
protected:
  HANDLE hDlg;
  LONG_PTR Param;
  char *Title, *Bottom;
  int Count;
  int fexit;
  COORD Console;

  BYTE ColorDlgList[10];
  BYTE ColorDialog;

  static LONG_PTR WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
  void ResizeConsole();
  void InsItem(int Index, char *Text, bool bSeparator=false);
  void DelItem(int Index);

  virtual void Init();
  //GCC не любит чистых виртуальных функций (+7Kb к размеру плагина)
  virtual LONG_PTR KeyPress(LONG_PTR Key, int Pos) { return FALSE; }
  virtual bool MouseClick(int Bottom, int Pos) { return FALSE; }
  virtual int Close(int Index) { return FALSE; }
public:
  int Run(char *Title, char *Bottom, LONG_PTR Param);
};

LONG_PTR WINAPI TMenu::DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
  static TMenu *This;
  switch (Msg)
  {
    case DN_INITDIALOG:
      This=(TMenu *)Param2;
      This->hDlg=hDlg;
      This->Init();
      return TRUE;
    case DN_CTLCOLORDIALOG:
      return This->ColorDialog;
    case DN_CTLCOLORDLGLIST:
    {
      FarListColors *flc=(FarListColors *)Param2;
      flc->Flags=flc->Reserved=0;
      flc->ColorCount=sizeof(This->ColorDlgList);
      flc->Colors=This->ColorDlgList;
      return true;
    }
    case DN_KEY:
    {
      FarListPos flp;
      Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&flp);
      return This->KeyPress(Param2, flp.SelectPos);
    }
    case DN_MOUSECLICK:
    {
      FarListPos flp;
      Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&flp);
        return This->MouseClick(((MOUSE_EVENT_RECORD *)Param2)->dwButtonState, flp.SelectPos);
      return false;
    }
    case DN_RESIZECONSOLE:
      This->Console=*((COORD *)Param2);
      This->ResizeConsole();
      return true;
  }
  return Info.DefDlgProc(hDlg, Msg, Param1,Param2);
}

void TMenu::ResizeConsole()
{
  int MaxItemWidth = max(lstrlen(Title), lstrlen(Bottom));
  if (MaxItemWidth) MaxItemWidth--;
  FarListGetItem List;
  for (List.ItemIndex=0; List.ItemIndex<Count; List.ItemIndex++)
  {
    Info.SendDlgMessage(hDlg, DM_LISTGETITEM, 0, (LONG_PTR)&List);
    MaxItemWidth = max(lstrlen(List.Item.Text), MaxItemWidth);
  }
  int Height = min(Console.Y-5-4, Count);
  int Width  = min(Console.X-3-6, MaxItemWidth);
  FarDialogItem Item;
  Info.SendDlgMessage(hDlg, DM_GETDLGITEM, 0, (LONG_PTR)&Item);
  Item.X2 = Width+3+3;
  Item.Y2 = Height+2;
  if (Count!=Height)
    Item.Flags &= ~DIF_LISTWRAPMODE;
  else
    Item.Flags |= DIF_LISTWRAPMODE;
  Info.SendDlgMessage(hDlg, DM_SETDLGITEM, 0, (LONG_PTR)&Item);
  COORD c;
  c.X = Width+3+6;
  c.Y = Height+4;
  Info.SendDlgMessage(hDlg, DM_RESIZEDIALOG, 0, (LONG_PTR)&c);
  c.X = c.Y = -1;
  Info.SendDlgMessage(hDlg, DM_MOVEDIALOG, true, (LONG_PTR)&c);
}

void TMenu::InsItem(int Index, char *Text, bool bSeparator)
{
  FarListInsert fli;
  fli.Index = Index;
  FarListItem &Item = fli.Item;
  memset(&Item, 0, sizeof(Item));
  FSF.TruncPathStr(Text, min(Console.X-8, 127));
  lstrcpy(fli.Item.Text, Text);
  if (bSeparator) fli.Item.Flags=LIF_SEPARATOR;
  Info.SendDlgMessage(hDlg, DM_LISTINSERT, 0, (LONG_PTR)&fli);
  Count++;
}

void TMenu::DelItem(int Index)
{
  FarListDelete fld = {Index, 1};
  Info.SendDlgMessage(hDlg, DM_LISTDELETE, 0, (LONG_PTR)&fld);
  Count--;
}

int TMenu::Run(char *Title, char *Bottom, LONG_PTR Param)
{
  this->Param=Param;
  this->Title=Title;
  this->Bottom=Bottom;
  Count=0;
  fexit=0;
  //взять размеры консольного окна
  HANDLE hConOut = CreateFile("CONOUT$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  CONSOLE_SCREEN_BUFFER_INFO csbiNfo;
  if (GetConsoleScreenBufferInfo(hConOut, &csbiNfo))
  {
    Console.X=csbiNfo.dwSize.X;
    Console.Y=csbiNfo.dwSize.Y;
  }
  else
  {
    Console.X=80;
    Console.Y=25;
  }
  CloseHandle(hConOut);

  InitDialogItem InitItems[]= {
    {DI_LISTBOX, 2,1,0,0,0,0,DIF_LISTNOAMPERSAND,0,0},
  };
  FarDialogItem Items[COUNT(InitItems)];
  memset(Items, 0, sizeof(Items));
  InitDialogItems(InitItems, Items, COUNT(InitItems));

  int Ret=Info.DialogEx(Info.ModuleNumber, -1,-1, 0, 0, "Contents", Items, COUNT(Items), 0, 0, DlgProc, (LONG_PTR)this);
//  if (Ret!=-1)
  {
    Ret=Items[0].ListPos;
    Ret=Close(Ret);
  }
  return Ret;
}

void TMenu::Init()
{
  FarListTitles flt;
  flt.Title=Title;
  flt.Bottom=Bottom;
  Info.SendDlgMessage(hDlg, DM_LISTSETTITLES, 0, (LONG_PTR)&flt);

  BYTE ColorIndex[]=
  {
    COL_MENUBOX,
    COL_MENUBOX,
    COL_MENUTITLE,
    COL_MENUTEXT,
    COL_MENUHIGHLIGHT,
    COL_MENUBOX,
    COL_MENUSELECTEDTEXT,
    COL_MENUSELECTEDHIGHLIGHT,
    COL_MENUSCROLLBAR,
    COL_MENUDISABLEDTEXT
  };
  for (int i=0; i<min(COUNT(ColorIndex), COUNT(ColorDlgList)); i++)
    ColorDlgList[i]=Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)ColorIndex[i]);

  //диалог красим цветом рамки меню
  ColorDialog=ColorDlgList[1];

  ResizeConsole();
}
