// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <algorithm>

#include <endian/stream_writer.hpp>
#include <endian/big_endian.hpp>

#include <bitter/msb0_writer.hpp>

namespace chunkie
{

/// The object serializer cuts objects into buffers of any size
/// The serializer will append headers to the objects and the buffers.
/// One header at the start of each buffer, and one header for each
/// additional object within a buffer.
///
/// The length of these can be adjusted with the HeaderType template
/// parameter, providing maximum objects lengths given the following:
/// HeaderType = uint8_t  -> object sizes must be smaller than 128 bytes
/// HeaderType = uint16_t -> object sizes must be smaller than 32768 bytes
/// HeaderType = uint32_t -> object sizes must be smaller than 2^31 bytes
/// HeaderType = uint64_t -> object sizes must be smaller than 2^63 bytes
/// A HeaderType of uint32_t is default and typically supports large enough
/// objects. If objects a small a smaller header type can be used to reduce
/// the added overhead.
template<typename HeaderType = uint32_t>
class serializer
{
public:

    using header_type = HeaderType;

    static const header_type max_object_size;

    static const header_type header_size;

private:

    // The header consists of a size and a start bit
    using header_writer =
        bitter::msb0_writer<header_type, 1, (sizeof(header_type) * 8) - 1>;

public:

    void set_object(const uint8_t* object, header_type size)
    {
        assert(object != nullptr && "Null pointer provided");
        assert(size > 0 && "Object is empty");
        assert(size <= max_object_size && "object too big for header type");
        assert(m_object == nullptr && "Last object not proccessed");

        m_object = object;
        m_object_size = size;
        m_object_remaining = size;
    }

    /// Check if a prevously set object has been completely processed
    /// @return false if some data from the set object has not been written
    /// to a buffer 
    bool object_proccessed() const
    {
        return m_object == nullptr;
    }

    /// @return the maximal number of bytes that can be written to a buffer.
    header_type max_write_buffer_size() const
    {
        assert(m_object != nullptr && "No object set");
        return header_size + m_object_remaining;
    }

    /// Write size bytes to the provided buffer
    /// fails if buffer is provided that is larger than what can be written
    /// @param data the buffer to write to
    /// @param the size of teh buffer
    void write_buffer(uint8_t* data, header_type size)
    {
        assert(data != nullptr && "Null pointer provided");
        assert(size > header_size && "Buffer too small for header");
        assert(size <= max_write_buffer_size() &&
               "Buffer larger resulting write of all remaining data");
        assert(m_object != nullptr && "No object set");

        endian::stream_writer<endian::big_endian> writer(data, size);

        auto header = header_writer();
        auto start = m_object_remaining == m_object_size;
        header.template field<0>(start);
        header.template field<1>(m_object_remaining);

        writer.write(header.data());
        auto bytes = std::min<header_type>(
            size - header_size, m_object_remaining);

        writer.write(m_object, bytes);
        m_object += bytes;
        m_object_remaining -= bytes;

        // object done
        if (m_object_remaining == 0)
        {
            m_object = nullptr;
            m_object_size = 0;
        }
    }

private:

    const uint8_t* m_object = nullptr;

    header_type m_object_size = 0;

    header_type m_object_remaining = 0;
};
template<class T>
const T serializer<T>::max_object_size = std::numeric_limits<T>::max() / 2;

template<class T>
const T serializer<T>::header_size = sizeof(T);
}
