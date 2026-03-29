#include "Asset/Source/SourceLoader.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace
{
    static uint64 ToUnixLikeTicks(const std::filesystem::file_time_type& InTime)
    {
        const auto Ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(InTime.time_since_epoch()).count();
        return static_cast<uint64>(Ns);
    }
} // namespace

bool FSourceLoader::QueryFileInfo(const FWString& Path, uint64& OutFileSize,
                                  uint64& OutWriteTimeTicks)
{
    OutFileSize = 0;
    OutWriteTimeTicks = 0;

    std::error_code             Ec;
    const std::filesystem::path FilePath(Path);

    if (!std::filesystem::exists(FilePath, Ec) || Ec)
    {
        return false;
    }

    OutFileSize = static_cast<uint64>(std::filesystem::file_size(FilePath, Ec));
    if (Ec)
    {
        return false;
    }

    const auto WriteTime = std::filesystem::last_write_time(FilePath, Ec);
    if (Ec)
    {
        return false;
    }

    OutWriteTimeTicks = ToUnixLikeTicks(WriteTime);
    return true;
}

bool FSourceLoader::ReadAllBytes(const FWString& Path, TArray<uint8>& OutBytes)
{
    OutBytes.clear();

    std::ifstream File(std::filesystem::path(Path), std::ios::binary);
    if (!File)
    {
        return false;
    }

    File.seekg(0, std::ios::end);
    const std::streamoff Size = File.tellg();
    if (Size < 0)
    {
        return false;
    }

    File.seekg(0, std::ios::beg);

    OutBytes.resize(static_cast<size_t>(Size));
    if (Size > 0)
    {
        File.read(reinterpret_cast<char*>(OutBytes.data()), Size);
        if (!File)
        {
            OutBytes.clear();
            return false;
        }
    }

    return true;
}
