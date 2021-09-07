// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <gtest/gtest.h>

#include <chunkie/deserializer.hpp>
#include <chunkie/serializer.hpp>

#include <vector>

TEST(test_chunkie, basic)
{
    uint32_t max_packet_size = 1200;

    chunkie::serializer<uint32_t> serializer;
    chunkie::deserializer<uint32_t> deserializer;

    uint32_t i = 0;

    while (i < 10000)
    {

        std::vector<uint8_t> input;
        std::vector<uint8_t> output;

        input.resize(1 + (rand() % 20000), 'x');

        serializer.set_object(input.data(), input.size());

        while (!serializer.object_proccessed())
        {

            std::vector<uint8_t> packet(max_packet_size);

            if (packet.size() > serializer.max_write_buffer_size())
            {
                packet.resize(serializer.max_write_buffer_size());
            }

            serializer.write_buffer(packet.data(), packet.size());

            if (rand() % 2)
            {
                continue;
            }

            deserializer.set_buffer(packet.data(), packet.size());

            while (!deserializer.buffer_proccessed())
            {

                output.resize(deserializer.object_size());
                deserializer.write_to_object(output.data());
            }
        }

        ++i;
    }
}
