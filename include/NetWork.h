//
// Created by 穆琰鑫 on 2024/10/11.
//

#ifndef HACK_CHAT_NETWORK_H
#define HACK_CHAT_NETWORK_H


#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bimap.hpp>
#include <iostream>
#include <string>
#include <regex>
#include <optional>
#include <nlohmann/json.hpp>
#include "FL/Fl_Text_Buffer.H"

//使用boost的tcp命名空间
using tcp=boost::asio::ip::tcp;
//使用boost的error_coee命名空间
using error_code=boost::system::error_code;

// 使用nlohmann的json命名空间
using json = nlohmann::json;

struct RequestMessage {
    std::string type;       // "connect", "get_channel_list", "join_channel", "send_message"
    std::string username;   // 用户名，所有请求都携带用户名
    std::string channel;    // 针对 "join_channel" 和 "send_message" 类型
    std::string content;    // 针对 "send_message" 类型的消息内容

    // 序列化：将 RequestMessage 转为 JSON 格式
    json to_json() const {
        return {
                {"type", type},
                {"username", username},
                {"channel", channel},
                {"content", content}
        };
    }

    // 反序列化：从 JSON 格式转换为 RequestMessage 结构
    static RequestMessage from_json(const nlohmann::json& json_data) {
        RequestMessage msg;
        msg.type = json_data.at("type").get<std::string>();
        msg.username = json_data.at("username").get<std::string>();
        if (json_data.contains("channel"))
            msg.channel = json_data.at("channel").get<std::string>();
        if (json_data.contains("content"))
            msg.content = json_data.at("content").get<std::string>();
        return msg;
    }
};

struct ResponseMessage {
    std::string type;       // "connect", "channel_list", "join_channel", "error"
    std::string status;     // 成功或者失败的状态信息，比如 "success", "error"
    json content; // 响应内容，使用 JSON 存储，可以是数组、对象或字符串等

    // 序列化：将 ResponseMessage 转为 JSON 格式
    json to_json() const {
        return {
                {"type", type},
                {"status", status},
                {"content", content}
        };
    }

    // 反序列化：从 JSON 格式转换为 ResponseMessage 结构
    static ResponseMessage from_json(const json& json_data) {
        ResponseMessage response;
        response.type = json_data.at("type").get<std::string>();
        response.status = json_data.at("status").get<std::string>();
        if (json_data.contains("content"))
            response.content = json_data.at("content");
        return response;
    }
};


std::optional<std::string> validate_ip_or_hostname(const std::string& input);
bool validate_port(const std::string& port);

/**
 * @brief 客户端网络类，负责与服务器通信
 */
class ClientNetwork {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using ChannelListCallback = std::function<void(const std::vector<std::string>&)>;
    /**
     * @brief 构造函数
     * @param server 服务器地址
     * @param port 服务器端口
     * @param username 用户名
     */
    ClientNetwork(std::string  server, std::string  port, std::string  username);

    /**
     * @brief 析构函数，关闭socket
     */
    ~ClientNetwork();

    /**
     * @brief 启动异步运行时
     */
    void start_io_context();

    /**
     * @brief 连接到服务器
     */
    bool connect_to_server();

    /**
     * @brief 获取服务器上的频道列表
     */
    void get_channel_list();

    /**
     * @brief 加入指定频道
     * @param channel 频道名称
     */
    void join_channel(const std::string& channel);

    /**
     * @brief 向指定频道发送消息
     * @param channel 频道名称
     * @param message 发送的消息
     */
    void send_message(const std::string& message);

    /**
     * @brief 开始持续接收服务器发送的消息。
     */
    void start_receiving();

    /**
     * @brief 处理从服务器接收到的消息。
     * @param message 从服务器接收到的响应消息。
     */
    void handle_server_message(const ResponseMessage& message);

    /**
     * @brief 设置信息转发展示回调
     * @param callback 上层gui给定的回调函数
     */
    void setMessageCallback(MessageCallback callback);
    /**
     * @brief 设置频道设置回调
     * @param callback 上层gui给定的频道设置回调
     */
    void setChannelListCallback(ChannelListCallback callback);

    std::string username_;                ///< 用户名
    std::string channel_;                 ///<选择的频道名
    boost::asio::io_context io_context_;  ///< Boost Asio的IO上下文
private:
    boost::asio::ip::tcp::socket socket_; ///< TCP socket
    std::string server_;                  ///< 服务器地址
    std::string port_;                    ///< 服务器端口

    MessageCallback message_callback_;
    ChannelListCallback channel_list_callback_;
};

/**
 * @brief 服务器网络类，处理客户端连接与消息转发
 */
class ServerNetwork {
public:
    /**
     * @brief 构造函数，初始化服务器和频道
     * @param port 服务器端口
     * @param channels 服务器上可用的频道
     */
    ServerNetwork(short port, const std::vector<std::string>& channels);

    /**
     * @brief 运行服务器，接受客户端连接
     */
    void run_server();

private:
    /**
     * @brief 接受客户端连接
     */
    void accept_connection();

    /**
     * @brief 处理客户端连接，读取并解析客户端的请求
     * @param socket 客户端的TCP socket
     */
    void handle_client(std::shared_ptr<tcp::socket> socket_ptr);

    /**
     * @brief 向频道中的所有客户端发送消息
     * @param channel 频道名称
     * @param message 消息内容
     * @param sender 消息发送者的用户名
     */
    void send_message_to_channel(const std::string& channel, const std::string& message, const std::string& sender);

    boost::asio::io_context io_context_;  ///< Boost Asio的IO上下文
    boost::asio::ip::tcp::acceptor acceptor_; ///< 接受客户端连接的对象
    std::vector<std::string> channels_; ///< 存储channels变量
    std::unordered_map<std::string, std::vector<std::string>> channel_members_; ///< 频道和成员名的映射
    boost::bimap<std::string, std::shared_ptr<tcp::socket>> client_usernames_;;  ///< 用户名和socket的双向映射
};

#endif //HACK_CHAT_NETWORK_H
