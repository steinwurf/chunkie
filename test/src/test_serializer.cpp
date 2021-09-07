// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <gtest/gtest.h>

#include <chunkie/serializer.hpp>

#include <algorithm>
#include <vector>

TEST(test_serializer, basic)
{
    using serializer_type = chunkie::serializer<uint32_t>;
    serializer_type serializer;

    EXPECT_EQ(4U, serializer_type::header_size);
    EXPECT_EQ(2147483647U, serializer_type::max_object_size);
    EXPECT_TRUE(serializer.object_proccessed());

    std::vector<uint8_t> object = {1, 2, 3, 4};
    std::vector<uint8_t> expected_buffer = {0b10000000, 0, 0, 4, 1, 2, 3, 4};

    {
        serializer.set_object(object.data(), object.size());
        std::vector<uint8_t> buffer(serializer.max_write_buffer_size());
        serializer.write_buffer(buffer.data(), buffer.size());

        EXPECT_EQ(object.size() + serializer_type::header_size, buffer.size());
        EXPECT_TRUE(serializer.object_proccessed());
        EXPECT_EQ(expected_buffer, buffer);
    }

    {
        serializer.set_object(object.data(), object.size());
        std::vector<uint8_t> buffer(serializer.max_write_buffer_size());
        serializer.write_buffer(buffer.data(), buffer.size());

        EXPECT_EQ(object.size() + serializer_type::header_size, buffer.size());
        EXPECT_TRUE(serializer.object_proccessed());
        EXPECT_EQ(expected_buffer, buffer);
    }
}

// write an object to multiple buffers
TEST(test_serializer, write_partial_objects)
{
    using serializer_type = chunkie::serializer<uint32_t>;
    serializer_type serializer;

    std::vector<std::vector<uint8_t>> objects = {
        {0, 1, 2, 3}, {4, 5, 6, 7, 8, 9},
        {10, 11, 12}, {13, 14, 15, 16, 17, 18, 19, 20},
        {21},         {22, 23, 24}};

    std::vector<std::vector<uint8_t>> expected_buffers = {
        {0b10000000, 0, 0, 4, 0},
        {0b00000000, 0, 0, 3, 1, 2},
        {0b00000000, 0, 0, 1, 3, 0, 0},

        {0b10000000, 0, 0, 6, 4, 5, 6, 7},
        {0b00000000, 0, 0, 2, 8, 9, 0, 0, 0},

        {0b10000000, 0, 0, 3, 10, 11, 12, 0, 0, 0},

        {0b10000000, 0, 0, 8, 13, 14, 15, 16, 17, 18, 19},
        {0b00000000, 0, 0, 1, 20, 0, 0, 0, 0, 0, 0, 0},

        {
            0b10000000,
            0,
            0,
            1,
            21,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        },

        {0b10000000, 0, 0, 3, 22, 23, 24, 0, 0, 0, 0, 0, 0, 0},

    };

    std::vector<std::vector<uint8_t>> buffers;

    // We write to buffer of growing size 5,6,7,8, ... bytes
    uint32_t buffer_size = 5;

    for (const auto& object : objects)
    {
        serializer.set_object(object.data(), object.size());

        while (!serializer.object_proccessed())
        {
            std::vector<uint8_t> buffer(buffer_size, 0U);
            buffer_size++;

            auto bytes = std::min<uint32_t>(buffer.size(),
                                            serializer.max_write_buffer_size());

            serializer.write_buffer(buffer.data(), bytes);

            buffers.push_back(buffer);
        }
    }

    EXPECT_EQ(expected_buffers, buffers);
}

TEST(test_serializer, max_object_size)
{
    EXPECT_EQ(127U, chunkie::serializer<uint8_t>::max_object_size);
    EXPECT_EQ(32767U, chunkie::serializer<uint16_t>::max_object_size);
    EXPECT_EQ(2147483647U, chunkie::serializer<uint32_t>::max_object_size);
    EXPECT_EQ(9223372036854775807U,
              chunkie::serializer<uint64_t>::max_object_size);
}
