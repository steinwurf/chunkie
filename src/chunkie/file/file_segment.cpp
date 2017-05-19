// Copyright (c) 2017 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include "file_segment.hpp"

#include <endian/big_endian.hpp>
#include <endian/stream_reader.hpp>
#include <endian/stream_writer.hpp>

namespace chunkie
{

file_segment::file_segment(uint32_t id,
                           uint32_t size,
                           uint64_t offset,
                           uint64_t file_size,
                           uint32_t checksum,
                           const std::string& filename,
                           const uint8_t* data) :
    m_id(id),
    m_size(size),
    m_offset(offset),
    m_file_size(file_size),
    m_checksum(checksum),
    m_filename(filename),
    m_data(data)
{
}


uint32_t file_segment::size_serialized() const
{
    return (sizeof(m_id) + sizeof(m_size) + sizeof(m_offset) +
            sizeof(m_file_size) + sizeof(m_checksum) +
            sizeof(uint32_t) + // filename length field
            m_filename.length() +
            m_size);
}


// Return the name of the corresponding file
std::string file_segment::filename() const
{
    return m_filename;
}

file_segment file_segment::from_buffer(const uint8_t* buffer,
                                       uint32_t bufsize,
                                       std::error_code& error)  noexcept
{
    uint32_t header_min_length = sizeof(m_id) + sizeof(m_size) + sizeof(m_offset) +
                                 sizeof(m_file_size) + sizeof(m_checksum) + sizeof(uint32_t);

    if (bufsize < header_min_length)
    {
        error = std::make_error_code(std::errc::message_size);
        return file_segment();
    }

    endian::stream_reader<endian::big_endian> deserializer(buffer, bufsize);

    uint32_t id;
    uint32_t size;
    uint64_t offset;
    uint64_t file_size;
    uint32_t checksum;
    uint32_t filename_length;
    std::string filename;
    const uint8_t* data;

    deserializer.read(id);
    deserializer.read(size);
    deserializer.read(offset);
    deserializer.read(file_size);
    deserializer.read(checksum);
    deserializer.read(filename_length);

    if (bufsize != header_min_length + filename_length + size)
    {
        error = std::make_error_code(std::errc::message_size);
        return file_segment();
    }

    // Throw/error_code would be better
    assert(bufsize >= 8 * sizeof(uint32_t) + filename_length + size);

    filename = std::string(deserializer.remaining_data(),
                           deserializer.remaining_data() + filename_length);
    data = deserializer.remaining_data() + filename_length;

    return file_segment(id, size, offset, file_size, checksum, filename, data);
}

file_segment file_segment::from_buffer(const std::vector<uint8_t>& buffer,
                                       std::error_code& error)  noexcept
{
    return from_buffer(buffer.data(), buffer.size(), error);
}

file_segment file_segment::from_buffer(const uint8_t* buffer, uint32_t bufsize)
{
    std::error_code error;
    file_segment fs = from_buffer(buffer, bufsize, error);
    if (error)
        throw error;
    return fs;
}

file_segment file_segment::from_buffer(const std::vector<uint8_t>& buffer)
{
    return from_buffer(buffer.data(), buffer.size());
}

void file_segment::serialize(uint8_t* buffer, uint32_t size) const
{
    assert(size >= size_serialized());
    assert(m_size != 0 && "Can not serialize an empty segment");

    endian::stream_writer<endian::big_endian> serializer(buffer, size);

    serializer.write(m_id);
    serializer.write(m_size);
    serializer.write(m_offset);
    serializer.write(m_file_size);
    serializer.write(m_checksum);
    serializer.write(static_cast<uint32_t>(m_filename.length()));
    serializer.write((uint8_t*) m_filename.data(), m_filename.length());
    serializer.write(m_data, m_size);
}

void file_segment::serialize(std::vector<uint8_t>& buffer) const
{
    buffer.resize(size_serialized());
    serialize(buffer.data(), buffer.size());
}

uint64_t file_segment::file_size() const
{
    return m_file_size;
}
}
