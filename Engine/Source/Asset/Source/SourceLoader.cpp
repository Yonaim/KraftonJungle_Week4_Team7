#include "Asset/Source/SourceLoader.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace Asset
{

    namespace
    {
        static uint64 ToUnixLikeTicks(const std::filesystem::file_time_type& InTime)
        {
            const auto Ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(InTime.time_since_epoch())
                    .count();
            return static_cast<uint64>(Ns);
        }
    } // namespace

    bool FSourceLoader::QueryFileInfo(const std::filesystem::path& Path, uint64& OutFileSize,
                                      uint64& OutWriteTimeTicks)
    {
        OutFileSize = 0;
        OutWriteTimeTicks = 0;

        std::error_code Ec;
        if (!std::filesystem::exists(Path, Ec) || Ec)
        {
            return false;
        }

        OutFileSize = static_cast<uint64>(std::filesystem::file_size(Path, Ec));
        if (Ec)
        {
            return false;
        }

        const auto WriteTime = std::filesystem::last_write_time(Path, Ec);
        if (Ec)
        {
            return false;
        }

        OutWriteTimeTicks = ToUnixLikeTicks(WriteTime);
        return true;
    }

    bool FSourceLoader::ReadAllBytes(const std::filesystem::path& Path, TArray<uint8>& OutBytes)
    {
        OutBytes.clear();

        std::ifstream File(Path, std::ios::binary);
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

} // namespace Asset
