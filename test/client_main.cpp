//
// Created by 穆琰鑫 on 2024/10/15.
//

#include <iostream>
#include "../include/NetWork.h"
#include "../include/ChatClientGUI.h"

int main() {
    try {
        // 定义服务器地址、端口和客户端用户名
        std::string server_address = "127.0.0.1";
        std::string server_port = "12345";
        std::string username = "Alice";

        // 创建客户端网络类对象
        ClientNetwork client(server_address, server_port, username);

        // 尝试连接服务器
        if (client.connect_to_server()) {
            std::cout << "Connected to server as " << username << std::endl;

            // 获取频道列表
            std::vector<std::string> channels = client.get_channel_list();
            std::cout << "Available channels: ";
            for (const auto& channel : channels) {
                std::cout << channel << " ";
            }
            std::cout << std::endl;

            // 加入一个频道
            std::string channel_to_join = "SciFi";
            if (client.join_channel(channel_to_join)) {
                std::cout << "Joined channel: " << channel_to_join << std::endl;

                // 发送一条消息
                std::string message = "Hello, everyone!";
                if (client.send_message(message)) {
                    std::cout << "Message sent: " << message << std::endl;
                }
            } else {
                std::cerr << "Failed to join channel: " << channel_to_join << std::endl;
            }
        } else {
            std::cerr << "Failed to connect to server" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }

    return 0;
}