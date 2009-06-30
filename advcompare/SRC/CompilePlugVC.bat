@echo off
@cls

rem =============== Use Microsoft Visual Studio .NET 2003 ======================

@call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

rem  ======================== Set name and version ... =========================

@set PlugName=Compare
@set fileversion=1,71,0,30
@set fileversion_str=1.71 build 30
@set MyDir=%CD%
@set MyFarDir=C:\Program Files\Far
@set MyReleaseDir=%MyFarDir%\Plugins\%PlugName%
@set PlugReg=AdvCompare
@set comments=Current developer: samlyukov^<at^>gmail.com
@set companyname=Eugene Roshal ^& FAR Group
@set filedescription=Advanced File Compare 2 for FAR Manager
@set legalcopyright=Copyright © Eugene Roshal 1996-2000, Copyright © 2000 FAR Group


rem  ==================== Make %PlugName%.def file... ==========================

@if not exist %PlugName%.def (
echo Make %PlugName%.def file...

@echo LIBRARY           %PlugName%                         > %PlugName%.def
@echo EXPORTS                                              >> %PlugName%.def
@echo   SetStartupInfo                                     >> %PlugName%.def
@echo   GetPluginInfo                                      >> %PlugName%.def
@echo   OpenPlugin                                         >> %PlugName%.def
@echo   GetMinFarVersion                                   >> %PlugName%.def
@echo   ExitFAR                                            >> %PlugName%.def
@echo   Configure                                          >> %PlugName%.def
rem @echo   OpenFilePlugin                                     >> %PlugName%.def
rem @echo   SetFindList                                        >> %PlugName%.def
rem @echo   ProcessEditorInput                                 >> %PlugName%.def
rem @echo   ProcessEditorEvent                                 >> %PlugName%.def
rem @echo   ProcessViewerEvent                                 >> %PlugName%.def
@echo   ClosePlugin                                        >> %PlugName%.def
@echo   GetOpenPluginInfo                                  >> %PlugName%.def
@echo   GetFindData                                        >> %PlugName%.def
rem @echo   FreeFindData                                       >> %PlugName%.def
rem @echo   FreeVirtualFindData                                >> %PlugName%.def
rem @echo   SetDirectory                                       >> %PlugName%.def
rem @echo   GetFiles                                           >> %PlugName%.def
rem @echo   PutFiles                                           >> %PlugName%.def
rem @echo   DeleteFiles                                        >> %PlugName%.def
rem @echo   MakeDirectory                                      >> %PlugName%.def
rem @echo   ProcessHostFile                                    >> %PlugName%.def
rem @echo   ProcessKey                                         >> %PlugName%.def
rem @echo   ProcessEvent                                       >> %PlugName%.def
rem @echo   Compare                                            >> %PlugName%.def

@if exist %PlugName%.def echo ... succesfully
)

rem  ================== Make %PlugName%.rc file... =============================

@if not exist %PlugName%.rc (
echo Make %PlugName%.rc file...

@echo #define VERSIONINFO_1   1                                 > %PlugName%.rc
@echo.                                                          >> %PlugName%.rc
@echo VERSIONINFO_1  VERSIONINFO                                >> %PlugName%.rc
@echo FILEVERSION    %fileversion%                              >> %PlugName%.rc
@echo PRODUCTVERSION 1,71,0,0                                   >> %PlugName%.rc
@echo FILEFLAGSMASK  0x0                                        >> %PlugName%.rc
@echo FILEFLAGS      0x0                                        >> %PlugName%.rc
@echo FILEOS         0x4                                        >> %PlugName%.rc
@echo FILETYPE       0x2                                        >> %PlugName%.rc
@echo FILESUBTYPE    0x0                                        >> %PlugName%.rc
@echo {                                                         >> %PlugName%.rc
@echo   BLOCK "StringFileInfo"                                  >> %PlugName%.rc
@echo   {                                                       >> %PlugName%.rc
@echo     BLOCK "000004E4"                                      >> %PlugName%.rc
@echo     {                                                     >> %PlugName%.rc
@echo       VALUE "CompanyName",      "%companyname%\0"         >> %PlugName%.rc
@echo       VALUE "FileDescription",  "%filedescription%\0"     >> %PlugName%.rc
@echo       VALUE "FileVersion",      "%fileversion_str%\0"     >> %PlugName%.rc
@echo       VALUE "InternalName",     "%PlugName%\0"            >> %PlugName%.rc
@echo       VALUE "LegalCopyright",   "%legalcopyright%\0"      >> %PlugName%.rc
@echo       VALUE "OriginalFilename", "%PlugName%.dll\0"        >> %PlugName%.rc
@echo       VALUE "ProductName",      "FAR Manager\0"           >> %PlugName%.rc
@echo       VALUE "ProductVersion",   "1.71\0"                  >> %PlugName%.rc
@echo       VALUE "Comments",         "%comments%\0"            >> %PlugName%.rc
@echo     }                                                     >> %PlugName%.rc
@echo   }                                                       >> %PlugName%.rc
@echo   BLOCK "VarFileInfo"                                     >> %PlugName%.rc
@echo   {                                                       >> %PlugName%.rc
@echo     VALUE "Translation", 0, 0x4e4                         >> %PlugName%.rc
@echo   }                                                       >> %PlugName%.rc
@echo }                                                         >> %PlugName%.rc

@if exist %PlugName%.rc echo ... succesfully
)

