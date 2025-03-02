/**
 * @file face_app.h
 * @brief
 *   watch face main file.
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include "lvgl.h"
#include "face/face_app.h"
#include "ability_manager/ability_manager.h"

using namespace mooncake;

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef enum {
    Face_App_Page_Max = 9999,
} Face_App_Page_E;

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static AbilityManager* get_ability_manager();

/****************************************************************************
 * CLASS FUNCTIONS
 ****************************************************************************/

/**
 * @details Face_App构造函数，主要用于初始化Face_App的资源
 * @param name 表示用户传入的App名字
*/
Face_App::Face_App(char *name)
{
    setAppInfo().name = std::string(name);
    LV_LOG_USER("Face_App().");
}

/**
 * @details onOpen函数是Face_App的共有函数。
 *          通过重写父类AppAbility的onOpen方法来实现自定义的打开操作。
 *          该方法计划会有如下功能：
 *          1. 使用页面管理器来创建第一个页面
 *          2. 使用页面管理器来创建App的后台处理程序
 *          3. 注册与底层模块通信的句柄
*/
void Face_App::onOpen()
{
    LV_LOG_USER("Face_App onOpen.");

    // create new UI for UI Application.
    // 1. create all page that regist in watch app.


    // create work ability for other Application.
}

/**
 * @details onRunning函数是Face_App的共有函数。
 *          通过重写父类AppAbility的onRunning方法来实现自定义的消息处理操作。
 *          该方法计划会有如下功能：
 *          1. 处理来自系统的消息
 *          2. 处理底层过来的消息
 *          3. 处理页面与页面相互传递的消息
*/
void Face_App::onRunning()
{
    LV_LOG_USER("Face_App onRunning.");
}

/**
 * @details onClose函数是Face_App的共有函数。
 *          onClose的处理正好与onOpen函数处理相反，类似于setup和teardown的关系
*/
void Face_App::onClose()
{
    LV_LOG_USER("Face_App onClose.");

    // clean UI Page for UI Application.

    // clean work context for other Application.
}

/****************************************************************************
 * STATIC VARIABLES
 ****************************************************************************/
static std::unique_ptr<AbilityManager> _ability_manager;

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/

/**
 * @note Login App私有的函数，静止非Login App调用
 * @details 获取Login App的页面管理器，该函数定义在cpp文件中
 * @return 返回页面管理器句柄
*/
static AbilityManager* get_ability_manager()
{
  if (_ability_manager == nullptr) {
    _ability_manager = std::make_unique<AbilityManager>();
  }
  return _ability_manager.get();
}
