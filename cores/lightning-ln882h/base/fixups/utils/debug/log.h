#ifndef __cplusplus

#undef log_printf
#define __wrap_sprintf __wrap_ln_sprintf
#include_next "utils/debug/log.h"
#undef __wrap_sprintf
#undef log_printf
#define log_printf(...)     __wrap_ln_printf(__VA_ARGS__)
#define __sprintf(tag, fct, ...) __wrap_ln_printf(tag);__wrap_ln_vprintf(__VA_ARGS__)

// Redefine LOG_LVL_CTRL
#undef LOG_LVL_CTRL
#define LOG_LVL_CTRL                LOG_LVL_DEBUG

#endif //__cplusplus */

