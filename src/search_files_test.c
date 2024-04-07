// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_strset.h"
#include "koh_common.h"
#include "munit.h"
#include <assert.h>
#include <dirent.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const bool verbose = true;
static const char *dir_name = "./file2search";
static const int create_files_num = 100;

struct FileSearchFixture {
    char    **files,            // все файлы
            **num_files,        // имена состоят из цифр
            **alpha_files;      // имена состоят букв
    int files_num, num_files_num, alpha_files_num;
};

static void tear_down_search_fixture(void *f);

/*
static StrSetAction set_printer(const char *key, void *ctx) {
    printf("set_printer: '%s'\n", key);
    return SSA_remove;
}
*/

static MunitResult test_new_shutdown_common(
    const MunitParameter params[], 
    void* data, 
    struct FilesSearchSetup *setup,
    char **lines,
    int lines_num
) {
    //struct FileSearchFixture *fixture = data;
    struct FilesSearchResult fsr = {};
    if (verbose) {
        printf(
            "test_new_shutdown_common: setup %s\n",
            koh_files_search_setup_2str(setup)
        );
    }
    fsr = koh_search_files(setup);
    printf("fsr.num %d\n", fsr.num);

    StrSet *s1 = strset_new(), *s2 = strset_new();

    for (int i = 0; i < fsr.num; i++) {
        strset_add(s1, fsr.names[i]);
    }

    printf("s1:\n\n\n");
    FILE *tmp_file = fopen("f1.txt", "w");
    assert(tmp_file);
    //strset_print(s1, stdout);
    fclose(tmp_file);
    printf("\n\n\n");

    assert(lines_num >= 0);
    for (int i = 0; i < lines_num; i++) {
        strset_add(s2, lines[i]);
    }

    printf("s2:\n\n\n");
    tmp_file = fopen("f2.txt", "w");
    //strset_print(s2, tmp_file);
    printf("\n\n\n");
    fclose(tmp_file);

    bool is_set_eq = strset_compare(s1, s2);
    printf("test_new_shutdown: is_set_eq %s\n", is_set_eq ? "true" : "false");

    if (!is_set_eq)
        tear_down_search_fixture(data);
    munit_assert(is_set_eq);

    koh_search_files_shutdown(&fsr);

    printf("test_new_shutdown: %p\n", data);

    strset_free(s1);
    strset_free(s2);

    return MUNIT_OK;
}


static MunitResult test_new_shutdown(
    const MunitParameter params[], void* data
) {
    struct FileSearchFixture *f = data;

    /*
    test_new_shutdown_common(params, data, &(struct FilesSearchSetup) {
        .regex_pattern = ".*",
        .path = dir_name,
        .deep = -1,
        .engine_pcre2 = false,
    }, f->files, f->files_num);
    */

    test_new_shutdown_common(params, data, &(struct FilesSearchSetup) {
        .regex_pattern = ".*",
        .path = dir_name,
        .deep = -1,
        .engine_pcre2 = true,
    }, f->files, f->files_num);

    return MUNIT_OK;
}

// TODO: С какими именами создавать файлы?
static void* setup_search_fixture(
        const MunitParameter params[], void* user_data) {

    printf("setup_files_create:\n");
    (void) params;

    // TODO: 
    // Создать каталог для файлов.
    //munit_assert(mkdir(dir_name, S_IRWXU) == -1);

    struct stat st = {};
    if (stat(dir_name, &st) == -1) {
        // каталога нету, создаю
        int ret = mkdir(dir_name, S_IRWXU);
        printf("ret %d\n", ret);
    }

    struct FileSearchFixture *fixture = calloc(1, sizeof(*fixture));
    assert(fixture);

    // Создать сколько-то файлов.
    fixture->files_num = create_files_num;
    size_t sz = sizeof(fixture->files[0]);
    fixture->files = calloc(fixture->files_num, sz);
    fixture->alpha_files = calloc(fixture->files_num, sz);
    fixture->num_files = calloc(fixture->files_num, sz);

    int i = 0;
    // имена с цифрами
    for (; i < fixture->files_num / 2; i++) {
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
        fixture->num_files[fixture->num_files_num++] = strdup(path);
        fclose(f);
    }

    // имена с буквами
    for (; i < fixture->files_num; i++) {
        char path[128] = {};
        char num[32] = {};
        sprintf(num, "%c", i % 26 + 'a');
        strcat(path, dir_name);
        strcat(path, "/");
        strcat(path, num);
        //printf("path: '%s'\n", path);
        FILE *f = fopen(path, "w");
        munit_assert_ptr_not_null(f);
        //fixture->files[i] = strdup(path);
        fixture->alpha_files[fixture->alpha_files_num++] = strdup(path);
        fclose(f);
    }

    return fixture;
}

void rmdir_recursive(const char *path) {
    printf("rmdir_recursive: path '%s'\n", path);
    struct dirent *entry = NULL;
    DIR *dir = opendir(path);
    assert(dir);
    while ((entry = readdir(dir))) {
        switch (entry->d_type) {
            case DT_DIR: {
                if (!strcmp(entry->d_name, ".") || 
                    !strcmp(entry->d_name, ".."))
                    break;

                char new_path[1024] = {};

                strcat(new_path, path);
                strcat(new_path, "/");
                strcat(new_path, entry->d_name);

                rmdir_recursive(new_path);
                break;
            }
            case DT_REG: {
                char full_path[1024] = {};
                sprintf(full_path, "%s/%s", path, entry->d_name);
                printf("rmdir_recursive: DT_REG %s\n", full_path);
                remove(full_path);
                break;
            }
        }
    }
    closedir(dir);
}

static void tear_down_search_fixture(void *f) {
    assert(f);
    struct FileSearchFixture *fixture = f;

    int res = 0;
    for (int i = 0; i < fixture->files_num; i++) {
        res = remove(fixture->files[i]);
        /*munit_assert(res == 0);*/
        //printf("tear_down_search_fixture: remove returned %d\n", res);
        free(fixture->files[i]);
    }

    rmdir_recursive(dir_name);
    res = rmdir(dir_name);
    munit_assert(res == 0);
    /*printf("tear_down_search_fixture: rmdir returned %d\n", res);*/

    for (int j = 0; j < fixture->alpha_files_num; j++) {
        free(fixture->alpha_files[j]);
    }

    for (int j = 0; j < fixture->num_files_num; j++) {
        free(fixture->num_files[j]);
    }

    if (fixture->files)
        free(fixture->files);
    if (fixture->alpha_files)
        free(fixture->alpha_files);
    if (fixture->num_files)
        free(fixture->num_files);

    memset(f, 0, sizeof(*f));
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
