/****************************************************************************
 * compare4_DLG.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 **************************** ShowDialog functions **************************
 ****************************************************************************/

/****************************************************************************
 * ID-константы диалога
 ****************************************************************************/
enum {
  DlgBORDER = 0,  // 0
  DlgLMASK,       // 1
  DlgEMASK,       // 2
  DlgCMP,         // 3
  DlgCMPNAME,     // 4
  DlgCASE,        // 5
  DlgCMPSIZE,     // 6
  DlgCMPTIME,     // 7
  DlgPRECISION,   // 8
  DlgTIMEZONE,    // 9
  DlgCMPCONTENTS, //10
  DlgLPERCENT,    //11
  DlgEPERCENT,    //12
  DlgLFILTER,     //13
  DlgEFILTER,     //14

  DlgSEP1,        //15
  DlgSUBFOLDER,   //16
  DlgLMAXDEPTH,   //17
  DlgEMAXDEPTH,   //18
  DlgSELECTED,    //19
  DlgFIRSTDIFF,   //20
  DlgPANEL,       //21

  DlgSEP2,        //22
  DlgOK,          //23
#ifdef CACHE
  DlgCACHE,
#endif
  DlgCANCEL,      //24
  //DlgMETHOD       //25
};

/****************************************************************************
 * Структура для сохранения опций.
 ****************************************************************************/
static struct ParamStore StoreDlg[] = {
  {DlgBORDER,      0,                        0},
  {DlgLMASK,       0,                        0},
  {DlgEMASK,       0,                        0},

  {DlgCMP,         0,                        0},
  {DlgCMPNAME,     "CompareName",            &Opt.CompareName},
  {DlgCASE,        "IgnoreCase",             &Opt.IgnoreCase},
  {DlgCMPSIZE,     "CompareSize",            &Opt.CompareSize},
  {DlgCMPTIME,     "CompareTime",            &Opt.CompareTime},
  {DlgPRECISION,   "LowPrecisionTime",       &Opt.LowPrecisionTime},
  {DlgTIMEZONE,    "IgnoreTimeZone",         &Opt.IgnoreTimeZone},
  {DlgCMPCONTENTS, "CompareContents",        &Opt.CompareContents},
  {DlgLPERCENT,    0,                        0},
  {DlgEPERCENT,    0,                        0},
  {DlgLFILTER,     "Filter",                 &Opt.Filter},
  {DlgEFILTER,     "FilterTemplatesN",       &Opt.FilterTemplatesN},

  {DlgSEP1,        0,                        0},
  {DlgSUBFOLDER,   "ProcessSubfolders",      &Opt.ProcessSubfolders},
  {DlgLMAXDEPTH,   0,                        0},
  {DlgEMAXDEPTH,   "MaxScanDepth",           &Opt.MaxScanDepth},
  {DlgSELECTED,    "ProcessSelected",        &Opt.ProcessSelected},
  {DlgFIRSTDIFF,   "ProcessTillFirstDiff",   &Opt.ProcessTillFirstDiff},
  {DlgPANEL,       "Panel",                  &Opt.Panel}
};

/****************************************************************************
 * Обработчик диалога для ShowDialog
 ****************************************************************************/
