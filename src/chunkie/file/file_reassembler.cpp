// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <sstream>

#include <endian/stream_reader.hpp>
#include <endian/big_endian.hpp>

#include "file_reassembler.hpp"

namespace chunkie
{
    void file_reassembler::initiate(boost::filesystem::path filename, 
                                    uint64_t offset, uint64_t total_size)
    {
        assert(!m_initiated && "Initiate should only be called once");
        assert(m_offset == 0 && "Should not initiate when offset is not zero");

        m_offset = offset;
        m_total_size = total_size;

        if (boost::filesystem::exists(m_path / filename))
        {
            uint32_t num = 0;
            std::string name;
            
            do
            {
                std::stringstream convert;
                convert << filename.string() << "." << num++;
                name = convert.str();
            }
            while (boost::filesystem::exists(m_path / name)); 
            
            filename = name;
        }
        m_filename = filename;

        m_file.open((m_path / filename).string(),
                    std::ios::binary | std::ios::out);
        m_initiated = true;
    }

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

        // Read filename as stated by sender
        std::string tmpfilename(filename_length, '\0');
        reader.read((uint8_t*) &tmpfilename.front(), filename_length);
        boost::filesystem::path filename(tmpfilename);

        if (!m_initiated)
        {
            initiate(filename, offset, total_size);
        }

        assert(offset == m_offset&&
               "Offset mismatch receiving file!");
        assert(total_size == m_total_size &&
               "Size mismatch receiving file!");

        // match only on filename stems (we might add extensions if path exist)
        boost::filesystem::path stem1 = filename;
        boost::filesystem::path stem2 = m_filename;

        while(stem1.has_extension())
            stem1 = stem1.stem();
        while (stem2.has_extension())
            stem2 = stem2.stem();
        
        assert(stem1 == stem2 && "filename mismatch!");

        // Update the offset value
        m_offset += reader.remaining_size();

        m_file.write((char*) reader.remaining_data(),
                     reader.remaining_size());
    }

}
