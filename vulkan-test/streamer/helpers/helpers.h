//
// Created by admin on 2025/11/7.
//
#ifndef ZETAENGINE_HELPERS_H
#define ZETAENGINE_HELPERS_H



#include <chrono>
#include <map>
#include <mutex>
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

namespace helpers
{
    template <typename... Args>
    void log(Zeta::ELogLevel log_level, const std::string& message, Args&&... args)
    {
        //if (!Zeta::g_engine || !Zeta::g_engine->log_system()) return;
        //Zeta::g_engine->log_system()->log(log_level,std::string("[psm]") + message, std::forward<Args>(args)...);
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

    using time_point = std::chrono::time_point<std::chrono::steady_clock>;

    inline time_point now(){return std::chrono::high_resolution_clock::now(); }
    inline thread_local time_point current;
    inline void time_mark(){current = now();}
    inline auto time_cost_print(std::string_view str, time_point mark = current)
    {
        log_info("{} : {: .3f}ms", str.empty() ? std::string("cost") : std::string(str) + " cost",
                 std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - mark).count() * 1000);
    };

    void saveBMP_RGBA(const std::string& filename, const unsigned char* rgbaData, const int width, const int height);

    struct FrameInfo
    {
        time_point capture_time;
        time_point encod_send_time;
        time_point encod_complete_time;
        time_point send_time;
    };

    extern std::map<int, FrameInfo> g_FrameInfo;
    extern std::mutex g_Mutex;

    inline FrameInfo& get_frame_info(int pts){std::scoped_lock lock(g_Mutex); return g_FrameInfo[pts];};
    inline void pop_frame_info(int pts) {std::scoped_lock lock(g_Mutex); g_FrameInfo.erase(pts); }

}

#endif // ZETAENGINE_HELPERS_H
