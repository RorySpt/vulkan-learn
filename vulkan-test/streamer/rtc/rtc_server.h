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
private:
    void on_message(nlohmann::json message);
    void dispatch(std::function<void()> task);
    struct Impl;
    std::shared_ptr<Impl> pImpl;
    //
};

#endif // ZETAENGINE_RTC_SERVER_H
