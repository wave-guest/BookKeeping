#pragma once

#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
#if defined(MYWINDOW_LIBRARY)
#define MYWINDOW_EXPORT __declspec(dllexport)
#else
#define MYWINDOW_EXPORT __declspec(dllimport)
#endif
#else
#define MYWINDOW_EXPORT
#endif
