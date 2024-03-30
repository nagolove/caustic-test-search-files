// vim: set colorcolumn=85
// vim: fdm=marker

/*#include "koh_common.h"*/
#include "munit.h"
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/*static bool verbose = false;*/

static MunitResult test_new_shutdown(
    const MunitParameter params[], void* data
) {
    printf("test_new_shutdown: %p\n", data);
    return MUNIT_OK;
}

static const char *dir_name = "./file2search";
static char **files = NULL;
static int files_num = 0;

static void* setup_files_create(
        const MunitParameter params[], void* user_data) {

    printf("setup_files_create:\n");
    (void) params;

    // TODO: 
    // Создать каталог для файлов.
    //munit_assert(mkdir(dir_name, S_IRWXU) == -1);
    int ret = mkdir(dir_name, S_IRWXU);
    printf("ret %d\n", ret);

    // Создать сколько-то файлов.
    files_num = 1000;
    for (int i = 0; i < files_num; i++) {
        char path[128] = {};
        strcat(path, dir_name);
        strcat(path, "/");
        char num[32] = {};
        sprintf(num, "%d", i);
        printf("path: '%s'\n", path);
        FILE *f = fopen(path, "w");
        //munit_assert_ptr_not_null(f);
        fclose(f);
    }

    return (void*) (uintptr_t) 0xdeadbeef;
}


static MunitTest t_suite_common[] = {
    {
        .name =  (char*) "/new_shutdown",
        .test = test_new_shutdown,
        .setup = setup_files_create,
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
