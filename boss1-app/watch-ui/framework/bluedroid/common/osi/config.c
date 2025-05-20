/**
 * @file config.c
 * @brief 移植components\bt\common\osi\config.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bt_common.h"
#include "osi/allocator.h"
#include "osi/config.h"
#include "osi/list.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/

#define LOG_TAG "bt_osi_config"

#define CONFIG_FILE_MAX_SIZE             (1536)//1.5k
#define CONFIG_FILE_DEFAULE_LENGTH       (2048)
#define CONFIG_KEY                       "bt_cfg_key"

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/

typedef struct {
    char *key;
    char *value;
} entry_s;

typedef struct {
    char *name;
    list_t *entries;
} section_s;

struct config_t {
    list_t *sections;
};

// Empty definition; this type is aliased to list_node_t.
struct config_section_iter_t {};

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/

static void config_parse(int fd, config_t *config);
static section_s *section_new(const char *name);
static void section_free(void *ptr);
static section_s *section_find(const config_t *config, const char *section);
static entry_s *entry_new(const char *key, const char *value);
static void entry_free(void *ptr);
static entry_s *entry_find(const config_t *config, const char *section, const char *key);
static int get_config_size(const config_t *config);

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details 创建空的配置对象
 * @return 成功时返回配置对象，失败时返回NULL
 */
config_t *config_new_empty(void)
{
    config_t *config = osi_calloc(sizeof(config_t));
    if (!config) {
        OSI_TRACE_ERROR("%s unable to allocate memory for config_t.\n", __func__);
        goto error;
    }

    config->sections = osi_list_new(section_free);
    if (!config->sections) {
        OSI_TRACE_ERROR("%s unable to allocate list for sections.\n", __func__);
        goto error;
    }

    return config;

error:;
    config_free(config);
    return NULL;
}

/**
 * @details 创建配置对象
 * @param filename表示配置的文件名字
 * @return 成功时返回配置对象，失败时返回NULL
 */
config_t *config_new(const char *filename)
{
    assert(filename != NULL);

    config_t *config = config_new_empty();
    if (!config) {
        return NULL;
    }

    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        OSI_TRACE_ERROR("%s unable to open '%s'\n", __func__, filename);
        config_free(config);
        return NULL;
    }

    config_parse(fd, config);
    close(fd);
    return config;
}

/**
 * @details 释放配置对象
 * @param config表示配置的对象
 * @return 无
 */
void config_free(config_t *config)
{
    if (!config) {
        return;
    }

    osi_list_free(config->sections);
    osi_free(config);
}

/**
 * @details 在配置对象中查找指定的段
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @return 成功返回true,失败返回false
 */
bool config_has_section(const config_t *config, const char *section)
{
    assert(config != NULL);
    assert(section != NULL);

    return (section_find(config, section) != NULL);
}

/**
 * @details 在配置对象中查找指定的段和key
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @return 成功返回true,失败返回false
 */
bool config_has_key(const config_t *config, const char *section, const char *key)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    return (entry_find(config, section, key) != NULL);
}

/**
 * @details 遍历配置对象中的段，查找指定的key和value
 * @param config表示配置的对象
 * @param key表示需要查找的key
 * @param key_value表示需要查找的value
 * @return 成功返回true,失败返回false
 */
bool config_has_key_in_section(config_t *config, const char *key, char *key_value)
{
    OSI_TRACE_DEBUG("key = %s, value = %s", key, key_value);
    for (const list_node_t *node = osi_list_begin(config->sections); node != osi_list_end(config->sections); node = osi_list_next(node)) {
        const section_s *section = (const section_s *)osi_list_node(node);

        for (const list_node_t *data = osi_list_begin(section->entries); data != osi_list_end(section->entries); data = osi_list_next(data)) {
            entry_s *entry = osi_list_node(data);
            OSI_TRACE_DEBUG("entry->key = %s, entry->value = %s", entry->key, entry->value);
            if (!strcmp(entry->key, key) && !strcmp(entry->value, key_value)) {
                OSI_TRACE_DEBUG("%s, the irk aready in the flash.", __func__);
                return true;
            }
        }
    }

    return false;
}

