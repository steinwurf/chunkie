// Copyright (c) 2017 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <map>
#include <string>

#include "../checksum/checksum.hpp"

#include "file_segment.hpp"

namespace chunkie
{

/// Reassembles a file previously cut into chunks by the file_segmenter
/// Template argument <File> must implement interface
///     char_type* File::data()
///     size_type File::size()
///
/// - boost::iostreams::mapped_file does this.
template<class File>
class file_reassembler
{
    // Structure for tracking saved segments
    struct segment_tracker
    {
        uint64_t m_offset; // The segment offset
        uint32_t m_size;   // The segment size
    };

public:
    // Construct the reassembler with filename, file size and base path
    // as arguments. Base path denotes the directory in which the reassembled
    // file should be saved. Default is current path.
    file_reassembler(File& file, std::string filename) :
        m_file(file),
        m_filename(filename)
    {
    }

    // Return the size of the file being reassembled
    uint64_t file_size() const
    {
        return m_file.size();
    }

    // Return the name of the file being reassembled
    std::string filename() const
    {
        return m_filename;
    }

    // Save file segment data to the file.
    void write_segment(const file_segment& segment)
    {
        // throw/error_code probably better here
        assert(segment.filename() == m_filename && "Filename must match");
        assert(segment.m_file_size == m_file.size() && "File size must match");
        assert(!has_segment(segment.m_id));

        // A throw/error_code on checksum mismatch is probably better
        uint32_t checksum = detail::crc32(segment.m_data, segment.m_size);
        assert(checksum == segment.m_checksum && "Checksum mismatch");

        std::copy(segment.m_data,
                  segment.m_data + segment.m_size,
                  m_file.data() + segment.m_offset);

        m_segments[segment.m_id] = {segment.m_offset, segment.m_size};
    }

    // return the amount of bytes reassembled so far
    uint64_t reassembled_bytes() const
    {
        uint64_t segment_size_sum = 0;
        for (const auto& kv : m_segments)
        {
            const segment_tracker& s = kv.second;
            segment_size_sum += s.m_size;
        }
        return segment_size_sum;
    }

    // Check if segment id 'id' is saved
    bool has_segment(uint32_t sid) const
    {
        return bool(m_segments.count(sid));
    }

    // Check if segment is saved already
    bool has_segment(const file_segment& segment) const
    {
        return has_segment(segment.m_id);
    }

private:

    // Path and file
    File& m_file;
    const std::string m_filename;

    // Tracker of received segments
    std::map<uint32_t, segment_tracker> m_segments;
};
}
