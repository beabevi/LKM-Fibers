#pragma once


#ifdef USERSPACE

#include "src/ult.h"

#define ConvertThreadToFiber() ult_convert()
#define CreateFiber(dwStackSize, lpStartAddress, lpParameter) ult_creat(dwStackSize, lpStartAddress, lpParameter)
#define SwitchToFiber(lpFiber) ult_switch_to(lpFiber)
#define FlsAlloc(lpCallback) fls_alloc()
#define FlsFree(dwFlsIndex)	fls_free(dwFlsIndex)
#define FlsGetValue(dwFlsIndex) fls_get(dwFlsIndex)
#define FlsSetValue(dwFlsIndex, lpFlsData) fls_set((dwFlsIndex), (long long)(lpFlsData))

#else


// TODO:
// Here you should point to the invocation of your code!
// See README.md for further details.

#include "../../include/lib/fibers.h"

#define ConvertThreadToFiber() to_fiber()
#define CreateFiber(dwStackSize, lpStartAddress, lpParameter) create_fiber(dwStackSize, lpStartAddress, lpParameter)
#define SwitchToFiber(lpFiber) switch_fiber(lpFiber);
#define FlsAlloc(lpCallback) fls_alloc()
#define FlsFree(dwFlsIndex) fls_free(dwFlsIndex)
#define FlsGetValue(dwFlsIndex) fls_get(dwFlsIndex)
#define FlsSetValue(dwFlsIndex, lpFlsData) fls_set(dwFlsIndex, lpFlsData)

#endif /* USERSPACE */
