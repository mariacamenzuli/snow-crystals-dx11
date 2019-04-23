#pragma once

#include <vector>
#include <fstream>
#include <filesystem>

namespace FileReader {
    inline uintmax_t getFileSize(const std::string& filename) {
        const std::tr2::sys::path path(filename);
        if (exists(path) && is_regular_file(path)) {
            return file_size(path);
        }
        return 0;
    }

    inline std::vector<uint8_t> readBinaryFile(const std::string& filename) {
        std::vector<uint8_t> data;

        std::ifstream fs;
        fs.open(filename, std::ifstream::in | std::ifstream::binary);
        if (fs.good()) {
            auto size = getFileSize(filename);
            data.resize(static_cast<size_t>(size));
            fs.seekg(0, std::ios::beg);
            fs.read(reinterpret_cast<char*>(&data[0]), size);
            fs.close();
        } else {
            throw std::runtime_error(strerror(errno));
        }
        return data;
    }

}
