cmake_minimum_required(VERSION 3.25)
project(hack_chat)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO 添加Windows MSVC和LinuxGCC 编译支持
# 设置第三方库的根目录
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

# 添加 Boost 库
set(BOOST_ROOT "${THIRD_PARTY_DIR}/boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

# 添加 FLTK 库
set(FLTK_ROOT "${THIRD_PARTY_DIR}/fltk")
set(FLTK_INCLUDE_DIR "${FLTK_ROOT}/include")
set(FLTK_LIBRARY_DIR "${FLTK_ROOT}/lib")

# 手动添加 FLTK 库
list(APPEND FLTK_LIBS
        "${FLTK_LIBRARY_DIR}/libfltk.a"
        "${FLTK_LIBRARY_DIR}/libfltk_forms.a"
        "${FLTK_LIBRARY_DIR}/libfltk_gl.a"
        "${FLTK_LIBRARY_DIR}/libfltk_images.a"
        "${FLTK_LIBRARY_DIR}/libcomctl32.a"
        )

# 添加 SQLite3 源码及头文件路径
set(SQLITE3_DIR "${THIRD_PARTY_DIR}/sqlite3")
add_library(sqlite3 STATIC ${SQLITE3_DIR}/sqlite3.c)

# 手动包含 SQLite3 头文件路径
include_directories(${SQLITE3_DIR})

# 添加 nlohmann 库（直接使用 json.hpp 文件）
set(NLOHMANN_DIR "${THIRD_PARTY_DIR}/nlohmann")

# 手动包含 nlohmann 头文件路径
include_directories(${NLOHMANN_DIR})

# 包含头文件路径
include_directories(
        ${BOOST_INCLUDEDIR}
        ${FLTK_INCLUDE_DIR}
        ${NLOHMANN_DIR} # 包含 nlohmann 的 hpp 文件
)

# 链接库路径
link_directories(
        ${BOOST_LIBRARYDIR}
)

# 添加可执行文件
add_executable(hack_chat main.cpp src/ChatClientGUI.cpp include/ChatClientGUI.h include/ChatClientGUI.h src/NetWork.cpp include/NetWork.h)

# 链接库文件
target_link_libraries(hack_chat
        ${Boost_LIBRARIES}
        sqlite3
        ${FLTK_LIBS}
        ws2_32
        mswsock
        pthread
        ${BOOST_LIBRARYDIR}/libboost_thread-mgw13-mt-x64-1_86.dll.a
        ${BOOST_LIBRARYDIR}/libboost_system-mgw13-mt-x64-1_86.dll.a
        )

# 显示编译信息
message(STATUS "Boost include dirs: ${BOOST_INCLUDEDIR}")
message(STATUS "Boost library dirs: ${BOOST_LIBRARYDIR}")
message(STATUS "FLTK include dir: ${FLTK_INCLUDE_DIR}")
message(STATUS "FLTK libraries: ${FLTK_LIBS}")
message(STATUS "nlohmann dir: ${NLOHMANN_DIR}")

# 添加前期测试文件
# 添加客户端可执行文件
add_executable(client_main src/ChatClientGUI.cpp include/ChatClientGUI.h  src/NetWork.cpp include/NetWork.h test/client_main.cpp)

# 链接客户端库文件
target_link_libraries(client_main
        ${Boost_LIBRARIES}
        sqlite3
        pthread
        ${FLTK_LIBS}
        ${BOOST_LIBRARYDIR}/libboost_thread-mgw13-mt-x64-1_86.dll.a
        ${BOOST_LIBRARYDIR}/libboost_system-mgw13-mt-x64-1_86.dll.a
        ws2_32
        mswsock
        )

# 添加服务器可执行文件
add_executable(server_main server_main.cpp src/ChatClientGUI.cpp include/ChatClientGUI.h src/NetWork.cpp include/NetWork.h)

# 链接服务器库文件
target_link_libraries(server_main
        ${Boost_LIBRARIES}
        sqlite3
        pthread
        ${FLTK_LIBS}
        ${BOOST_LIBRARYDIR}/libboost_thread-mgw13-mt-x64-1_86.dll.a
        ${BOOST_LIBRARYDIR}/libboost_system-mgw13-mt-x64-1_86.dll.a
        ws2_32
        mswsock
        )
