# hack_chat

`hack_chat` 是一个基于 **Boost** 和 **FLTK** 的 C++ 在线聊天室软件，支持多人实时聊天，频道管理，并通过 JSON 进行消息交互。项目的设计旨在提供一个简洁、高效且易于扩展的聊天系统。

---

## 功能特色
- **用户管理**：支持用户名登录，每位用户仅能加入一个频道。
- **频道系统**：提供多频道功能，用户可在多个频道之间切换。
- **JSON 数据通信**：所有客户端与服务器间的消息均采用 JSON 格式，确保结构清晰且易于解析。
- **基于Boost的网络模块**：实现高效的网络通信。
- **FLTK图形界面**：简洁的用户界面，支持输入服务器地址、端口及用户名登录。
- **跨平台支持**：支持在 Windows、Linux 和 macOS 上运行。

---
## 项目结构

    ```bash
    hack_chat/
    │
    ├── src/                    # 源代码文件
    │   ├── network/            # 网络模块
    │   ├── gui/                # FLTK 界面模块
    │   ├── client_main.cpp     # 客户端入口
    │   ├── server_main.cpp     # 服务器入口
    │
    ├── third_party/            # 第三方库 (Boost, FLTK, SQLite3, nlohmann)
    ├── CMakeLists.txt          # CMake 配置文件
    └── README.md               # 项目说明文件

---

## 快速开始

### 依赖项
- **C++17** 或更高版本的编译器
- **Boost 1.86.0** （含线程库和系统库）
- **FLTK** （用于图形界面）
- **nlohmann/json** （用于 JSON 解析）
- **SQLite3** （可选：用于保存聊天记录）

### 安装和编译

1. 克隆仓库：
   ```bash
   git clone https://github.com/your-username/hack_chat.git
   cd hack_chat

2. 创建并进入构建目录：

    ```bash
    mkdir build && cd build

3. 运行 CMake 并编译项目：

    ```bash
    cmake ..
    make

4. 运行客户端或服务器：
- 启动服务器：
  ```bash
  ./server_main
- 启动客户端：
    ```bash
    ./client_main