/**
 * @details 遍历配置对象中的段，查找指定key的value
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @param def_value表示默认的值
 * @return 成功返回对应的value,失败返回def_value
 */
int config_get_int(const config_t *config, const char *section, const char *key, int def_value)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    entry_s *entry = entry_find(config, section, key);
    if (!entry) {
        return def_value;
    }

    char *endptr;
    int ret = strtol(entry->value, &endptr, 0);
    return (*endptr == '\0') ? ret : def_value;
}

/**
 * @details 遍历配置对象中的段，查找指定key的value
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @param def_value表示默认的值
 * @return 成功返回对应的value,失败返回def_value
 */
bool config_get_bool(const config_t *config, const char *section, const char *key, bool def_value)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    entry_s *entry = entry_find(config, section, key);
    if (!entry) {
        return def_value;
    }

    if (!strcmp(entry->value, "true")) {
        return true;
    }
    if (!strcmp(entry->value, "false")) {
        return false;
    }

    return def_value;
}

/**
 * @details 遍历配置对象中的段，查找指定key的value
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @param def_value表示默认的值
 * @return 成功返回对应的value,失败返回def_value
 */
const char *config_get_string(const config_t *config, const char *section, const char *key, const char *def_value)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    entry_s *entry = entry_find(config, section, key);
    if (!entry) {
        return def_value;
    }

    return entry->value;
}

/**
 * @details 在配置对象中的段，指定key和value
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @param value表示设置的值
 * @return 无
 */
void config_set_int(config_t *config, const char *section, const char *key, int value)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    char value_str[32] = { 0 };
    sprintf(value_str, "%d", value);
    config_set_string(config, section, key, value_str, false);
}

/**
 * @details 在配置对象中的段，指定key和value
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @param value表示设置的值
 * @return 无
 */
void config_set_bool(config_t *config, const char *section, const char *key, bool value)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);

    config_set_string(config, section, key, value ? "true" : "false", false);
}

/**
 * @details 在配置对象中的段，指定key和value
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要查找的key
 * @param value表示设置的值
 * @param insert_b ack表示是否插入后面，1表示插入后面，0表示插入前面
 * @return 无
 */
void config_set_string(config_t *config, const char *section, const char *key, const char *value, bool insert_back)
{
    section_s *sec = section_find(config, section);
    if (!sec) {
        sec = section_new(section);
        if (insert_back) {
            osi_list_append(config->sections, sec);
        } else {
            osi_list_prepend(config->sections, sec);
        }
    }

    for (const list_node_t *node = osi_list_begin(sec->entries); node != osi_list_end(sec->entries); node = osi_list_next(node)) {
        entry_s *entry = osi_list_node(node);
        if (!strcmp(entry->key, key)) {
            osi_free(entry->value);
            entry->value = osi_strdup(value);
            return;
        }
    }

    entry_s *entry = entry_new(key, value);
    osi_list_append(sec->entries, entry);
}

/**
 * @details 在配置对象中，删除指定的段
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @return 成功时返回true,失败时返回false
 */
bool config_remove_section(config_t *config, const char *section)
{
    assert(config != NULL);
    assert(section != NULL);

    section_s *sec = section_find(config, section);
    if (!sec) {
        return false;
    }

    return osi_list_remove(config->sections, sec);
}

/**
 * @details 在配置对象中，如果指定的段在是第一个，则直接返回。否则，找到后将其移动到第一个处。
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @return 成功时返回true,失败时返回false
 */
bool config_update_newest_section(config_t *config, const char *section)
{
    assert(config != NULL);
    assert(section != NULL);

    list_node_t *first_node = osi_list_begin(config->sections);
    if (first_node == NULL) {
        return false;
    }
    section_s *first_sec = osi_list_node(first_node);
    if (strcmp(first_sec->name, section) == 0) {
        return true;
    }

    for (const list_node_t *node = osi_list_begin(config->sections); node != osi_list_end(config->sections); node = osi_list_next(node)) {
        section_s *sec = osi_list_node(node);
        if (strcmp(sec->name, section) == 0) {
            osi_list_delete(config->sections, sec);
            osi_list_prepend(config->sections, sec);
            return true;
        }
    }

    return false;
}

