---
title: C++ 对照 C 详解 — 以 LedService 为例
tags:
  - docs/learning
  - cpp
  - embedded
status: active
---

# C++ 对照 C 详解 — 以 LedService 为例

本文把 `led_service.h` 和 `led_service.cpp` 中的每一个 C++ 语法点拆开，和 C 版本做对照，帮助熟悉 C 的开发者建立"这个东西在 C 里怎么对应"的直觉。

---

## 1. 全貌对照

### C++ 版本（当前代码）

```cpp
class LedService {
public:
    enum class Mode { OFF, ON, BLINK, BREATHE };
    static LedService& Instance();
    void Init();
    void SetMode(Mode mode);
    void Update();
private:
    LedService();
    BspGpio_t  m_gpio;
    Mode       m_mode;
    uint32_t   m_lastToggleTick;
    uint32_t   m_blinkPeriodMs;
    bool       m_state;
    void TurnOn();
    void TurnOff();
    void UpdateBlink();
};
```

调用方式：`LedService::Instance().SetMode(LedService::Mode::BLINK);`

### 如果用 C 来写同样的功能

```c
typedef enum { LED_MODE_OFF, LED_MODE_ON, LED_MODE_BLINK, LED_MODE_BREATHE } LedMode;

typedef struct {
    BspGpio_t  gpio;
    LedMode    mode;
    uint32_t   lastToggleTick;
    uint32_t   blinkPeriodMs;
    bool       state;
} LedService;

LedService* LedService_Instance(void);
void LedService_Init(LedService *self);
void LedService_SetMode(LedService *self, LedMode mode);
void LedService_Update(LedService *self);
```

调用方式：`LedService_SetMode(LedService_Instance(), LED_MODE_BLINK);`

C 版本也不难写，只是"状态"和"操作"是分开的。C++ 的 class 做的事情就是**把数据和对数据的操作绑在一起**，然后加上访问控制。

---

## 2. 逐个语法点拆解

### 2.1 class 是什么

```cpp
class LedService {
    // ...
};
```

`class` 本质上就是一个自带访问控制的 `struct`。在 C 里你写 `typedef struct { ... } LedService;`，所有字段外部都能直接读写。C++ 的 `class` 默认把所有东西藏起来（`private`），你得显式声明哪些是对外暴露的（`public`）。

| C | C++ |
|---|-----|
| `typedef struct { int x; } Foo;` | `class Foo { public: int x; };` |
| 所有字段外部可见 | 默认所有字段外部不可见 |
| 无访问控制 | `public` / `private` 控制 |

### 2.2 public 和 private

```cpp
class LedService {
public:         // 这下面的，外部可以调用
    void Init();
    void Update();
private:        // 这下面的，外部不能碰
    BspGpio_t m_gpio;
    void TurnOn();
};
```

C 里没有这个机制，你能做到的最接近的事情是：把内部字段放在 `.c` 文件里用 `static` 藏起来，头文件里只暴露函数。但那样你就不能用栈上的结构体了，只能靠不透明指针（opaque pointer）。

C++ 的 `private` 让你在头文件里把数据声明写出来（编译器需要知道大小），但同时禁止外部代码访问。效果就像在 C 里写了注释 `/* 以下字段请勿直接使用 */`，但编译器会真的拦住你。

### 2.3 成员变量和 m_ 前缀

```cpp
private:
    BspGpio_t  m_gpio;
    Mode       m_mode;
    uint32_t   m_lastToggleTick;
```

这些就是 C 里 `struct` 的字段。`m_` 前缀（member 的缩写）是约定，告诉读代码的人"这是类的成员变量，不是局部变量"。

```cpp
void LedService::UpdateBlink() {
    uint32_t now = BspTimOs_GetTick();  // now 是局部变量（没有 m_）
    if (now - m_lastToggleTick >= m_blinkPeriodMs) {  // m_ 开头的是成员变量
        m_lastToggleTick = now;
    }
}
```

对应 C 里的写法：

```c
static uint32_t s_lastToggleTick;  // C 用 s_ 前缀表示 static 全局

void LedService_Update(void) {
    uint32_t now = BspTimOs_GetTick();
    if (now - s_lastToggleTick >= s_blinkPeriodMs) {
        s_lastToggleTick = now;
    }
}
```

### 2.4 方法（成员函数）和 :: 作用域

```cpp
void LedService::Init()
{
    BspGpio_Init(&m_gpio);
}
```

`LedService::Init()` 的意思是"LedService 这个类的 Init 方法"。`::` 是 C++ 的作用域解析运算符，类似 C 里的模块前缀。

