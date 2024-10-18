//
// Created by 穆琰鑫 on 2024/10/15.
//

#include <iostream>
#include <vector>
#include "include/NetWork.h"
#include "include/ChatClientGUI.h"

int main() {
    try {
        //TODO 增加从json文件中读取服务器数据内容

        // 定义服务器监听的端口和可用的频道列表
        short port = 12345;
        std::vector<std::string> channels = {"SciFi", "Tech", "General"};

        // 创建服务器网络类对象
        ServerNetwork server(port, channels);

        // 启动服务器，等待客户端连接
        std::cout << "Server is running on port " << port << "..." << std::endl;
        server.run_server();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}