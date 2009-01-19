@echo off
@cls
rem =============== Use Microsoft Visual Studio .NET 2003 ======================

@call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"


rem  ======================== Compile Pcre.dll file...==========================

if not exist config.h copy config.h.generic config.h
if not exist pcre.h copy pcre.h.generic pcre.h

if not exist pcre_chartables.c cl /DHAVE_CONFIG_H dftables.c
if exist dftables.exe dftables.exe -L pcre_chartables.c
if exist dftables.exe del dftables.exe>nul
if exist dftables.obj del dftables.obj>nul

cl /nologo /O1igy /Zp2 /GF /LD /G7 /GR- /EHsc /DHAVE_CONFIG_H pcre_compile.c pcre_chartables.c pcre_config.c pcre_dfa_exec.c pcre_exec.c pcre_fullinfo.c pcre_get.c pcre_globals.c pcre_info.c pcre_maketables.c pcre_newline.c pcre_ord2utf8.c pcre_refcount.c pcre_study.c pcre_tables.c pcre_try_flipped.c pcre_ucp_searchfuncs.c pcre_valid_utf8.c pcre_version.c pcre_xclass.c /link /nologo /subsystem:console /machine:I386 /opt:nowin98 /noentry /nodefaultlib /def:pcre.def kernel32.lib MSVCRT60.LIB /map:pcre.map /out:pcre.dll /implib:pcre.lib /merge:.rdata=.text
if exist *.obj del *.obj>nul
if exist *.exp del *.exp>nul