| C | C++ |
|---|-----|
| `void LedService_Init(LedService *self)` | `void LedService::Init()` |
| 手动传 `self` 指针 | 编译器自动传 `this` 指针（隐含） |
| 用 `self->gpio` 访问字段 | 直接用 `m_gpio` 访问（隐含 `this->m_gpio`） |

C++ 里每个非 static 方法都有一个隐含的 `this` 指针指向当前对象。你写 `m_gpio` 编译器实际理解成 `this->m_gpio`，跟 C 里写 `self->gpio` 是一回事。

### 2.5 构造函数和初始化列表

```cpp
LedService::LedService()
    : m_mode(Mode::OFF)
    , m_lastToggleTick(0)
    , m_blinkPeriodMs(500)
    , m_state(false)
{
    m_gpio.port = GPIOF;
    m_gpio.pin = GPIO_PIN_10;
    m_gpio.active_high = true;
}
```

构造函数就是对象创建时自动调用的初始化函数。名字和类名相同（`LedService::LedService()`），没有返回值。

冒号后面那一串叫**初始化列表**，是 C++ 特有的语法：

```cpp
: m_mode(Mode::OFF)          // 把 m_mode 初始化为 OFF
, m_lastToggleTick(0)        // 把 m_lastToggleTick 初始化为 0
, m_blinkPeriodMs(500)       // 把 m_blinkPeriodMs 初始化为 500
, m_state(false)             // 把 m_state 初始化为 false
```

它和你在花括号里写赋值效果一样：

```cpp
LedService::LedService() {
    m_mode = Mode::OFF;
    m_lastToggleTick = 0;
    m_blinkPeriodMs = 500;
    m_state = false;
    // ...
}
```

但初始化列表在某些场景下更高效（对复杂类型少一次拷贝），也是 C++ 社区推荐的写法。对于 `int`、`bool` 这种基本类型，两种写法没区别。

对应 C 里就是初始化一个结构体：

```c
static LedService s_led = {
    .mode = LED_MODE_OFF,
    .lastToggleTick = 0,
    .blinkPeriodMs = 500,
    .state = false,
};
```

### 2.6 单例模式：static Instance()

```cpp
class LedService {
public:
    static LedService& Instance();
private:
    LedService();  // 构造函数设为 private
};

LedService& LedService::Instance()
{
    static LedService instance;  // 局部 static，只初始化一次
    return instance;
}
```

单例解决一个问题：**怎么保证某个东西全局只有一个？**

三个要素的组合拳：

- **`private` 构造函数**：不让外部代码创建新实例。外面写 `LedService a;` 编译器直接报错
- **`static Instance()`**：唯一的获取途径。`static` 成员函数不属于某个具体对象，而是属于类本身
- **函数内 `static` 变量**：第一次执行到这里时才构造（调用构造函数），之后每次进来直接返回同一个对象

对应 C 里的做法：

```c
static LedService s_led;  // 文件级 static，程序启动就存在

LedService* LedService_Instance(void) {
    return &s_led;
}
```

效果一样，都是全局唯一实例。区别在于 C++ 版本是"第一次用到时才构造"（延迟初始化），C 版本是"程序一启动就分配好了"。

#### 什么时候不用单例

如果硬件上有多个同类设备（比如两颗 LED），就不该用单例。把构造函数改成 `public`，让调用方创建多个实例即可：

```cpp
LedService led1, led2;
led1.Init(GPIOF, GPIO_PIN_10);
led2.Init(GPIOF, GPIO_PIN_9);
```

C++ 的 class 不带单例时，用法和 C 的 struct 一模一样——定义多个变量，每个变量有自己的数据。单例只是加了一把锁，说"这个东西全局只能有一个"。

选择标准：硬件上只有一个的东西（升级服务、版本存储）用单例；可能有多个的东西（LED、传感器通道）不用单例，让调用方自己创建实例。

### 2.7 enum class

```cpp
enum class Mode { OFF, ON, BLINK, BREATHE };
```

`enum class` 是 C++11 引入的强类型枚举，和 C 的 `enum` 的区别：

| | C 的 enum | C++ 的 enum class |
|---|---|---|
| 作用域 | 枚举值泄露到外层 | 枚举值在类名作用域内 |
| 类型安全 | 可以和 int 隐式转换 | 不能和 int 隐式转换 |
| 使用方式 | `LED_MODE_BLINK` | `Mode::BLINK` |

C 的写法：

```c
typedef enum { LED_MODE_OFF, LED_MODE_ON, LED_MODE_BLINK } LedMode;
LedMode m = LED_MODE_BLINK;
int x = LED_MODE_BLINK + 1;  // 编译通过（不安全）
```

C++ 的写法：

