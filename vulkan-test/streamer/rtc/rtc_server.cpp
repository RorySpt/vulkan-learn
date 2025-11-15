//
// Created by admin on 2025/11/5.
//

#include "rtc_server.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

#include <rtc/websocket.hpp>
#include <rtc/peerconnection.hpp>
#include <rtc/rtppacketizationconfig.hpp>
#include <rtc/rtcpnackresponder.hpp>
#include <rtc/rtcpsrreporter.hpp>
#include <rtc/h264rtpdepacketizer.hpp>
#include <rtc/h264rtppacketizer.hpp>
#include <task_queue.h>
#include <ranges>
#include <ostream>
#include "helpers/helpers.h"

#include <chrono>
constexpr std::string_view defaultIPAddress = "127.0.0.1";
constexpr uint16_t defaultPort = 8000;

using namespace std::chrono_literals;
namespace details
{
    template <typename... Args>
    void log(Zeta::ELogLevel log_level, const std::string& message, Args&&... args)
    {
        //if (!Zeta::g_engine || !Zeta::g_engine->log_system()) return;
        //Zeta::g_engine->log_system()->log(log_level,std::string("[rtc]") + message, std::forward<Args>(args)...);
        if (log_level < Zeta::ELogLevel::Error) {
            std::vprint_unicode(std::cout, message, std::make_format_args(args...));
        }else {
            std::vprint_unicode(std::cerr, message, std::make_format_args(args...));
        }

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
struct TrackWarper
{
    std::shared_ptr<rtc::Track> track;
    std::shared_ptr<rtc::RtcpSrReporter> sender;
};
struct WeakPtrCompare {
    bool operator()(const std::weak_ptr<rtc::Track>& lhs,
                   const std::weak_ptr<rtc::Track>& rhs) const {
        auto l = lhs.lock();
        auto r = rhs.lock();
        if (!l && !r) return false; // 两者都过期，视为相等
        if (!l) return true;        // 左边过期，认为左边小
        if (!r) return false;       // 右边过期，认为右边小
        return l < r;               // 比较原始指针
    }
};

struct PeerConnectionWarper {
    std::shared_ptr<rtc::PeerConnection> peerConnection_;
    std::vector<std::shared_ptr<rtc::DataChannel>> data_channels_;
    TrackWarper video_tracks_;

    std::set<std::weak_ptr<rtc::Track>, WeakPtrCompare> ready_tracks_;
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

    std::jthread wait_connect_thread;

    using clock = std::chrono::system_clock;
    std::unordered_map<std::string, std::chrono::time_point<clock>> last_ping_time_points;

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
    ws->onOpen([]()
    {
        details::log_info("WebSocket connected, signaling ready");
    });
    ws->onClosed([]()
    {
        details::log_info("WebSocket closed" );
    });
    ws->onError([](const std::string &error)
    {
        details::log_error( "WebSocket failed: {}", error);
    });

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
    pImpl->wait_connect_thread = std::jthread([ws = pImpl->socket, url = std::move(url)](const std::stop_token& token)
    {
        // 一直等待，知道连接上信令
         while (!ws->isOpen() && !token.stop_requested() ) {
            if (ws->isClosed())
                ws->open(url);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
};
void rtc_server::send_frame(std::vector<std::byte> data, const std::chrono::duration<double> timestamp,
                            const int pts) const
{
    namespace rv = std::ranges::views;
    // 派发到发送线程
    dispatch([=,this, data = std::move(data)]
    {
        const auto mark = helpers::now();
        int i = 0; [[maybe_unused]]int size = 0;
        for (const auto track: pImpl->connections | rv::values
            | rv::transform([](PeerConnectionWarper& warper)
            {
                return warper.ready_tracks_;
            }) | rv::join
            | rv::filter([](const std::weak_ptr<rtc::Track>& track)
            {
                return !track.expired() && track.lock()->isOpen();
            })| rv::transform([](const std::weak_ptr<rtc::Track>& track)
            {
                return track.lock();
            })
            ) // 遍历所有连接中的有效的track
        {
            track->sendFrame(data,timestamp);
            ++i;
            size += data.size();
        }
        helpers::get_frame_info(pts).send_time = helpers::now();

        helpers::FrameInfo Info = helpers::get_frame_info(pts);
        if (i > 0)
        {
            details::log_info("send_frame to {} tracks, {} bytes, pts: {}, total delay: {: .2f}ms, encod_send delay: {: .2f}ms, encod_complete_time delay: {: .2f}ms"
                , i,size, pts
            ,  std::chrono::duration<double>(Info.send_time - Info.capture_time).count() * 1000
            , std::chrono::duration<double>(Info.send_time - Info.encod_send_time).count() * 1000
            , std::chrono::duration<double>(Info.send_time - Info.encod_complete_time).count() * 1000
            );
            helpers::time_cost_print("send frame", mark);
        }


    });
}
TrackWarper add_video_track(const std::shared_ptr<rtc::PeerConnection>& pc, const std::function<void (std::weak_ptr<rtc::Track>)>& onOpen)
{
    auto payloadType = 96;
    auto name = "video-stream";
    auto msid = "stream1";
    auto trackId = "video-track-1";
    auto ssrc = 1;



    auto video_desc = rtc::Description::Video("video-stream");
    video_desc.addH264Codec(payloadType);
    video_desc.addSSRC(ssrc, name, msid, trackId);
    auto track = pc->addTrack(static_cast<rtc::Description::Media>(video_desc));
    // create RTP configuration
    auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(ssrc, name, payloadType, rtc::H264RtpPacketizer::ClockRate);
    // create packetizer
    auto packetizer = std::make_shared<rtc::H264RtpPacketizer>(rtc::NalUnit::Separator::LongStartSequence, rtpConfig);
    // add RTCP SR handler
    auto srReporter = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = std::make_shared<rtc::RtcpNackResponder>();
    packetizer->addToChain(nackResponder);


    // set handler
    track->setMediaHandler(packetizer);
    track->onOpen([track = std::weak_ptr{track}, onOpen = std::move(onOpen)]()
    {
        onOpen(track);
    });
    track->onClosed([]()
    {
        details::log_info("track closed");
    });
    track->onError([](std::string error)
    {
        details::log_info("track error: {}", error);
    });

    TrackWarper track_warper;
    track_warper.sender = srReporter;
    track_warper.track = track;
    return track_warper;
}
void rtc_server::on_message(nlohmann::json message) const
{

    std::string id = message.value("id", "");
    std::string type = message.value("type", "");

    if (id.empty() || type.empty()) return;

    details::log_info("receive {} from {}",type,  id);
    if (type == "request") {
        PeerConnectionWarper connection_warper;
        connection_warper.peerConnection_ = std::make_shared<rtc::PeerConnection>(pImpl->config);
        const auto& pc = connection_warper.peerConnection_;


        auto video_desc = rtc::Description::Video("video-stream");
        connection_warper.video_tracks_ = add_video_track(pc,
            [id, this](std::weak_ptr<rtc::Track> track)
        {
            // todo start video stream here
            dispatch([this, id, track]
            {
                if (!track.expired())
                if (const auto it_pc = pImpl->connections.find(id);
                    it_pc != pImpl->connections.end())
                {
                    it_pc->second.ready_tracks_.insert(track);
                }
            });
            details::log_info("Video from  {} opened", id);
        });
        pc->onStateChange([this, id](rtc::PeerConnection::State state)
            {
                if (state == rtc::PeerConnection::State::Disconnected ||
                    state == rtc::PeerConnection::State::Failed ||
                    state == rtc::PeerConnection::State::Closed) {
                    dispatch([this, id, state]
                            {
                                // remove disconnected client
                                pImpl->connections.erase(id);
                                pImpl->last_ping_time_points.erase(id);
                                details::log_info("{} to {}", magic_enum::enum_name(state), id);
                            });
                }
            });
        pc->onGatheringStateChange(
                [this, wpc = std::weak_ptr(pc), id](const rtc::PeerConnection::GatheringState state) {
                if (state == rtc::PeerConnection::GatheringState::Complete) {
                    // replay
                    if (const auto peer_connection = wpc.lock()) {
                        const auto description = peer_connection->localDescription();
                        nlohmann::json replay_message = {
                            {"id", id},
                            {"type", description->typeString()},
                            {"sdp", std::string(description.value())}
                        };
                        details::log_info("send {} from {}",description->typeString(), id);
                        // Gathering complete, send answer
                        dispatch([this, replay_message]
                        {
                            pImpl->socket->send(replay_message.dump());
                        });
                    }
                }
            });
        auto dc = pc->createDataChannel("ping-pong");
        dc->onOpen([wdc = std::weak_ptr(dc), &last_ping_time_point = pImpl->last_ping_time_points[id]]() {
                if (const auto data_channel = wdc.lock()) {
                    last_ping_time_point = Impl::clock::now();
                    data_channel->send("Ping");
                }
            });
        dc->onMessage(nullptr, [id, wdc = std::weak_ptr(dc), &last_ping_time_point = pImpl->last_ping_time_points[id]](std::string msg) {
                auto now = Impl::clock::now();
                details::log_info("[ping-pong]received from {}, delay: {}ms", id, std::chrono::duration<double>(now - last_ping_time_point).count() * 1000);
                if (auto dc = wdc.lock()) {
                    dc->send("Ping");
                    last_ping_time_point = now;
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
void rtc_server::dispatch(std::function<void()> task) const {
    task::AsyncTask(pImpl->thread_id, std::move(task));
}