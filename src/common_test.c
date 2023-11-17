// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
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

static MunitResult test_bitstr_uint64_t(
    const MunitParameter params[], void* data
) {

    {
        uint64_t value = 0b000000;
        munit_assert_string_equal(
            to_bitstr_uint64_t(value),
            "0000000000000000000000000000000000000000000000000000000000000000"
        );
    }

    {
        uint64_t value = 0b000001;
        munit_assert_string_equal(
            to_bitstr_uint64_t(value),
            "0000000000000000000000000000000000000000000000000000000000000001"
        );
    }

    {
        uint64_t value = 
            0b1100000000110000000000100000000000000000101000000000000000000011;
        munit_assert_string_equal(
            to_bitstr_uint64_t(value),
            "1100000000110000000000100000000000000000101000000000000000000011"
        );
    }

    {
        uint64_t value = 
            0b1000000000000000000000000000000000000000000000000000000000000000;
        munit_assert_string_equal(
            to_bitstr_uint64_t(value),
             "1000000000000000000000000000000000000000000000000000000000000000"
        );
    }

    return MUNIT_OK;
}

static MunitResult test_less_or_eq_pow2(
    const MunitParameter params[], void* data
) {
    munit_assert(koh_less_or_eq_pow2(2) == 2);
    munit_assert(koh_less_or_eq_pow2(3) == 2);
    munit_assert(koh_less_or_eq_pow2(32) == 32);
    munit_assert(koh_less_or_eq_pow2(63) == 32);
    munit_assert(koh_less_or_eq_pow2(65) == 64);
    munit_assert(koh_less_or_eq_pow2(20) == 16);
    munit_assert(koh_less_or_eq_pow2(11) == 8);
    return MUNIT_OK;
}



static MunitResult test_is_pow2(
    const MunitParameter params[], void* data
) {
    munit_assert(koh_is_pow2(2));
    munit_assert(koh_is_pow2(32));
    munit_assert(koh_is_pow2(64));
    munit_assert(koh_is_pow2(2048));
    munit_assert(!koh_is_pow2(20));
    munit_assert(!koh_is_pow2(11));
    return MUNIT_OK;
}

static MunitResult test_base_name(
    const MunitParameter params[], void* data
) {

    {
        const char *out = koh_extract_path(".lua");
        const char *ref = "";
        munit_assert_string_equal(ref, out);
    }

    {
        const char *out = koh_extract_path("/file");
        const char *ref = "";
        munit_assert_string_equal(ref, out);
    }

    {
        // Что делать с корневым каталогом?
        const char *out = koh_extract_path("/");
        const char *ref = "";
        munit_assert_string_equal(ref, out);
    }

    {
        const char *out = koh_extract_path("/some/path/dir1/dir2/file");
        const char *ref = "/some/path/dir1/dir2";
        munit_assert_string_equal(ref, out);
    }

    {
        const char *out = koh_extract_path("/some/path/file");
        const char *ref = "/some/path";
        munit_assert_string_equal(ref, out);
    }

    {
        const char *out = koh_extract_path("c.lua");
        const char *ref = "";
        munit_assert_string_equal(ref, out);
    }

    {
        // XXX: Удалять точку?
        const char *out = koh_extract_path("./file.exe");
        const char *ref = ".";
        munit_assert_string_equal(ref, out);
    }

    return MUNIT_OK;
}

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

// Проверка воспроизводимости последовательности чисел генератором при
// заданном семени
static MunitResult test_random(const MunitParameter params[], void* data) {
    {
        xorshift32_state rnd; 
        const int num = 10000;
        uint32_t rnd_values[num];

        memset(rnd_values, 0, sizeof(rnd_values));

        uint32_t seed = 124123;
        rnd.a = seed;

        for (int j = 0; j < num; j++) {
            rnd_values[j] = xorshift32_rand(&rnd);
        }

        rnd.a = seed;
        for (int j = 0; j < num; j++) {
            uint32_t value = xorshift32_rand(&rnd);
            munit_assert_uint32(value, ==, rnd_values[j]);
        }
    }
    return MUNIT_OK;
}

static MunitTest t_suite_random[] = {
    {
        .name =  (char*) "/random",
        .test = test_random,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static MunitTest t_suite_common[] = {
    {
        .name =  (char*) "/extract_filename",
        .test = test_extract_filename,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        (char*) "/koh_basename",
        test_base_name,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    {
        (char*) "/koh_is_pow2",
        test_is_pow2,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    {
        (char*) "/koh_less_or_eq_pow2",
        test_less_or_eq_pow2 ,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    {
        (char*) "/to_bitstr_uint64_t",
        test_bitstr_uint64_t,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};


static MunitSuite suite_nested[] = {
    {
        .prefix = (char*) "random",
        .tests =  t_suite_random,
        .suites = NULL,
        .iterations = 1,
        .options = MUNIT_SUITE_OPTION_NONE,
    },
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE } 
};

static const MunitSuite suite_root = {
    .prefix = (char*) "common",
    .tests =  t_suite_common,
    .suites = suite_nested,
    //.suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
};

int main(int argc, char **argv) {
    return munit_suite_main(&suite_root, (void*) "µnit", argc, argv);
}
