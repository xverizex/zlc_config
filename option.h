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
        int64_t val_int64;
        char *val_str;
        char **val_array_str;
        bool *val_array_bool;
        int64_t *val_array_int64;
        int32_t *val_array_int32;
    } v;
    char *name;
    char *comment;
    int size_array;
};

#endif //ZL_CONFIG_EXAMPLE_OPTION_H
