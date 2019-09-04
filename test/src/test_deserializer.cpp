// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <gtest/gtest.h>

#include <chunkie/deserializer.hpp>

#include <vector>

TEST(test_deserializer, basic)
{
    using deserializer_type = chunkie::deserializer<uint32_t>;
    deserializer_type deserializer;

    std::vector<uint8_t> buffer = {0x80,0,0,4,0,1,2,3};
    std::vector<uint8_t> expected_object = {0,1,2,3};

    {
        deserializer.set_buffer(buffer.data(), buffer.size());
        EXPECT_FALSE(deserializer.buffer_proccessed());
        EXPECT_FALSE(deserializer.object_completed());
        EXPECT_EQ(4U, deserializer.object_size());

        std::vector<uint8_t> message(4U);
        deserializer.write_to_object(message.data());

        EXPECT_TRUE(deserializer.buffer_proccessed());
        EXPECT_TRUE(deserializer.object_completed());
        EXPECT_EQ(expected_object, message);
    }

    {
        deserializer.set_buffer(buffer.data(), buffer.size());
        EXPECT_FALSE(deserializer.buffer_proccessed());
        EXPECT_EQ(4U, deserializer.object_size());

        std::vector<uint8_t> message(4U);
        deserializer.write_to_object(message.data());

        EXPECT_TRUE(deserializer.buffer_proccessed());
        EXPECT_TRUE(deserializer.object_completed());
        EXPECT_EQ(expected_object, message);
    }
}

// Many objects in a single buffer
TEST(test_deserializer, many_objects_in_buffer)
{
    using deserializer_type = chunkie::deserializer<uint32_t>;
    deserializer_type deserializer;

    std::vector<uint8_t> buffer =
        {
            0b10000000,0,0,4, 0,1,2,3,
            0b10000000,0,0,6, 4,5,6,7,8,9,
            0b10000000,0,0,3, 10,11,12,
            0b10000000,0,0,8, 13,14,15,16,17,18,19,20,
            0b10000000,0,0,1, 21,
            0b10000000,0,0,3, 22,23,24
        };

    std::vector<std::vector<uint8_t>> expected_object =
        {
            { 0,1,2,3 },
            { 4,5,6,7,8,9 },
            { 10,11,12 },
            { 13,14,15,16,17,18,19,20 },
            { 21 },
            { 22,23,24 }
        };

    std::vector<std::vector<uint8_t>> objects;
    deserializer.set_buffer(buffer.data(), buffer.size());
    EXPECT_FALSE(deserializer.buffer_proccessed());

    while (!deserializer.buffer_proccessed())
    {
        std::vector<uint8_t> object(deserializer.object_size());
        deserializer.write_to_object(object.data());
        EXPECT_TRUE(deserializer.object_completed());
        objects.push_back(object);
    }

    EXPECT_TRUE(deserializer.buffer_proccessed());
    EXPECT_EQ(expected_object, objects);
}

// object spanning 2 and 3 buffers
TEST(test_deserializer, buffer_overlap_objects)
{
    using deserializer_type = chunkie::deserializer<uint32_t>;
    deserializer_type deserializer;

    std::vector<std::vector<uint8_t>> buffers =
        {
            {
                0b10000000,0,0,4, 0,1
        },
            {
                0b00000000,0,0,2, 2,3,
                0b10000000,0,0,17, 4,5,6,7,8,9
        },
            {
                0b00000000,0,0,11, 10,11,12
        },
            {
                0b00000000,0,0,8, 13,14,15,16,17,18,19,20,
                0b10000000,0,0,1, 21
        },
            {
                0b10000000,0,0,3, 22,23,24
        }
        };

    std::vector<std::vector<uint8_t>> expected_object =
        {
            { 0,1, 2,3 },
            { 4,5,6,7,8,9, 10,11,12, 13,14,15,16,17,18,19,20 },
            { 21 },
            { 22,23,24 }
        };

    std::vector<std::vector<uint8_t>> objects;
    std::vector<uint8_t> object;

    for (const auto& buffer : buffers)
    {
        deserializer.set_buffer(buffer.data(), buffer.size());
        EXPECT_FALSE(deserializer.buffer_proccessed());

        while (!deserializer.buffer_proccessed())
        {
            object.resize(deserializer.object_size());

            deserializer.write_to_object(object.data());
            if (deserializer.object_completed())
            {
                objects.push_back(object);
            }
        }
    }

    EXPECT_TRUE(deserializer.buffer_proccessed());
    EXPECT_EQ(expected_object, objects);
}

// receiving zero padded buffers
TEST(test_deserializer, zero_padded_buffers)
{
    using deserializer_type = chunkie::deserializer<uint32_t>;
    deserializer_type deserializer;

    std::vector<std::vector<uint8_t>> buffers =
        {
            {
                0b10000000,0,0,4, 0,1
        },
            {
                0b00000000,0,0,2, 2,3,
                0b10000000,0,0,17, 4,5,6,7,8,9,
        },
            {
                0b00000000,0,0,11, 10,11,12
        },
            {
                0b00000000,0,0,8, 13,14,15,16,17,18,19,20,
                0b10000000,0,0,1, 21, 0,0,0,0,0,0
        },
            {
                0b10000000,0,0,3, 22,23,24, 0,0,0,0,0,0,0,0,0
        }
        };

    std::vector<std::vector<uint8_t>> expected_object =
        {
            { 0,1, 2,3 },
            { 4,5,6,7,8,9, 10,11,12, 13,14,15,16,17,18,19,20 },
            { 21 },
            { 22,23,24 }
        };

    std::vector<std::vector<uint8_t>> objects;
    std::vector<uint8_t> object;

    for (const auto& buffer : buffers)
    {
        deserializer.set_buffer(buffer.data(), buffer.size());
        EXPECT_FALSE(deserializer.buffer_proccessed());

        while (!deserializer.buffer_proccessed() && deserializer.object_size() > 0)
        {
            object.resize(deserializer.object_size());

            deserializer.write_to_object(object.data());
            if (deserializer.object_completed())
            {
                objects.push_back(object);
                object.clear();
            }
        }
    }

    EXPECT_TRUE(deserializer.buffer_proccessed());
    EXPECT_EQ(expected_object, objects);
}

