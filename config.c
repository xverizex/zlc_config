//
// Created by cf on 6/11/22.
//

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct zl_config {
    char *config_file;
    struct zl_group *group;
    int size_group;
    pthread_mutex_t mutex;

    void (*error_func)(struct zl_config *cfg, int group, int opt, const char *p_group, const char *p_option, int error);
};

struct zl_config *zl_config_init (int size_groups) {
    struct zl_config *cfg = calloc (1, sizeof (struct zl_config));
    cfg->group = calloc (size_groups, sizeof (struct zl_group));
    cfg->size_group = size_groups;
    cfg->config_file = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy (&cfg->mutex, &mutex, sizeof (mutex));

    pthread_mutex_init(&cfg->mutex, NULL);
    return cfg;
}

void zl_config_init_group (struct zl_config *cfg, int group, const char *name, int size_names) {
    pthread_mutex_lock(&cfg->mutex);
    if (cfg->group[group].opt) {
        free (cfg->group[group].opt);
    }
    if (cfg->group[group].name) {
        free (cfg->group[group].name);
    }
    cfg->group[group].opt = calloc (size_names, sizeof (struct zl_option));
    cfg->group[group].name = strdup (name);
    cfg->group[group].size_opt = size_names;
    pthread_mutex_unlock(&cfg->mutex);
}

void zl_config_set_filepath (struct zl_config *cfg, const char *filepath) {
    pthread_mutex_lock(&cfg->mutex);
    if (cfg->config_file) free (cfg->config_file);
    cfg->config_file = strdup (filepath);
    pthread_mutex_unlock(&cfg->mutex);
}

void zl_config_set_comment_option (struct zl_config *cfg, int group, int opt, const char *comment) {
    pthread_mutex_lock(&cfg->mutex);
    struct zl_option *option = &cfg->group[group].opt[opt];
    if (option->comment) free (option->comment);
    option->comment = strdup (comment);
    pthread_mutex_unlock(&cfg->mutex);
}

static void print_value_option (FILE *fp, struct zl_group *g, int opt) {
    switch (g->opt[opt].type) {
        case ZL_TYPE_ARRAY_STRING:
            fprintf (fp, "[");
            for (int i = 0; i < g->opt[opt].size_array; i++) {
                char *val = g->opt[opt].v.val_array_str[i];
                int len = strlen (val);
                fprintf (fp, "\"");
                for (int l = 0; l < len; l++) {
                    if (val[l] == '"') fprintf (fp, "\\");
                    fprintf (fp, "%c", val[l]);
                }
                fprintf (fp, "\"");
                if ((i + 1) < g->opt[opt].size_array) fprintf (fp, ", ");
            }
            fprintf (fp, "]");
            fprintf (fp, "\n");
            break;
        case ZL_TYPE_ARRAY_INT64:
            fprintf (fp, "[");
            for (int i = 0; i < g->opt[opt].size_array; i++) {
                fprintf (fp, "%ld", g->opt[opt].v.val_array_int64[i]);
                if ((i + 1) < g->opt[opt].size_array) fprintf (fp, ", ");
            }
            fprintf (fp, "]");
            fprintf (fp, "\n");
            break;
        case ZL_TYPE_ARRAY_INT32:
            fprintf (fp, "[");
            for (int i = 0; i < g->opt[opt].size_array; i++) {
                fprintf (fp, "%d", g->opt[opt].v.val_array_int32[i]);
                if ((i + 1) < g->opt[opt].size_array) fprintf (fp, ", ");
            }
            fprintf (fp, "]");
            fprintf (fp, "\n");
            break;
        case ZL_TYPE_ARRAY_BOOL:
            fprintf (fp, "[");
            for (int i = 0; i < g->opt[opt].size_array; i++) {
                fprintf (fp, "%s", g->opt[opt].v.val_array_bool[i] == 0 ? "false" : "true");
                if ((i + 1) < g->opt[opt].size_array) fprintf (fp, ", ");
            }
            fprintf (fp, "]");
            fprintf (fp, "\n");
            break;
        case ZL_TYPE_STRING:
        {
            char *val = g->opt[opt].v.val_str;
            int len = strlen (val);
            fprintf (fp, "\"");
            for (int l = 0; l < len; l++) {
                if (val[l] == '"') fprintf (fp, "\\");
                fprintf (fp, "%c", val[l]);
            }
            fprintf (fp, "\"");
            fprintf (fp, "\n");
        }
            break;
        case ZL_TYPE_BOOL:
            fprintf (fp, "%s", g->opt[opt].v.val_bool == 0 ? "false" : "true");
            fprintf (fp, "\n");
            break;
        case ZL_TYPE_INT32:
            fprintf (fp, "%d", g->opt[opt].v.val_int32);
            fprintf (fp, "\n");
            break;
        case ZL_TYPE_INT64:
            fprintf (fp, "%ld", g->opt[opt].v.val_int64);
            fprintf (fp, "\n");
            break;
        default:
            break;
    }
}

