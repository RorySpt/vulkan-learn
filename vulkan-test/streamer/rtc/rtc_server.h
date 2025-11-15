//
// Created by admin on 2025/11/5.
//

#ifndef ZETAENGINE_RTC_SERVER_H
#define ZETAENGINE_RTC_SERVER_H
#include "nlohmann/json.hpp"

#include <memory>

class rtc_server
{
public:
    rtc_server();

    // 发送一帧视频数据，只有存在连接并且有视频轨道时才会被处理
    void send_frame(std::vector<std::byte> data, std::chrono::duration<double> timestamp, int pts = 0) const;
private:
    // 收到websocket连接
    void on_message(nlohmann::json message) const;

    // 将任务派发到内部线程执行
    void dispatch(std::function<void()> task) const;


    struct Impl;
    std::shared_ptr<Impl> pImpl;
    //
};

#endif // ZETAENGINE_RTC_SERVER_H
