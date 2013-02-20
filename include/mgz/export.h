#ifdef _WIN32
#define __WIN32__ 1
#endif
#if defined(__WIN32__) || defined(_WIN32)
#ifdef MGZ_EXPORTS
#define MGZ_API __declspec(dllexport)
#else
#define MGZ_API __declspec(dllimport)
#endif
#else
#define MGZ_API
#define __cdecl
#endif
