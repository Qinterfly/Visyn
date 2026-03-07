#include <cmath>
#include <fstream>

#include "vaufxfile.h"

using namespace Visom;

//! Read a numeric field
template<typename T>
void readField(std::ifstream& file, T& value)
{
    file.read(reinterpret_cast<char*>(&value), sizeof(T));
}

//! Read a name field
std::u16string readTextField(std::ifstream& file, int capacity)
{
    std::vector<char16_t> buffer(capacity);
    file.read(reinterpret_cast<char*>(buffer.data()), capacity * sizeof(char16_t));
    int length = 0;
    for (int i = 0; i != capacity; ++i)
    {
        if (buffer[i] == '\0')
            break;
        ++length;
    }
    return std::u16string(buffer.data(), length);
}

VaufxHeader::VaufxHeader()
    : signature(0)
    , dataOffset(0)
    , sampleRate(0.0f)
    , timeInSeconds(0.0)
    , timeInSamples(0)
    , chanCount(0)
    , subheaderSize(0)
    , chunkSize(0)
{
}

VaufxSubheader::VaufxSubheader()
    : maxRmsAccel(0.0f)
    , maxAcc(0.0f)
    , maxVel(0.0f)
    , maxDisp(0.0f)
    , source(0)
    , dimension(0)
    , name()
    , hasParasiteDC(0)
{
}

VaufxFile::VaufxFile(fs::path const& path)
    : mPath(path)
{
}

//! Check if the file exists
bool VaufxFile::exists() const
{
    return fs::exists(mPath);
}

//! Retrieve the header data
VaufxHeader VaufxFile::readHeader()
{
    VaufxHeader header;

    // Open the file for reading
    std::ifstream file(mPath.string(), std::ios_base::binary);
    if (!file.is_open())
        return header;

    // Retrieve the data
    file.seekg(0, std::ios_base::beg);
    readField(file, header.signature);
    readField(file, header.dataOffset);
    readField(file, header.sampleRate);
    readField(file, header.timeInSeconds);
    readField(file, header.timeInSamples);
    readField(file, header.chanCount);
    readField(file, header.subheaderSize);
    readField(file, header.chunkSize);

    // Close the file
    file.close();

    return header;
}

//! Retrieve all the subheaders
std::vector<VaufxSubheader> VaufxFile::readSubheaders(VaufxHeader const& header)
{
    // Constants
    const int kNameLength = 260;
    const int kBaseOffset = 48;

    // Allocate the result
    std::vector<VaufxSubheader> subheaders(header.chanCount);

    // Open the file for reading
    std::ifstream file(mPath.string(), std::ios_base::binary);
    if (!file.is_open())
        return subheaders;

    // Process all the subheaders
    for (int i = 0; i != header.chanCount; ++i)
    {
        VaufxSubheader& subheader = subheaders[i];

        // Calculate and seek to position
        std::streampos pos = kBaseOffset + i * header.subheaderSize;
        file.seekg(pos, std::ios::beg);

        // Read all the fields with error checking
        readField(file, subheader.maxRmsAccel);
        readField(file, subheader.maxAcc);
        readField(file, subheader.maxVel);
        readField(file, subheader.maxDisp);
        readField(file, subheader.source);
        readField(file, subheader.dimension);
        subheader.name = readTextField(file, kNameLength);
    }

    // Close the file
    file.close();

    return subheaders;
}

//! Retrieve all the channel data
std::vector<float> VaufxFile::readData(VaufxHeader const& header, int channelNum)
{
    return readData(header, channelNum, 0, header.timeInSamples);
}

//! Retrieve the channel data using the specified interval of samples
std::vector<float> VaufxFile::readData(VaufxHeader const& header, int channelNum, uint64_t sampleFrom, uint64_t sampleTo)
{
    // Sanity check
    if (channelNum > header.chanCount || sampleTo > header.timeInSamples)
        return {};

    // Open the file for reading
    std::ifstream file(mPath.string(), std::ios_base::binary);
    if (!file.is_open())
        return {};

    // Set boundary values
    uint64_t samplesToRead = sampleTo;
    uint64_t sf = sampleFrom;
    uint64_t st = sampleTo;

    // Set offset values
    uint32_t offset = sf;
    uint32_t offsetInChunks = std::floor((double) offset / header.chunkSize);
    uint32_t startChunkOffset = header.dataOffset + (header.chanCount * (4 * header.chunkSize) + 8) * offsetInChunks;

    // Read the chunk size
    file.seekg(startChunkOffset, std::ios::beg);
    int32_t currChunkSize;
    readField(file, currChunkSize);
    readField(file, currChunkSize);
    uint32_t offsetInBytes = startChunkOffset + 8 + channelNum * 4 * currChunkSize;
    startChunkOffset = startChunkOffset + 8 + header.chanCount * 4 * currChunkSize;

    // Read the data
    uint64_t numSamples = sampleTo - sf;
    std::vector<float> tmpd(currChunkSize);
    std::vector<float> data(numSamples);
    uint64_t RS = 0;
    uint64_t SO = sf % currChunkSize;
    offsetInBytes = offsetInBytes + SO * 4;
    while (RS < samplesToRead)
    {
        file.seekg(offsetInBytes, std::ios::beg);
        int32_t STR = std::min(currChunkSize, (int32_t) std::min(samplesToRead - RS, currChunkSize - SO));

        // Read float data
        file.read(reinterpret_cast<char*>(tmpd.data()), STR * sizeof(float));

        // Check if the read was successful
        if (file.gcount() != STR * sizeof(float))
            STR = std::floor((double) file.gcount() / sizeof(float));

        // Copy tmpd to data1
        std::copy(tmpd.begin(), tmpd.begin() + STR, data.begin() + RS);
        RS += STR;
        SO = 0;

        // Read the next chunk size
        file.seekg(startChunkOffset, std::ios::beg);
        readField(file, currChunkSize);
        readField(file, currChunkSize);
        offsetInBytes = startChunkOffset + 8 + channelNum * 4 * currChunkSize;
        startChunkOffset = startChunkOffset + 8 + header.chanCount * 4 * currChunkSize;
    }

    // Close the file
    file.close();

    return data;
}
