/****************************************************************************
 * compare3_CFG.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 ****************************** Config functions ****************************
 ****************************************************************************/

/****************************************************************************
 * ID-константы диалога параметров
 ****************************************************************************/
enum {
  CfgBORDER = 0,  // 0
  CfgASTERISKS,   // 1
  CfgCACHE,       // 2
  CfgSHOWMSG,     // 3
  CfgSOUND,       // 4
  CfgLBUFSIZE,    // 5
  CfgEBUFSIZE,    // 6
  CfgLDIFFPOG,    // 7
  CfgEDIFFPOG,    // 8

  CfgSEP1,        // 9
  CfgOK,          // 10
  CfgCANCEL       // 11
};

/****************************************************************************
 * Структура для сохранения опций
 ****************************************************************************/
static struct ParamStore StoreCfg[] = {
  {CfgBORDER,      0,                        0},
  {CfgASTERISKS,   "StartupWithAsterisks",   &Opt.StartupWithAsterisks},
  {CfgCACHE,       "Cache",                  &Opt.Cache},
  {CfgSHOWMSG,     "ShowMessage",            &Opt.ShowMessage},
  {CfgSOUND,       "Sound",                  &Opt.Sound},
  {CfgLBUFSIZE,    "UseBufferSize",          &Opt.UseBufSize},
  {CfgEBUFSIZE,    "CompareBufferSize",      &Opt.BufSize},
  {CfgLDIFFPOG,    "RunDiffProgram",         &Opt.RunDiffProgram},
  {CfgEDIFFPOG,    "PathDiffProgram",        (int*)Opt.PathDiffProgram}
};

/****************************************************************************
 * Обработчик диалога для Config
 ****************************************************************************/
static LONG_PTR WINAPI ConfigProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
  switch (Msg)
  {
    case DN_BTNCLICK:
      if (Param1 == CfgLBUFSIZE)
      {
        if (Param2)
          Info.SendDlgMessage(hDlg, DM_ENABLE, CfgEBUFSIZE, true);
        else
          Info.SendDlgMessage(hDlg, DM_ENABLE, CfgEBUFSIZE, false);
      }
      else if (Param1 == CfgLDIFFPOG)
      {
        if (Param2)
          Info.SendDlgMessage(hDlg, DM_ENABLE, CfgEDIFFPOG, true);
        else
          Info.SendDlgMessage(hDlg, DM_ENABLE, CfgEDIFFPOG, false);
      }
      break;

    /************************************************************************/

    case DN_KEY:
      if ( (Param2 == KEY_ENTER || Param2 == (KEY_CTRL|KEY_ENTER) || Param2 == (KEY_RCTRL|KEY_ENTER)) &&
           !(CfgCANCEL == Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0)))
      {
        char buf[7];
        Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, CfgEBUFSIZE, (LONG_PTR)buf);
        int n = FSF.atoi(buf);
        if ((n==0 || n%64) && Info.SendDlgMessage(hDlg, DM_GETCHECK, CfgLBUFSIZE, 0))
        {
          Info.SendDlgMessage(hDlg, DM_SETFOCUS, CfgEBUFSIZE, 0);
          return true;
        }
      }
      break;
  }

  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/****************************************************************************
 * Функция для изменения/получения настроек плагина
 ****************************************************************************/
