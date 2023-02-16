// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

// FileDataReader.hpp
#pragma once

#include "IDataReader.hpp"
#include "myUtils.hpp"

namespace my {
struct FileDataReader : IDataReader {
    virtual size_t getSize(bool refresh = false) const noexcept override {
        if (refresh || m_size <= 0) {
            fs::path p = m_filePath;
            std::error_code ec;
            m_size = fs::file_size(p, ec);
            if (ec) {
                return 0;
            }
        }
        return m_size;
    }
    virtual size_t getPos(bool refresh = false) const noexcept override {
        if (m_pos == 0 || refresh) {
            m_pos = m_f.tellg();
        }
        return m_pos;
    }

    // does not throw, you must check you got what you wanted,
    // and is ok.
    virtual std::streamoff seek(std::streamoff pos, bool& ok,
        std::ios::seekdir dir = std::ios::beg) noexcept override {
        m_pos = utils::file_seek(pos, m_f, ok, dir);
        return m_pos;
    }

    // will throw if you don't get all the data you asked for
    virtual std::string_view read(size_t&& nBytes) override {
        auto read
            = utils::file_read_some(m_buf, m_f, nBytes, fs::path{m_filePath});
        if (read >= 0) m_pos += read;
        return std::string_view(m_buf.data(), m_buf.size());
    }

    // read into your own buffer. Does not throw, but returns
    // how many bytes you actually got.
    virtual size_t readInto(std::string& s, size_t nBytes) override {
        try {
            auto sz = static_cast<size_t>(
                utils::file_read_some(s, m_f, nBytes, m_filePath));
            m_pos += sz;
            return sz;
        } catch (std::exception&) {
            return 0;
        }
    }

    virtual size_t readInto(char* dest, size_t nBytes) noexcept override {
        try {
            m_f.read(dest, nBytes);
            m_pos += nBytes;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            m_pos += m_f.gcount();
            return m_f.gcount();
        }
        return m_f.gcount();
    }

    virtual const std::string& data() const noexcept override { return m_buf; }

    // throws system_error on failure, hopefully with a meaningful message.
    FileDataReader(const std::string& filePath,
        unsigned int openFlags = std::ios::in | std::ios::binary)
        : m_filePath(filePath) {
        utils::file_open(m_f, filePath, openFlags, true);
        m_f.exceptions(0);
        m_f.clear();
    }

    private:
    mutable std::fstream m_f;
    std::string m_buf;
    std::string m_filePath;
    mutable size_type m_size = 0;
    mutable std::streampos m_pos = 0;
};

} // namespace my
