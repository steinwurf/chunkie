// Copyright (c) 2017 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <algorithm>
#include <cmath>
#include <string>

#include "file_segment.hpp"
#include "../checksum/checksum.hpp"

namespace chunkie
{

/// Cuts a file into smaller chunks. Enables an application to only load
/// part of a file for transmission.
///
/// structure of a file segment:
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

/// Template argument <File> must implement interface
///     const char_type* File::data()
///     size_type File::size()
///
/// - boost::iostreams::mapped_file does this.
template<class File>
class file_segmenter
{

public:
    file_segmenter(File& file,
                   std::string filename,
                   uint32_t segment_size = 1'000'000) :
        m_file(file),
        m_filename(filename),
        m_segment_size(segment_size),
        m_segments(
            (uint32_t) std::ceil(double(m_file.size()) / m_segment_size)),
        m_header_size(8u * sizeof(uint32_t) + m_filename.length())
    {
    }

    std::string filename() const
    {
        return m_filename;
    }

    uint32_t segments() const
    {
        return m_segments;
    }

    uint32_t file_size() const
    {
        return m_file.size();
    }

    file_segment read_segment(uint32_t id) const
    {
        assert(id < m_segments &&
               "Can only get segments with ID less than number of segments");

        uint64_t offset = id * m_segment_size;
        const uint8_t* data_ptr = (const uint8_t*) m_file.data() + offset;
        uint32_t size =
            std::min((uint64_t) m_segment_size, file_size() - offset);
        uint32_t checksum = detail::crc32(data_ptr, size);

        return {id,
                size,
                offset,
                file_size(),
                checksum,
                m_filename,
                data_ptr};
    }

private:

    // Path and file
    File& m_file;
    const std::string m_filename;

    const uint32_t m_segment_size;
    const uint32_t m_segments;

    // Header
    const uint32_t m_header_size;
};
}
