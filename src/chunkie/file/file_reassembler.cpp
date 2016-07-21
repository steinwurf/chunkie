// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <endian/stream_reader.hpp>
#include <endian/big_endian.hpp>

#include "file_reassembler.hpp"

namespace chunkie
{
    void file_reassembler::save(const std::vector<uint8_t>& filedata)
    {
        // Write header to segment
        endian::stream_reader<endian::big_endian> reader(filedata.data(),
        filedata.size());

        uint64_t offset;
        uint64_t total_size;
        uint16_t filename_length;

        reader.read(offset);
        reader.read(total_size);
        reader.read(filename_length);


        std::string filename(filename_length, '\0');
        reader.read((uint8_t*) &filename.front(), filename_length);

        if (!initiated)
        {
            m_offset = offset;
            m_total_size = total_size;
            m_filename_length = filename_length;
            m_filename = filename;

            m_file.open((m_path / m_filename).string(),
            std::ios::binary | std::ios::out);
            initiated = true;
        }

        assert(offset == m_offset &&
               "Offset mismatch receiving file!");
        assert(total_size == m_total_size &&
               "Size mismatch receiving file!");
        assert(filename_length == m_filename_length &&
               "filename length mismatch!");
        assert(filename == m_filename && "filename mismatch!");

        // Update the offset value
        m_offset += reader.remaining_size();

        m_file.write((char*) reader.remaining_data(),
                     reader.remaining_size());
    }
}
