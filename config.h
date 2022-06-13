//
// Created by cf on 6/11/22.
//

#ifndef ZL_CONFIG_EXAMPLE_CONFIG_H
#define ZL_CONFIG_EXAMPLE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "option.h"
#include "group.h"
#include <pthread.h>

enum ZLErrors {
    ZL_ERROR_PARSE_GROUP = -1,
    ZL_ERROR_UNKNOWN_GROUP = -2,
    ZL_ERROR_UNKNOWN_NAME = -3,
    ZL_ERROR_PARSE_NAME = -4,
    ZL_ERROR_BOOL = -5,
    ZL_ERROR_INT32 = -6,
    ZL_ERROR_INT64 = -7,
    ZL_ERROR_STRING = -8,
    ZL_ERROR_PARSE_STRING = -9,
    ZL_ERROR_PARSE_ARRAY_STRING = -10,
    ZL_ERROR_PARSE_ARRAY_BOOL = -11,
    ZL_ERROR_PARSE_ARRAY_INT64 = -12,
    ZL_ERROR_CONFIG_SAVE = -13
};
enum ZLTypeName {
    ZL_TYPE_BOOL,
    ZL_TYPE_INT32,
    ZL_TYPE_INT64,
    ZL_TYPE_STRING,
    ZL_TYPE_ARRAY_INT32,
    ZL_TYPE_ARRAY_INT64,
    ZL_TYPE_ARRAY_BOOL,
    ZL_TYPE_ARRAY_STRING
};

struct zl_config;


struct zl_config *zl_config_init(int size_groups);
void zl_config_set_error_func(struct zl_config *cfg, void (*)(struct zl_config *cfg, int group, int opt, const char *p_group, const char *p_option, int error));
void zl_config_set_filepath (struct zl_config *cfg, const char *filepath);
int zl_config_parse(struct zl_config *cfg, const char *filepath);
void zl_config_init_group(struct zl_config *cfg, int group, const char *name, int size_names);
void zl_config_add_option(struct zl_config *cfg, int group, int name, int type, const char *str_name, void *default_value);
void zl_config_set_comment_option (struct zl_config *cfg, int group, int opt, const char *comment);
int zl_config_save (struct zl_config *cfg);
const char *zl_config_str_error(struct zl_config *cfg, const int err);
bool zl_config_get_bool(struct zl_config *cfg, int group, int name);
int32_t zl_config_get_int32(struct zl_config *cfg, int group, int name);
int64_t zl_config_get_int64(struct zl_config *cfg, int group, int name);
const char *zl_config_get_string(struct zl_config *cfg, int group, int name);
const char **zl_config_get_array_string(struct zl_config *cfg, int group, int name, int *size);
const bool *zl_config_get_array_bool(struct zl_config *cfg, int group, int name, int *size);
const int64_t *zl_config_get_array_int64(struct zl_config *cfg, int group, int name, int *size);

#ifdef __cplusplus
};
#endif

#endif //ZL_CONFIG_EXAMPLE_CONFIG_H