static LONG_PTR WINAPI ShowDialogProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
  switch (Msg)
  {
    case DN_BTNCLICK:
      if (Param1 == DlgCMPNAME)
      {
        if (Param2)
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgCASE, true);
        else
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgCASE, false);
      }
      //------------
      else if (Param1 == DlgCMPTIME)
      {
        if (Param2)
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgPRECISION, true);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgTIMEZONE, true);
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgPRECISION, false);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgTIMEZONE, false);
        }
      }
      //------------
      else if (Param1 == DlgCMPCONTENTS)
      {
        if (Param2 && (!bAPanelWithCRC && !bPPanelWithCRC))
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLFILTER, true);
          if (Info.SendDlgMessage(hDlg, DM_GETCHECK, DlgLFILTER, 0))
          {
            Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLPERCENT, false);
            Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEPERCENT, false);
            Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEFILTER, true);
          }
          else
          {
            Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLPERCENT, true);
            Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEPERCENT, true);
          }
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLFILTER, false);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEFILTER, false);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLPERCENT, false);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEPERCENT, false);
        }
      }
      else if (Param1 == DlgLFILTER)
      {
        if (Param2)
        {
          FarListPos NewPos = {0, -1};
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLPERCENT, false);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEPERCENT, false);
          Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, DlgEPERCENT, (LONG_PTR)&NewPos);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEFILTER, true);
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLPERCENT, true);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEPERCENT, true);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEFILTER, false);
        }
      }
      //------------
      else if (Param1 == DlgSUBFOLDER)
      {
        if (Param2 == 2)
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLMAXDEPTH, true);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEMAXDEPTH, true);
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgLMAXDEPTH, false);
          Info.SendDlgMessage(hDlg, DM_ENABLE, DlgEMAXDEPTH, false);
        }
      }
      break;

    /************************************************************************/

    case DN_MOUSECLICK:
      if (Param1==DlgOK && ((MOUSE_EVENT_RECORD *)Param2)->dwButtonState==RIGHTMOST_BUTTON_PRESSED)
      {
        Opt.UseCacheResult = 0;
        goto IgnoreCacheMouse;
      }
      // ------------
      else if (Param1==DlgSEP2 && ((MOUSE_EVENT_RECORD *)Param2)->dwEventFlags==DOUBLE_CLICK)
        goto ClearCacheMouse;
      // ------------
      else if (Param1==DlgBORDER && ((MOUSE_EVENT_RECORD *)Param2)->dwButtonState==RIGHTMOST_BUTTON_PRESSED)
        goto ShowConfigMouse;
      // ------------
      return false;

    /************************************************************************/

    case DN_KEY:
      if ( (Param2 == KEY_ENTER || Param2 == (KEY_CTRL|KEY_ENTER) || Param2 == (KEY_RCTRL|KEY_ENTER)) &&
           !( DlgCANCEL == Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0) ) )
      {
        IgnoreCacheMouse:
        bool bCorrect = false;
        char mask[NM*5];
        Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, DlgEMASK, (LONG_PTR)mask);

        if (mask)
        {
          bCorrect = true;
          if (*FSF.Trim(mask))
          {
            const char EXCLUDEMASKSEPARATOR = '|';  // разделитель масок
            char *exclude = strrchr(mask, EXCLUDEMASKSEPARATOR);
            if (strchr(mask, EXCLUDEMASKSEPARATOR) != exclude)
              bCorrect = false;
            if (exclude)
            {
              ++exclude;
              if (*exclude == '\0') bCorrect = false;
            }
          }
          else
            Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgEMASK, (LONG_PTR)"*.*");
        }

        if (!bCorrect)
        {
          ErrorMsg(MIncorrectMaskTitle, MIncorrectMaskBody);
          Info.SendDlgMessage(hDlg, DM_SETFOCUS, DlgEMASK, 0);
          return true;
        }

        if (Param2 == (KEY_CTRL|KEY_ENTER) || Param2 == (KEY_RCTRL|KEY_ENTER))
          Opt.UseCacheResult = 0;

        Info.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
      }
      // ----------
      else if (Param2 == (KEY_CTRL|KEY_DEL) || Param2 == (KEY_RCTRL|KEY_DEL))
      {
        ClearCacheMouse:
        if (CacheResult.rci)
        {
          if (Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, 0) & FCS_DELETE)
          {
            if (YesNoMsg(MClearCacheTitle, MClearCacheBody))
              goto ClearCache;
          }
          else
          ClearCache:
          {
            free(CacheResult.rci);
            CacheResult.rci = 0;
            CacheResult.iCount = 0;
            Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgSEP2, (LONG_PTR)"");
            Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
          }
        }
        return true;
      }
      // -----------
      else if (Param2 == (KEY_ALT|KEY_SHIFT|KEY_F9) || Param2 == (KEY_RALT|KEY_SHIFT|KEY_F9))
      {
        ShowConfigMouse:
        Info.SendDlgMessage(hDlg, DM_SHOWDIALOG, 0, 0);
        Config();
        Info.SendDlgMessage(hDlg, DM_SHOWDIALOG, 1, 0);
        return true;
      }
      break;
  }

  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/****************************************************************************
 * Разбор масок на маски-вкл. и маски-искл.
 ****************************************************************************/
