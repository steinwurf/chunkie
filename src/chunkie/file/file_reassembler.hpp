// Copyright (c) 2015 Steinwurf ApS
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

    /// Reassembles a file previously cut into chunks by the file_segmenter
    class file_reassembler
    {
    public:

        file_reassembler(boost::filesystem::path base_dir
        = boost::filesystem::current_path()) :
            m_path(base_dir)
        { }

        ~file_reassembler()
        {
            if (m_file.is_open())
            {
                m_file.close();
            }
        }

        // Save data to a file
        // @param filedata a vector reference with file data incl. headers
        // to reconstruct file
        void save(const std::vector<uint8_t>& filedata);

        // Return true if no more data available
        bool end_of_file() const
        {
            return m_total_size == m_offset;
        }

        uint64_t offset()
        {
            return m_offset;
        }

        uint64_t size()
        {
            return m_total_size;
        }

        const std::string& name()
        {
            return m_filename.string();
        }

    private:

        void initiate(boost::filesystem::path filename, 
                      uint64_t offset, uint64_t total_size);

    private:

        // Path and file
        boost::filesystem::path m_path;
        std::ofstream m_file;
        boost::filesystem::path m_filename;

        // Size and offset
        uint64_t m_total_size = 0;
        uint64_t m_offset = 0;

        bool m_initiated = false;
    };
}
