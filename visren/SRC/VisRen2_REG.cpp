/****************************************************************************
 * VisRen2_REG.cpp
 *
 * Plugin module for FAR Manager 1.75
 *
 * Copyright (c) 2007-2010 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 ******************************* RegKey functions ***************************
 ****************************************************************************/

static HKEY CreateOrOpenRegKey(bool bCreate, LPCTSTR cpRootKey, TCHAR *cpKey = _T(""))
{
  TCHAR FullKeyName[NM];
  FSF.sprintf(FullKeyName, _T("%s%s%s"), cpRootKey, (*cpKey ? _T("\\") : _T("")), cpKey);

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
static bool GetRegKey(HKEY hKey, LPCTSTR ValueName, DWORD &ValueData)
{
  // DWORD
  DWORD Type, DataSize = sizeof(ValueData);
  if (RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE)&ValueData, &DataSize) == ERROR_SUCCESS)
    return true;
  return false;
}

static bool GetRegKey(HKEY hKey, LPCTSTR ValueName, TCHAR *ValueData, DWORD DataSize)
{
  // REG_SZ
  DWORD Type;
  if (RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE)ValueData, &DataSize) == ERROR_SUCCESS)
    return true;
  return false;
}
//---------
static void SetRegKey(LPCTSTR cpRootKey, LPCTSTR cpKey, LPCTSTR cpValueName, DWORD dwValueData)
{
  // DWORD
  HKEY hKey = CreateOrOpenRegKey(true, cpRootKey, (TCHAR *)cpKey);
  RegSetValueEx(hKey, cpValueName, 0, REG_DWORD, (LPBYTE)&dwValueData, sizeof(dwValueData));
  if (hKey) RegCloseKey(hKey);
}

static void SetRegKey(LPCTSTR cpRootKey, LPCTSTR cpKey, LPCTSTR cpValueName, TCHAR *cpValueData)
{
  // REG_SZ
  HKEY hKey = CreateOrOpenRegKey(true, cpRootKey, (TCHAR *)cpKey);
  RegSetValueEx(hKey, cpValueName, 0, REG_SZ, (CONST LPBYTE)cpValueData, lstrlen(cpValueData)+1);
  if (hKey) RegCloseKey(hKey);
}
//---------
static void DeleteRegKey(LPCTSTR cpRootKey, LPCTSTR cpKey)
{
  TCHAR FullKeyName[NM];
  FSF.sprintf(FullKeyName, _T("%s%s%s"), cpRootKey, (*cpKey ? _T("\\") : _T("")), cpKey);
  RegDeleteKey(HKEY_CURRENT_USER, FullKeyName);
}

static void DeleteRegValue(LPCTSTR cpRootKey, LPCTSTR cpKey, LPCTSTR cpValueName)
{
  TCHAR FullKeyName[NM];
  FSF.sprintf(FullKeyName, _T("%s%s%s"), cpRootKey, (*cpKey ? _T("\\") : _T("")), cpKey);

  HKEY hKey;
  if (RegOpenKeyEx(HKEY_CURRENT_USER, FullKeyName, 0, KEY_WRITE, &hKey)==ERROR_SUCCESS)
  {
    RegDeleteValue(hKey, cpValueName);
    RegCloseKey(hKey);
  }
}
