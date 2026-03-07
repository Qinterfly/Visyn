#ifndef VAUFXFILE_H
#define VAUFXFILE_H

#include <filesystem>
#include <vector>

namespace Visom
{

//! Record description
struct VaufxHeader
{
    VaufxHeader();
    ~VaufxHeader() = default;

    uint64_t signature;
    uint32_t dataOffset;
    float sampleRate;
    double timeInSeconds;
    uint64_t timeInSamples;
    int32_t chanCount;
    int32_t subheaderSize;
    int32_t chunkSize;
};

//! Channel description
struct VaufxSubheader
{
    VaufxSubheader();
    ~VaufxSubheader() = default;

    float maxRmsAccel;
    float maxAcc;
    float maxVel;
    float maxDisp;
    uint32_t source;
    uint32_t dimension;
    std::u16string name;
    int32_t hasParasiteDC;
};

namespace fs = std::filesystem;

class VaufxFile
{
public:
    VaufxFile(fs::path const& path);
    ~VaufxFile() = default;

    bool exists() const;

    VaufxHeader readHeader();
    std::vector<VaufxSubheader> readSubheaders(VaufxHeader const& header);
    std::vector<float> readData(VaufxHeader const& header, int channelNum);
    std::vector<float> readData(VaufxHeader const& header, int channelNum, uint64_t sampleFrom, uint64_t sampleTo);

private:
    fs::path mPath;
};

}

#endif // VAUFXFILE_H
