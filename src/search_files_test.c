// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_strset.h"
#include "koh_common.h"
#include "munit.h"
#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*static bool verbose = false;*/

const char *dir_name = "./file2search";

struct FileSearchFixture {
    char **files;
    int files_num;
};

static StrSetAction set_printer(const char *key, void *ctx) {
    printf("set_printer: '%s'\n", key);
    return SSA_remove;
}

static MunitResult test_new_shutdown_common(
    const MunitParameter params[], void* data, struct FilesSearchSetup *setup
) {
    struct FileSearchFixture *fixture = data;
    struct FilesSearchResult fsr = {};
    fsr = koh_search_files(setup);
    printf("fsr.num %d\n", fsr.num);

    StrSet *s1 = strset_new(), *s2 = strset_new();

    for (int i = 0; i < fsr.num; i++) {
        strset_add(s1, fsr.names[i]);
    }

    for (int i = 0; i < fixture->files_num; i++) {
        strset_add(s2, fixture->files[i]);
    }

    bool is_set_eq = strset_compare(s1, s2);
    printf("test_new_shutdown: is_set_eq %s\n", is_set_eq ? "true" : "false");

    munit_assert(is_set_eq);
    //strset_each(s1, set_printer, NULL);
    //printf("--------------------------------\n");
    //strset_each(s2, set_printer, NULL);

    koh_search_files_shutdown(&fsr);

    printf("test_new_shutdown: %p\n", data);

    strset_free(s1);
    strset_free(s2);

    return MUNIT_OK;
}


static MunitResult test_new_shutdown(
    const MunitParameter params[], void* data
) {
    struct FileSearchFixture *fixture = data;

    test_new_shutdown_common(params, data, &(struct FilesSearchSetup) {
        .regex_pattern = ".*",
        .path = dir_name,
        .deep = -1,
        .engine_pcre2 = false,
    });

    /*
    test_new_shutdown_common(params, data, &(struct FilesSearchSetup) {
        .regex_pattern = ".*",
        .path = dir_name,
        .deep = -1,
        .engine_pcre2 = true,
    });
    */

    return MUNIT_OK;
}

static void* setup_search_fixture(
        const MunitParameter params[], void* user_data) {

    printf("setup_files_create:\n");
    (void) params;

    // TODO: 
    // Создать каталог для файлов.
    //munit_assert(mkdir(dir_name, S_IRWXU) == -1);

    struct stat st = {};
    if (stat(dir_name, &st) == -1) {
        int ret = mkdir(dir_name, S_IRWXU);
        printf("ret %d\n", ret);
    }

    struct FileSearchFixture *fixture = calloc(1, sizeof(*fixture));
    assert(fixture);

    // Создать сколько-то файлов.
    fixture->files_num = 1000;
    fixture->files = calloc(fixture->files_num, sizeof(fixture->files[0]));

    for (int i = 0; i < fixture->files_num; i++) {
        char path[128] = {};
        char num[32] = {};
        sprintf(num, "%d", i);
        strcat(path, dir_name);
        strcat(path, "/");
        strcat(path, num);
        //printf("path: '%s'\n", path);
        FILE *f = fopen(path, "w");
        munit_assert_ptr_not_null(f);
        fixture->files[i] = strdup(path);
        fclose(f);
    }

    return fixture;
}

static void tear_down_search_fixture(void *f) {
    assert(f);
    struct FileSearchFixture *fixture = f;

    int res = 0;
    for (int i = 0; i < fixture->files_num; i++) {
        res = remove(fixture->files[i]);
        munit_assert(res == 0);
        //printf("tear_down_search_fixture: remove returned %d\n", res);
        free(fixture->files[i]);
    }

    res = rmdir(dir_name);
    munit_assert(res == 0);
    /*printf("tear_down_search_fixture: rmdir returned %d\n", res);*/

    if (fixture->files)
        free(fixture->files);

    free(f);
}

static MunitTest t_suite_common[] = {
    {
        .name =  (char*) "/new_shutdown",
        .test = test_new_shutdown,
        .setup = setup_search_fixture,
        .tear_down = tear_down_search_fixture,
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
