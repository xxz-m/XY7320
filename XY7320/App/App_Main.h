/**
 * @file    App_Main.h
 * @brief   App 层入口
 *
 *          负责按依赖顺序初始化所有 Services，并创建 OS 任务。
 *          main() 调用顺序：App_Main_Init() → App_Main_Start() → OS_Start()
 */

#ifndef XY7320_APP_MAIN_H
#define XY7320_APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化 App 层
 *
 * 调用时机：在 main() 的 HAL 初始化之后、App_Main_Start 之前。
 * 责任：按依赖顺序初始化所有 Services（先 LogService 才能让其他 Init 写日志；
 *       最后初始化 AdcService 以确保 BSP ADC 先就绪）。
 */
void App_Main_Init(void);

/**
 * @brief  启动 App 层
 *
 * 调用时机：在 OS_Start() 之前。本函数返回后由 main 调用 OS_Start()。
 * 责任：创建所有 OS 任务（LED、UpdateConfig、ModeManager）。
 */
void App_Main_Start(void);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_APP_MAIN_H */