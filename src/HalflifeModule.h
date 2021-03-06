#ifndef __INCLUDE_HALFLIFEMODULE_H__
#define __INCLUDE_HALFLIFEMODULE_H__

#include "Macros.h"
#include "BoostPythonSM.h"

extern "C" __declspec(dllexport) void initHalflife();
extern void destroyHalflife();
extern void unloadThreadStateHalflife(PyThreadState *threadState);

#endif