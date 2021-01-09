#ifdef _WIN64
#define PLATFORM " x64"
#elif defined _WIN32
#define PLATFORM " x86"
#else
#define PLATFORM ""
#endif

#define PLUGIN_VER_MAJOR 3
#define PLUGIN_VER_MINOR 2
#define PLUGIN_VER_PATCH 0
#define PLUGIN_DESC L"Visual renaming files for Far Manager 3" PLATFORM
#define PLUGIN_NAME L"VisRen"
#define PLUGIN_FILENAME L"VisRen.dll"
#define PLUGIN_COPYRIGHT L"© Alexey Samlyukov, 2007-2019. © FarPlugins Team, 2020 -"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define PLUGIN_VERSION STRINGIZE(PLUGIN_VER_MAJOR) "." STRINGIZE(PLUGIN_VER_MINOR) "." STRINGIZE(PLUGIN_VER_PATCH)