int zl_config_save (struct zl_config *cfg) {
    pthread_mutex_lock(&cfg->mutex);

    FILE *fp = fopen (cfg->config_file, "w");

    for (int grp = 0; grp < cfg->size_group; grp++) {
        fprintf (fp, "[%s]\n", cfg->group[grp].name);

        struct zl_group *g = &cfg->group[grp];
        /* save all options */
        for (int opt = 0; opt < g->size_opt; opt++) {
            if (g->opt[opt].comment) {
                fprintf (fp, "# %s\n", g->opt[opt].comment);
            }
            fprintf (fp, "%s = ", g->opt[opt].name);
            print_value_option (fp, g, opt);
        }
    }

    fclose (fp);

    pthread_mutex_unlock(&cfg->mutex);
}

void zl_config_add_option (struct zl_config *cfg, int group, int name, int type, const char *str_name, void *default_value) {
    pthread_mutex_lock(&cfg->mutex);

    cfg->group[group].opt[name].type = type;
    cfg->group[group].opt[name].name = strdup (str_name);
    switch(type) {
        case ZL_TYPE_INT32: cfg->group[group].opt[name].v.val_int32 = default_value ? (int32_t) default_value: 0; break;
        case ZL_TYPE_INT64: cfg->group[group].opt[name].v.val_int64 = default_value ? (int64_t) default_value: 0L; break;
        case ZL_TYPE_BOOL: cfg->group[group].opt[name].v.val_bool = default_value ? ((bool) default_value): false; break;
        case ZL_TYPE_STRING: cfg->group[group].opt[name].v.val_str = default_value ? (char *) default_value: NULL; break;
        case ZL_TYPE_ARRAY_BOOL: cfg->group[group].opt[name].v.val_array_bool = default_value ? (bool *) default_value: NULL; break;
        case ZL_TYPE_ARRAY_INT32: cfg->group[group].opt[name].v.val_array_int32 = default_value ? (int32_t *) default_value: NULL; break;
        case ZL_TYPE_ARRAY_INT64: cfg->group[group].opt[name].v.val_array_int64 = default_value ? (int64_t *) default_value: NULL; break;
        case ZL_TYPE_ARRAY_STRING: cfg->group[group].opt[name].v.val_array_str = default_value ? (char **) default_value: NULL; break;
        default:
            break;
    }

    pthread_mutex_unlock(&cfg->mutex);
}

void zl_config_set_error_func (struct zl_config *cfg, void (*func) (struct zl_config *cfg, int group, int opt,const char *group_name, const char *opt_name, int error)) {
    cfg->error_func = func;
}


bool zl_config_get_bool (struct zl_config *cfg, int group, int name) {
    pthread_mutex_lock(&cfg->mutex);
    bool val = cfg->group[group].opt[name].v.val_bool;
    pthread_mutex_unlock(&cfg->mutex);
    return val;
}

int32_t zl_config_get_int32 (struct zl_config *cfg, int group, int name) {
    pthread_mutex_lock(&cfg->mutex);
    int32_t val = cfg->group[group].opt[name].v.val_int32;
    pthread_mutex_unlock(&cfg->mutex);
}

int64_t zl_config_get_int64 (struct zl_config *cfg, int group, int name) {
    pthread_mutex_lock(&cfg->mutex);
    int64_t val = cfg->group[group].opt[name].v.val_int64;
    pthread_mutex_unlock(&cfg->mutex);
    return val;
}

