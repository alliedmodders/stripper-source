#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ISmmPluginExt.h>

#if defined _MSC_VER
#define EXPORT extern "C" __declspec(dllexport)
#define PLATFORM_EXT	".dll"
#define PATH_SEP_CHAR	"\\"
#define vsnprintf		_vsnprintf
#define WINDOWS_MEAN_AND_LEAN
#include <windows.h>
inline bool IsPathSepChar(char c) { return (c == '/' || c == '\\'); }
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#define PLATFORM_EXT		".so"
#define PATH_SEP_CHAR		"/"
#define LoadLibrary(x)		dlopen(x, RTLD_NOW)
#define GetProcAddress(x,s)	dlsym(x,s)
#define FreeLibrary(x)		dlclose(x)
typedef void *		HINSTANCE;
#include <dlfcn.h>
inline bool IsPathSepChar(char c) { return (c == '/'); }
#endif

#define MMS_1_4_EP1_FILE		"stripper.14.ep1" PLATFORM_EXT
#define MMS_1_6_EP1_FILE		"stripper.16.ep1" PLATFORM_EXT
#define MMS_1_6_EP2_FILE		"stripper.16.ep2" PLATFORM_EXT
#define MMS_1_6_TF2_FILE		"stripper.16.tf2" PLATFORM_EXT
#define MMS_1_6_DODS_FILE		"stripper.16.dods" PLATFORM_EXT
#define MMS_1_6_HL2DM_FILE		"stripper.16.hl2dm" PLATFORM_EXT
#define MMS_1_6_ND_FILE			"stripper.16.nd" PLATFORM_EXT
#define MMS_1_6_L4D_FILE		"stripper.16.l4d" PLATFORM_EXT
#define MMS_1_6_L4D2_FILE		"stripper.16.l4d2" PLATFORM_EXT
#define MMS_1_6_DARKM_FILE		"stripper.16.darkm" PLATFORM_EXT
#define MMS_1_6_SWARM_FILE		"stripper.16.swarm" PLATFORM_EXT
#define MMS_1_6_BGT_FILE		"stripper.16.bgt" PLATFORM_EXT
#define MMS_1_6_CSGO_FILE		"stripper.16.csgo" PLATFORM_EXT
#define MMS_1_6_CSS_FILE		"stripper.16.css" PLATFORM_EXT
#define MMS_1_6_INS_FILE		"stripper.16.ins" PLATFORM_EXT
#define MMS_1_6_SDK2013_FILE	"stripper.16.sdk2013" PLATFORM_EXT
#define MMS_1_6_BMS_FILE		"stripper.16.bms" PLATFORM_EXT

HINSTANCE stripper_library = NULL;

bool GetFileOfAddress(void *pAddr, char *buffer, size_t maxlength)
{
#if defined _MSC_VER
	MEMORY_BASIC_INFORMATION mem;
	if (!VirtualQuery(pAddr, &mem, sizeof(mem)))
		return false;
	if (mem.AllocationBase == NULL)
		return false;
	HMODULE dll = (HMODULE)mem.AllocationBase;
	GetModuleFileName(dll, (LPTSTR)buffer, maxlength);
#else
	Dl_info info;
	if (!dladdr(pAddr, &info))
		return false;
	if (!info.dli_fbase || !info.dli_fname)
		return false;
	const char *dllpath = info.dli_fname;
	snprintf(buffer, maxlength, "%s", dllpath);
#endif
	return true;
}

size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	size_t len = vsnprintf(buffer, maxlength, fmt, ap);
	va_end(ap);

	if (len >= maxlength)
	{
		len = maxlength - 1;
		buffer[len] = '\0';
	}

	return len;
}

METAMOD_PLUGIN *TryAndLoadLibrary(const char *path)
{
	if ((stripper_library = LoadLibrary(path)) == NULL)
	{
		return NULL;
	}

	METAMOD_FN_ORIG_LOAD ld = (METAMOD_FN_ORIG_LOAD)GetProcAddress(stripper_library, "CreateInterface");
	if (ld == NULL)
	{
		FreeLibrary(stripper_library);
		stripper_library = NULL;
		return NULL;
	}

	int ret;
	METAMOD_PLUGIN *pl = (METAMOD_PLUGIN *)ld(METAMOD_PLAPI_NAME, &ret);
	if (pl == NULL)
	{
		FreeLibrary(stripper_library);
		stripper_library = NULL;
		return NULL;
	}

	return pl;
}

