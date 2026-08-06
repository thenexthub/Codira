#ifndef CODIRA_UTIL_H_
#define CODIRA_UTIL_H_

#if defined(__clang__) || defined(_MSC_VER)
#define CODIRA_CALLTYPE __cdecl
#else
#define CODIRA_CALLTYPE
#endif

#endif
