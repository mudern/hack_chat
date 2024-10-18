//
// Created by 穆琰鑫 on 2024/10/11.
//

#include "../include/NetWork.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <sstream>
#include <utility>

/*
 * 检验服务器IP地址或域名，返回 std::optional<std::string>
 * \param input 输入待检验的ip地址
 * \return 如果是 IPv4 地址返回 "ipv4"，如果是域名返回 "domain"，否则返回 std::nullopt
 */
std::optional<std::string> validate_ip_or_hostname(const std::string& input) {
    // 检验 IPv4 地址
    const std::regex ipv4_pattern(
            R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");

    // 检验域名
    const std::regex hostname_pattern(
            R"(^(([a-zA-Z0-9](-?[a-zA-Z0-9])*)\.)+[a-zA-Z]{2,}$)");

    // 如果输入是合法的 IPv4 地址，返回 "ipv4"
    if (std::regex_match(input, ipv4_pattern)) {
        return "ipv4";
    }
        // 如果输入是合法的域名，返回 "domain"
    else if (std::regex_match(input, hostname_pattern)) {
        return "domain";
    }

    // 如果既不是合法的 IPv4 地址，也不是合法的域名，返回 std::nullopt
    return std::nullopt;
}

/*
 * 端口校验函数
 * \param port 输入待检验的端口
 * \return 输出是否是合法端口范围
 */
bool validate_port(const std::string& port) {
    try {
        int port_num = std::stoi(port);
        return (port_num > 0 && port_num <= 65535);
    } catch (...) {
        return false;
    }
}

ClientNetwork::ClientNetwork(std::string  server, std::string  port, std::string  username)
        : socket_(io_context_), server_(std::move(server)), port_(std::move(port)), username_(std::move(username)){
}

ClientNetwork::~ClientNetwork() {
    socket_.close();
}

void ClientNetwork::start_io_context() {
    io_context_.run();
}

bool ClientNetwork::connect_to_server() {
    boost::asio::ip::tcp::resolver resolver(io_context_);
    boost::system::error_code ec;
    auto endpoints = resolver.resolve(server_, port_, ec);
    if (!ec) {
        boost::asio::connect(socket_, endpoints, ec);
        if (!ec) {
            // 连接成功后发送用户名
            RequestMessage request = {"connect", username_, "", ""};
            boost::asio::write(socket_, boost::asio::buffer(request.to_json().dump() + "\n"));
            return true;
        }
    }
    return false;
}

void ClientNetwork::get_channel_list() {
    // 发送获取频道列表的请求
    RequestMessage request = {"get_channel_list", username_, "", ""};
    boost::asio::write(socket_, boost::asio::buffer(request.to_json().dump() + "\n"));

}

void ClientNetwork::join_channel(const std::string& channel) {
    // 发送加入频道请求
    RequestMessage request = {"join_channel", username_, channel, ""};
    channel_=channel;
    boost::asio::write(socket_, boost::asio::buffer(request.to_json().dump() + "\n"));

}

void ClientNetwork::send_message(const std::string& message) {
    // 发送消息
    RequestMessage request = {"send_message", username_, channel_, message};
    boost::asio::write(socket_, boost::asio::buffer(request.to_json().dump() + "\n"));
}

/**
 * @brief 启动异步操作以持续接收来自服务器的消息。
 * 这个函数使用 boost::asio::async_read_until 来不断读取服务器发来的消息直到遇到换行符。
 */
void ClientNetwork::start_receiving() {
    auto buffer = std::make_shared<std::string>();
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(*buffer), "\n",
                                  [this, buffer](const boost::system::error_code& ec, std::size_t length) {
          if (!ec && length > 0) {
              std::string data(buffer->substr(0, length));
              buffer->erase(0, length);  // 清除已读取的数据

              auto response = ResponseMessage::from_json(json::parse(data));
              handle_server_message(response);
              // 继续监听更多消息
              this->start_receiving();
          } else {
              std::cerr << "Error receiving: " << ec.message() << std::endl;
              if (ec != boost::asio::error::eof) {
                  this->start_receiving();
              }
          }
        }
    );
}

/**
 * @brief 处理从服务器接收到的消息。
 * 根据消息类型，执行相应操作，例如显示聊天消息。
 * @param message 从服务器接收到的响应消息。
 */
void ClientNetwork::handle_server_message(const ResponseMessage& response) {
    std::cout << "[Client] Parsed message:" << std::endl;
    std::cout << "  Type: " << response.type << std::endl;
    std::cout << "  Status " << response.status << std::endl;
    if (response.type == "send_message" && message_callback_) {
        std::cout << "[Client] Handling send_message." << std::endl;

        // 检查 content 是否是对象，并提取其中的字段
        if (response.content.is_object()) {
            auto content = response.content;

            // 提取 sender、channel 和 message 内容
            std::string sender = content["sender"];
            std::string channel = content["channel"];
            std::string message = content["content"];

            std::string full_message =sender + ": " + message;
            std::cout<<full_message<<std::endl;
            message_callback_(full_message);  // 调用回调函数，显示消息
        }
    } else if (response.type == "channel_list" && channel_list_callback_) {
        std::cout << "[Client] Handling channel_list." << std::endl;
        if (response.content.is_array()) {
            std::vector<std::string> channels = response.content.get<std::vector<std::string>>();
            channel_list_callback_(channels);
        }
    }
}

void ClientNetwork::setMessageCallback(MessageCallback callback) {
    message_callback_ = callback;
}

void ClientNetwork::setChannelListCallback(ChannelListCallback callback) {
    channel_list_callback_ = callback;
}

/**
 * @brief 构造函数，初始化服务器和频道
 * @param port 服务器端口
 * @param channels 服务器上可用的频道
 */