/**
 * @details 在配置对象的段中，移除指定的key
 * @param config表示配置的对象
 * @param section表示需要查找的段
 * @param key表示需要移除的key
 * @return 成功时返回true,失败时返回false
 */
bool config_remove_key(config_t *config, const char *section, const char *key)
{
    assert(config != NULL);
    assert(section != NULL);
    assert(key != NULL);
    bool ret;

    section_s *sec = section_find(config, section);
    entry_s *entry = entry_find(config, section, key);
    if (!sec || !entry) {
        return false;
    }

    ret = osi_list_remove(sec->entries, entry);
    if (osi_list_length(sec->entries) == 0) {
        OSI_TRACE_DEBUG("%s remove section name:%s",__func__, section);
        ret &= config_remove_section(config, section);
    }
    return ret;
}

/**
 * @details 返回配置对象的段中，第一个段
 * @param config表示配置的对象
 * @return 成功时返回非空,失败时返回NULL
 */
const config_section_node_t *config_section_begin(const config_t *config)
{
    assert(config != NULL);
    return (const config_section_node_t *)osi_list_begin(config->sections);
}

/**
 * @details 返回配置对象的段中，最后一个段
 * @param config表示配置的对象
 * @return 成功时返回非空,失败时返回NULL
 */
const config_section_node_t *config_section_end(const config_t *config)
{
    assert(config != NULL);
    return (const config_section_node_t *)osi_list_end(config->sections);
}

/**
 * @details 返回配置对象的段中，下一个数据
 * @param node表示当前配置对象的段中的数据
 * @return 成功时返回非空,失败时返回NULL
 */
const config_section_node_t *config_section_next(const config_section_node_t *node)
{
    assert(node != NULL);
    return (const config_section_node_t *)osi_list_next((const list_node_t *)node);
}

/**
 * @details 返回当前数据，所属段的名字
 * @param node表示当前配置对象的段中的数据
 * @return 成功时返回非空,失败时返回NULL
 */
const char *config_section_name(const config_section_node_t *node)
{
    assert(node != NULL);
    const list_node_t *lnode = (const list_node_t *)node;
    const section_s *section = (const section_s *)osi_list_node(lnode);
    return section->name;
}

/**
 * @details 将配置对象中的内容保存到文件中
 * @param config表示当前配置对象
 * @param filename表示需要存放的文件名
 * @return 成功时返回非空,失败时返回NULL
 */
bool config_save(const config_t *config, const char *filename)
{
    assert(config != NULL);
    assert(filename != NULL);
    assert(*filename != '\0');

    int err_code = 0;
    int fd;
    char *line = osi_calloc(1024);
    int config_size = get_config_size(config);
    char *buf = osi_calloc(config_size);
    if (!line || !buf) {
        err_code |= 0x01;
        goto error;
    }

    fd = open(filename, O_RDWR, 0644);
    if (fd < 0) {
        err_code |= 0x02;
        goto error;
    }

    int w_cnt, w_cnt_total = 0;
    for (const list_node_t *node = osi_list_begin(config->sections); node != osi_list_end(config->sections); node = osi_list_next(node)) {
        const section_s *section = (const section_s *)osi_list_node(node);
        w_cnt = snprintf(line, 1024, "[%s]\n", section->name);
        if(w_cnt < 0) {
            OSI_TRACE_ERROR("snprintf error w_cnt %d.",w_cnt);
            err_code |= 0x10;
            goto error;
        }
        if(w_cnt_total + w_cnt > config_size) {
            OSI_TRACE_ERROR("%s, memcpy size (w_cnt + w_cnt_total = %d) is larger than buffer size (config_size = %d).", __func__, (w_cnt + w_cnt_total), config_size);
            err_code |= 0x20;
            goto error;
        }
        OSI_TRACE_DEBUG("section name: %s, w_cnt + w_cnt_total = %d\n", section->name, w_cnt + w_cnt_total);
        memcpy(buf + w_cnt_total, line, w_cnt);
        w_cnt_total += w_cnt;

        for (const list_node_t *enode = osi_list_begin(section->entries); enode != osi_list_end(section->entries); enode = osi_list_next(enode)) {
            const entry_s *entry = (const entry_s *)osi_list_node(enode);
            OSI_TRACE_DEBUG("(key, val): (%s, %s)\n", entry->key, entry->value);
            w_cnt = snprintf(line, 1024, "%s = %s\n", entry->key, entry->value);
            if(w_cnt < 0) {
                OSI_TRACE_ERROR("snprintf error w_cnt %d.",w_cnt);
                err_code |= 0x10;
                goto error;
            }
            if(w_cnt_total + w_cnt > config_size) {
                OSI_TRACE_ERROR("%s, memcpy size (w_cnt + w_cnt_total = %d) is larger than buffer size.(config_size = %d)", __func__, (w_cnt + w_cnt_total), config_size);
                err_code |= 0x20;
                goto error;
            }
            OSI_TRACE_DEBUG("%s, w_cnt + w_cnt_total = %d", __func__, w_cnt + w_cnt_total);
            memcpy(buf + w_cnt_total, line, w_cnt);
            w_cnt_total += w_cnt;
        }

        // Only add a separating newline if there are more sections.
        if (osi_list_next(node) != osi_list_end(config->sections)) {
            buf[w_cnt_total] = '\n';
            w_cnt_total += 1;
        } else {
            break;
        }
    }
    buf[w_cnt_total] = '\0';

    int err = write(fd, buf, w_cnt_total);
    if (err != w_cnt_total) {
        close(fd);
        err_code |= 0x04;
        goto error;
    }

    close(fd);
    osi_free(line);
    osi_free(buf);
    return true;

error:
    if (buf) {
        osi_free(buf);
    }
    if (line) {
        osi_free(line);
    }
    if (err_code) {
        OSI_TRACE_ERROR("%s, err_code: 0x%x\n", __func__, err_code);
    }
    return false;
}

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/

