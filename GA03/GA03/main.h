#ifndef _STUDY_H_
#define _STUDY_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

#define W1_MIN 20
#define W1_MAX 80
#define W2_MIN 50
#define W2_MAX 100
#define W3_MIN 50
#define W3_MAX 100
#define T2_MIN 50
#define T2_MAX 120
#define T3_MIN 50
#define T3_MAX 120
#define T4_MIN 50
#define T4_MAX 120

#if BUILD_DLL
#define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
#define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

extern "C" DLLIMPORT void study(int state, int* w1, int* w2, int *w3, int* t2, int *t3, int *t4);
extern "C" DLLIMPORT void feedback(int value, int debug);
extern "C" DLLIMPORT void setTarget(int target);
extern "C" DLLIMPORT int getTarget();

extern "C" DLLIMPORT int getGeneInfo(int num, int attr);
extern "C" DLLIMPORT int getGeneration();
extern "C" DLLIMPORT int getGeneNum();

extern "C" DLLIMPORT void logVoltage(float voltage, int value);

#endif /* _DLL_H_ */
