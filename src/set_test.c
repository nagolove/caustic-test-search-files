// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_destral_ecs.h"
#include "koh_set.h"
#include "munit.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Vectors {
    Vector2 *vecs;
    int     num;
};

static koh_SetAction iter_set_remove_all(
    const void *key, int key_len, void *udata
) {
    return koh_SA_remove_next;
}

static koh_SetAction iter_set_cmp(const void *key, int key_len, void *udata) {
    struct Vectors *lines = udata;

    for (int i = 0; i < lines->num; ++i) {
        if (!memcmp(&lines->vecs[i], key, key_len)) {
            return koh_SA_next;
        }
    }

    //printf("iter_set: key %s not found in each itertor\n", key);
    munit_assert(false);

    return koh_SA_next;
}

struct NumbersCtx {
    uint32_t    *arr;
    int         num;
};

static koh_SetAction iter_each(const void *key, int key_len, void *udata) {
    struct NumbersCtx *ctx = udata;
    for (int i = 0; i < ctx->num; ++i) {
        if (ctx->arr[i] == *((uint32_t*)key))
            return koh_SA_next;
    }
    munit_assert(false);
    return koh_SA_next;
}

static MunitResult test_each(
    const MunitParameter params[], void* data
) {
    uint32_t nums[] = {
        1, 3, 5, 7, 11, 13, 15,
    };
    int nums_num = sizeof(nums) / sizeof(nums[0]);

    koh_Set *set = set_new();
    for (int i = 0; i< nums_num; ++i) {
        set_add(set, &nums[i], sizeof(nums[i]));
    }

    struct NumbersCtx ctx = {
        .arr = nums, 
        .num = nums_num,
    };
    set_each(set, iter_each, &ctx);
    set_free(set);

    return MUNIT_OK;
}

static MunitResult test_compare(
    const MunitParameter params[], void* data
) {

    Vector2 vecs1[] = {
        { 1.,    0. },
        { 12.,   0. },
        { 0.1,   0. },
        { 1.3, -0.1 },
        { 0.,   0.5 },
        { 14,    0. },
    };
    int vecs1_num = sizeof(vecs1) / sizeof(vecs1[0]);

    Vector2 vecs2[] = {
        { 1.,    0.1 },
        { 12.,   0. },
        { 0.1,   0.1 },
        { 1.3, -0.1 },
        { 0.,   0.5 },
        { 14,    0. },
    };
    int vecs2_num = sizeof(vecs2) / sizeof(vecs2[0]);

    koh_Set *set1 = set_new();
    koh_Set *set2 = set_new();
    koh_Set *set3 = set_new();

    /*
     set1 == set3
     set1 != set2
     */

    for (int i = 0; i< vecs1_num; ++i) {
        set_add(set1, &vecs1[i], sizeof(vecs1[0]));
    }

    for (int i = 0; i< vecs2_num; ++i) {
        set_add(set2, &vecs2[i], sizeof(vecs2[0]));
    }

    for (int i = 0; i< vecs1_num; ++i) {
        set_add(set3, &vecs1[i], sizeof(vecs1[0]));
    }

    munit_assert(set_compare(set1, set3));
    munit_assert(!set_compare(set1, set2));

    set_free(set1);
    set_free(set2);
    set_free(set3);

    return MUNIT_OK;
}

struct TestAddRemoveCtx {
    int *examples;
    int examples_num;
    int examples_remove_value;
};

static koh_SetAction iter_set_remove(
    const void *key, int key_len, void *udata
) {
    const int *key_value = key;
    if (!udata) {
        fprintf(stderr, "iter_set_check: udata == NULL\n");
        abort();
    }

    struct TestAddRemoveCtx *ctx = udata;
    if (*key_value == ctx->examples_remove_value) {
        return koh_SA_break;
    }
    return koh_SA_break;
}

static koh_SetAction iter_set_check(
    const void *key, int key_len, void *udata
) {
    const int *key_value = key;
    if (!udata) {
        fprintf(stderr, "iter_set_check: udata == NULL\n");
        abort();
    }

    struct TestAddRemoveCtx *ctx = udata;
    for (int i = 0; i < ctx->examples_num; ++i) {
        if (ctx->examples[i] == *key_value) {
            return koh_SA_next;
        }
    }

    printf("iter_set_remove: key_value = %d not found\n", *key_value);
    munit_assert(false);
    return koh_SA_next;
}

