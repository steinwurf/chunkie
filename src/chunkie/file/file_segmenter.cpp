// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <endian/stream_writer.hpp>
#include <endian/big_endian.hpp>

#include "file_segmenter.hpp"

namespace chunkie
{
    const std::vector<uint8_t>& file_segmenter::load()
    {
        m_segment.resize(m_max_segment_size);
        // Write header to segment
        endian::stream_writer<endian::big_endian> writer(m_segment.data(),
        m_segment.size());
        writer.write(m_offset);
        writer.write(m_total_size);
        writer.write(m_filename_length);
        writer.write((uint8_t*) &m_filename.front(), m_filename.size());

        assert(writer.position() == m_header_size);

        // Write data to segment
        uint32_t bytes = m_file.readsome((char*)writer.remaining_data(),
        writer.remaining_size());

        m_segment.resize(bytes + m_header_size);

        m_offset += bytes;

        if (m_offset == m_total_size)
        {
            m_end_of_file = true;
        }
        if (bytes == 0)
        {
            m_segment.clear();
        }

        return m_segment;
    }
}
