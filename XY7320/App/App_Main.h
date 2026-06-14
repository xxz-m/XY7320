//
// Created by Administrator on 2026/6/14.
//

#ifndef XY7320_APP_MAIN_H
#define XY7320_APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif
/**
 * 初始化 App 层
 * 在 main() 中调用，初始化所有 Services
 */
void App_Main_Init(void);
/**
 * 启动 App 层
 * 创建所有 OS 任务
 */
void App_Main_Start(void);

#ifdef __cplusplus
}
#endif
#endif //XY7320_APP_MAIN_H
