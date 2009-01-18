/****************************************************************************
 * HotDir.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2003 Alexander Arefiev
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/

/* Current developer: samlyukov<at>gmail.com  */

/* $ Revision: 07.1 $ */

//#define _FAR_NO_NAMELESS_UNIONS
#define _FAR_USE_FARFINDDATA
#define COUNT(Msg) (sizeof(Msg)/sizeof(*Msg))

#include "..\..\plugin.hpp"
#include "..\..\farkeys.hpp"
#include "..\..\farcolor.hpp"

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
static char   PluginRootKey[NM];
static bool bOldFAR = false;


/****************************************************************************
 * Нужны для отключения генерации startup-кода при компиляции под GCC
 ****************************************************************************/
#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return true;
}
#endif

/****************************************************************************
 * Константы для извлечения строк из .lng файла
 ****************************************************************************/
enum {
  MHotDirTitle=0,

  MShotrcutTitle,
  MShotrcutButton,

  MRootFolder,
  MEditButton,

  MHotkey,
  MAllias,
  MPath,
  MRunMacro,
  MOK,
  MCurrent,
  MUnderCursor,
  MCancel,

  MLength512Title,
  MLength512Body,

  MRunMacroBody,

  MDelItem,

  MYes,
  MNo,
};

/****************************************************************************
 * Обёртка сервисной функции FAR: получение строки из .lng-файла
 ****************************************************************************/
char *GetMsg(int MsgId)
{
  return((char *)Info.GetMsg(Info.ModuleNumber, MsgId));
}

/****************************************************************************
 * Функция для преобразования массива структур InitDialogItem в FarDialogItem.
 ****************************************************************************/
struct InitDialogItem {
  int      Type;
  int      X1, Y1, X2, Y2;
  int      Focus;
  int      Selected;
  unsigned Flags;
  int      DefaultButton;
  char    *Data;
};

void InitDialogItems(const InitDialogItem *Init, FarDialogItem *Item, int ItemsNumber)
{
  while(ItemsNumber--)
  {
    Item->Type           = Init->Type;
    Item->X1             = Init->X1;
    Item->Y1             = Init->Y1;
    Item->X2             = Init->X2;
    Item->Y2             = Init->Y2;
    Item->Focus          = Init->Focus;
    Item->Selected       = Init->Selected;
    Item->Flags          = Init->Flags;
    Item->DefaultButton  = Init->DefaultButton;
    lstrcpy( Item->Data, ((unsigned int)Init->Data < 2000) ?
             GetMsg((unsigned int)Init->Data) : Init->Data );

    Item++;
    Init++;
  }
}

// Сообщение для отладки
static int DebugMsg(char *msg, char *msg2 = " ", int i = 1000)
{
  char *MsgItems[] = {"DebugMsg", "", "", ""};
  char buf[80]; FSF.itoa(i, buf,10);
  MsgItems[1] = msg2;
  MsgItems[2] = msg;
  MsgItems[3] = buf;
  if (!Info.Message( Info.ModuleNumber,
                     FMSG_WARNING|FMSG_MB_OKCANCEL,
                     0,
                     MsgItems,
                     sizeof(MsgItems) / sizeof(MsgItems[0]),
                     2 )) return 1;
   return 0;
}

#include "HD_REG.CPP"
#include "HD_TMenu.cpp"
#include "HD_HkMenu.cpp"
#include "HD_DirMenu.cpp"
#include "HD_UpMenu.cpp"

/****************************************************************************
 ***************************** Exported functions ***************************
 ****************************************************************************/

int WINAPI _export GetMinFarVersion(void) { return MAKEFARVERSION(1,71,2232); }

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;

  if (Info->StructSize >= (int)sizeof(struct PluginStartupInfo))
  {
    FSF=*Info->FSF;
    ::Info.FSF=&FSF;

    FSF.sprintf(PluginRootKey, "%s\\HotDir", Info->RootKey);
  }
  else
    bOldFAR=true;
}

void WINAPI _export GetPluginInfo(PluginInfo *Info)
{
  static const char *PluginMenuStrings[1];

  Info->StructSize        = sizeof(PluginInfo);
 // Info->Flags             = PF_PRELOAD;
  *PluginMenuStrings      = GetMsg(MHotDirTitle);
  Info->PluginMenuStrings = PluginMenuStrings;
  Info->PluginMenuStringsNumber=COUNT(PluginMenuStrings);
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom, INT_PTR Item)
{
  if (bOldFAR)
    return INVALID_HANDLE_VALUE;

  PanelInfo pi;
  if ( !Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELSHORTINFO, &pi)
       || pi.PanelType != PTYPE_FILEPANEL )
    return INVALID_HANDLE_VALUE;

  char Section[15], NoAmpersand[15];
  THotKeyMenu HotKeyMenu;
  int Code;

  while ((Code=HotKeyMenu.Run(GetMsg(MShotrcutTitle), GetMsg(MShotrcutButton), (LONG_PTR)Section))!=-1)
  {
    while (1)
    {
      LONG_PTR Key=0;
      TDirMenu DirMenu;
      for (int i=0, j=0; i<15; i++)
        if (Section[i]!='&')
        {
          NoAmpersand[j]=Section[i]; j++;
        }

      LONG_PTR Sect=DirMenu.Run(NoAmpersand, GetMsg(MEditButton), (LONG_PTR)&Key);
      if (!Key)                                                               //  Enter
        return INVALID_HANDLE_VALUE;
      if (Key==KEY_BS)                                                        //  Backspace
        break;
      if (Key==(KEY_CTRL|KEY_BACKSLASH) || Key==(KEY_RCTRL|KEY_BACKSLASH) || Key==KEY_BACKSLASH)  //  Ctrl-'\'
      {
        TUpDirMenu UpDirMenu;
        if (UpDirMenu.Run(NoAmpersand, GetMsg(MShotrcutButton), Sect)!=-1)
          return INVALID_HANDLE_VALUE;
        continue;
      }
      Code=HotKeyMenu.Close(Code+Key-KEY_UP);                                 //  Left || Right
    }
  }
  return INVALID_HANDLE_VALUE;
}