EXPORT METAMOD_PLUGIN *CreateInterface_MMS(const MetamodVersionInfo *mvi, const MetamodLoaderInfo *mli)
{
	/* We can only handle API v2 right now.  In case the plugin API gets bumped,  
	 * we ignore other cases.
	 */
	if (mvi->api_major != 2)
	{
		return NULL;
	}

	const char *file = NULL;

	switch (mvi->source_engine)
	{
		case SOURCE_ENGINE_ORANGEBOX:
			file = MMS_1_6_EP2_FILE;
			break;
		case SOURCE_ENGINE_TF2:
			file = MMS_1_6_TF2_FILE;
			break;
		case SOURCE_ENGINE_DODS:
			file = MMS_1_6_DODS_FILE;
			break;
		case SOURCE_ENGINE_HL2DM:
			file = MMS_1_6_HL2DM_FILE;
			break;
		case SOURCE_ENGINE_NUCLEARDAWN:
			file = MMS_1_6_ND_FILE;
			break;
		case SOURCE_ENGINE_ORIGINAL:
		case SOURCE_ENGINE_EPISODEONE:
			file = MMS_1_6_EP1_FILE;
			break;
		case SOURCE_ENGINE_LEFT4DEAD:
			file = MMS_1_6_L4D_FILE;
			break;
		case SOURCE_ENGINE_LEFT4DEAD2:
			file = MMS_1_6_L4D2_FILE;
			break;
		case SOURCE_ENGINE_DARKMESSIAH:
			file = MMS_1_6_DARKM_FILE;
			break;
		case SOURCE_ENGINE_ALIENSWARM:
			file = MMS_1_6_SWARM_FILE;
			break;
		case SOURCE_ENGINE_BLOODYGOODTIME:
			file = MMS_1_6_BGT_FILE;
			break;
		case SOURCE_ENGINE_CSS:
			file = MMS_1_6_CSS_FILE;
			break;
		case SOURCE_ENGINE_CSGO:
			file = MMS_1_6_CSGO_FILE;
			break;
		case SOURCE_ENGINE_INSURGENCY:
			file = MMS_1_6_INS_FILE;
			break;
		case SOURCE_ENGINE_SDK2013:
			file = MMS_1_6_SDK2013_FILE;
			break;
		case SOURCE_ENGINE_BMS:
			file = MMS_1_6_BMS_FILE;
			break;
	}

	char our_path[256];
	UTIL_Format(our_path, sizeof(our_path), "%s" PATH_SEP_CHAR "%s", mli->pl_path, file);

	return TryAndLoadLibrary(our_path);
}

EXPORT void UnloadInterface_MMS()
{
	if (stripper_library != NULL)
	{
		FreeLibrary(stripper_library);
		stripper_library = NULL;
	}
}

EXPORT void *CreateInterface(const char *pName, int *pReturnCode)
{
	if (strcmp(pName, METAMOD_PLAPI_NAME) == 0)
	{
		char our_file[256];
		if (!GetFileOfAddress((void *)CreateInterface_MMS, our_file, sizeof(our_file)))
		{
			return NULL;
		}

		/* Go backwards and get the first token */
		size_t len = strlen(our_file);
		for (size_t i = len; i-- > 0;)
		{
			if (IsPathSepChar(our_file[i]))
			{
				our_file[i] = '\0';
				break;
			}
		}

		char new_file[256];
		UTIL_Format(new_file, sizeof(new_file), "%s" PATH_SEP_CHAR MMS_1_4_EP1_FILE, our_file);

		return TryAndLoadLibrary(new_file);
	}

	return NULL;
}

#if defined _MSC_VER
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_DETACH)
	{
		UnloadInterface_MMS();
	}
	return TRUE;
}
#else
__attribute__((destructor))
static void gcc_fini()
{
	UnloadInterface_MMS();
}
#endif

