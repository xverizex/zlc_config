//
// Created by cf on 6/11/22.
//

#ifndef ZL_CONFIG_EXAMPLE_CONFIG_H
#define ZL_CONFIG_EXAMPLE_CONFIG_H

#include "option.h"
#include "group.h"

enum ZLErrors {
    ZL_ERROR_PARSE_GROUP = -1,
    ZL_ERROR_UNKNOWN_GROUP = -2,
    ZL_ERROR_UNKNOWN_NAME = -3,
    ZL_ERROR_PARSE_NAME = -4,
    ZL_ERROR_BOOL = -5,
    ZL_ERROR_INT32 = -6
};
enum ZLTypeName {
    ZL_TYPE_BOOL,
    ZL_TYPE_INT32,
    ZL_TYPE_INT64
};

struct zl_config {
    struct group *group;
    int size_group;
    void (*error_func) (struct zl_config *cfg, int group, int opt, int error);
};

struct zl_config *zl_config_init (int size_groups);
void zl_config_set_error_func (struct zl_config *cfg, void (*) (struct zl_config *cfg, int group, int opt, int error));
int zl_config_parse (struct zl_config *cfg, const char *filepath);
void zl_config_init_group (struct zl_config *cfg, int group, const char *name, int size_names);
void zl_config_add_option (struct zl_config *cfg, int group, int name, int type, const char *str_name);
const char *zl_config_str_error (struct zl_config *cfg, const int err);
bool zl_config_get_bool (struct zl_config *cfg, int group, int name);
int32_t zl_config_get_int32 (struct zl_config *cfg, int group, int name);

#endif //ZL_CONFIG_EXAMPLE_CONFIG_H
