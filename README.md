# zlc_config

This is a configure library which present a simple interface for get params from a config file.

# How this use?

There is a config file
```commandline
[Net]
debug = true
port = 8000

[Main]
debug = true
count = 16384
coin = "BTC and \"trust network\" and simple hello"
private = [ most powerful, better, high top]
tonus = [124, 364, 512]
bools = [true, false, false]
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
    OPT_MAIN_COUNT,
    OPT_MAIN_COIN,
    OPT_MAIN_PRIVATE,
    OPT_MAIN_TONUS,
    OPT_MAIN_BOOLS,
    N_MAIN_OPTS
};
```
Also you may to set an error function.
```C++
void error_func (struct zl_config *cfg, int group, int opt, int error) {
    printf ("error: %d\n", error);
}
```
Now, init a config structure and initialize groups and options.
```C++
    struct zl_config *cfg = zl_config_init(N_GROUPS);
    zl_config_init_group(cfg, GROUP_NET, "Net", N_NET_OPTS);
    zl_config_init_group(cfg, GROUP_MAIN, "Main", N_MAIN_OPTS);

    zl_config_add_option(cfg, GROUP_NET, OPT_NET_DEBUG, ZL_TYPE_BOOL, "debug");
    zl_config_add_option(cfg, GROUP_NET, OPT_NET_PORT, ZL_TYPE_INT32, "port");

    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_DEBUG, ZL_TYPE_BOOL, "debug");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_COUNT, ZL_TYPE_INT32, "count");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_COIN, ZL_TYPE_STRING, "coin");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, ZL_TYPE_ARRAY_STRING, "private");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_TONUS, ZL_TYPE_ARRAY_INT64, "tonus");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_BOOLS, ZL_TYPE_ARRAY_BOOL, "bools");

    zl_config_parse(cfg, "../test.conf");
```
Now, you are to possible get a config value.
```c++
    int port = zl_config_get_int32(cfg, GROUP_NET, OPT_NET_PORT);
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
    OPT_MAIN_COUNT,
    OPT_MAIN_COIN,
    OPT_MAIN_PRIVATE,
    OPT_MAIN_TONUS,
    OPT_MAIN_BOOLS,
    N_MAIN_OPTS
};

void error_func (struct zl_config *cfg, int group, int opt, int error) {
    printf ("error: %d\n", error);
}

int main() {
    struct zl_config *cfg = zl_config_init(N_GROUPS);
    zl_config_init_group(cfg, GROUP_NET, "Net", N_NET_OPTS);
    zl_config_init_group(cfg, GROUP_MAIN, "Main", N_MAIN_OPTS);

    zl_config_add_option(cfg, GROUP_NET, OPT_NET_DEBUG, ZL_TYPE_BOOL, "debug");
    zl_config_add_option(cfg, GROUP_NET, OPT_NET_PORT, ZL_TYPE_INT32, "port");

    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_DEBUG, ZL_TYPE_BOOL, "debug");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_COUNT, ZL_TYPE_INT32, "count");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_COIN, ZL_TYPE_STRING, "coin");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, ZL_TYPE_ARRAY_STRING, "private");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_TONUS, ZL_TYPE_ARRAY_INT64, "tonus");
    zl_config_add_option(cfg, GROUP_MAIN, OPT_MAIN_BOOLS, ZL_TYPE_ARRAY_BOOL, "bools");





    zl_config_parse(cfg, "../test.conf");

    int port = zl_config_get_int32(cfg, GROUP_NET, OPT_NET_PORT);
    const char *coin = zl_config_get_string(cfg, GROUP_MAIN, OPT_MAIN_COIN);

    printf ("port: %d\n", port);
    printf ("coin: %s\n", coin);

    /* print arrays */
    int size_private;
    const char **str_array = zl_config_get_array_string(cfg, GROUP_MAIN, OPT_MAIN_PRIVATE, &size_private);
    for (int i = 0; i < size_private; i++) {
        printf ("%s\n", str_array[i]);
    }

    int size_tonus;
    const int64_t *tonus = zl_config_get_array_int64(cfg, GROUP_MAIN, OPT_MAIN_TONUS, &size_tonus);
    for (int i = 0; i < size_tonus; i++) {
        printf ("%ld\n", tonus[i]);
    }

    int size_bools;
    const bool *bools = zl_config_get_array_int64(cfg, GROUP_MAIN, OPT_MAIN_BOOLS, &size_bools);
    for (int i = 0; i < size_bools; i++) {
        printf ("%d\n", bools[i]);
    }

}
```

# Output
```commandline
port: 8000
coin: BTC and "trust network" and simple hello
most powerful
better
high top
124
364
512
1
0
0
```
