//
// Created by 穆琰鑫 on 2024/10/11.
//

#ifndef HACK_CHAT_CHATCLIENTGUI_H
#define HACK_CHAT_CHATCLIENTGUI_H

#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include "FL/Fl.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Input.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Hold_Browser.H"
#include "FL/Fl_Text_Display.H"
#include "FL/Fl_Box.H"
#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include "NetWork.h"

/// 封装客户端界面和逻辑的类
class ChatClientGUI {
private:
    Fl_Window* window;               // 主窗口
    Fl_Group* main_group;            // 主界面组
    Fl_Group* chat_group;            // 聊天界面组

    // main_group 中的控件
    Fl_Input* username_input;        // 输入用户名的控件
    Fl_Input* server_input;          // 输入服务器IP的控件
    Fl_Input* port_input;            // 输入服务器端口的控件
    Fl_Button* connect_button;       // 连接按钮
    Fl_Hold_Browser* channel_browser;// 频道列表控件

    // chat_group 中的控件
    Fl_Text_Display* chat_display;   // 聊天消息显示
    Fl_Input* chat_input;            // 聊天输入框
    Fl_Button* send_button;          // 发送按钮
    Fl_Button* return_button;        // 返回按钮
    Fl_Text_Buffer* text_buffer;     // 聊天框的文本缓冲区

    // 封装网络相关类
    std::unique_ptr<ClientNetwork> client_network_;
    std::thread io_thread_;  // 用于运行 io_context 的线程

public:
    ChatClientGUI(int width, int height, const char* title);

    /// 显示窗口
    void show();

    /// 静态回调函数用于连接服务器
    /// \param w 控件指针
    /// \param data 数据
    static void connect_cb(Fl_Widget* w, void* data);

    /// 连接服务器并更新频道列表
    void connect_server();

    /// 静态回调函数用于处理频道选择
    /// \param w 控件指针
    /// \param data 数据
    static void channel_select_cb(Fl_Widget* w, void* data);

    /// 频道选择处理逻辑
    void channel_selected();

    /// 静态回调函数用于返回到频道选择界面
    /// \param w 控件指针
    /// \param data 数据
    static void return_cb(Fl_Widget* w, void* data);

    /// 静态回调函数用于发送消息
    /// \param w 控件指针
    /// \param data 数据
    static void send_cb(Fl_Widget* w, void* data);

    /// 跳转到聊天界面
    /// \param channel_name 跳转的频道名
    void switch_to_chat(const char* channel_name);

    /// 跳转到频道选择界面
    void switch_to_channel();

    /// 发送消息的逻辑
    void send_message();

    /// 显示用户名为空的错误窗口
    static void show_error(const std::string& error_message);

     /**
     * @brief 显示从服务器接收到的消息。
     * 此方法更新聊天界面的文本缓冲区以包含新消息。
     * @param message 需要显示的消息文本。
     */
     void display_message(const std::string& message);
};


#endif //HACK_CHAT_CHATCLIENTGUI_H
