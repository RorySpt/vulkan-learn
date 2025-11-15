//
// Created by admin on 2025/11/7.
//

#include "helpers.h"

#include <fstream>
namespace helpers
{
    std::map<int, FrameInfo> g_FrameInfo;
    std::mutex g_Mutex;
}

#pragma pack(push, 1)
struct BMPFileHeader
{
    uint16_t signature = 0x4D42; // "BM"
    uint32_t fileSize;
    uint16_t reserved1 = 0;
    uint16_t reserved2 = 0;
    uint32_t dataOffset = 54; // 文件头+信息头的大小
};

struct BMPInfoHeader
{
    uint32_t headerSize = 40;
    int32_t width;
    int32_t height;
    uint16_t planes = 1;
    uint16_t bitsPerPixel = 32; // 32位RGBA
    uint32_t compression = 0; // 不压缩
    uint32_t imageSize;
    int32_t xPixelsPerMeter = 0;
    int32_t yPixelsPerMeter = 0;
    uint32_t colorsUsed = 0;
    uint32_t importantColors = 0;
};
#pragma pack(pop)
void helpers::saveBMP_RGBA(const std::string& filename, const unsigned char* rgbaData, const int width, const int height)
{
    // 对于32位RGBA，每行不需要填充（已经是4字节对齐）
    int rowSize = width * 4;
    int imageSize = rowSize * height;
    int fileSize = 54 + imageSize;

    BMPFileHeader fileHeader;
    fileHeader.fileSize = fileSize;

    BMPInfoHeader infoHeader;
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.bitsPerPixel = 32; // 重要：改为32位
    infoHeader.imageSize = imageSize;

    std::vector<unsigned char> buffer;
    buffer.reserve(fileSize);

    auto write_buffer = [&buffer](char* p, const size_t size) { buffer.insert(buffer.end(), p, p + size); };

    // 写入文件头和信息头
    write_buffer(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    write_buffer(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    // 写入像素数据（BMP是从下到上存储，BGRA格式）
    std::vector<unsigned char> row(rowSize);

    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            int srcIndex = (y * width + x) * 4; // 源数据索引（RGBA）
            int dstIndex = x * 4; // 目标行索引（BGRA）

            // RGBA转BGRA（BMP使用BGRA格式）
            row[dstIndex + 0] = rgbaData[srcIndex + 2]; // B
            row[dstIndex + 1] = rgbaData[srcIndex + 1]; // G
            row[dstIndex + 2] = rgbaData[srcIndex + 0]; // R
            row[dstIndex + 3] = rgbaData[srcIndex + 3]; // A
        }
        write_buffer(reinterpret_cast<char*>(row.data()), rowSize);
    }
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("无法创建文件: " + filename);
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}