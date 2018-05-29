// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <gtest/gtest.h>

#include <chunkie/serializer.hpp>

#include <vector>

TEST(test_serializer, basic)
{
    using serializer_type = chunkie::serializer<uint32_t>;
    serializer_type serializer;

    EXPECT_EQ(4U, serializer_type::header_size);
    EXPECT_EQ(2147483647U, serializer_type::max_object_size);
    EXPECT_TRUE(serializer.object_proccessed());

    std::vector<uint8_t> object = {1,2,3,4};
    std::vector<uint8_t> expected_buffer = {0b10000000,0,0,4,1,2,3,4};

    {
        std::vector<uint8_t> buffer(20);
        serializer.set_object(object.data(), object.size());
        auto bytes = serializer.write_to_buffer(buffer.data(), buffer.size());
        buffer.resize(bytes);

        EXPECT_EQ(bytes, object.size() + serializer_type::header_size);
        EXPECT_TRUE(serializer.object_proccessed());
        EXPECT_EQ(expected_buffer, buffer);
    }

    {
        std::vector<uint8_t> buffer(20);
        serializer.set_object(object.data(), object.size());
        auto bytes = serializer.write_to_buffer(buffer.data(), buffer.size());
        buffer.resize(bytes);

        EXPECT_EQ(bytes, object.size() + serializer_type::header_size);
        EXPECT_TRUE(serializer.object_proccessed());
        EXPECT_EQ(expected_buffer, buffer);
    }
}

// write an object to multiple buffers
TEST(test_serializer, write_partial_objects)
{
    using serializer_type = chunkie::serializer<uint32_t>;
    serializer_type serializer;

    std::vector<std::vector<uint8_t>> objects =
        {
            { 0,1,2,3 },
            { 4,5,6,7,8,9 },
            { 10,11,12 },
            { 13,14,15,16,17,18,19,20 },
            { 21 },
            { 22,23,24 }
        };

    std::vector<std::vector<uint8_t>> buffers;

    // add a buffer of size 5,6,7, ... , 14 bytes
    for (uint32_t bytes = 5; bytes < 15; bytes++)
    {
        buffers.emplace_back(bytes);
    }

    std::vector<std::vector<uint8_t>> expected_buffers =
        {
            { 0b10000000,0,0,4, 0 },
            { 0b00000000,0,0,3, 1,2 },
            { 0b00000000,0,0,1, 3,0,0 },

            { 0b10000000,0,0,6, 4,5,6,7 },
            { 0b00000000,0,0,2, 8,9,0,0,0 },

            { 0b10000000,0,0,3, 10,11,12,0,0,0 },

            { 0b10000000,0,0,8, 13,14,15,16,17,18,19 },
            { 0b00000000,0,0,1, 20,0,0,0,0,0,0,0 },

            { 0b10000000,0,0,1, 21,0,0,0,0,0,0,0,0, },

            { 0b10000000,0,0,3, 22,23,24,0,0,0,0,0,0,0 },

        };

    auto buffer = buffers.begin();

    for (const auto& object : objects)
    {
        serializer.set_object(object.data(), object.size());

        while (!serializer.object_proccessed())
        {
            serializer.write_to_buffer(buffer->data(), buffer->size());
            buffer++;
        }
    }

    EXPECT_EQ(expected_buffers, buffers);
}

TEST(test_serializer, max_object_size)
{
    EXPECT_EQ(127U, chunkie::serializer<uint8_t>::max_object_size);
    EXPECT_EQ(32767U, chunkie::serializer<uint16_t>::max_object_size);
    EXPECT_EQ(2147483647U, chunkie::serializer<uint32_t>::max_object_size);
    EXPECT_EQ(9223372036854775807U, chunkie::serializer<uint64_t>::max_object_size);
}