const char *zl_config_get_string (struct zl_config *cfg, int group, int name) {
    pthread_mutex_lock(&cfg->mutex);
    const char *str = cfg->group[group].opt[name].v.val_str;
    pthread_mutex_unlock(&cfg->mutex);
    return str;
}

const char **zl_config_get_array_string (struct zl_config *cfg, int group, int name, int *size) {
    pthread_mutex_lock(&cfg->mutex);
    *size = cfg->group[group].opt[name].size_array;
    char **val = cfg->group[group].opt[name].v.val_array_str;
    pthread_mutex_unlock(&cfg->mutex);
    return val;
}

const bool *zl_config_get_array_bool (struct zl_config *cfg, int group, int name, int *size) {
    pthread_mutex_lock(&cfg->mutex);
    *size = cfg->group[group].opt[name].size_array;
    bool *val = cfg->group[group].opt[name].v.val_array_bool;
    pthread_mutex_unlock(&cfg->mutex);
}

const int64_t *zl_config_get_array_int64 (struct zl_config *cfg, int group, int name, int *size) {
    pthread_mutex_lock(&cfg->mutex);
    *size = cfg->group[group].opt[name].size_array;
    int64_t *val = cfg->group[group].opt[name].v.val_array_int64;
    pthread_mutex_unlock(&cfg->mutex);
    return val;
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

static int32_t parse_int32 (char *data, int *error, int *pos) {
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
        result = atoi (data);
    }

    if (temp_e > 0) *e = temp_e;
    if (temp_s > 0) *s = temp_s;

    return result;
}