/**
 * @details 获取配置对象内存的大小
 * @param config表示当前配置对象
 * @return 成功时返回配置的大小,失败时返回0
 */
static int get_config_size(const config_t *config)
{
    assert(config != NULL);

    int w_len = 0, total_size = 0;

    for (const list_node_t *node = osi_list_begin(config->sections); node != osi_list_end(config->sections); node = osi_list_next(node)) {
        const section_s *section = (const section_s *)osi_list_node(node);
        w_len = strlen(section->name) + strlen("[]\n");// format "[section->name]\n"
        total_size += w_len;

        for (const list_node_t *enode = osi_list_begin(section->entries); enode != osi_list_end(section->entries); enode = osi_list_next(enode)) {
            const entry_s *entry = (const entry_s *)osi_list_node(enode);
            w_len = strlen(entry->key) + strlen(entry->value) + strlen(" = \n");// format "entry->key = entry->value\n"
            total_size += w_len;
        }

        // Only add a separating newline if there are more sections.
        if (osi_list_next(node) != osi_list_end(config->sections)) {
                total_size ++;  //'\n'
        } else {
            break;
        }
    }
    total_size ++; //'\0'
    return total_size;
}

/**
 * @details 去除字符串两边的空格
 * @param config表示要处理的字符串
 * @return 成功时返回处理后的字符串,失败时返回NULL
 */
static char *trim(char *str)
{
    while (isspace((unsigned char)(*str))) {
        ++str;
    }

    if (!*str) {
        return str;
    }

    char *end_str = str + strlen(str) - 1;
    while (end_str > str && isspace((unsigned char)(*end_str))) {
        --end_str;
    }

    end_str[1] = '\0';
    return str;
}

/**
 * @details 解析文件中的配置
 * @param fd表示打开的文件句柄
 * @param config表示要处理的字符串
 * @return 无
 */