static int Config(bool bShowDlg = true)
{
  const unsigned int dW = 56;   // ширина
  const unsigned int dH = 12;   // высота

  static struct InitDialogItem InitItems[] = {
    /* 0*/{DI_DOUBLEBOX,3, 1,dW-4,dH-2, 0, 0,                        0, 0, (char *)MCompareTitle},
    /* 1*/{DI_CHECKBOX, 5, 2,   0,   0, 0, 1,                        0, 0, (char *)MWithAsterisks},
    /* 2*/{DI_CHECKBOX, 5, 3,   0,   0, 0, 1,                        0, 0, (char *)MCache},
    /* 3*/{DI_CHECKBOX, 5, 4,   0,   0, 0, 1,                        0, 0, (char *)MShowMessage},
    /* 4*/{DI_CHECKBOX, 0, 4,   0,   0, 0, 1,                        0, 0, (char *)MSound},
    /* 5*/{DI_CHECKBOX, 5, 5,   0,   0, 0, 0,                        0, 0, (char *)MBufSize},
    /* 6*/{DI_FIXEDIT,  0, 5,   5,   0, 0, (int)"999999", DIF_MASKEDIT, 0, "1024"},
    /* 7*/{DI_CHECKBOX, 5, 6,   0,   0, 0, 0,                        0, 0, (char *)MDiffProg},
    /* 8*/{DI_EDIT,     9, 7,dW-6,   0, 0, (int)"AdvCmpDiffProg", DIF_HISTORY, 0, ""},

    /* 9*/{DI_TEXT,    -1, 8,   0,   0, 0, 0,            DIF_SEPARATOR, 0, ""},
    /*10*/{DI_BUTTON,   0, 9,   0,   0, 0, 0,          DIF_CENTERGROUP, 1, (char *)MOK},
    /*11*/{DI_BUTTON,   0, 9,   0,   0, 0, 0,          DIF_CENTERGROUP, 0, (char *)MCancel}
  };
  struct FarDialogItem DialogItems[sizeof(InitItems) / sizeof(InitItems[0])];
  memset(DialogItems, 0, sizeof(DialogItems));

  InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

  // динамические координаты для строк
  DialogItems[CfgSOUND].X1     = DialogItems[CfgSHOWMSG].X1 + lstrlen(DialogItems[CfgSHOWMSG].Data.Data)
                                 - (strchr(DialogItems[CfgSHOWMSG].Data.Data, '&')?1:0) + 6;
  DialogItems[CfgSOUND].X2    += DialogItems[CfgSOUND].X1;
  DialogItems[CfgEBUFSIZE].X1  = DialogItems[CfgLBUFSIZE].X1 + lstrlen(DialogItems[CfgLBUFSIZE].Data.Data)
                                 - (strchr(DialogItems[CfgLBUFSIZE].Data.Data, '&')?1:0) + 5;
  DialogItems[CfgEBUFSIZE].X2 += DialogItems[CfgEBUFSIZE].X1;

  int i;
  HKEY hKey;
  const DWORD ReadBlock = 65536;     // размер блока чтения

  // читаем настройки из реестра
  if (hKey = CreateOrOpenRegKey(false, PluginRootKey))
  {
    for (i=0; i < sizeof(StoreCfg)/sizeof(StoreCfg[0]); i++)
    {
      char cpRegValue[NM]={'\0'};
      DWORD dwRegValue = 0;
      if (i != CfgEDIFFPOG && GetRegKey(hKey, StoreCfg[i].RegName, dwRegValue))
      {
        if (i == CfgEBUFSIZE)
          FSF.itoa((((dwRegValue ? dwRegValue-1 : dwRegValue)/ReadBlock+1)*ReadBlock)/1024, DialogItems[i].Data.Data, 10);
        else
          DialogItems[i].Param.Selected = dwRegValue;
      }
      else if (GetRegKey(hKey, StoreCfg[i].RegName, cpRegValue, sizeof(cpRegValue)))
        lstrcpy(DialogItems[i].Data.Data, cpRegValue);
    }
    RegCloseKey(hKey);
  }

  if (!DialogItems[CfgLBUFSIZE].Param.Selected)
    DialogItems[CfgEBUFSIZE].Flags |= DIF_DISABLE;
  if (!DialogItems[CfgLDIFFPOG].Param.Selected)
    DialogItems[CfgEDIFFPOG].Flags |= DIF_DISABLE;

  // Если нужно показать диалог
  if (bShowDlg)
  {
    int ExitCode = Info.DialogEx( Info.ModuleNumber,
                                  -1, -1, dW, dH,
                                  "CmpConfig",
                                  DialogItems,
                                  sizeof(DialogItems) / sizeof(DialogItems[0]),
                                  0, 0,
                                  ConfigProc,
                                  0 );
    if (ExitCode == CfgCANCEL || ExitCode == -1)
      return false;
  }

  Opt.StartupWithAsterisks = DialogItems[CfgASTERISKS].Param.Selected;
  Opt.Cache                = DialogItems[CfgCACHE].Param.Selected;
  Opt.ShowMessage          = DialogItems[CfgSHOWMSG].Param.Selected;
  Opt.Sound                = DialogItems[CfgSOUND].Param.Selected;
  Opt.UseBufSize           = DialogItems[CfgLBUFSIZE].Param.Selected;
  Opt.BufSize              = Opt.UseBufSize ?
                              FSF.atoi(DialogItems[CfgEBUFSIZE].Data.Data)*1024 : (bRemovableDrive ? ReadBlock : ReadBlock<<4);
  lstrcpy(Opt.PathDiffProgram, DialogItems[CfgEDIFFPOG].Data.Data);
  Opt.RunDiffProgram       = (*FSF.Trim(Opt.PathDiffProgram) && DialogItems[CfgLDIFFPOG].Param.Selected);
  Opt.ProcessHidden        = Info.AdvControl(Info.ModuleNumber, ACTL_GETPANELSETTINGS, 0) & FPS_SHOWHIDDENANDSYSTEMFILES;

  for (i=0; i < sizeof(StoreCfg)/sizeof(StoreCfg[0]); i++)
    if (StoreCfg[i].RegName && !(DialogItems[i].Flags & DIF_DISABLE))
      if (i == CfgEDIFFPOG)
        SetRegKey(PluginRootKey, "", StoreCfg[i].RegName, (char *)StoreCfg[i].StoreToOpt);
      else
        SetRegKey(PluginRootKey, "", StoreCfg[i].RegName, (DWORD)*StoreCfg[i].StoreToOpt);

  return true;
}
