/**
 * @file test_framework.c
 * @brief framework 测试工具
 * @attention 仅供学习使用，禁止其他公司进行商业用途
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/

#include <stdio.h>
#include <nuttx/config.h>

#include "system/readline.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define TEST_FRAMEWORK_VER    "1.00"
#define FWTEST_HELP_TEXT(x) x

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef int (*cmd_func)(char *pargs);


/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
struct test_cmd_s
{
  FAR const char *cmd;       /* The command text */
  FAR const char *arghelp;   /* Text describing the args */
  cmd_func  pfunc;      /* Pointer to command handler */
  FAR const char *help;      /* The help text */
};

/****************************************************************************
 * GLOBAL PROPERTY
 ****************************************************************************/
#ifdef CONFIG_BT_TEST_OSI_ENABLED
extern void test_osi_code(void);
#endif
#ifdef CONFIG_BT_TEST_BT_ENABLED
extern void test_bt_code(void);
#endif

/****************************************************************************
 * STATIC PROPERTY
 ****************************************************************************/
static int test_bt(char *parg);
static int test_cmd_quit(char *parg);
static int test_cmd_help(char *parg);

static const struct test_cmd_s g_test_cmds[] =
{
  {
    "h",
    "",
    test_cmd_help,
    FWTEST_HELP_TEXT("Display help for commands")
  },
  {
    "help",
    "",
    test_cmd_help,
    FWTEST_HELP_TEXT("Display help for commands")
  },
  {
    "q",
    "",
    test_cmd_quit,
    FWTEST_HELP_TEXT("Exit FWTest")
  },
  {
    "quit",
    "",
    test_cmd_quit,
    FWTEST_HELP_TEXT("Exit FWTest")
  },
  {
    "bt",
    "[help|osi|bt]",
    test_bt,
    FWTEST_HELP_TEXT("Display commands for Test Bluedorid code")
  },
};

static const int g_cmd_count = sizeof(g_test_cmds) / sizeof(struct test_cmd_s);

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/
static int test_bt(char *parg)
{
  if (strcmp(parg, "help") == 0) {
      printf("\nTest Bluedroid Options:\n");
      printf("     osi: display help info for the osi code of Bluedroid.\n");
      printf("      bt: display help info for the bt code of Bluedroid.\n");
      printf("    help: display help info for Test Bluedroid.\n");
      printf("\n");
      return OK;
  }

#ifdef CONFIG_BT_TEST_OSI_ENABLED
  if (strcmp(parg, "osi") == 0) {
      // 检查 "osi"
      test_osi_code();
      return OK;
  }
#endif

#ifdef CONFIG_BT_TEST_BT_ENABLED
  else if (strcmp(parg, "bt") == 0) {
      // 检查 "bt"
      test_bt_code();
      return OK;
  }
#endif
  else {
    // 如果没有匹配的命令，显示帮助信息
    test_cmd_help(NULL);
  }
  return OK;
}

static int test_cmd_help(char *parg)
{
  int len;
  int maxlen = 0;
  int x;
  int c;

  /* Calculate length of longest cmd + arghelp */
  for (x = 0; x < g_cmd_count; x++) {
    len = strlen(g_test_cmds[x].cmd) + strlen(g_test_cmds[x].arghelp);
    if (len > maxlen) {
      maxlen = len;
    }
  }

  printf("FWTest commands\n================\n");
  for (x = 0; x < g_cmd_count; x++) {
    /* Print the command and it's arguments */
    printf("  %s %s", g_test_cmds[x].cmd, g_test_cmds[x].arghelp);

    /* Calculate number of spaces to print before the help text */
    len = maxlen - (strlen(g_test_cmds[x].cmd) + strlen(g_test_cmds[x].arghelp));
    for (c = 0; c < len; c++) {
    printf(" ");
    }

    printf("  : %s\n", g_test_cmds[x].help);
  }

  return OK;
}

static int test_cmd_quit(char *parg)
{
  return OK;
}

int main(int argc, char *argv[])
{
  int x;
  int len;
  int running;
  FAR char *cmd;
  FAR char *arg;
  char buffer[CONFIG_NSH_LINELEN];

  printf("\nFWTest version " TEST_FRAMEWORK_VER "\n");
  printf("h for commands, q to exit\n\n");

  running = TRUE;
  while (running) {
    printf("FWTest> ");
    fflush(stdout);
    /* Read a line from the terminal */

    len = readline_stream(buffer, sizeof(buffer), stdin, stdout);
    if (len > 0) {
      buffer[len] = '\0';
      if (strncmp(buffer, "!", 1) != 0) {
        /* nxlooper command */
        if (buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        /* Parse the command from the argument */
        cmd = strtok_r(buffer, " \n", &arg);
        if (cmd == NULL)
        {
            continue;
        }

        /* Find the command in our cmd array */
        for (x = 0; x < g_cmd_count; x++) {
            uint32_t len2 = 
              strlen(cmd) > strlen(g_test_cmds[x].cmd)?strlen(g_test_cmds[x].cmd):strlen(cmd);
            if (strncmp(cmd, g_test_cmds[x].cmd, len2) == 0)
            {
                /* Command found.  Call it's handler if not NULL */
                if (g_test_cmds[x].pfunc != NULL)
                {
                    g_test_cmds[x].pfunc(arg);
                }

                /* Test if it is a quit command */
                if (g_test_cmds[x].pfunc == test_cmd_quit)
                {
                    running = FALSE;
                }
                break;
            }
        }
      } else {
        printf("%s: unknown FWTest command\n", buffer);
      }
    }
  }
  return OK;
}