static bool ProcessMasks(const char *Masks)
{
  char *mask = (char *) malloc(NM);
  char *MasksIncl = Opt.MasksInclude;
  char *MasksExcl = Opt.MasksExclude;
  const char EXCLUDEMASKSEPARATOR = '|';
  bool rc = false;

  if (mask)
  {
    lstrcpyn(mask, Masks, NM);
    if (*FSF.Trim(mask))
    {
      if (strchr(mask, EXCLUDEMASKSEPARATOR))
      {
        rc = true;
        while (*mask != EXCLUDEMASKSEPARATOR)
          *(MasksIncl++) = *(mask++);
        *MasksIncl = '\0';
        if (!*Opt.MasksInclude)
          lstrcpy(Opt.MasksInclude, "*.*");

        mask++;
        while (*mask)
          *(MasksExcl++) = *(mask++);
        *MasksExcl = '\0';
      }
      else
      {
        rc = true;
        lstrcpyn(Opt.MasksInclude, Masks, NM);
        lstrcpy(Opt.MasksExclude, "");
      }
    }
    free(mask);
  }

  return rc;
}


/****************************************************************************
 * Читает настройки из реестра, показывает диалог с опциями сравнения,
 * заполняет структуру Opt, сохраняет (если надо) новые настройки в реестре,
 * возвращает true, если пользователь нажал OK
 ****************************************************************************/
