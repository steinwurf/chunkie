// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <limits>
#include <iostream>
#include <random>
#include <sstream>

#include <chunkie/checksum/checksum.hpp>

#include <chunkie/file/file_segmenter.hpp>
#include <chunkie/file/file_reassembler.hpp>


// Test Fixture
class test_file_segment_reassemble : public ::testing::Test
{
public:

    virtual void SetUp()
    {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch();

        m_timestamp_id = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now).count();

        std::stringstream convert_id;
        convert_id << "testfile" << "_" << m_timestamp_id << ".tmp";
        m_filename_in = convert_id.str();

        // Create a file with random contents in current directory.
        // Make sure it doesnt exists already.
        uint64_t test_file_size = 1000000; // 1 MB file size

        // random value distribution
        std::mt19937 engine(0);
        std::uniform_int_distribution<uint32_t> randval(0, 255);

        // If this file exists already, create a new one
        if(boost::filesystem::exists(m_path / m_filename_in))
        {
            uint32_t num = 0;
            std::string name;

            do
            {
                std::stringstream convert;
                convert << m_filename_in << "." << num++;
                name = convert.str();
            }
            while (boost::filesystem::exists(m_path / name)); 

            m_filename_in = name;
        }

        std::ofstream dummy_file((m_path / m_filename_in).string(),
                                 std::ios::binary | std::ios::out);

        for (uint32_t i = 0; i < test_file_size; ++i)
            dummy_file.put((uint32_t)randval(engine));
        dummy_file.close();
    }

    virtual void TearDown()
    {
        // Verify checksum of files:
        uint64_t szin = boost::filesystem::file_size(m_path / m_filename_in);
        uint64_t szout = boost::filesystem::file_size(m_path / m_filename_out);

        EXPECT_EQ(szin, szout) << "in/output files should be same size";

        std::ifstream infile( (m_path / m_filename_in).string() );
        std::ifstream outfile( (m_path / m_filename_out).string() );

        uint64_t index = 0;
        while (!infile.eof())
            EXPECT_EQ(infile.get(), outfile.get())
                << "Index " << index << "must be the same";

        // Delete the created dummy file
        EXPECT_TRUE(boost::filesystem::remove(m_path / m_filename_in))
            << "Test file " << m_filename_in << " not found. Not removed.";
        
        // Delete the 'received' file
        EXPECT_TRUE(boost::filesystem::remove(m_path / m_filename_out))
            << "Test file " << m_filename_out << " not found. Not removed.";
    }

protected:

    boost::filesystem::path m_path = boost::filesystem::current_path();

    uint64_t m_timestamp_id;
        
    std::string m_filename_in = "not_specified_yet";
    std::string m_filename_out = "not_specified_yet";
};


TEST_F(test_file_segment_reassemble, run)
{
    uint32_t max_segment_size = 10240; // load 10kiB at a time

    chunkie::file_segmenter fs(m_path / m_filename_in, max_segment_size);
    chunkie::file_reassembler fr(m_path);

    while (!fs.end_of_file())
    {
        std::vector<uint8_t> buffer = fs.load();

        EXPECT_GE(max_segment_size, buffer.size())
                << "Segments must not be larger than specified size";
        ASSERT_NE(0u, buffer.size()) << "Segments MUST be larger than 0";
        fr.save(buffer);
    }

    EXPECT_TRUE(fr.end_of_file())
            << "Reassembler should finish when segmenter is done";

    EXPECT_EQ(fr.size(), fs.size())
            << "The two files must be the same size";

    m_filename_out = fr.name();
}
