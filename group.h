//
// Created by cf on 6/11/22.
//

#ifndef ZL_CONFIG_EXAMPLE_GROUP_H
#define ZL_CONFIG_EXAMPLE_GROUP_H
#include "option.h"
struct zl_group {
    char *name;
    struct zl_option *opt;
    int size_opt;

};
#endif //ZL_CONFIG_EXAMPLE_GROUP_H
