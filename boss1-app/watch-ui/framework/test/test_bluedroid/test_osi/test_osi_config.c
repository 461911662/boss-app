/**
 * @file test_osi_list.c
 * @brief 这个文件包含osi的config测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <unistd.h>
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/allocator.h"
#include "osi/list.h"
#include "osi/config.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_CONFIG_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_CONFIG_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_CONFIG_TEST)
{
}

TEST(BT_OSI_CONFIG_TEST, config_new_free)
{
    const char *filename = "/tmp/osi_config_test.txt";
    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_EQUAL(0, access(filename, F_OK));
    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_has_empty_section)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_FALSE(config_has_section(config, "name"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_has_empty_key)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_FALSE(config_has_key(config, "head", "hair"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_has_empty_key_in_section1)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_FALSE(config_has_key_in_section(config, "hair", "black"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_get_empty_int)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_EQUAL(20, config_get_int(config, "person", "age", 20));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_get_empty_bool)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_EQUAL(false, config_get_bool(config, "person", "has_mouth", false));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_get_empty_string)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    TEST_ASSERT_EQUAL_STRING("xiaozhang", config_get_string(config, "person", "name", "xiaozhang"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_int_get)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_int(config, "person", "age", 18);

    TEST_ASSERT_EQUAL(18, config_get_int(config, "person", "age", 20));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_bool_get)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_bool(config, "person", "has_mouth", true);

    TEST_ASSERT_EQUAL(true, config_get_bool(config, "person", "has_mouth", false));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_string_get)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_EQUAL_STRING("xiaozhang", config_get_string(config, "person", "name", "xiaoyang"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_remove_section)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_TRUE(config_remove_section(config, "person"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_update_newest_section1)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_TRUE(config_update_newest_section(config, "person"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_update_newest_section2)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "animal", "name", "xiaohua", true);
    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_TRUE(config_update_newest_section(config, "person"));

    const config_section_node_t *section = config_section_begin(config);
    TEST_ASSERT_EQUAL_STRING("person", config_section_name(section));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_set_remove_key)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_TRUE(config_remove_key(config, "person", "name"));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_section_begin_end_name)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "animal", "name", "xiaohua", true);
    config_set_string(config, "person", "name", "xiaozhang", true);

    const config_section_node_t *section = NULL;
    for (section = config_section_begin(config); section != config_section_end(config);) {
        if (strcmp(config_section_name(section), "person") == 0) {
            break;
        }
        
        section = config_section_next(section);
    }
    TEST_ASSERT_NOT_NULL(section);
    TEST_ASSERT_EQUAL_STRING("person", config_section_name(section));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_new_config_save)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "animal", "name", "xiaohua", true);
    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_TRUE(config_save(config, filename));

    config_free(config);
}

TEST(BT_OSI_CONFIG_TEST, config_config_parse)
{
    const char *filename = "/tmp/osi_config_test.txt";

    TEST_ASSERT_EQUAL(0, unlink(filename));

    config_t *config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    config_set_string(config, "animal", "name", "xiaohua", true);
    config_set_string(config, "person", "name", "xiaozhang", true);

    TEST_ASSERT_TRUE(config_save(config, filename));
    config_free(config);

    config = config_new(filename);
    TEST_ASSERT_NOT_NULL(config);

    const config_section_node_t *section = NULL;
    for (section = config_section_begin(config); section != config_section_end(config);) {
        if (strcmp(config_section_name(section), "person") == 0) {
            break;
        }

        section = config_section_next(section);
    }
    TEST_ASSERT_NOT_NULL(section);
    TEST_ASSERT_EQUAL_STRING("person", config_section_name(section));

    config_free(config);
}

TEST_GROUP_RUNNER(BT_OSI_CONFIG_TEST)
{
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_free);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_has_empty_section);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_has_empty_key);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_has_empty_key_in_section1);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_get_empty_int);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_get_empty_bool);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_get_empty_string);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_int_get);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_bool_get);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_string_get);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_remove_section);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_update_newest_section1);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_update_newest_section2);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_set_remove_key);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_section_begin_end_name);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_new_config_save);
  RUN_TEST_CASE(BT_OSI_CONFIG_TEST, config_config_parse);
}
