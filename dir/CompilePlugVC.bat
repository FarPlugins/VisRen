@echo off
@cls

rem =============== Use Microsoft Visual Studio .NET 2003 ======================

@call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

rem  ======================== Set name and version ... =========================

@set PlugName=Dir
@set fileversion=1,71,0,4
@set fileversion_str=1.71 build 4
@set comments=Current developer: samlyukov^<at^>gmail.com
@set companyname=Eugene Roshal ^& FAR Group
@set filedescription=DIR WIN XP/2003/Vista parse for FAR Manager
@set legalcopyright=Copyright © Alexander Arefiev 2001


rem  ==================== Make %PlugName%.def file... ==========================

@if not exist %PlugName%.def (
echo Make %PlugName%.def file...

@echo LIBRARY           %PlugName%                         > %PlugName%.def
@echo EXPORTS                                              >> %PlugName%.def
@echo   IsArchive                                          >> %PlugName%.def
@echo   OpenArchive                                        >> %PlugName%.def
@echo   GetArcItem                                         >> %PlugName%.def
@echo   CloseArchive                                       >> %PlugName%.def
@echo   GetFormatName                                      >> %PlugName%.def
rem @echo   SetFarInfo                                         >> %PlugName%.def

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
@echo       VALUE "OriginalFilename", "%PlugName%.fmt\0"        >> %PlugName%.rc
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

rem  ==================== Compile %PlugName%.fmt file...========================

@if exist %PlugName%.fmt del %PlugName%.fmt>nul
@rc /l 0x4E4 %PlugName%.rc

@echo !!!!!!!  Compile %PlugName%.fmt with MSVCRT.dll ...  !!!!!!!
@echo ***************
@cl /Zp2 /O1igy /GF /Gr /GR- /GX- /LD /G7 %PlugName%.cpp /link /subsystem:console /machine:I386 /opt:nowin98 /noentry /nodefaultlib /def:%PlugName%.def kernel32.lib MSVCRT60.LIB %PlugName%.res /map:"%PlugName%.map" /out:"%PlugName%.fmt" /merge:.rdata=.text

@if exist %PlugName%.exp del %PlugName%.exp>nul
@if exist %PlugName%.obj del %PlugName%.obj>nul
@if exist %PlugName%.lib del %PlugName%.lib>nul
@if exist %PlugName%.res del %PlugName%.res>nul
@if exist %PlugName%.def del %PlugName%.def>nul
@if exist %PlugName%.rc  del %PlugName%.rc>nul

echo ***************