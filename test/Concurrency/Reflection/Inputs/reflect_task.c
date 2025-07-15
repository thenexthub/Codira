
#include <stdint.h>

uintptr_t language_task_getCurrent();

uintptr_t getCurrentTaskShim() { return language_task_getCurrent(); }
