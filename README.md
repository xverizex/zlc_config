# zlc_config

This is a configure library which present a simple interface for get params from a config file.

# How do this use?

There is a config file
```commandline
[Net]
debug = true
port = 8000
[Main]
debug = true
private = ["most \"powerful\"", "better", "high top"]

```

Define enum groups and options for each group.
```C++
enum Groups {
    GROUP_NET,
    GROUP_MAIN,
    N_GROUPS
};

enum OptionsNet {
    OPT_NET_DEBUG,
    OPT_NET_PORT,
    N_NET_OPTS
};

enum OptionsMain {
    OPT_MAIN_DEBUG,
    OPT_MAIN_PRIVATE,
    N_MAIN_OPTS
};
```
Also you may to set an error function.
```C++
void error_func (struct zl_config *cfg, int group, int opt, int error) {
    printf ("error: %d\n", error);
}
```
Now, init config structure and initialize groups and options.
```C++
    struct zl_config *cfg = zl_config_init(N_GROUPS);
    zl_config_init_group(cfg, GROUP_NET, "Net", N_NET_OPTS);
    zl_config_init_group(cfg, GROUP_MAIN, "Main", N_MAIN_OPTS);

    zl_config_add_option(cfg, GROUP_NET, OPT_NET_DEBUG, ZL_TYPE_BOOL, "debug", NULL);
    zl_config_add_option(cfg, GROUP_NET, OPT_NET_PORT, ZL_TYPE_INT32, "port", NULL);

    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_DEBUG, ZL_TYPE_BOOL, "debug", (void *) true);
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, ZL_TYPE_ARRAY_STRING, "private", NULL);

    zl_config_set_comment_option(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, "set various text");

    zl_config_parse(cfg, "../test.conf");
```
Now, you are to possible get a config value.
```c++
    int port = zl_config_get_int32(cfg, GROUP_NET, OPT_NET_PORT);
    bool debug_main = zl_config_get_bool(cfg, GROUP_MAIN, OPT_MAIN_DEBUG);

    printf ("port: %d\n", port);
    printf ("debug_main: %s\n", debug_main == 0 ? "false" : "true");
```

You can save this and a config file will be look like this. 
We add a comment to a field *private*.
```commandline
[Net]
debug = true
port = 8000
[Main]
debug = true
# set various text
private = [most "powerful", better, high top]
```
# All code
```c++
#include <stdio.h>
#include "zlc_config/config.h"


enum Groups {
    GROUP_NET,
    GROUP_MAIN,
    N_GROUPS
};

enum OptionsNet {
    OPT_NET_DEBUG,
    OPT_NET_PORT,
    N_NET_OPTS
};

enum OptionsMain {
    OPT_MAIN_DEBUG,
    OPT_MAIN_PRIVATE,
    N_MAIN_OPTS
};

void error_func (struct zl_config *cfg, int group, int opt, int error) {
    printf ("error: %d\n", error);
}

int main() {
    struct zl_config *cfg = zl_config_init(N_GROUPS);
    zl_config_init_group(cfg, GROUP_NET, "Net", N_NET_OPTS);
    zl_config_init_group(cfg, GROUP_MAIN, "Main", N_MAIN_OPTS);

    zl_config_add_option(cfg, GROUP_NET, OPT_NET_DEBUG, ZL_TYPE_BOOL, "debug", NULL);
    zl_config_add_option(cfg, GROUP_NET, OPT_NET_PORT, ZL_TYPE_INT32, "port", NULL);

    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_DEBUG, ZL_TYPE_BOOL, "debug", (void *) true);
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, ZL_TYPE_ARRAY_STRING, "private", NULL);

    zl_config_set_comment_option(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, "set various text");

    zl_config_parse(cfg, "../test.conf");

    int port = zl_config_get_int32(cfg, GROUP_NET, OPT_NET_PORT);
    bool debug_main = zl_config_get_bool(cfg, GROUP_MAIN, OPT_MAIN_DEBUG);

    printf ("port: %d\n", port);
    printf ("debug_main: %s\n", debug_main == 0 ? "false" : "true");

    /* print arrays private */
    int size_private;
    const char **str_array = zl_config_get_array_string(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, &size_private);
    for (int i = 0; i < size_private; i++) {
        printf ("%s\n", str_array[i]);
    }

    zl_config_save(cfg);
}
```

# Output
```commandline
port: 8000
debug_main: true
most "powerful"
better
high top
```
