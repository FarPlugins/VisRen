/****************************************************************************
 * compare8_PNL.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 1996-2000 Eugene Roshal
 * Copyright (c) 2000-2007 FAR group
 * Copyright (c) 2006-2007 Alexey Samlyukov
 ****************************************************************************/

/***
class CmpPanel
{
  PluginPanelItem *CmpPanelItem;
  int CmpItemsNumber;
  void FreePanelItem();

public:
  CmpPanel();
  ~CmpPanel();

  void GetOpenPluginInfo(struct OpenPluginInfo *Info);
  int GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
  void BuildItem( const char *ACurDir, const char *PCurDir,
                  const FAR_FIND_DATA *AData, const FAR_FIND_DATA *PData );
  int CheckItem();
};
***/

CmpPanel::CmpPanel()
{
  CmpPanelItem   = 0;
  CmpItemsNumber = 0;
}

CmpPanel::~CmpPanel()
{
  FreePanelItem();
}

void CmpPanel::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize = sizeof(*Info);
  Info->Flags      = OPIF_ADDDOTS|OPIF_SHOWNAMESONLY|OPIF_SHOWPRESERVECASE|OPIF_USEHIGHLIGHTING;
  Info->CurDir     = "";
  Info->PanelTitle = GetMsg(MPanelTitle);

  const unsigned ArraySIZE = 10;
  static struct PanelMode PanelModesArray[ArraySIZE];
  memset(PanelModesArray, 0, sizeof(PanelModesArray));
  static const char *ColumnTitles1[6];
    ColumnTitles1[0] = GetMsg(MName);
    ColumnTitles1[1] = GetMsg(MSize);
    ColumnTitles1[2] = GetMsg(MTime);
    ColumnTitles1[3] = GetMsg(MTime);
    ColumnTitles1[4] = GetMsg(MSize);
    ColumnTitles1[5] = GetMsg(MName);
  static const char *ColumnTitles2[2];
    ColumnTitles2[0] = GetMsg(MName);
    ColumnTitles2[1] = GetMsg(MName);

  for (int i=0; i<ArraySIZE; i++)
  {
    PanelModesArray[i].FullScreen = true;
    if (i%2)
    {
      PanelModesArray[i].ColumnTitles       = (char**)ColumnTitles1;
      PanelModesArray[i].ColumnTypes        = "C0,SCF,DM,DC,PCF,C1";
      PanelModesArray[i].ColumnWidths       = "0,8,17,17,8,0";
      PanelModesArray[i].StatusColumnTypes  = "C2,SC,PC,C3";
      PanelModesArray[i].StatusColumnWidths = "0,17,17,0";
    }
    else
    {
      PanelModesArray[i].ColumnTitles       = (char**)ColumnTitles2;
      PanelModesArray[i].ColumnTypes        = "C2,C3";
      PanelModesArray[i].ColumnWidths       = "0,0";
      PanelModesArray[i].StatusColumnTypes  = "C0,C1";
      PanelModesArray[i].StatusColumnWidths = "0,0";
    }
  }
  Info->PanelModesArray  = PanelModesArray;
  Info->PanelModesNumber = ArraySIZE;
  Info->StartSortMode    = SM_UNSORTED;
}

int CmpPanel::GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
  *pPanelItem   = CmpPanelItem;
  *pItemsNumber = CmpItemsNumber;
  return true;
}

void CmpPanel::BuildItem( const char *ACurDir, const char *PCurDir,
                          const FAR_FIND_DATA *AData, const FAR_FIND_DATA *PData )
{
  if ( struct PluginPanelItem *NewPanelItem =
       (PluginPanelItem *)realloc(CmpPanelItem, (CmpItemsNumber+1) * sizeof(*NewPanelItem)) )
  {
    CmpPanelItem = NewPanelItem;
    PluginPanelItem *CurItem = &CmpPanelItem[CmpItemsNumber++];
    memset(CurItem, 0, sizeof(PluginPanelItem));
    lstrcpy(CurItem->FindData.cFileName, *AData->cFileName ? AData->cFileName : PData->cFileName);
    if (AData->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY || PData->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      CurItem->FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

    CurItem->FindData.nFileSizeHigh   = AData->nFileSizeHigh;                           //
    CurItem->FindData.nFileSizeLow    = AData->nFileSizeLow;                            // Размер А
    CurItem->FindData.ftLastWriteTime = AData->ftLastWriteTime;                         // Время А
    CurItem->PackSizeHigh             = PData->nFileSizeHigh;                           //
    CurItem->PackSize                 = PData->nFileSizeLow;                            // Размер Р
    CurItem->FindData.ftCreationTime  = PData->ftLastWriteTime;                         // Время Р

    CurItem->CustomColumnData         = (LPSTR*)malloc(sizeof(LPSTR)*4);
    CurItem->CustomColumnNumber       = 4;
    CurItem->CustomColumnData[0]      = (char*)malloc(lstrlen(*AData->cFileName ? AData->cFileName : "")+1);
    lstrcpy(CurItem->CustomColumnData[0], *AData->cFileName ? AData->cFileName : "");   // Имя A
    CurItem->CustomColumnData[1]      = (char*)malloc(lstrlen(*PData->cFileName ? PData->cFileName : "")+1);
    lstrcpy(CurItem->CustomColumnData[1], *PData->cFileName ? PData->cFileName : "");   // Имя Р
    CurItem->CustomColumnData[2]      = (char*)malloc(lstrlen(*AData->cFileName ? BuildFullFilename(ACurDir,AData->cFileName) : "")+1);
    lstrcpy(CurItem->CustomColumnData[2], *AData->cFileName ? BuildFullFilename(ACurDir,AData->cFileName) : "");
    CurItem->CustomColumnData[3]      = (char*)malloc(lstrlen(*PData->cFileName ? BuildFullFilename(PCurDir,PData->cFileName) : "")+1);
    lstrcpy(CurItem->CustomColumnData[3], *PData->cFileName ? BuildFullFilename(PCurDir,PData->cFileName) : "");
  }
  else
  {
    ErrorMsg(MNoMemTitle, MNoMemBody);
    FreePanelItem();
    Opt.Panel = 0;
  }
}

void CmpPanel::FreePanelItem()
{
  if (CmpPanelItem)
  {
    for (int i=0; i<CmpItemsNumber; i++)
    {
      for (int j=0; j<CmpPanelItem[i].CustomColumnNumber; j++)
        free(CmpPanelItem[i].CustomColumnData[j]);
      free(CmpPanelItem[i].CustomColumnData);
    }
    free(CmpPanelItem);
  }
  CmpPanelItem   = 0;
  CmpItemsNumber = 0;
}

int CmpPanel::CheckItem()
{
  return CmpItemsNumber;
}