// throw away some buffers to handle loss
TEST(test_deserializer, lost_buffer)
{
    using deserializer_type = chunkie::deserializer<uint32_t>;
    deserializer_type deserializer;

    std::vector<std::vector<uint8_t>> buffers =
        {
            {
                0b10000000,0,0,4, 0,1
        },
            {
                // 0b00000000,0,0,2, 2,3,
                0b10000000,0,0,6, 4,5,6,7,8,9,
                // 0b10000000,0,0,3, 10,11,12,
                0b10000000,0,0,9, 13,14,15,16,17,18,19,20,
        },
            {
                0b00000000,0,0,1, 21,
                0b10000000,0,0,3, 22,23,24
        },

            // {
            //     0b10000000,0,0,4, 0,1
            // },
            {
                0b00000000,0,0,2, 2,3,
                // 0b10000000,0,0,6, 4,5,6,7,8,9,
                0b10000000,0,0,3, 10,11,12,
                0b10000000,0,0,9, 13,14,15,16,17,18,19,20,
        },
            {
                // 0b00000000,0,0,1, 21,
                0b10000000,0,0,3, 22,23,24
        },

            {
                0b10000000,0,0,4, 0,1
        },
            {
                0b00000000,0,0,2, 2,3,
                0b10000000,0,0,17, 4,5,6,7,8,9
        },
            // {
            // 0b00000000,0,0,11, 10,11,12
            // },
            {
                0b00000000,0,0,8, 13,14,15,16,17,18,19,20,
                0b10000000,0,0,1, 21
        },
            {
                0b10000000,0,0,3, 22,23,24
        }
        };

    std::vector<std::vector<uint8_t>> expected_object =
        {
            // {0,1, 2,3},
            {4,5,6,7,8,9},
            // {10,11,12},
            {13,14,15,16,17,18,19,20, 21},
            {22,23,24},

            // {0,1, 2,3},
            // {4,5,6,7,8,9},
            {10,11,12},
            // {13,14,15,16,17,18,19,20, 21},
            {22,23,24},

            { 0,1, 2,3 },
            // { 4,5,6,7,8,9, 10,11,12, 13,14,15,16,17,18,19,20 },
            { 21 },
            { 22,23,24 }
        };

    std::vector<std::vector<uint8_t>> objects;
    std::vector<uint8_t> object;

    for (const auto& buffer : buffers)
    {
        deserializer.set_buffer(buffer.data(), buffer.size());
        EXPECT_FALSE(deserializer.buffer_proccessed());

        while (!deserializer.buffer_proccessed())
        {
            object.resize(deserializer.object_size());

            deserializer.write_to_object(object.data());
            if (deserializer.object_completed())
            {
                objects.push_back(object);
            }
        }
    }

    EXPECT_TRUE(deserializer.buffer_proccessed());
    EXPECT_EQ(expected_object, objects);
}

// throw away some buffers to handle loss with padding in an error prone place
TEST(test_deserializer, lost_buffers_and_zero_padding)
{
    using deserializer_type = chunkie::deserializer<uint32_t>;
    deserializer_type deserializer;

    std::vector<std::vector<uint8_t>> buffers =
        {
            { 0b10000000,0,0,4, 0,1 },
            // { 0b00000000,0,0,2, 2,3 },
            // { 0b10000000,0,0,6, 4,5,6 },
            { 0b00000000,0,0,3, 6,7,8, 0,0,0 }, // padded buffer
            { 0b10000000,0,0,3, 9,10,11},
            { 0b10000000,0,0,6, 12,13,14},
            // { 0b00000000,0,0,6, 15,16,17},
            // { 0b10000000,0,0,5, 18,19,20},
            { 0b00000000,0,0,2, 21,22, 0,0,0,0,0,0,0}
        };

    std::vector<bool> processed_expectations =
        {
            false, true, false, false, true
        };

    std::vector<std::vector<uint8_t>> expected_object = { { 9,10,11 } };

    std::vector<std::vector<uint8_t>> objects;
    std::vector<uint8_t> object;

    for (uint32_t i = 0; i < buffers.size(); i++)
    {
        const auto& buffer = buffers.at(i);
        bool processed_expectation = processed_expectations.at(i);
        deserializer.set_buffer(buffer.data(), buffer.size());

        EXPECT_EQ(processed_expectation, deserializer.buffer_proccessed());

        while (!deserializer.buffer_proccessed())
        {
            object.resize(deserializer.object_size());

            deserializer.write_to_object(object.data());
            if (deserializer.object_completed())
            {
                objects.push_back(object);
            }
        }
    }

    EXPECT_TRUE(deserializer.buffer_proccessed());
    EXPECT_EQ(expected_object, objects);
}
