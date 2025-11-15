//
// Created by admin on 2025/11/11.
//

#include "av_helpers.h"

AVCodec* av_helpers::find_hw_encoder(enum AVCodecID codec_id, const char* hw_type)
{
    const AVCodec* codec = NULL;
    void* iter = NULL;

    while ((codec = av_codec_iterate(&iter)))
    {
        if (av_codec_is_encoder(codec) && codec->id == codec_id && strstr(codec->name, hw_type))
        {
            return (AVCodec*)codec;
        }
    }
    return NULL;
}

std::vector<const AVCodec*> av_helpers::list_video_encoder()
{
    std::vector<const AVCodec*> list;
    void* iter = nullptr;
    while (auto codec = av_codec_iterate(&iter))
    {
        if (av_codec_is_encoder(codec) && (codec->capabilities & AV_CODEC_CAP_HARDWARE))
        {
            list.push_back(codec);
        }
    }
    return list;
}

std::string  av_helpers::get_error_string(int error_code)
{
    char err_buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(error_code, err_buf, AV_ERROR_MAX_STRING_SIZE);
    return std::string(err_buf);
}