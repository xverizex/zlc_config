//
// Created by cf on 6/11/22.
//

#ifndef ZL_CONFIG_EXAMPLE_OPTION_H
#define ZL_CONFIG_EXAMPLE_OPTION_H

#include <stdint.h>
#include <stdbool.h>

struct option {
    int type;
    union {
        bool val_bool;
        int32_t val_int32;
        int32_t val_int32_array;
        int64_t val_int64;
        int64_t val_int64_array;
        char *val_str;
        char **val_array;
    } v;
    char *name;
};

#endif //ZL_CONFIG_EXAMPLE_OPTION_H
