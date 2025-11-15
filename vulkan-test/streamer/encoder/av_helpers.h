//
// Created by admin on 2025/11/11.
//

#ifndef ZETAENGINE_AV_HELPERS_H
#define ZETAENGINE_AV_HELPERS_H
#include <vector>
#include <string>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>

}
class av_helpers
{
public:
    static AVCodec* find_hw_encoder(enum AVCodecID codec_id, const char* hw_type);
    static std::vector<const AVCodec*> list_video_encoder();
    static  std::string  get_error_string(int error_code) ;
};

#endif // ZETAENGINE_AV_HELPERS_H
