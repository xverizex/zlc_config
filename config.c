//
// Created by cf on 6/11/22.
//

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct zl_config *zl_config_init (int size_groups) {
    struct zl_config *cfg = calloc (1, sizeof (struct zl_config));
    cfg->group = calloc (size_groups, sizeof (struct group));
    cfg->size_group = size_groups;
    return cfg;
}

void zl_config_init_group (struct zl_config *cfg, int group, const char *name, int size_names) {
    cfg->group[group].opt = calloc (size_names, sizeof (struct option));
    cfg->group[group].name = strdup (name);
    cfg->group[group].size_opt = size_names;
}

void zl_config_add_option (struct zl_config *cfg, int group, int name, int type, const char *str_name) {
    cfg->group[group].opt[name].type = type;
    cfg->group[group].opt[name].name = strdup (str_name);
}

void zl_config_set_error_func (struct zl_config *cfg, void (*func) (struct zl_config *cfg, int group, int opt, int error)) {
    cfg->error_func = func;
}


bool zl_config_get_bool (struct zl_config *cfg, int group, int name) {
    return cfg->group[group].opt[name].v.val_bool;
}

int32_t zl_config_get_int32 (struct zl_config *cfg, int group, int name) {
    return cfg->group[group].opt[name].v.val_int32;
}

int64_t zl_config_get_int64 (struct zl_config *cfg, int group, int name) {
    return cfg->group[group].opt[name].v.val_int64;
}

const char *zl_config_get_string (struct zl_config *cfg, int group, int name) {
    return cfg->group[group].opt[name].v.val_str;
}

static bool parse_bool (char *data, int *error, int *pos) {
    *error = 0;
    *pos = 0;

    char temp_e = 0;
    char temp_s = 0;
    char *e = strchr (data, '\n');
    if (e) {
        temp_e = *e;
        *e = 0;
        *pos = e - data;
    }

    char *s = strchr (data, ' ');
    if (s) {
        temp_s = *s;
        *s = 0;
        if (!e) {
            *pos = s - data;
        }
    }

    bool result = false;
    if (!strncmp (data, "true", 5)) result = true;
    else if (!strncmp (data, "false", 6)) result = false;
    else *error = ZL_ERROR_BOOL;

    if (temp_e > 0) *e = temp_e;
    if (temp_s > 0) *s = temp_s;

    return result;
}

static int32_t parse_int (char *data, int *error, int *pos, bool is_64) {
    *error = 0;

    char temp_e = 0;
    char temp_s = 0;
    char *e = strchr (data, '\n');
    if (e) {
        temp_e = *e;
        *e = 0;
        *pos = e - data;
    }

    char *s = strchr (data, ' ');
    if (s) {
        temp_s = *s;
        *s = 0;
        if (!e) {
            *pos = s - data;
        }
    }

    int32_t result = 0;

    int len = strlen (data);
    for (int i = 0; i < len; i++) {
        if (data[i] < '0' || data[i] > '9') {
            *error = ZL_ERROR_INT32;
            break;
        }
    }

    if (*error == 0) {
        if (is_64) result = atol (data);
        else result = atoi (data);
    }

    if (temp_e > 0) *e = temp_e;
    if (temp_s > 0) *s = temp_s;

    return result;
}

static char *parse_string (char *data, int *error, int *pos) {
    *error = 0;

    if (data[0] != '"') {
        *error = ZL_ERROR_PARSE_STRING;
        return NULL;
    }

    char temp_e = 0;
    char temp_s = 0;
    char *temp_r = 0;
    char *e = strchr (data, '\n');
    if (e) {
        temp_e = *e;
        *e = 0;
        *pos = e - data;
    }

    int len = strlen (data);

    int i = 1;
    int quotes = 0;
    for (; i <= len; i++) {
        if (data[i] == '\\' && data[i + 1] == '"') {
            i += 1;
            quotes++;
            continue;
        }
        if (data[i] == '"') {
            i--;
            break;
        }
    }

    int total = i - quotes;

    char *str = calloc (total + 1, 1);

    int index_data = 1;
    for (int i = 0; i < total; i++) {
        if (data[index_data] == '\\' && data[index_data + 1] == '"') index_data++;
        str[i] = data[index_data++];
    }
    str[total] = 0;

    return str;
}

static int get_index_group (struct zl_config *cfg, const char *name) {
    for (int i = 0; i < cfg->size_group; i++) {
        if (!strncmp (cfg->group[i].name, name, strlen (name) + 1)) return i;
    }

    return -1;
}

static int get_index_opt (struct zl_config *cfg, int group, const char *name) {
    struct group *grp = &cfg->group[group];
    for (int i = 0; i < grp->size_opt; i++) {
        if (!strncmp (grp->opt[i].name, name, strlen (name) + 1)) return i;
    }

    return -1;
}