static int64_t parse_int64 (char *data, int *error, int *pos) {
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
            *error = ZL_ERROR_INT64;
            break;
        }
    }

    if (*error == 0) {
        result = atol (data);
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

static char *parse_string_quote (char *data, int *error, int *pos) {
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

    *pos = i;

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

static char **parse_array_string (char *data, int *error, int *pos, int *coun) {
    *error = 0;
    *coun = 0;

    if (data[0] != '[') {
        *error = ZL_ERROR_PARSE_ARRAY_STRING;
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
    } else {
        *pos = strlen (data);
    }

    int len = strlen (data);

    int count = 0;
    int state_quote = 0;
    int sym = 0;
    for (int i = 0; (data[i] != ']' && data[i] != '\0') && i < len; i++) {
        if (data[i] == '\\' && data[i + 1] == '"') { i++; continue; }
        if (data[i] == '"') { sym = 1; state_quote++; }
        if (data[i] == ',') {
		sym = 1;
            if (state_quote == 2) {
                count++;
                state_quote = 0;
            }
        }
    }
    if (sym == 1) count++;


    char **str = calloc (count, sizeof (void *));
    if (count == 0) {
	    *coun = count;
	    return str;
    }

    int index_str = 0;

    int i = 1;
    while (data[i] == ' ') i++;

    for (; data[i] != '\0' && i < len; i++) {
        int error;
        int pos;
        while (data[i] == ' ') i++;
        str[index_str] = parse_string_quote(&data[i], &error, &pos);
        i += pos;
        index_str++;
        i += 2;
        while (data[i] == ' ') i++;
        if (data[i] == ',' || data[i] == ']') {
            if (data[i] == ']') {
                break;
            }
            i++;
            while (data[i] == ' ') i++;
            i--;
        }
    }

    *coun = count;

    return str;
}

static bool *parse_array_bool (char *data, int *error, int *pos, int *coun) {
    *error = 0;
    *coun = 0;

    if (data[0] != '[') {
        *error = ZL_ERROR_PARSE_ARRAY_STRING;
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

    int count = 0;
    int sym = 0;
    for (int i = 0; (data[i] != ']' && data[i] != '\0') && i < len; i++) {
	    if (data[i] > 32 && data[i] <= 127) sym = 1;
        if (data[i] == ',') count++;
    }
    if (sym == 1) count++;

    char **str = calloc (count, sizeof (void *));
    int index_str = 0;
    if (count == 0) {
	    *coun = count;
	    return str;
    }

    int i = 1;
    while (data[i] == ' ') i++;
    int start = i;
    for (; data[i] != '\0' && i < len; i++) {
        while (data[i] == ' ') i++;
        if (data[i] == ',' || data[i] == ']') {
            int len = i - start;
            str[index_str] = calloc (len + 1, 1);
            strncpy (str[index_str], &data[start], len);
            char *e = strrchr (str[index_str], ' ');
            if (e) *e = 0;
            index_str++;
            if (data[i] == ']') break;
            i++;
            while (data[i] == ' ') i++;
            start = i;

        }
    }

    *coun = count;


    for (int i = 0; i < count; i++) {
        if (!strncmp (str[i], "true", 5) || !strncmp (str[i], "false", 6)) continue;

        for (int i = 0; i < count; i++) {
            free (str[i]);
        }
        free (str);
        *coun = 0;
        *error = ZL_ERROR_PARSE_ARRAY_BOOL;
        str = NULL;
        goto out;
    }

    bool *b = calloc (count, sizeof(bool));
    for (int i = 0; i < count; i++) {
        if (!strncmp (str[i], "true", 5)) {
            b[i] = true;
        } else {
            b[i] = false;
        }
    }

out:
    return b;
}

static int64_t *parse_array_int64 (char *data, int *error, int *pos, int *coun) {
    *error = 0;
    *coun = 0;

    if (data[0] != '[') {
        *error = ZL_ERROR_PARSE_ARRAY_INT64;
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


    int count = 0;
    int sym = 0;
    for (int i = 0; (data[i] != ']' && data[i] != '\0') && i < len; i++) {
	    if (data[i] > 32 && data[i] < 127) sym = 1;
        if (data[i] == ',') count++;
    }
    if (sym == 1) count++;

    char **str = calloc (count, sizeof (void *));
    int index_str = 0;
    if (count == 0) {
	    *coun = count;
	    return str;
    }

    int i = 1;
    while (data[i] == ' ') i++;
    int start = i;
    for (; data[i] != '\0' && i < len; i++) {
        while (data[i] == ' ') i++;
        if (data[i] == ',' || data[i] == ']') {
            int len = i - start;
            str[index_str] = calloc (len + 1, 1);
            strncpy (str[index_str], &data[start], len);
            char *e = strrchr (str[index_str], ' ');
            if (e) *e = 0;
            index_str++;
            if (data[i] == ']') break;
            i++;
            while (data[i] == ' ') i++;
            start = i;

        }
    }

    *coun = count;

    for (int i = 0; i < count; i++) {
        int len_str_item = strlen (str[i]);
        int invalid = 0;
        for (int ii = 0; ii < len_str_item; ii++) {
            if (str[i][ii] < '0' || str[i][ii] > '9') invalid = 1;
        }

        if (invalid) {
            for (int i = 0; i < count; i++) {
                free (str[i]);
            }
            free (str);
            *coun = 0;
            *error = ZL_ERROR_PARSE_ARRAY_INT64;
            str = NULL;
            goto out;
        }

    }

    int64_t *b = calloc (count, sizeof(int64_t));
    for (int i = 0; i < count; i++) {
        b[i] = atol (str[i]);
    }

    out:
    return b;
}

static int get_index_group (struct zl_config *cfg, const char *name) {
    for (int i = 0; i < cfg->size_group; i++) {
        if (!strncmp (cfg->group[i].name, name, strlen (name) + 1)) return i;
    }

    return -1;
}

static int get_index_opt (struct zl_config *cfg, int group, const char *name) {
    struct zl_group *grp = &cfg->group[group];
    for (int i = 0; i < grp->size_opt; i++) {
        if (!strncmp (grp->opt[i].name, name, strlen (name) + 1)) return i;
    }

    return -1;
}

static char *get_data_file (const char *filepath, size_t *size_file) {
    *size_file = 0;
    FILE *fp = fopen (filepath, "r");
    if (!fp) {
	    perror ("fopen file config");
	    return NULL;
    }
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
    pthread_mutex_lock(&cfg->mutex);
    if (cfg->config_file) {
        free (cfg->config_file);
    }
    cfg->config_file = strdup (filepath);

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
            while (data[i] != '\n' && i < size_file) i++;
            continue;
        }
        if ((data[i] == '\r' || data[i] == '\n')) {
            if (gri > 0) {
                if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, ZL_ERROR_PARSE_GROUP);
                pthread_mutex_unlock(&cfg->mutex);
                return ZL_ERROR_PARSE_GROUP;
            }
            continue;
        }
        if (data[i] == '[' && gri != 0) {
            free (data);
            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, ZL_ERROR_PARSE_GROUP);
            pthread_mutex_unlock(&cfg->mutex);
            return ZL_ERROR_PARSE_GROUP;
        } else if (data[i] == '[' && state != STATE_END_NAME) {
            state = STATE_NEW_GROUP;
            continue;
        }

        if (data[i] == ']' && gri == 0) {
            free (data);
            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, ZL_ERROR_PARSE_GROUP);
            pthread_mutex_unlock(&cfg->mutex);
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
                    if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, ZL_ERROR_UNKNOWN_GROUP);
                    pthread_mutex_unlock(&cfg->mutex);
                    return ZL_ERROR_UNKNOWN_GROUP;
                }
                cfg->group[index_group].name = strdup (group_name);
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
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, ZL_ERROR_UNKNOWN_NAME);
                            pthread_mutex_unlock(&cfg->mutex);
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
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
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
                        int32_t ret = parse_int32(&data[i], &error, &pos);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
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
                        int64_t ret = parse_int64(&data[i], &error, &pos);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
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
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
                            return error;
                        }

                        if (cfg->group[index_group].opt[index_opt].v.val_str) free (cfg->group[index_group].opt[index_opt].v.val_str);
                        cfg->group[index_group].opt[index_opt].v.val_str = ret;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                        break;
                    case ZL_TYPE_ARRAY_STRING:
                    {
                        int error;
                        int pos;
                        int count = 0;
                        char **ret = parse_array_string(&data[i], &error, &pos, &count);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
                            return error;
                        }

                        if (cfg->group[index_group].opt[index_opt].v.val_array_str) {
                            struct zl_option *opt = &cfg->group[index_group].opt[index_opt];
                            for (int i = 0; i < opt->size_array; i++) {
                                free (opt->v.val_array_str[i]);
                            }
                            free (opt->v.val_array_str);
                            cfg->group[index_group].opt[index_opt].v.val_array_str = NULL;
                        }
                        cfg->group[index_group].opt[index_opt].v.val_array_str = ret;
                        cfg->group[index_group].opt[index_opt].size_array = count;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                    break;
                    case ZL_TYPE_ARRAY_BOOL:
                    {

                        int error;
                        int pos;
                        int count = 0;
                        bool *ret = parse_array_bool(&data[i], &error, &pos, &count);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
                            return error;
                        }

                        if (cfg->group[index_group].opt[index_opt].v.val_array_bool) {
                            free (cfg->group[index_group].opt[index_opt].v.val_array_bool);
                        }
                        cfg->group[index_group].opt[index_opt].v.val_array_bool = ret;
                        cfg->group[index_group].opt[index_opt].size_array = count;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                    break;
                    case ZL_TYPE_ARRAY_INT64:
                    {

                        int error;
                        int pos;
                        int count = 0;
                        int64_t *ret = parse_array_int64(&data[i], &error, &pos, &count);
                        i += pos;
                        if (error < 0) {
                            free (data);
                            if (cfg->error_func) cfg->error_func (cfg, index_group, index_opt, group_name, opt_name, error);
                            pthread_mutex_unlock(&cfg->mutex);
                            return error;
                        }

                        if (cfg->group[index_group].opt[index_opt].v.val_array_int64) {
                            free (cfg->group[index_group].opt[index_opt].v.val_array_int64);
                        }
                        cfg->group[index_group].opt[index_opt].v.val_array_int64 = ret;
                        cfg->group[index_group].opt[index_opt].size_array = count;
                        state = STATE_NEW_NAME;
                        opi = 0;
                    }
                        break;
                }

                break;
        }

    }

    pthread_mutex_unlock(&cfg->mutex);
}
