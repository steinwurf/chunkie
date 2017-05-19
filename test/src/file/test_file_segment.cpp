// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <gtest/gtest.h>

#include <vector>

#include <chunkie/file/file_segment.hpp>

TEST(test_file_segment, serialize_parse_from_buffer)
{
    uint32_t segment_id = 1337;
    uint32_t segment_size = 523;
    uint64_t segment_offset = 42000;
    uint64_t file_size = 65000;
    uint32_t checksum = 15;
    std::string filename = "Testfile.ext";
    std::vector<uint8_t> segment_data(523, 'b');

    std::vector<uint8_t> buffer;

    {
        chunkie::file_segment fs(segment_id,
                                 segment_size,
                                 segment_offset,
                                 file_size,
                                 checksum,
                                 filename,
                                 segment_data.data());

        fs.serialize(buffer);
    }

    // fs object out of scope

    {
        // parse success
        std::error_code error;
        auto fs = chunkie::file_segment::from_buffer(buffer, error);

        EXPECT_FALSE(error)
            << "Expected success but got error " << error.message();

        EXPECT_EQ(segment_id, fs.m_id);
        EXPECT_EQ(segment_size, fs.m_size);
        EXPECT_EQ(segment_offset, fs.m_offset);
        EXPECT_EQ(checksum, fs.m_checksum);
        EXPECT_EQ(file_size, fs.m_file_size);
        EXPECT_EQ(filename, fs.m_filename);

        // create vector with parsed data
        std::vector<uint8_t> parsed_data(
            fs.m_data, fs.m_data + fs.m_size);
        EXPECT_EQ(segment_data, parsed_data);
    }

    {
        // parse fail message_size 1
        std::vector<uint8_t> buffer_copy = buffer;
        buffer_copy.resize(8);
        std::error_code error;
        chunkie::file_segment::from_buffer(buffer_copy, error);

        EXPECT_EQ(std::errc::message_size, error)
            << "Expected errc::message_size but got error " << error.message();
    }

    {
        // parse fail message_size 2
        std::vector<uint8_t> buffer_copy = buffer;
        buffer_copy.resize(buffer.size() - 1);
        std::error_code error;
        chunkie::file_segment::from_buffer(buffer_copy, error);

        EXPECT_EQ(std::errc::message_size, error)
            << "Expected errc::message_size but got error " << error.message();
    }

    {
        // parse fail message_size 2
        std::vector<uint8_t> buffer_copy = buffer;
        buffer_copy.resize(buffer.size() - 1);
        std::error_code error;

        try
        {
            chunkie::file_segment::from_buffer(buffer_copy);
            FAIL() << "file_segment::from_buffer did not fail as expected.";
        }
        catch (const std::error_code& error)
        {
            EXPECT_EQ(std::errc::message_size, error)
                << "Expected errc::message_size but got error "
                << error.message();
        }
    }
}