ServerNetwork::ServerNetwork(short port, const std::vector<std::string>& channels)
:acceptor_(io_context_, tcp::endpoint(tcp::v4(), port))
{
    this->channels_=channels;
    // 初始化每个频道，将频道名作为键，空的std::vector作为值
    for (const std::string& channel : channels) {
        // 在 channel_members_ 中为每个频道初始化一个空的 vector
        this->channel_members_[channel] = std::vector<std::string>();
    }
}

/**
 * @brief 运行服务器，接受客户端连接
 */
void ServerNetwork::run_server() {
    accept_connection();
    io_context_.run();
}

/**
 * @brief 接受客户端连接
 */
void ServerNetwork::accept_connection() {
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
            auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
            handle_client(socket_ptr);
        }
        accept_connection();  // 继续接受新的连接
    });
}

/**
 * @brief 处理客户端连接，读取并解析客户端的请求
 * @param socket 客户端的TCP socket
 */
void ServerNetwork::handle_client(std::shared_ptr<tcp::socket> socket_ptr) {
    auto self = this;  // 为了在异步操作中保持类的生命周期
    auto buffer = std::make_shared<std::string>();

    boost::asio::async_read_until(*socket_ptr, boost::asio::dynamic_buffer(*buffer), "\n",
                                  [this, self, socket_ptr,buffer](error_code ec, std::size_t length) {
        if (!ec) {
            std::string data(buffer->substr(0, length));
            buffer->erase(0, length);  // 清除已读取的数据

            if (data.empty()) {
                std::cerr << "[Error] Received empty input, skipping." << std::endl;
                return;
            }

            // 使用 RequestMessage 结构体进行 JSON 反序列化
            RequestMessage message = RequestMessage::from_json(nlohmann::json::parse(data));

            std::cout << "[Server] Parsed message:" << std::endl;
            std::cout << "  Type: " << message.type << std::endl;
            std::cout << "  Username: " << message.username << std::endl;
            std::cout << "  Channel: " << message.channel << std::endl;
            std::cout << "  Content: " << message.content << std::endl;

            if (message.type == "connect") {
                // 处理连接请求
                client_usernames_.insert({message.username, socket_ptr});  // 将用户名与socket关联

                // 发送确认消息
                ResponseMessage response_message = {"connect", "success", "Username registered"};
                boost::asio::async_write(*socket_ptr,
                                         boost::asio::buffer(response_message.to_json().dump() + "\n"),
                                         [](boost::system::error_code, std::size_t) {});

            } else if (message.type == "get_channel_list") {
                // 处理获取频道列表请求，使用 JSON 数组返回
                nlohmann::json channel_list_json = channels_;

                // 使用 ResponseMessage 构建频道列表响应
                ResponseMessage response_message = {"channel_list", "success", channel_list_json};
                boost::asio::async_write(*socket_ptr,
                                         boost::asio::buffer(response_message.to_json().dump() + "\n"),
                                         [](boost::system::error_code, std::size_t) {});

            } else if (message.type == "join_channel") {
                // 处理加入频道请求
                std::string username = message.username;
                std::string new_channel_name = message.channel;

                // 检查用户是否已经在其他频道中
                for (auto& [channel_name, members] : channel_members_) {
                    auto it = std::find(members.begin(), members.end(), username);
                    if (it != members.end()) {
                        // 用户已经在这个频道，先移除用户
                        members.erase(it);
                        std::cout << "User " << username << " removed from channel " << channel_name << std::endl;
                        break;  // 一个用户只能在一个频道，找到并移除后即可退出循环
                    }
                }

                // 将用户加入到新的频道
                channel_members_[new_channel_name].push_back(username);
                std::cout << "User " << username << " joined channel " << new_channel_name << std::endl;

                // 发送确认消息
                // 发送加入频道确认消息，使用 ResponseMessage 结构体
                ResponseMessage response_message = {"join_channel", "success", "Joined " + new_channel_name};
                boost::asio::async_write(*socket_ptr,
                                         boost::asio::buffer(response_message.to_json().dump() + "\n"),
                                         [](boost::system::error_code, std::size_t) {});

            } else if (message.type == "send_message") {
                auto socket_it = client_usernames_.left.find("mudern");
                auto& member_socket = socket_it->second;
                // 处理发送消息请求
                send_message_to_channel(message.channel, message.content, message.username);
            }

            // 继续监听同一个客户端的消息
            handle_client(socket_ptr);
        }else{
            std::cerr << "[Error] Read error: " << ec.message() << ", possible client disconnect." << std::endl;
        }
    });
}

/**
 * @brief 向频道中的所有客户端发送消息
 * @param channel 频道名称
 * @param message 消息内容
 * @param sender 消息发送者的用户名
 */
void ServerNetwork::send_message_to_channel(const std::string& channel, const std::string& message, const std::string& sender) {
    auto it = channel_members_.find(channel);
    if (it != channel_members_.end()) {
        // 使用 ResponseMessage 结构体构建要发送的消息
        ResponseMessage full_message = {"send_message", "success",
                                            {{"sender", sender}, {"channel", channel}, {"content", message}}};

        std::string full_message_str = full_message.to_json().dump() + "\n";  // 序列化为 JSON 字符串

        // 遍历该频道的所有成员，发送消息
        for (const auto& username : it->second) {
            // 查找用户名对应的 socket
            auto socket_it = client_usernames_.left.find(username);
            if (socket_it != client_usernames_.left.end()) {
                std::cout << "Sending to user: " << username << std::endl;
                auto& member_socket = socket_it->second;  // 获取对应的 socket
                // 使用 async_write 发送消息给客户端
                boost::asio::async_write(*member_socket, boost::asio::buffer(full_message_str),
                                         [](boost::system::error_code, std::size_t) {});
            }
        }
    }
}