static char *get_data_file (const char *filepath, size_t *size_file) {
    *size_file = 0;
    FILE *fp = fopen (filepath, "r");
    fseek (fp, 0, SEEK_END);
    *size_file = ftell (fp);
    fseek (fp, 0, SEEK_SET);
    char *data = malloc (*size_file + 1);
    data[*size_file] = 0;
    fread (data, 1, *size_file, fp);
    fclose (fp);
    return data;
}



int zl_config_parse (struct zl_config *cfg, const char *filepath) {
    size_t size_file = 0L;
    char *data = get_data_file(filepath, &size_file);

    char group_name[20];
    char opt_name[20];

    int gri = 0;
    int opi = 0;
    enum States {
        STATE_START,
        STATE_NEW_GROUP,
        STATE_END_GROUP,
        STATE_NEW_NAME,
        STATE_END_NAME
    };
    int state = STATE_START;
    int index_group = 0;
    int index_opt = 0;

    int type_of_opt = 0;

    /* parse groups */
    for (int i = 0; i < size_file; i++) {
        if (data[i] == '#') {
            while (data[i] != '\n') i++;
            continue;
        }
        if ((data[i] == '\r' || data[i] == '\n')) {
            if (gri > 0) {
                if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, ZL_ERROR_PARSE_GROUP);
                return ZL_ERROR_PARSE_GROUP;
            }
            continue;
        }
        if (data[i] == '[' && gri != 0) {
            free (data);
            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, ZL_ERROR_PARSE_GROUP);
            return ZL_ERROR_PARSE_GROUP;
        } else if (data[i] == '[') {
            state = STATE_NEW_GROUP;
            continue;
        }

        if (data[i] == ']' && gri == 0) {
            free (data);
            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, ZL_ERROR_PARSE_GROUP);
            return ZL_ERROR_PARSE_GROUP;
        } else if (data[i] == ']'){
            state = STATE_END_GROUP;
        }

        switch (state) {
            case STATE_NEW_GROUP:
                group_name[gri++] = data[i];
                break;
            case STATE_END_GROUP:
                group_name[gri] = 0;
                gri = 0;
                state = STATE_NEW_NAME;
                index_group = get_index_group (cfg, group_name);
                if (index_group == -1) {
                    free (data);
                    if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, ZL_ERROR_UNKNOWN_GROUP);
                    return ZL_ERROR_UNKNOWN_GROUP;
                }
                break;
            case STATE_NEW_NAME:
                if ((data[i] == ' ' || data[i] == '=') && opi == 0) {
                    free (data);
                    return ZL_ERROR_PARSE_NAME;
                } else {
                    if (data[i] == ' ' || data[i] == '=') {
                        state = STATE_END_NAME;
                        while (data[i] == ' ') i++;
                        if (data[i] == '=') i++;
                        opt_name[opi] = 0;
                        index_opt = get_index_opt (cfg, index_group, opt_name);
                        if (index_opt == -1) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, ZL_ERROR_UNKNOWN_NAME);
                            return ZL_ERROR_UNKNOWN_NAME;
                        }
                        break;
                    }
                    opt_name[opi++] = data[i];
                }
                break;
            case STATE_END_NAME:
                while (data[i] == ' ') i++;
                type_of_opt = cfg->group[index_group].opt[index_opt].type;
                switch (type_of_opt) {
                    case ZL_TYPE_BOOL:
                    {
                        int error;
                        int pos;
                        bool ret = parse_bool(&data[i], &error, &pos);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, error);
                            return error;
                        }
                        cfg->group[index_group].opt[index_opt].v.val_bool = ret;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                    break;
                    case ZL_TYPE_INT32:
                    {
                        int error;
                        int pos;
                        int32_t ret = parse_int(&data[i], &error, &pos, false);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, error);
                            return error;
                        }

                        cfg->group[index_group].opt[index_opt].v.val_int32 = ret;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                    break;
                    case ZL_TYPE_INT64:
                    {
                        int error;
                        int pos;
                        int32_t ret = parse_int(&data[i], &error, &pos, true);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, error);
                            return error;
                        }

                        cfg->group[index_group].opt[index_opt].v.val_int32 = ret;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                        break;
                    case ZL_TYPE_STRING:
                    {
                        int error;
                        int pos;
                        char *ret = parse_string(&data[i], &error, &pos);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, error);
                            return error;
                        }

                        if (cfg->group[index_group].opt[index_opt].v.val_str) free (cfg->group[index_group].opt[index_opt].v.val_str);
                        cfg->group[index_group].opt[index_opt].v.val_str = ret;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                        break;
                }
                break;
        }

    }
}