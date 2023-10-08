#ifndef EMULATOR_UTIL_TIME_H
#define EMULATOR_UTIL_TIME_H

#include <time.h>

#define NS_PER_SECOND 1000000000
#define MS_PER_SECOND 1000

void sub_timespec(struct timespec *t1, struct timespec *t2, struct timespec *td);

#endif //EMULATOR_UTIL_TIME_H
