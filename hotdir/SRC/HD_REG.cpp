/****************************************************************************
 * HD_REG.cpp
 *
 * Plugin module for FAR Manager 1.71
 *
 * Copyright (c) 2007 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 ******************************* RegKey functions ***************************
 ****************************************************************************/

static HKEY CreateOrOpenRegKey(bool bCreate, const char *cpRootKey, char *cpKey = "")
{
  char FullKeyName[NM];
  FSF.sprintf(FullKeyName, "%s%s%s", cpRootKey, (*cpKey ? "\\" : ""), cpKey);

  HKEY hKey; DWORD Disposition;
  if (bCreate)
  {
    if ( RegCreateKeyEx(HKEY_CURRENT_USER, FullKeyName, 0, 0, 0, KEY_ALL_ACCESS,
         0, &hKey, &Disposition) != ERROR_SUCCESS )
      return(0);
  }
  else
  {
    if (RegOpenKeyEx(HKEY_CURRENT_USER, FullKeyName, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
      return(0);
  }
  return hKey;
}
//---------
static bool GetRegKey(HKEY hKey, const char *ValueName, DWORD &ValueData)
{
  // DWORD
  DWORD Type, DataSize = sizeof(ValueData);
  if (RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE)&ValueData, &DataSize) == ERROR_SUCCESS)
    return true;
  return false;
}

static bool GetRegKey(HKEY hKey, const char *ValueName, char *ValueData, DWORD DataSize)
{
  // REG_SZ
  DWORD Type;
  if (RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE)ValueData, &DataSize) == ERROR_SUCCESS)
    return true;
  return false;
}
//---------
static void SetRegKey(const char *cpRootKey, const char *cpKey, const char *cpValueName, DWORD dwValueData)
{
  // DWORD
  HKEY hKey = CreateOrOpenRegKey(true, cpRootKey, (char *)cpKey);
  RegSetValueEx(hKey, cpValueName, 0, REG_DWORD, (LPBYTE)&dwValueData, sizeof(dwValueData));
  if (hKey) RegCloseKey(hKey);
}

static void SetRegKey(const char *cpRootKey, const char *cpKey, const char *cpValueName, char *cpValueData)
{
  // REG_SZ
  HKEY hKey = CreateOrOpenRegKey(true, cpRootKey, (char *)cpKey);
  RegSetValueEx(hKey, cpValueName, 0, REG_SZ, (CONST LPBYTE)cpValueData, lstrlen(cpValueData)+1);
  if (hKey) RegCloseKey(hKey);
}
//---------
static void DeleteRegKey(const char *cpRootKey, const char *cpKey)
{
  char FullKeyName[NM];
  FSF.sprintf(FullKeyName, "%s%s%s", cpRootKey, (*cpKey ? "\\" : ""), cpKey);
  RegDeleteKey(HKEY_CURRENT_USER, FullKeyName);
}

static void DeleteRegValue(const char *cpRootKey, const char *cpKey, const char *cpValueName)
{
  char FullKeyName[NM];
  FSF.sprintf(FullKeyName, "%s%s%s", cpRootKey, (*cpKey ? "\\" : ""), cpKey);

  HKEY hKey;
  if (RegOpenKeyEx(HKEY_CURRENT_USER, FullKeyName, 0, KEY_WRITE, &hKey)==ERROR_SUCCESS)
  {
    RegDeleteValue(hKey, cpValueName);
    RegCloseKey(hKey);
  }
}
