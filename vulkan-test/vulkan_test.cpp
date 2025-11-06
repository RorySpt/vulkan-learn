//
// Created by admin on 2025/11/5.
//

#include <thread>
#include <print>
#include <rtc/rtc_server.h>


int main() {
    rtc_server server;

    std::println("{}", "server start");
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
}