```cpp
enum class Mode { OFF, ON, BLINK };
Mode m = Mode::BLINK;
int x = Mode::BLINK + 1;  // 编译报错！
```

因为 `enum class` 的值有作用域（`Mode::`），所以不需要像 C 那样手动加 `LED_MODE_` 前缀来防冲突。

### 2.8 const 方法

```cpp
Mode GetMode() const;
```

末尾的 `const` 表示"这个方法不会修改对象的任何成员变量"。相当于对编译器承诺：我只读数据，不改数据。

对应 C 里最接近的是参数加 `const`：

```c
LedMode LedService_GetMode(const LedService *self);
```

加 `const` 的好处是让读代码的人一眼知道"调用这个函数不会改变状态"，编译器也会帮你检查。

---

## 3. .h 和 .cpp 的分工

C 里你习惯了 `.h` 放声明、`.c` 放实现。C++ 里 `.h` 和 `.cpp` 的关系完全一样：

| 文件 | 放什么 | 类比 C |
|------|--------|--------|
| `led_service.h` | 类定义（有哪些字段、哪些方法） | `.h` 里放 `struct` 定义 + 函数声明 |
| `led_service.cpp` | 方法实现（每段代码怎么写） | `.c` 里放函数实现 |

---

## 4. 完整 C 版本对照

把整个 `LedService` 完全用 C 重写：

```c
/* ===== led_service.h ===== */
#ifndef XY7320_LED_SERVICE_H
#define XY7320_LED_SERVICE_H

#include "bsp_gpio.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LED_MODE_OFF, LED_MODE_ON, LED_MODE_BLINK, LED_MODE_BREATHE
} LedMode;

void LedService_Init(void);
void LedService_SetMode(LedMode mode);
LedMode LedService_GetMode(void);
void LedService_Update(void);

#endif

/* ===== led_service.c ===== */
#include "led_service.h"
#include "bsp_tim_os.h"

static BspGpio_t  s_gpio;
static LedMode    s_mode           = LED_MODE_OFF;
static uint32_t   s_lastToggleTick = 0;
static uint32_t   s_blinkPeriodMs  = 500;
static bool       s_state          = false;

static void TurnOn(void);
static void TurnOff(void);
static void UpdateBlink(void);

void LedService_Init(void)
{
    s_gpio.port = GPIOF;
    s_gpio.pin = GPIO_PIN_10;
    s_gpio.active_high = true;
    BspGpio_Init(&s_gpio);
    TurnOff();
    s_mode = LED_MODE_OFF;
    s_lastToggleTick = 0;
    s_state = false;
}

void LedService_SetMode(LedMode mode)
{
    s_mode = mode;
    s_lastToggleTick = 0;
    if (mode == LED_MODE_OFF) TurnOff();
    else if (mode == LED_MODE_ON) TurnOn();
}

LedMode LedService_GetMode(void) { return s_mode; }

void LedService_Update(void)
{
    switch (s_mode) {
    case LED_MODE_OFF:    TurnOn();      break;
    case LED_MODE_ON:     TurnOff();     break;
    case LED_MODE_BLINK:  UpdateBlink(); break;
    case LED_MODE_BREATHE: break;
    }
}

static void TurnOn(void)  { BspGpio_On(&s_gpio);  s_state = true; }
static void TurnOff(void) { BspGpio_Off(&s_gpio); s_state = false; }

static void UpdateBlink(void) {
    uint32_t now = BspTimOs_GetTick();
    if (now - s_lastToggleTick >= s_blinkPeriodMs) {
        s_lastToggleTick = now;
        BspGpio_Toggle(&s_gpio);
        s_state = !s_state;
    }
}
```

逻辑完全一样。C++ 的 class 做的事情，C 用 `static` 全局变量 + `static` 内部函数 + 带前缀的对外函数也能做到。区别只是语法糖的多少：C++ 用 `class` + `private` 让编译器帮你强制访问控制，C 用 `static` + 约定来"软限制"。

---

## 5. 什么时候用 C++，什么时候用 C

本项目的做法：

- **BSP 层用 C**：这些模块本质上是"给硬件套一层函数壳"，不需要类、不需要继承，用 C 最自然
- **Services 层用 C++**：有较复杂的状态管理、需要单例、需要隐藏内部实现时，class 的 `private`/`public` + 构造函数比 C 的 `static` 全局变量更有组织性
- **Domain 层用 C**：纯算法、纯计算，不需要状态管理，C 最干净
- **App 层用 C++**：要调用 Services 层的 C++ 类，自然用 C++ 来写更方便。入口函数用 `extern "C"` 导出，让 System 层的调度器（C 写的）能调用
