// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>


namespace chunkie
{

    /// Cuts a file into smaller chunks. Enables an application to only load
    /// part of a file for transmission.
    class file_segmenter
    {
    public:

        file_segmenter(boost::filesystem::path path,
        uint32_t max_segment_size = 10e6) :
            m_path(path),
            m_file(path.c_str(), std::ios::binary | std::ios::in),
            m_filename(m_path.filename().string()),
            m_max_segment_size(max_segment_size),
            m_segment(max_segment_size),
            m_total_size(boost::filesystem::file_size(path)),
            m_filename_length(m_filename.length()),
            m_header_size(sizeof(m_offset) + sizeof(m_total_size)
            + sizeof(m_filename_length) + m_filename_length)
        { }

        ~file_segmenter()
        {
            if (m_file.is_open())
            {
                m_file.close();
            }
        }

        // Load data from file
        // returns a const vector reference with a chunk of file data incl.
        // headers to reconstruct file. Returned vector data is only valid until
        // next load() is called
        const std::vector<uint8_t>& load();

        // Return true if no more data available
        bool end_of_file() const
        {
            return m_end_of_file;
        }

        uint64_t offset()
        {
            return m_offset;
        }

        uint64_t size()
        {
            return m_total_size;
        }



    private:

        // Path and file
        boost::filesystem::path m_path;
        std::ifstream m_file;
        std::string m_filename;

        // File segment
        const uint32_t m_max_segment_size;
        std::vector<uint8_t> m_segment;

        // Size and offset
        const uint64_t m_total_size = 0;
        uint64_t m_offset = 0;

        // Header
        const uint16_t m_filename_length;
        const uint32_t m_header_size;

        bool m_end_of_file = false;
    };
}
