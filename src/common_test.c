// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_common.h"
#include "koh_metaloader.h"
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

static MunitResult test_extract_filename(
    const MunitParameter params[], void* data
) {

    {
        const char *in = "in.lua";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        //printf("test_extract_filename: in '%s', out '%s'\n", in, out);
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "in";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "/in";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "/in.lua";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "//usr/in.lua";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "./in.lua";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "/usr/share/code/secret/password/in.lua";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    {
        const char *in = "/usr/share/code/secret/password/in";
        const char *res = "in";
        const char *out = extract_filename(in, ".lua");
        /*printf("test_extract_filename: in '%s', out '%s'\n", in, out);*/
        munit_assert_string_equal(res, out);
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    {
        (char*) "/extract_filename",
        test_extract_filename,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "common", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
