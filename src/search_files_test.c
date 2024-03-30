// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_b2.h"
#include "box2d/box2d.h"
#include "box2d/types.h"
#include "koh_common.h"
#include "koh_metaloader.h"
#include "koh_rand.h"
#include "koh_set.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "munit.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool verbose = false;

static MunitResult test_new_shutdown(
    const MunitParameter params[], void* data
) {
    return MUNIT_OK;
}

static MunitTest t_suite_common[] = {
    {
        .name =  (char*) "/new_shutdown",
        .test = test_new_shutdown,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};


static const MunitSuite suite_root = {
    .prefix = (char*) "search_files",
    .tests =  t_suite_common,
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
};

int main(int argc, char **argv) {
    return munit_suite_main(&suite_root, (void*) "µnit", argc, argv);
}
