#pragma once

#ifdef LIBUNITTESTSUIT_EXPORTS
#define LIBUNITTESTSUIT_API __declspec(dllexport)
#else
#define LIBUNITTESTSUIT_API /*__declspec(dllimport) //not required for modules*/
#endif // LIBUNITTESTSUIT_EXPORTS

