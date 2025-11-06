//
// Created by admin on 2025/11/5.
//

#include "rtc_server.h"

#include "nlohmann/json.hpp"

#include <rtc/websocket.hpp>
#include <rtc/peerconnection.hpp>
#include <TaskQueue.h>
#include <format>

constexpr std::string_view defaultIPAddress = "127.0.0.1";
constexpr uint16_t defaultPort = 8000;

namespace Zeta {
    enum class ELogLevel : uint8_t
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };
}
namespace details
{
    template <typename... Args>
    void log(Zeta::ELogLevel log_level, const std::string& message, Args&&... args)
    {
        //if (!Zeta::g_engine || !Zeta::g_engine->log_system()) return;
        ////if (log_level > Zeta::ELogLevel::Warning)
        ////{
        ////    auto prefix = std::format("[{}(Line {}) {}]",  __FILE__, __LINE__, __FUNCTION__);
        ////    Zeta::g_engine->log_system()->log(log_level, std::string(buffer) + __VA_ARGS__);
        ////}
        ////else
        ////{
        ////    g_engine->log_system()->log(LOG_LEVEL, __VA_ARGS__);
        //// }
        //Zeta::g_engine->log_system()->log(log_level, message, std::forward<Args>(args)...);

        //std::println(log_level > Zeta::ELogLevel::Warning ? std::cerr : std::cout,
        //    "[{}]{}", magic_enum::enum_name(log_level), std::vformat(message, std::make_format_args(std::forward<Args>(args)...)));
    };
    template <typename... Args>
    void log_info(const std::string& message, Args&&... args)
    {
        log(Zeta::ELogLevel::Info, message, std::forward<Args>(args)...);
    }
template <typename... Args>
   void log_error(const std::string& message, Args&&... args)
    {
        log(Zeta::ELogLevel::Error, message, std::forward<Args>(args)...);
    }
}
struct PeerConnectionWarper {
    std::shared_ptr<rtc::PeerConnection> peerConnection_;
    std::vector<std::shared_ptr<rtc::DataChannel>> data_channels_;
};

struct rtc_server::Impl
{
    std::shared_ptr<rtc::WebSocket> socket;
    std::string ip_address = defaultIPAddress.data();
    uint16_t port = defaultPort;
    std::string localId = "server";

    rtc::Configuration config;
    std::jthread thread;
    std::jthread::id thread_id;


    std::unordered_map<std::string, PeerConnectionWarper> connections;
};

rtc_server::rtc_server()
    : pImpl(std::make_shared<Impl>())
{
    std::string stunServer = "stun:stun.l.google.com:19302";
    pImpl->config.iceServers.emplace_back(stunServer);
    pImpl->config.disableAutoNegotiation = true;


    pImpl->thread = std::jthread([](const std::stop_token& token)
    {
        task::g_TaskScheduler.registerCurrentThread();
        while (!token.stop_requested())
        {
            task::g_TaskScheduler.blockCurrentUntilQueueNotEmpty();
            task::g_TaskScheduler.executeCurrentThreadAll();
        }
        task::g_TaskScheduler.unregisterCurrentThread();
    });
    pImpl->thread_id = pImpl->thread.get_id();
    pImpl->socket = std::make_shared<rtc::WebSocket>();

    auto& ws = pImpl->socket;
    ws->onOpen([]() { details::log_info("WebSocket connected, signaling ready");});
    ws->onClosed([]() { details::log_info("WebSocket closed" ); });
    ws->onError([](const std::string &error) { details::log_error( "WebSocket failed: "); });

    ws->onMessage([&, id = std::this_thread::get_id()](std::variant<rtc::binary, std::string> data) {
        if (!holds_alternative<std::string>(data))
            return;
        nlohmann::json message = nlohmann::json::parse(get<std::string>(data));
        dispatch([this, message]
            {
                on_message(message);
            });
    });

    const std::string url = std::format("ws://{}:{}/{}", pImpl->ip_address, pImpl->port, pImpl->localId);
    details::log_info("URL is {}", url);
    ws->open(url);
}
void rtc_server::on_message(nlohmann::json message)
{
    auto it = message.find("id");
    if (it == message.end())
        return;
    std::string id = it->get<std::string>();

    it = message.find("type");
    if (it == message.end())
        return;
    std::string type = it->get<std::string>();

    details::log_info("receive {} from {}",type,  id);
    if (type == "request") {
        PeerConnectionWarper connection_warper;
        connection_warper.peerConnection_ = std::make_shared<rtc::PeerConnection>(pImpl->config);
        const auto& pc = connection_warper.peerConnection_;

        pc->onStateChange([this, id](rtc::PeerConnection::State state)
            {
                if (state == rtc::PeerConnection::State::Disconnected ||
                    state == rtc::PeerConnection::State::Failed ||
                    state == rtc::PeerConnection::State::Closed) {
                    dispatch([this, id, state]
                            {
                                // remove disconnected client
                                pImpl->connections.erase(id);
                                details::log_info("{} to {}", magic_enum::enum_name(state), id);
                            });
                }
            });
        pc->onGatheringStateChange(
                [this, wpc = std::weak_ptr(pc), id](rtc::PeerConnection::GatheringState state) {
                // std::cout << "Gathering State: " << state << std::endl;
                if (state == rtc::PeerConnection::GatheringState::Complete) {
                    // replay
                    if(auto pc = wpc.lock()) {
                        auto description = pc->localDescription();
                        nlohmann::json message = {
                            {"id", id},
                            {"type", description->typeString()},
                            {"sdp", std::string(description.value())}
                        };
                        details::log_info("send {} from {}",description->typeString(), id);
                        // Gathering complete, send answer
                        dispatch([this, message]
                        {
                            pImpl->socket->send(message.dump());
                        });
                    }
                }
            });
        auto dc = pc->createDataChannel("ping-pong");
        dc->onOpen([id, wdc = std::weak_ptr(dc)]() {
                if (auto dc = wdc.lock()) {
                    dc->send("Ping");
                }
            });
        dc->onMessage(nullptr, [id, wdc = std::weak_ptr(dc)](std::string msg) {
                details::log_info("[ping-pong]received pong from {}", id);
                if (auto dc = wdc.lock()) {
                    dc->send("Ping");
                }
            });
        connection_warper.data_channels_.emplace_back(std::move(dc));
        pc->setLocalDescription();
        pImpl->connections.try_emplace(id, std::move(connection_warper));
    } else if (type == "answer") {
        if (auto jt = pImpl->connections.find(id); jt != pImpl->connections.end()) {
            auto& pc = jt->second;
            auto sdp = message["sdp"].get<std::string>();
            auto description = rtc::Description(sdp, type);
            pc.peerConnection_->setRemoteDescription(description);
        }
    }
}
void rtc_server::dispatch(std::function<void()> task)
{
    task::AsyncTask(pImpl->thread_id, std::move(task));
}

