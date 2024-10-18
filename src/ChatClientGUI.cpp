//
// Created by 穆琰鑫 on 2024/10/11.
//

#include "../include/ChatClientGUI.h"

ChatClientGUI::ChatClientGUI(int width, int height, const char *title)  {
    // 创建主窗口
    window = new Fl_Window(width, height, title);

    // 创建主界面 (main_group)
    main_group = new Fl_Group(10, 10, 580, 380);
    username_input = new Fl_Input(100, 40, 150, 30, "用户名:");
    server_input = new Fl_Input(100, 80, 150, 30, "服务器 IP:");
    port_input = new Fl_Input(100, 120, 150, 30, "端口:");
    connect_button = new Fl_Button(100, 160, 150, 30, "连接");

    channel_browser = new Fl_Hold_Browser(310, 10, 280, 380); // 频道选择控件
    channel_browser->callback(channel_select_cb, this);       // 频道选择回调

    main_group->end();  // 结束主界面组
    main_group->show(); // 初始显示主界面

    // 创建聊天界面 (chat_group)
    chat_group = new Fl_Group(10, 10, 580, 380);
    chat_display = new Fl_Text_Display(10, 10, 400, 300);
    text_buffer = new Fl_Text_Buffer();
    chat_display->buffer(text_buffer);

    chat_input = new Fl_Input(10, 320, 400, 30);
    send_button = new Fl_Button(420, 320, 100, 30, "发送");
    send_button->callback(send_cb, this);  // 发送按钮回调

    // 添加返回按钮
    return_button = new Fl_Button(420, 10, 100, 30, "返回");
    return_button->callback(return_cb, this);  // 返回按钮回调

    chat_group->end();
    chat_group->hide(); // 初始隐藏聊天界面

    // 设置连接按钮的回调
    connect_button->callback(connect_cb, this);

    window->end();
}

void ChatClientGUI::show() {
    window->show();
}

void ChatClientGUI::connect_cb(Fl_Widget *w, void *data) {
    ((ChatClientGUI*)data)->connect_server();
}

void ChatClientGUI::connect_server() {
    std::string username = username_input->value();
    std::string server = server_input->value();
    std::string port = port_input->value();

    if (username.empty()) {
        show_error("用户名不能为空！");
        return;
    }
    //TODO 增加服务器和端口的校验

    // 初始化网络模块
    client_network_ = std::make_unique<ClientNetwork>(server, port, username);

    client_network_->setMessageCallback([this](const std::string& msg) {
        std::cout<<"message call back"<<std::endl;
        Fl::lock();  // 确保线程安全地更新 GUI
        this->text_buffer->append((msg + "\n").c_str());
        Fl::unlock();
        Fl::awake();  // 刷新 GUI
    });
    client_network_->setChannelListCallback([this](const std::vector<std::string>& channels) {
        Fl::lock();
        this->channel_browser->clear();
        for (const auto& channel : channels) {
            this->channel_browser->add(channel.c_str());
        }
        Fl::unlock();
        Fl::awake();
    });

    if (client_network_->connect_to_server()) {
        client_network_->start_receiving();
        // 启动 io_context 的线程
        io_thread_ = std::thread([this]() {
            client_network_->io_context_.run();
        });

        // 获取频道列表
        boost::asio::post(client_network_->io_context_, [this]() {
            client_network_->get_channel_list();
        });
    } else {
        show_error("Failed to connect to server!");
    }
}

void ChatClientGUI::channel_select_cb(Fl_Widget *w, void *data) {
    ((ChatClientGUI*)data)->channel_selected();
}

void ChatClientGUI::channel_selected(){
    int selected = channel_browser->value();
    if (selected > 0) {
        const char* selected_channel = channel_browser->text(selected);
        client_network_->join_channel(selected_channel);
        switch_to_chat(selected_channel);
    }
}

void ChatClientGUI::return_cb(Fl_Widget *w, void *data) {
    ((ChatClientGUI*)data)->switch_to_channel();
}

void ChatClientGUI::send_cb(Fl_Widget *w, void *data) {
    ((ChatClientGUI*)data)->send_message();
}

void ChatClientGUI::switch_to_chat(const char *channel_name) {
    std::cout << "join channel: " << channel_name << std::endl;

    // 隐藏频道选择界面，显示聊天界面
    main_group->hide();
    chat_group->show();

    // 更新聊天显示（这里可以加载历史消息等）
    std::string welcome_message = std::string("welcome ") + channel_name + "\n";
    text_buffer->text(welcome_message.c_str());
}

void ChatClientGUI::switch_to_channel() {
    // 隐藏聊天界面，显示频道选择界面
    chat_group->hide();
    main_group->show();
}

void ChatClientGUI::send_message() {
    // 将chat_input的内容更新到文本显示中
    const char* message = chat_input->value();
    if (message && strlen(message) > 0) {
        // 清空输入框
        chat_input->value("");

        client_network_->send_message(message);
        chat_input->value("");
    }

}

void ChatClientGUI::show_error(const std::string& error_message) {
    auto* error_window = new Fl_Window(300, 100, "错误");
    auto* error_box = new Fl_Box(20, 30, 260, 40, error_message.c_str());
    auto* ok_button = new Fl_Button(100, 70, 100, 30, "确定");
    ok_button->callback([](Fl_Widget* w, void* data) {
        ((Fl_Window*)w->window())->hide();
    });
    error_window->end();
    error_window->set_modal();  // 设置为模态窗口
    error_window->show();
}

/**
 * @brief 将收到的消息添加到聊天窗口的显示缓冲区。
 * 此方法被设计为线程安全，可从多线程环境中调用。
 * @param message 从服务器接收到的消息文本，将被添加到聊天界面中。
 */
void ChatClientGUI::display_message(const std::string& message) {
    Fl::lock();
    std::string current_text = text_buffer->text();
    current_text += message + "\n";
    text_buffer->text(current_text.c_str());
    Fl::unlock();
    Fl::awake();
}