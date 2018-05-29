// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <algorithm>

#include <endian/big_endian.hpp>
#include <endian/stream_reader.hpp>

#include <bitter/msb0_reader.hpp>

namespace chunkie
{
template <typename HeaderType = uint32_t>
class deserializer
{
public:

    using header_type = HeaderType;

    static const header_type header_size;

private:
    // The header consists of a size and a start bit
    using header_reader =
        bitter::msb0_reader<header_type, 1, (sizeof(header_type) * 8) - 1>;

public:

    // Read from a buffer. buffers must be read in-order,
    void set_buffer(const uint8_t* data, uint32_t size)
    {
        assert(data != nullptr && "Null pointer provided");
        assert(size > header_size && "Buffer smaller than header");
        assert(m_buffer == nullptr && "Previous buffer not proccessed");
        assert(m_buffer_remaining == 0 && "Previous buffer not proccessed");

        read_header(data, size);
    }

    /// @returns true if all data in the set buffer have been processed
    bool buffer_proccessed() const
    {
        return m_buffer == nullptr;
    }

    /// @returns the size of the current object being parsed
    header_type object_size() const
    {
        return m_object_size;
    }

    /// @return true if the final part of a object was written
    void write_to_object(uint8_t* object)
    {
        assert(object != nullptr && "Null pointer provided");
        assert(m_buffer != nullptr && "No buffer set");
        assert(m_buffer_remaining != 0 && "No remaining data in buffer");

        m_object_completed = false;

        auto reader = endian::stream_reader<endian::big_endian>(
            m_buffer, m_buffer_remaining);
        auto bytes = std::min(m_buffer_remaining, m_object_remaining);

        auto offset = m_object_size - m_object_remaining;
        reader.read(object + offset, bytes);

        m_buffer += bytes;
        m_buffer_remaining -= bytes;
        m_object_remaining -= bytes;

        if (m_object_remaining == 0)
        {
            m_object_size = 0;
            m_object_completed = true;
        }

        read_header(m_buffer, m_buffer_remaining);
    }

    bool object_completed() const
    {
        return m_object_completed;
    }

private:

    void read_header(const uint8_t* data, uint32_t size)
    {
        assert(data != nullptr && "Null pointer provided");

        if (size <= header_size)
        {
            m_buffer = nullptr;
            m_buffer_remaining = 0;
            return;
        }

        auto reader = endian::stream_reader<endian::big_endian>(data, size);

        auto header_data = reader.read<header_type>();
        auto header = header_reader(header_data);
        auto start = header.template field<0>().template as<bool>();
        auto remaining = header.template field<1>().template as<header_type>();

        m_buffer = data + header_size;
        m_buffer_remaining = size - header_size;

        if (start == true)
        {
            m_object_size = remaining;
            m_object_remaining = remaining;
            return;
        }

        if ((remaining == m_object_remaining) && (remaining != 0))
        {
            return;
        }

        read_header(m_buffer + remaining, m_buffer_remaining - remaining);
    }

private:

    const uint8_t* m_buffer = nullptr;

    uint32_t m_buffer_remaining = 0;

    header_type m_object_size = 0;

    header_type m_object_remaining = 0;

    bool m_object_completed = false;
};

template<class T>
const T deserializer<T>::header_size = sizeof(T);
}