static MunitResult test_add_remove(
    const MunitParameter params[], void* data
) {
    int examples[] = {
        1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 20, 23, 24
    };
    koh_Set *set = set_new();

    int examples_num = sizeof(examples) / sizeof(examples[0]);
    struct TestAddRemoveCtx ctx = {
        .examples = calloc(sizeof(int), examples_num * 2),
        .examples_num = examples_num,
    };
    assert(ctx.examples);
    memcpy(ctx.examples, examples, sizeof(int) * examples_num);

    for (int i = 0; i < examples_num; i++) {
        set_add(set, &examples[i], sizeof(int));
    }

    set_each(set, iter_set_check, &ctx);

    set_remove(set, &ctx.examples[0], sizeof(int));
    ctx.examples[0] = 0;

    set_remove(set, &ctx.examples[1], sizeof(int));
    ctx.examples[1] = 0;

    set_each(set, iter_set_check, &ctx);

    ctx.examples[ctx.examples_num++] = 101;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 102;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 103;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 104;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 105;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    set_each(set, iter_set_check, &ctx);

    set_remove(set, &ctx.examples[5], sizeof(int));
    ctx.examples[5] = 0;

    set_remove(set, &ctx.examples[5], sizeof(int));
    ctx.examples[5] = 0;

    set_each(set, iter_set_check, &ctx);

    free(ctx.examples);
    set_free(set);
    return MUNIT_OK;
}

static MunitResult test_add_remove_each(
    const MunitParameter params[], void* data
) {
    int examples[] = {
        1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 20, 23, 24
    };
    koh_Set *set = set_new();

    int examples_num = sizeof(examples) / sizeof(examples[0]);
    struct TestAddRemoveCtx ctx = {
        .examples = calloc(sizeof(int), examples_num * 2),
        .examples_num = examples_num,
    };
    assert(ctx.examples);
    memcpy(ctx.examples, examples, sizeof(int) * examples_num);

    for (int i = 0; i < examples_num; i++) {
        set_add(set, &examples[i], sizeof(int));
    }

    set_each(set, iter_set_check, &ctx);

    set_each(set, iter_set_remove, &ctx);

    set_each(set, iter_set_check, &ctx);

    ctx.examples[ctx.examples_num++] = 101;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 102;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 103;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 104;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    ctx.examples[ctx.examples_num++] = 105;
    set_add(set, &ctx.examples[ctx.examples_num - 1], sizeof(int));

    set_each(set, iter_set_check, &ctx);

    set_remove(set, &ctx.examples[5], sizeof(int));
    ctx.examples[5] = 0;

    set_remove(set, &ctx.examples[5], sizeof(int));
    ctx.examples[5] = 0;

    set_each(set, iter_set_check, &ctx);

    free(ctx.examples);
    set_free(set);
    return MUNIT_OK;
}

static MunitResult test_new_add_exist_free(
    const MunitParameter params[], void* data
) {
    koh_Set *set = set_new();
    munit_assert_ptr_not_null(set);

    Vector2 vecs[] = {
        { 1.,    0. },
        { 12.,   0. },
        { 0.1,   0. },
        { 1.3, -0.1 },
        { 0.,   0.5 },
        { 14,    0. },
    };

    Vector2 other_vecs[] = {
        { -1.,   0. },
        { 12.,  NAN },
        { 0.1,  0.1 },
        { 1.3,  0.1 },
        { 0.,  0.51 },
        { -14,   0. },
    };

    int vecs_num = sizeof(vecs) / sizeof(vecs[0]);
    int other_vecs_num = sizeof(other_vecs) / sizeof(other_vecs[0]);

    for (int i = 0; i< vecs_num; ++i) {
        set_add(set, &vecs[i], sizeof(vecs[0]));
    }

    for (int i = 0; i< vecs_num; ++i) {
        munit_assert(set_exist(set, &vecs[i], sizeof(vecs[0])));
    }

    for (int i = 0; i< other_vecs_num; ++i) {
        munit_assert(!set_exist(set, &other_vecs[i], sizeof(other_vecs[0])));
    }

    struct Vectors vectors_ctx = {
        //.vecs = (Vector2**)vecs, 
        .vecs = vecs, 
        .num = vecs_num,
    };
    // проверка всех ключей
    set_each(set, iter_set_cmp, &vectors_ctx);

    // удаление все ключей
    set_each(set, iter_set_remove_all, &vectors_ctx);

    Vector2 pi_vec = { M_PI, M_PI };
    set_add(set, &pi_vec, sizeof(Vector2));

    for (int i = 0; i< vecs_num; ++i) {
        munit_assert(!set_exist(set, &vecs[i], sizeof(Vector2)));
    }

    munit_assert(set_exist(set, &pi_vec, sizeof(Vector2)));
    set_clear(set);
    munit_assert(!set_exist(set, &pi_vec, sizeof(Vector2)));

    set_free(set);
    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
  {
    (char*) "/add_remove_each",
    test_add_remove_each,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/add_remove",
    test_add_remove,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/new_add_exist_free",
    test_new_add_exist_free,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/each",
    test_each,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/compare",
    test_compare,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "set", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}