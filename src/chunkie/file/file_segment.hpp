// Copyright (c) 2017 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <string>
#include <system_error>
#include <vector>

namespace chunkie
{

/// Hold a segment of a file
/// Segment is only valid as long as the corresponding data is valid
///
/// Structure of a file segment:
/// @code
///  0                   1                   2                   3
///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       segment id                              |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       segment size                            |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       offset (MSB)                            |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       offset (LSB)                            |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       file size (MSB)                         |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       file size (LSB)                         |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       checksum                                |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       filename length                         |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       filename (`filename length` bytes)      |
/// |                       ... ... ... ...                         |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       DATA (`segment_size` bytes)             |
/// |                       ... ... ... ...                         |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///
/// Segment id: ID of this current segment. Usually an incrementing [uint32_t]
/// Segment size: Size of this segment [uint32_t]
/// offset: The file segment offset in the file in bytes [uint64_t]
/// file size: The size of the file in bytes [uint64_t]
/// crc32 checksum: checksum of the DATA field in this segment [uint32_t]
/// Filename length: Length of the filename for this segment [uint32_t]
/// Filename: The name of the file this segment belongs to. [buffer]
/// DATA: The file segment data - must be `segment size` bytes [buffer]
///
/// @endcode
struct file_segment
{
    file_segment(uint32_t id,
                 uint32_t size,
                 uint64_t offset,
                 uint64_t file_size,
                 uint32_t checksum,
                 const std::string& filename,
                 const uint8_t* data);

    // Construct file segment from a buffer. The file segment is only valid
    // as long as the buffer is valid
    static file_segment from_buffer(const uint8_t* buffer,
                                    uint32_t size,
                                    std::error_code& error) noexcept;
    static file_segment from_buffer(const std::vector<uint8_t>& buffer,
                                    std::error_code& error) noexcept;
    static file_segment from_buffer(const uint8_t* buffer, uint32_t size);
    static file_segment from_buffer(const std::vector<uint8_t>& buffer);

    // Get the size of the file segment (data plus headers)
    uint32_t size_serialized() const;

    void serialize(uint8_t* buffer, uint32_t size) const;
    void serialize(std::vector<uint8_t>& buffer) const;

    // Return the name of the corresponding file
    std::string filename() const;

    // Return the size of the corresponding file
    uint64_t file_size() const;

private:

    file_segment() {}

public:

    const uint32_t m_id = 0;
    const uint32_t m_size = 0;
    const uint64_t m_offset = 0;
    const uint64_t m_file_size = 0;
    const uint32_t m_checksum = 0;
    const std::string m_filename = "";
    const uint8_t* m_data = nullptr;
};
}
