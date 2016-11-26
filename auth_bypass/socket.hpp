/*
    MIT License

    Copyright (c) 2017, namreeb (legal@namreeb.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include <boost/asio.hpp>

// It's not fast, but we're not doing HFT so who cares.
class Socket
{
public:
    explicit Socket(std::string const& hostname, int port)
        : io_service_(), socket_(io_service_), resolver_(io_service_), query_(boost::asio::ip::tcp::v4(), hostname, std::to_string(port))
    {
    }

    boost::asio::ip::tcp::resolver::iterator Connect()
    {
        boost::asio::ip::tcp::resolver::iterator iterator = resolver_.resolve(query_);
        boost::asio::connect(socket_, iterator);
        return iterator;
    }

    void Disconnect()
    {
        socket_.close();
    }

    size_t BytesReadable()
    {
        boost::asio::socket_base::bytes_readable command(true);
        socket_.io_control(command);
        return command.get();
    }

    bool ReadRaw(void* p, std::size_t s)
    {
        boost::system::error_code error;
        std::size_t const len = boost::asio::read(socket_, boost::asio::buffer(p, s), error);
        if (len != s)
        {
            throw std::runtime_error("Invalid read size.");
        }
        return Validate(error);
    }

    bool WriteRaw(void const* p, std::size_t s)
    {
        boost::system::error_code error;
        std::size_t const len = boost::asio::write(socket_, boost::asio::buffer(p, s), error);
        if (len != s)
        {
            throw std::runtime_error("Invalid write size.");
        }
        return Validate(error);
    }

    void PushEndBuffer(void const* p, std::size_t s)
    {
        std::copy(static_cast<std::uint8_t const*>(p), static_cast<std::uint8_t const*>(p) + s, std::back_inserter(buffer_));
    }

    template <typename T> void PushEndBuffer(T const& t)
    {
        static_assert(std::is_pod<T>::value, "T must be POD.");
        PushEndBuffer(&t, sizeof(t));
    }

    void PopFrontBuffer(void* p, std::size_t s)
    {
        assert(!buffer_.empty());
        assert(buffer_.size() > s);

        if (buffer_.size() < s)
        {
            throw std::runtime_error("Invalid buffer size.");
        }

        std::copy(std::begin(buffer_), std::begin(buffer_) + s, static_cast<std::uint8_t*>(p));
        buffer_.erase(std::begin(buffer_), std::begin(buffer_) + s);
    }

    template <typename T> T PopFrontBuffer()
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be POD.");

        T t;
        PopFrontBuffer(&t, sizeof(t));
        return t;
    }

    bool ReadBuffer(std::size_t s)
    {
        assert(buffer_.empty());
        buffer_.resize(s);
        return ReadRaw(buffer_.data(), buffer_.size());
    }

    bool WriteBuffer()
    {
        assert(!buffer_.empty());
        auto const ret = WriteRaw(buffer_.data(), buffer_.size());
        buffer_.clear();
        return ret;
    }

    void ClearBuffer()
    {
        buffer_.clear();
    }

    std::size_t GetBufferSize() const
    {
        return buffer_.size();
    }

    template <typename T> bool Read(T& t)
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be POD.");

        return ReadRaw(&t, sizeof(t));
    }

    bool ReadString(std::string& out)
    {
        char byte;
        std::stringstream ret;

        do
        {
            if (!Read(byte))
                return false;

            if (!byte)
                break;

            ret << byte;
        } while (true);

        out = ret.str();
        return true;
    }

    template <typename T> bool Write(T const& t)
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be POD.");

        return WriteRaw(&t, sizeof(t));
    }

    boost::asio::ip::tcp::endpoint GetLocalEndpoint() const
    {
        return socket_.local_endpoint();
    }

    bool IsClosed() const
    {
        return !socket_.is_open();
    }

private:
    bool Validate(boost::system::error_code const& error) const
    {
        if (error == boost::asio::error::eof)
        {
            // Connection closed cleanly by peer.
            return false;
        }

        if (error)
        {
            // Some other error.
            throw boost::system::system_error(error);
        }

        return true;
    }

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::query query_;
    std::vector<std::uint8_t> buffer_;
};