static int ShowDialog(bool bAPanelWithRealNames, bool bPPanelWithRealNames, bool bCurrentItemFile, bool bSelectionPresent)
{
  const unsigned int dW = 56;   // ширина
  const unsigned int dH = 22;   // высота

  // Нужно ли при первоначальной загрузке плагина выставлять маску "*.*"?
  static char cAsterisks[4];
  Opt.StartupWithAsterisks ? lstrcpy(cAsterisks, "*.*") : lstrcpy(cAsterisks, "");

  // индикация непустого кэша
  static char cHotKey[dW-6];
  CacheResult.rci && (bAPanelWithRealNames && bPPanelWithRealNames) ?
                                               lstrcpy(cHotKey, GetMsg(MHotKey)) : lstrcpy(cHotKey, "");
  int color = Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_DIALOGSELECTEDBUTTON);

  static struct InitDialogItem InitItems[] = {
    /* 0*/{DI_DOUBLEBOX,3, 1,dW-4,dH-2, 0, 0,                                0, 0, (char *)MCompareTitle},
    /* 1*/{DI_TEXT,     5, 2,   0,   0, 0, 0,                                0, 0, (char *)MCompareMask},
    /* 2*/{DI_EDIT,     5, 3,dW-6,   0, 1, (int)"AdvCmpMask", DIF_USELASTHISTORY|DIF_HISTORY, 0, cAsterisks},
    /* 3*/{DI_TEXT,     5, 4,   0,   0, 0, 0,                                0, 0, (char *)MCompareBox},
    /* 4*/{DI_CHECKBOX, 5, 5,   0,   0, 0, 1,                                0, 0, (char *)MCompareName},
    /* 5*/{DI_CHECKBOX, 0, 5,   0,   0, 0, 1,                                0, 0, (char *)MCompareIgnoreCase},
    /* 6*/{DI_CHECKBOX, 5, 6,   0,   0, 0, 1,                                0, 0, (char *)MCompareSize},
    /* 7*/{DI_CHECKBOX, 5, 7,   0,   0, 0, 1,                                0, 0, (char *)MCompareTime},
    /* 8*/{DI_CHECKBOX, 9, 8,   0,   0, 0, 1,                                0, 0, (char *)MCompareLowPrecision},
    /* 9*/{DI_CHECKBOX, 9, 9,   0,   0, 0, 1,                                0, 0, (char *)MCompareIgnoreTimeZone},
    /*10*/{DI_CHECKBOX, 5,10,   0,   0, 0, 0,                                0, 0, (char *)MCompareContents},
    /*11*/{DI_TEXT,     0,10,   0,   0, 0, 0,                                0, 0, (char *)MPercent},
    /*12*/{DI_COMBOBOX, 0,10,   3,   0, 0, 0,DIF_LISTAUTOHIGHLIGHT|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE, 0, (char *)MPercent100},
    /*13*/{DI_CHECKBOX, 9,11,   0,   0, 0, 0,                                0, 0, (char *)MFilter},
    /*14*/{DI_COMBOBOX, 9,12,dW-6,   0, 0, 0,DIF_LISTAUTOHIGHLIGHT|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE, 0, ""},

    /*15*/{DI_TEXT,    -1,13,   0,   0, 0, 0,                    DIF_SEPARATOR, 0, (char *)MTitleOptions},
    /*16*/{DI_CHECKBOX, 5,14,   0,   0, 0, 0,                       DIF_3STATE, 0, (char *)MProcessSubfolders},
    /*17*/{DI_TEXT,     0,14,   0,   0, 0, 0,                                0, 0, (char *)MMaxScanDepth},
    /*18*/{DI_FIXEDIT,  0,14,   3,   0, 0, (int)"9999",           DIF_MASKEDIT, 0, "10"},
    /*19*/{DI_CHECKBOX, 5,15,   0,   0, 0, 0,                                0, 0, (char *)MProcessSelected},
    /*20*/{DI_CHECKBOX, 5,16,   0,   0, 0, 1,                                0, 0, (char *)MProcessTillFirstDiff},
    /*21*/{DI_CHECKBOX, 5,17,   0,   0, 0, 0,                                0, 0, (char *)MPanel},

    /*22*/{DI_TEXT,    -1,18,   0,   0, 0, 0,color|DIF_SETCOLOR|DIF_SEPARATOR2, 0, cHotKey},
    /*23*/{DI_BUTTON,   0,19,   0,   0, 0, 0,                  DIF_CENTERGROUP, 1, (char *)MOK},
#ifdef CACHE
    /*24*/{DI_BUTTON,   0,19,   0,   0, 0, 0,                  DIF_CENTERGROUP, 0, (char *)MFromCache},
#endif
    /*24*/{DI_BUTTON,   0,19,   0,   0, 0, 0,                  DIF_CENTERGROUP, 0, (char *)MCancel}
  };
  struct FarDialogItem DialogItems[sizeof(InitItems) / sizeof(InitItems[0])];
  memset(DialogItems, 0, sizeof(DialogItems));

  InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

  // динамические координаты для строк
  DialogItems[DlgCASE].X1      = DialogItems[DlgCMPNAME].X1 + lstrlen(DialogItems[DlgCMPNAME].Data.Data)
                                 - (strchr(DialogItems[DlgCMPNAME].Data.Data, '&')?1:0) + 5;
  DialogItems[DlgCASE].X2     += DialogItems[DlgCASE].X1;
  DialogItems[DlgLPERCENT].X1  = DialogItems[DlgCMPCONTENTS].X1 + lstrlen(DialogItems[DlgCMPCONTENTS].Data.Data)
                                 - (strchr(DialogItems[DlgCMPCONTENTS].Data.Data, '&')?1:0) + 5;
  DialogItems[DlgLPERCENT].X2 += DialogItems[DlgLPERCENT].X1;
  DialogItems[DlgEPERCENT].X1  = DialogItems[DlgLPERCENT].X1 + lstrlen(DialogItems[DlgLPERCENT].Data.Data)
                                 - (strchr(DialogItems[DlgLPERCENT].Data.Data, '&')?1:0) + 1;
  DialogItems[DlgEPERCENT].X2 += DialogItems[DlgEPERCENT].X1;
  DialogItems[DlgLMAXDEPTH].X1 = DialogItems[DlgSUBFOLDER].X1 + lstrlen(DialogItems[DlgSUBFOLDER].Data.Data)
                                 - (strchr(DialogItems[DlgSUBFOLDER].Data.Data, '&')?1:0) + 5;
  DialogItems[DlgLMAXDEPTH].X2 += DialogItems[DlgLMAXDEPTH].X1;
  DialogItems[DlgEMAXDEPTH].X1 = DialogItems[DlgLMAXDEPTH].X1 + lstrlen(DialogItems[DlgLMAXDEPTH].Data.Data)
                                 - (strchr(DialogItems[DlgLMAXDEPTH].Data.Data, '&')?1:0) + 1;
  DialogItems[DlgEMAXDEPTH].X2 += DialogItems[DlgEMAXDEPTH].X1;

  int i, n;
  HKEY hKey;
  Opt.UseCacheResult = 1;

  // читаем настройки из реестра
  if (hKey = CreateOrOpenRegKey(false, PluginRootKey))
  {
    for (i=0; i < sizeof(StoreDlg)/sizeof(StoreDlg[0]); i++)
    {
      char cpRegValue[NM]={'\0'};
      DWORD dwRegValue = 0;

      if (StoreDlg[i].RegName && GetRegKey(hKey, StoreDlg[i].RegName, dwRegValue))
      {
        if (DialogItems[i].Type == DI_CHECKBOX)
          DialogItems[i].Param.Selected = dwRegValue;
        else if (DialogItems[i].Type == DI_FIXEDIT)
          FSF.itoa(dwRegValue, DialogItems[i].Data.Data, 10);
        else if (i == DlgEFILTER)
          Opt.FilterTemplatesN = dwRegValue;
      }
/*      if (Store[i].RegName && GetRegKey(hKey, Store[i].RegName, cpRegValue, sizeof(cpRegValue)))
      {
        if (DialogItems[i].Type == DI_EDIT)
        {
          lstrcpy(DialogItems[i].Data.Data, cpRegValue);
          lstrcpy(Opt.FilterName, cpRegValue);
        }
        else if (i == OtrPATTERN)
          lstrcpy(Opt.FilterPattern, cpRegValue);
      }     */
    }
    RegCloseKey(hKey);
  }


  // комбинированный список с процентами 25%-100%
  FarListItem itemPercent[4];
  n = sizeof(itemPercent) / sizeof(itemPercent[0]);
  for (i = 0; i < n; i++)
  {
    itemPercent[i].Flags = 0;
    lstrcpy(itemPercent[i].Text, GetMsg(MPercent100+i));
  }
  FarList Percents = {n, itemPercent};
  DialogItems[DlgEPERCENT].Param.ListItems = &Percents;

  // комбинированный список с шаблонами фильтра по умолчанию
  FarListItem itemFilterTemplates[4];
  n = sizeof(itemFilterTemplates) / sizeof(itemFilterTemplates[0]);
  for (i = 0; i < n; i++)
  {
    if (i==n-1)
      itemFilterTemplates[i].Flags |= LIF_SEPARATOR;
    else
      itemFilterTemplates[i].Flags = 0;

    if (i==Opt.FilterTemplatesN)
      itemFilterTemplates[i].Flags |= LIF_SELECTED;
    else
      itemFilterTemplates[i].Flags &= ~LIF_SELECTED;

    lstrcpy(itemFilterTemplates[i].Text, GetMsg(MFilterIgnoreAllWhitespace+i));
  }
  FarList FilterTemplates = {n, itemFilterTemplates};
  DialogItems[DlgEFILTER].Param.ListItems = &FilterTemplates;


  if (!bCurrentItemFile)
  {
    DialogItems[DlgCMPNAME].Param.Selected = 1;
    DialogItems[DlgCMPNAME].Flags       |= DIF_DISABLE;
  }
  if (!DialogItems[DlgCMPNAME].Param.Selected)
    DialogItems[DlgCASE].Flags          |= DIF_DISABLE;
  //-----------
  if (!DialogItems[DlgCMPTIME].Param.Selected)
  {
    DialogItems[DlgPRECISION].Flags     |= DIF_DISABLE;
    DialogItems[DlgTIMEZONE].Flags      |= DIF_DISABLE;
  }
  //-----------
  if ( !((bAPanelWithRealNames && bPPanelWithRealNames) ||
         (bAPanelWithRealNames && bPPanelWithCRC) ||
         (bAPanelWithCRC && bPPanelWithCRC) ||
         (bAPanelWithCRC && bPPanelWithRealNames)) )
  {
    DialogItems[DlgCMPCONTENTS].Param.Selected = 0;
    DialogItems[DlgCMPCONTENTS].Flags   |= DIF_DISABLE;
    DialogItems[DlgLPERCENT].Flags      |= DIF_DISABLE;
    DialogItems[DlgEPERCENT].Flags      |= DIF_DISABLE;
    DialogItems[DlgLFILTER].Flags       |= DIF_DISABLE;
    DialogItems[DlgEFILTER].Flags       |= DIF_DISABLE;
  }
  if (!DialogItems[DlgCMPCONTENTS].Param.Selected ||
      (bAPanelWithCRC || bPPanelWithCRC))
  {
    DialogItems[DlgLPERCENT].Flags      |= DIF_DISABLE;
    DialogItems[DlgEPERCENT].Flags      |= DIF_DISABLE;
    DialogItems[DlgLFILTER].Flags       |= DIF_DISABLE;
    DialogItems[DlgEFILTER].Flags       |= DIF_DISABLE;
  }
  if (DialogItems[DlgLFILTER].Param.Selected)
  {
    DialogItems[DlgLPERCENT].Flags      |= DIF_DISABLE;
    DialogItems[DlgEPERCENT].Flags      |= DIF_DISABLE;
  }
  else
    DialogItems[DlgEFILTER].Flags       |= DIF_DISABLE;
  //------------
  if ( bPPluginPanels || (bAPluginPanels && !bAPanelWithCRC)
       || !(bAPanelWithDir || bPPanelWithDir) )
  {
    DialogItems[DlgSUBFOLDER].Param.Selected = 0;
    DialogItems[DlgSUBFOLDER].Flags     |= DIF_DISABLE;
    DialogItems[DlgLMAXDEPTH].Flags     |= DIF_DISABLE;
    DialogItems[DlgEMAXDEPTH].Flags     |= DIF_DISABLE;
  }
  else if (DialogItems[DlgSUBFOLDER].Param.Selected != 2)
  {
    DialogItems[DlgLMAXDEPTH].Flags     |= DIF_DISABLE;
    DialogItems[DlgEMAXDEPTH].Flags     |= DIF_DISABLE;
  }
  //------------
  if (!bSelectionPresent)
  {
    DialogItems[DlgSELECTED].Param.Selected = 0;
    DialogItems[DlgSELECTED].Flags      |= DIF_DISABLE;
  }
  //-------------
  if ( bAPluginPanels || bPPluginPanels)
  {
    DialogItems[DlgPANEL].Param.Selected = 0;
    DialogItems[DlgPANEL].Flags         |= DIF_DISABLE;
  }

  int ExitCode = Info.DialogEx( Info.ModuleNumber,
                                -1, -1, dW, dH,
                                "Contents",
                                DialogItems,
                                sizeof(DialogItems) / sizeof(DialogItems[0]),
                                0, 0,
                                ShowDialogProc,
                                0 );
  if (ExitCode == DlgCANCEL || ExitCode == -1) return -1;    // выходим
  //if (ExitCode == DlgMETHOD) return 1;  // переключаемся в другой режим

  bool bShowDlg = false;
  Config(bShowDlg);                    // перечитаем параметры плагина

  if (!ProcessMasks(DialogItems[DlgEMASK].Data.Data))
    return -1;
  Opt.CompareName            = DialogItems[DlgCMPNAME].Param.Selected;
  Opt.IgnoreCase             = DialogItems[DlgCASE].Param.Selected;
  Opt.CompareSize            = DialogItems[DlgCMPSIZE].Param.Selected;
  Opt.CompareTime            = DialogItems[DlgCMPTIME].Param.Selected;
  Opt.LowPrecisionTime       = DialogItems[DlgPRECISION].Param.Selected;
  Opt.IgnoreTimeZone         = DialogItems[DlgTIMEZONE].Param.Selected;
  Opt.CompareContents        = DialogItems[DlgCMPCONTENTS].Param.Selected;
  Opt.ContentsPercent        = DialogItems[DlgEPERCENT].Param.ListPos;
  Opt.Filter                 = DialogItems[DlgLFILTER].Param.Selected;
  Opt.FilterTemplatesN       = DialogItems[DlgEFILTER].Param.ListPos;
  Opt.ProcessSubfolders      = DialogItems[DlgSUBFOLDER].Param.Selected;
  Opt.MaxScanDepth           = FSF.atoi(DialogItems[DlgEMAXDEPTH].Data.Data);
  Opt.ProcessSelected        = DialogItems[DlgSELECTED].Param.Selected;
  Opt.ProcessTillFirstDiff   = DialogItems[DlgFIRSTDIFF].Param.Selected;
  Opt.Panel                  = DialogItems[DlgPANEL].Param.Selected;

  Opt.StartupWithAsterisks   = 0;   // для имитации поведения а-ля ФАР

  for (i=0; i < sizeof(StoreDlg)/sizeof(StoreDlg[0]); i++)
  {
    if (StoreDlg[i].RegName && !(DialogItems[i].Flags & DIF_DISABLE))
      SetRegKey(PluginRootKey, "", StoreDlg[i].RegName, (DWORD)*StoreDlg[i].StoreToOpt);
/*    if (i == DlgEFILTER || i == OtrPATTERN)
      SetRegKey(PluginRootKey, "", Store[i].RegName, (char *)Store[i].StoreToOpt);  */
  }

  return 0;  // сравниваем в обычном режиме
}
