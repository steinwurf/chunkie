// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <gtest/gtest.h>

#include <memory>
#include <random>
#include <deque>

#include <chunkie/checksum/checksum.hpp>

#include <chunkie/file/file_reassembler.hpp>
#include <chunkie/file/file_segmenter.hpp>

namespace
{
// Returns the first offset at which a data mismatch occurred.
// If all indices matched the file size will be returned
template <class File>
void compare_files(File& original, File& copy, uint64_t& result)
{
    ASSERT_EQ(original.size(), copy.size()) << "File size mismatch";

    for (uint64_t offset = 0; offset < original.size(); ++offset)
    {
        result = offset;
        uint8_t org = *(original.data() + offset);
        uint8_t cpy = *(copy.data() + offset);
        if (org != cpy)
            return;
    }
    result = original.size();
    return;
}
}

TEST(test_file_segment_reassemble, ordered_segments)
{
    using segmenter_type = chunkie::file_segmenter<std::vector<uint8_t>>;
    using reassembler_type = chunkie::file_reassembler<std::vector<uint8_t>>;

    std::string filename_in = "testfile.tmp";
    std::vector<uint8_t> filedata_in(10000);

    std::mt19937 engine(0);
    std::uniform_int_distribution<uint32_t> randval(0, 255);

    for (uint8_t& val : filedata_in)
        val = randval(engine);

    std::vector<uint8_t> filedata_reassembled;

    auto segmenter =
        std::make_unique<segmenter_type>(filedata_in, filename_in, 532);

    std::unique_ptr<reassembler_type> reassembler;

    // allocate buffer for serialized segment
    std::vector<uint8_t> buffer;

    for (uint32_t sid = 0; sid < segmenter->segments(); ++sid)
    {
        // scope for generated segment
        {
            chunkie::file_segment segment = segmenter->read_segment(sid);

            buffer.resize(segment.size_serialized(), 0);
            segment.serialize(buffer);
        } // segment goes of out scope

        // scope for reconstructed segment
        {
            auto segment = chunkie::file_segment::from_buffer(buffer);
            // Use the first segment to construct the reassembler
            if (!reassembler)
            {
                // Prepare the file object using info in segment
                filedata_reassembled.resize(segment.file_size());
                // Construct reassembler
                reassembler = std::make_unique<reassembler_type>(
                    filedata_reassembled, segment.filename());
            }
            ASSERT_FALSE(reassembler->has_segment(segment.m_id));

            reassembler->write_segment(segment);
        }
    }

    EXPECT_EQ(segmenter->file_size(), reassembler->reassembled_bytes());

    uint64_t result = 0;
    compare_files(filedata_in, filedata_reassembled, result);
    EXPECT_EQ(segmenter->file_size(), result) << "File mismatch at offset "
                                              << result;
}

TEST(test_file_segment_reassemble, reversed_segments)
{
    using segmenter_type = chunkie::file_segmenter<std::vector<uint8_t>>;
    using reassembler_type = chunkie::file_reassembler<std::vector<uint8_t>>;

    std::string filename_in = "testfile.tmp";
    std::vector<uint8_t> filedata_in(10000);

    std::mt19937 engine(0);
    std::uniform_int_distribution<uint32_t> randval(0, 255);

    for (uint8_t& val : filedata_in)
        val = randval(engine);

    std::vector<uint8_t> filedata_reassembled;

    auto segmenter =
        std::make_unique<segmenter_type>(filedata_in, filename_in, 324);

    std::unique_ptr<reassembler_type> reassembler;

    std::deque<uint32_t> sids;
    for (uint32_t i = 0; i < segmenter->segments(); ++i)
        sids.push_front(i);

    // allocate buffer for serialized segment
    std::vector<uint8_t> buffer;

    for (uint32_t sid : sids)
    {
        // scope for generated segment
        {
            chunkie::file_segment segment = segmenter->read_segment(sid);

            buffer.resize(segment.size_serialized(), 0);
            segment.serialize(buffer.data(), buffer.size());
        } // segment goes of out scope

        // scope for reconstructed segment
        {
            auto segment = chunkie::file_segment::from_buffer(buffer);
            // Use the first segment to construct the reassembler
            if (!reassembler)
            {
                // Prepare the file object using info in segment
                filedata_reassembled.resize(segment.file_size());
                // Construct reassembler
                reassembler = std::make_unique<reassembler_type>(
                    filedata_reassembled, segment.filename());
            }
            ASSERT_FALSE(reassembler->has_segment(segment.m_id));

            reassembler->write_segment(segment);
        }
    }

    EXPECT_EQ(segmenter->file_size(), reassembler->reassembled_bytes());

    uint64_t result = 0;
    compare_files(filedata_in, filedata_reassembled, result);
    EXPECT_EQ(segmenter->file_size(), result) << "File mismatch at offset "
                                              << result;
}

