# zlc_config

This is a configure library which present a simple interface for get params from a config file.

# How this use?
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

    zl_config_parse(cfg, "../test.conf");

    int port = zl_config_get_int32(cfg, GROUP_NET, OPT_NET_PORT);

    printf ("port: %d\n", port);
}

```