rem  ==================== Compile %PlugName%.dll file...========================

@cd ".."
@if exist %PlugName%.dll "%MyFarDir%\The Underscore\loader\LOADER.EXE" /u %PlugName%.dll
@if exist %PlugName%.dll del %PlugName%.dll>nul

@cd %MyDir%
@rc /l 0x4E4 %PlugName%.rc

@echo !!!!!!!  Compile %PlugName%.dll with MSVCRT.dll ...  !!!!!!!
@echo ***************
@cl /Zp2 /O1igy /GF /Gr /GR- /GX- /LD /G7 %PlugName%.cpp /link /subsystem:console /machine:I386 /opt:nowin98 /noentry /nodefaultlib /def:%PlugName%.def kernel32.lib advapi32.lib user32.lib shell32.lib MSVCRT60.LIB %PlugName%.res /map:"..\%PlugName%.map" /out:"..\%PlugName%.dll" /merge:.rdata=.text

@echo ***************
@if exist %PlugName%.exp del %PlugName%.exp>nul
@if exist %PlugName%.obj del %PlugName%.obj>nul
@if exist %PlugName%.lib del %PlugName%.lib>nul
@if exist %PlugName%.res del %PlugName%.res>nul
@if exist %PlugName%.def del %PlugName%.def>nul
@if exist %PlugName%.rc  del %PlugName%.rc>nul
@if exist $DelOld$.reg del $DelOld$.reg>nul


rem   ================== Delete old plugin settings... =========================
rem   using: "CompilePlugVC.bat -D"   or   "CompilePlugVC.bat -d"

@if [%1]==[-d] goto del
@if [%1]==[-D] goto del
@if [%2]==[-d] goto del
@if [%2]==[-D] goto del
@if [%3]==[-d] goto del
@if [%3]==[-D] goto del
@goto next2

:del
echo Delete old plugin settings...
@echo REGEDIT4                                             > $DelOld$.reg
@echo [-HKEY_CURRENT_USER\Software\Far\Plugins\%PlugReg%]    >> $DelOld$.reg
@start/wait regedit -s $DelOld$.reg
echo ... succesfully
@goto next2

rem   ======================= Copy to release... ===============================
rem   using: "CompilePlugVC.bat -R"   or   "CompilePlugVC.bat -r"

:next2
@if [%1]==[-r] goto CopyToRelease
@if [%1]==[-R] goto CopyToRelease
@if [%2]==[-r] goto CopyToRelease
@if [%2]==[-R] goto CopyToRelease
@if [%3]==[-r] goto CopyToRelease
@if [%3]==[-R] goto CopyToRelease
@goto next3

:CopyToRelease
echo Delete old plugin ...
@if exist "%MyReleaseDir%\%PlugName%.dll" "%MyFarDir%\The Underscore\loader\LOADER.EXE" /u "%MyReleaseDir%\%PlugName%.dll"
@del /Q "%MyReleaseDir%\*.*" >nul

echo Copy new plugin ...
@cd %MyDir%
@cd ".."
@copy "*.*" "%MyReleaseDir%" >nul
@if exist "%MyReleaseDir%\%PlugName%.dll" "%MyFarDir%\The Underscore\loader\LOADER.EXE" /l "%MyReleaseDir%\%PlugName%.dll"
@goto done

rem  ================= Load work %PlugName%.dll file... ========================

:next3
@cd %MyDir%
@cd ".."
@if exist %PlugName%.dll  "%MyFarDir%\The Underscore\loader\LOADER.EXE" /l %PlugName%.dll

:done
echo ***************