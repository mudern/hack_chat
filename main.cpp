#include "include/ChatClientGUI.h"

int main(int argc, char** argv) {
    try {
        // 创建并初始化 GUI 窗口
        ChatClientGUI chat_client(600, 400, "在线聊天室客户端");

        // 显示 GUI
        chat_client.show();

        // 启动 FLTK 的事件循环
        return Fl::run();
    } catch (const std::exception& e) {
        std::cerr << "程序遇到错误: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
