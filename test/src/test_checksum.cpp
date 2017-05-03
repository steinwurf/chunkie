// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <gtest/gtest.h>

#include <vector>
#include <random>

#include <chunkie/checksum/checksum.hpp>

TEST(test_checksum, checksum)
{
    std::mt19937 engine(0);

    std::uniform_int_distribution<uint32_t> randval(0, 255);
    std::uniform_int_distribution<uint32_t> randsize(5, 5000);

    uint32_t iterations = 1000;

    for (uint32_t i = 0; i < iterations; ++i)
    {
        uint32_t size = randsize(engine);
        SCOPED_TRACE(::testing::Message()
                     << "Verifying checksum for buffer of size " << size);

        std::vector<uint8_t> buffer(size);
        for (auto& val : buffer)
            val = (uint8_t)randval(engine);

        chunkie::write_checksum(buffer);

        std::vector<uint8_t> buffer_copy = buffer;

        ASSERT_EQ(size + sizeof(uint32_t), buffer.size())
            << "Checksum should have been appended";

        bool match = chunkie::read_checksum(buffer);

        ASSERT_TRUE(match) << "Checksum have to match";

        ASSERT_EQ(size, buffer.size())
            << "Matching checksum must be stripped from buffer";

        // Mess with the buffer (the copy) to ruin the checksum
        // remove the first byte
        buffer_copy.erase(buffer_copy.begin());

        match = chunkie::read_checksum(buffer_copy);

        ASSERT_FALSE(match) << "Checksum should not be correct";
        ASSERT_NE(size, buffer_copy.size())
            << "read_checksum modified buffer despite checksum mismatch";
    }
}