TEST(test_file_segment_reassemble, multiple_segment_copies)
{
    using segmenter_type = chunkie::file_segmenter<std::vector<uint8_t>>;
    using reassembler_type = chunkie::file_reassembler<std::vector<uint8_t>>;

    std::string filename_in = "testfile.tmp";
    std::vector<uint8_t> filedata_in(10000);

    std::mt19937 engine(0);
    std::uniform_int_distribution<uint32_t> randval(0, 255);

    for (uint8_t& val : filedata_in)
        val = randval(engine);

    std::vector<uint8_t> filedata_reassembled;

    auto segmenter =
        std::make_unique<segmenter_type>(filedata_in, filename_in, 753);

    std::unique_ptr<reassembler_type> reassembler;

    std::deque<uint32_t> sids;
    for (uint32_t i = 0; i < segmenter->segments(); ++i)
    {
        sids.push_front(i);
        sids.push_front(i);
    }

    // allocate buffer for serialized segment
    std::vector<uint8_t> buffer;

    for (uint32_t sid : sids)
    {
        // scope for generated segment
        {
            chunkie::file_segment segment = segmenter->read_segment(sid);

            buffer.resize(segment.size_serialized(), 0);
            segment.serialize(buffer.data(), buffer.size());
        } // segment goes of out scope

        // scope for reconstructed segment
        {
            auto segment = chunkie::file_segment::from_buffer(buffer.data(),
                                                              buffer.size());
            // Use the first segment to construct the reassembler
            if (!reassembler)
            {
                // Prepare the file object using info in segment
                filedata_reassembled.resize(segment.file_size());
                // Construct reassembler
                reassembler = std::make_unique<reassembler_type>(
                    filedata_reassembled, segment.filename());
            }

            if (!reassembler->has_segment(segment))
                reassembler->write_segment(segment);
        }
    }

    EXPECT_EQ(segmenter->file_size(), reassembler->reassembled_bytes());

    uint64_t result = 0;
    compare_files(filedata_in, filedata_reassembled, result);
    EXPECT_EQ(segmenter->file_size(), result) << "File mismatch at offset "
                                              << result;
}

TEST(test_file_segment_reassemble, scrambled_segments)
{
    using segmenter_type = chunkie::file_segmenter<std::vector<uint8_t>>;
    using reassembler_type = chunkie::file_reassembler<std::vector<uint8_t>>;

    std::string filename_in = "testfile.tmp";
    std::vector<uint8_t> filedata_in(10000);

    std::mt19937 engine(0);
    std::uniform_int_distribution<uint32_t> randval(0, 255);

    for (uint8_t& val : filedata_in)
        val = randval(engine);

    std::vector<uint8_t> filedata_reassembled;

    auto segmenter =
        std::make_unique<segmenter_type>(filedata_in, filename_in, 192);

    std::unique_ptr<reassembler_type> reassembler;

    std::vector<uint32_t> sids;
    for (uint32_t i = 0; i < segmenter->segments(); ++i)
        sids.push_back(i);
    std::vector<uint32_t> sids_ordered = sids;
    std::random_shuffle(sids.begin(), sids.end());

    // allocate buffer for serialized segments
    std::vector<uint8_t> buffer;

    for (uint32_t sid : sids)
    {
        // scope for generated segment
        {
            chunkie::file_segment segment = segmenter->read_segment(sid);

            buffer.resize(segment.size_serialized(), 0);
            segment.serialize(buffer.data(), buffer.size());
        } // segment goes of out scope

        // scope for reconstructed segment
        {
            auto segment = chunkie::file_segment::from_buffer(buffer.data(),
                                                              buffer.size());
            // Use the first segment to construct the reassembler
            if (!reassembler)
            {
                // Prepare the file object using info in segment
                filedata_reassembled.resize(segment.file_size());
                // Construct reassembler
                reassembler = std::make_unique<reassembler_type>(
                    filedata_reassembled, segment.filename());
            }
            ASSERT_FALSE(reassembler->has_segment(segment.m_id));

            reassembler->write_segment(segment);
        }
    }

    EXPECT_EQ(segmenter->file_size(), reassembler->reassembled_bytes());

    uint64_t result = 0;
    compare_files(filedata_in, filedata_reassembled, result);
    EXPECT_EQ(segmenter->file_size(), result) << "File mismatch at offset "
                                              << result;
}

TEST(test_file_segment_reassemble, lost_segments_fail)
{
    using segmenter_type = chunkie::file_segmenter<std::vector<uint8_t>>;
    using reassembler_type = chunkie::file_reassembler<std::vector<uint8_t>>;

    std::string filename_in = "testfile.tmp";
    std::vector<uint8_t> filedata_in(10000);

    std::mt19937 engine(0);
    std::uniform_int_distribution<uint32_t> randval(0, 255);

    for (uint8_t& val : filedata_in)
        val = randval(engine);

    std::vector<uint8_t> filedata_reassembled;

    auto segmenter =
        std::make_unique<segmenter_type>(filedata_in, filename_in, 333);

    std::unique_ptr<reassembler_type> reassembler;

    std::vector<uint32_t> sids;
    for (uint32_t i = 0; i < segmenter->segments(); ++i)
        sids.push_back(i);
    std::random_shuffle(sids.begin(), sids.end());
    sids.resize(sids.size() - 1);

    // allocate buffer for serialized segment
    std::vector<uint8_t> buffer;

    for (uint32_t sid : sids)
    {
        // scope for generated segment
        {
            chunkie::file_segment segment = segmenter->read_segment(sid);

            buffer.resize(segment.size_serialized(), 0);
            segment.serialize(buffer.data(), buffer.size());
        } // segment goes of out scope

        // scope for reconstructed segment
        {
            auto segment = chunkie::file_segment::from_buffer(buffer.data(),
                                                              buffer.size());
            // Use the first segment to construct the reassembler
            if (!reassembler)
            {
                // Prepare the file object using info in segment
                filedata_reassembled.resize(segment.file_size());
                // Construct reassembler
                reassembler = std::make_unique<reassembler_type>(
                    filedata_reassembled, segment.filename());
            }
            ASSERT_FALSE(reassembler->has_segment(segment.m_id));

            reassembler->write_segment(segment);
        }
    }

    EXPECT_GT(segmenter->file_size(), reassembler->reassembled_bytes());

    uint64_t result = 0;
    compare_files(filedata_in, filedata_reassembled, result);
    EXPECT_NE(segmenter->file_size(), result)
        << "File match even when segment was dropped:";
}
