// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.


#pragma once

#include <algorithm>
#include <memory>

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
    void set_buffer(const uint8_t* data, header_type size)
    {
        assert(data != nullptr && "Null pointer provided");
        assert(size > header_size && "Buffer smaller than header");
        assert(m_buffer_reader == nullptr && "Previous buffer not proccessed");

        m_buffer_reader = std::make_unique<endian::stream_reader<
                          endian::big_endian>>(data, size);

        read_header();
    }

    /// @returns true if all data in the set buffer have been processed
    bool buffer_proccessed() const
    {
        return m_buffer_reader == nullptr;
    }

    /// @returns the size of the current object being parsed
    header_type object_size() const
    {
        assert(!buffer_proccessed() && "No object data available,"
               "check that buffer is not processed before calling");
        return m_object_size;
    }

    /// Writes available bytes to the given pointer.
    void write_to_object(uint8_t* object)
    {
        assert(object != nullptr && "Null pointer provided");

        m_object_completed = false;

        auto bytes = std::min<header_type>(
            (header_type)m_buffer_reader->remaining_size(), m_object_remaining);

        auto offset = m_object_size - m_object_remaining;
        m_buffer_reader->read(object + offset, bytes);

        m_object_remaining -= bytes;

        if (m_object_remaining == 0)
        {
            m_object_size = 0;
            m_object_completed = true;
        }

        if (m_buffer_reader->remaining_size() > sizeof(header_type))
        {
            read_header();
            return;
        }

        m_buffer_reader = nullptr;
    }

    /// @return true if the final part of an object was written
    bool object_completed() const
    {
        return m_object_completed;
    }

private:

    void read_header()
    {
        auto header_data = m_buffer_reader->read<header_type>();
        auto header = header_reader(header_data);
        auto start = header.template field<0>().template as<bool>();
        auto remaining = header.template field<1>().template as<header_type>();

        // Start of new object
        if (start == true)
        {
            m_object_size = remaining;
            m_object_remaining = remaining;
            return;
        }

        // Continue reading object
        if ((remaining == m_object_remaining) && (remaining != 0))
        {
            return;
        }

        // Read next header if inside the current buffer
        if (m_buffer_reader->remaining_size() > remaining + sizeof(header_type))
        {
            m_buffer_reader->skip(remaining);

            read_header();
            return;
        }

        // Any remaining data do not contain a header to be read
        m_buffer_reader = nullptr;
    }

private:

    std::unique_ptr<endian::stream_reader<endian::big_endian>> m_buffer_reader =
        nullptr;

    header_type m_object_size = 0;

    header_type m_object_remaining = 0;

    bool m_object_completed = false;
};

template<class T>
const T deserializer<T>::header_size = sizeof(T);
}