static void config_parse(int fd, config_t *config)
{
    assert(fd != 0);
    assert(config != NULL);

    int line_num = 0;
    int err_code = 0;
    size_t total_length = 0;
    char *line = osi_calloc(1024);
    char *section = osi_calloc(1024);
    char *buf = NULL;

    total_length = read(fd, buf, CONFIG_FILE_DEFAULE_LENGTH);
    if (total_length <= 0) {
        goto error;
    }

    buf = osi_calloc(total_length);
    if (!line || !section || !buf) {
        err_code |= 0x01;
        goto error;
    }

    char *p_line_end;
    char *p_line_bgn = buf;

    while ( (p_line_bgn < buf + total_length - 1) && (p_line_end = strchr(p_line_bgn, '\n'))) {

        // get one line
        int line_len = p_line_end - p_line_bgn;
        if (line_len > 1023) {
            OSI_TRACE_WARNING("%s exceed max line length on line %d.\n", __func__, line_num);
            break;
        }
        memcpy(line, p_line_bgn, line_len);
        line[line_len] = '\0';
        p_line_bgn = p_line_end + 1;
        char *line_ptr = trim(line);
        ++line_num;

        // Skip blank and comment lines.
        if (*line_ptr == '\0' || *line_ptr == '#') {
            continue;
        }

        if (*line_ptr == '[') {
            size_t len = strlen(line_ptr);
            if (line_ptr[len - 1] != ']') {
                OSI_TRACE_WARNING("%s unterminated section name on line %d.\n", __func__, line_num);
                continue;
            }
            strncpy(section, line_ptr + 1, len - 2);
            section[len - 2] = '\0';
        } else {
            char *split = strchr(line_ptr, '=');
            if (!split) {
                OSI_TRACE_DEBUG("%s no key/value separator found on line %d.\n", __func__, line_num);
                continue;
            }
            *split = '\0';
            config_set_string(config, section, trim(line_ptr), trim(split + 1), true);
        }
    }

error:
    if (buf) {
        osi_free(buf);
    }
    if (line) {
        osi_free(line);
    }
    if (section) {
        osi_free(section);
    }
    if (err_code) {
        OSI_TRACE_ERROR("%s returned with err code: %d\n", __func__, err_code);
    }
}

/**
 * @details 创建一个指定名称的段
 * @param name表示段的名字
 * @return 成功时返回段对象，失败返回NULL
 */
static section_s *section_new(const char *name)
{
    section_s *section = osi_calloc(sizeof(section_s));
    if (!section) {
        return NULL;
    }

    section->name = osi_strdup(name);
    section->entries = osi_list_new(entry_free);
    return section;
}

/**
 * @details 释放一个指定的段
 * @param ptr表示要释放的段
 * @return 无
 */
static void section_free(void *ptr)
{
    if (!ptr) {
        return;
    }

    section_s *section = ptr;
    osi_free(section->name);
    osi_list_free(section->entries);
    osi_free(section);
}

/**
 * @details 在配置对象中查找段
 * @param config表示指定的配置对象
 * @param section表示要查找的段
 * @return 成功时返回找到的段，失败时返回NULL
 */
static section_s *section_find(const config_t *config, const char *section)
{
    for (const list_node_t *node = osi_list_begin(config->sections); node != osi_list_end(config->sections); node = osi_list_next(node)) {
        section_s *sec = osi_list_node(node);
        if (!strcmp(sec->name, section)) {
            return sec;
        }
    }

    return NULL;
}

/**
 * @details 创建KV的entry
 * @param key表示要创建的entry中的key
 * @param value表示要创建的entry中的value
 * @return 成功时返回创建的entry，失败时返回NULL
 */
static entry_s *entry_new(const char *key, const char *value)
{
    entry_s *entry = osi_calloc(sizeof(entry_s));
    if (!entry) {
        return NULL;
    }

    entry->key = osi_strdup(key);
    entry->value = osi_strdup(value);
    return entry;
}

/**
 * @details 释放指定的entry
 * @param ptr表示指定的entry
 * @return 无
 */
static void entry_free(void *ptr)
{
    if (!ptr) {
        return;
    }

    entry_s *entry = ptr;
    osi_free(entry->key);
    osi_free(entry->value);
    osi_free(entry);
}

/**
 * @details 在指定的配置和段中查找key的对象
 * @param config表示指定的配置对象
 * @param section表示指定的段
 * @param key表示指定的key
 * @return 成功时返回entry，失败时返回NULL
 */
static entry_s *entry_find(const config_t *config, const char *section, const char *key)
{
    section_s *sec = section_find(config, section);
    if (!sec) {
        return NULL;
    }

    for (const list_node_t *node = osi_list_begin(sec->entries); node != osi_list_end(sec->entries); node = osi_list_next(node)) {
        entry_s *entry = osi_list_node(node);
        if (!strcmp(entry->key, key)) {
            return entry;
        }
    }

    return NULL;
}
