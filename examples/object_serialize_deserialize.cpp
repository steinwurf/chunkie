// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.


#include <chunkie/serializer.hpp>
#include <chunkie/deserializer.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    chunkie::serializer<uint32_t> serializer;
    chunkie::deserializer<uint32_t> deserializer;

    // some objects to be sent
    std::vector<std::vector<uint8_t>> objects;
    for (auto size : {1337, 208, 5681, 540})
    {
        objects.emplace_back(size, rand());
    }

    std::vector<std::vector<uint8_t>> buffers;

    // send all objects
    for (const auto& object : objects)
    {
        serializer.set_object(object.data(), object.size());
        std::cout << "'Outputting' buffers from object of size: " <<
                  object.size() << std::endl;

        // The buffer size does not have to be the same for all buffers
        std::vector<uint8_t> buffer(100);

        // until the object have been processed, write buffers
        while (!serializer.object_proccessed())
        {

            auto bytes_written = serializer.write_to_buffer(
                buffer.data(), buffer.size());

            // output the new buffer
            buffers.emplace_back(buffer.data(), buffer.data() + bytes_written);
        }
    }

    std::vector<uint8_t> object;
    uint32_t objects_restored = 0;

    // consume all the buffers
    for (auto& buffer : buffers)
    {
        deserializer.set_buffer(buffer.data(), buffer.size());

        // keep writing to a object until the buffer have been consumed
        while (!deserializer.buffer_proccessed())
        {
            // resize the buffer to match the current object
            object.resize(deserializer.object_size());

            // write to the object
            deserializer.write_to_object(object.data());

            // if object completed do something with it
            if (deserializer.object_completed())
            {
                bool equals = object == objects[objects_restored];
                std::cout << "'Reassembled' object of size " <<
                          object.size() << " " <<
                          (equals ? "correctly" : "incorrectly") << std::endl;

                objects_restored++;
            }
        }
    }

    std::cout << '\n' << objects_restored << " Objects restored!" << '\n';

    return 0;
}
