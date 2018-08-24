// Copyright (c) 2018 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.


#include <chunkie/serializer.hpp>
#include <chunkie/deserializer.hpp>

#include <iostream>

// In this example objects are serialized to buffers of a maximum size with
// only one object present in each buffer. This would typically be relevant when
// objects are much larger than the buffers to which they are serialized and
// latency is a very high priority.
int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    chunkie::serializer<uint32_t> serializer;
    chunkie::deserializer<uint32_t> deserializer;

    uint32_t max_buffer_size = 1000;

    // some objects to be sent
    std::vector<std::vector<uint8_t>> objects;
    for (auto size : {1337, 28, 2681, 540, 12, 24, 48, 36, 212, 1024, 257, 42})
    {
        objects.emplace_back(size, rand());
    }

    std::vector<std::vector<uint8_t>> buffers;

    std::vector<uint8_t> buffer;

    // send all objects
    for (const auto& object : objects)
    {
        serializer.set_object(object.data(), object.size());


        // until the object have been processed, write buffers
        while (!serializer.object_proccessed())
        {
            // the size of the current buffer before appending aditional data
            auto old_size = buffer.size();

            auto buffer_size = std::min<uint32_t>(
                max_buffer_size, old_size + serializer.max_write_buffer_size());

            buffer.resize(buffer_size);

            serializer.write_buffer(buffer.data() + old_size,
                                    buffer.size() - old_size);

            std::cout << "Writing to buffer number " << buffers.size() <<
                      " current size is " << buffer.size() <<
                      ", from object of size " << object.size() << std::endl;

            if (buffer.size() == max_buffer_size)
            {
                buffers.emplace_back(buffer.begin(), buffer.end());
                buffer.clear();
            }
        }
    }

    // add the last partial completed buffer
    if (buffer.size() > 0)
    {
        buffer.resize(max_buffer_size, 0U);
        buffers.emplace_back(buffer.begin(), buffer.end());
    }


    std::cout << objects.size() << " objects serialized to " <<
              buffers.size() << " buffers." << std::endl << std::endl;

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
                std::cout << "Deserialized object of size " <<
                          object.size() << " " <<
                          (equals ? "correctly" : "incorrectly") << std::endl;

                objects_restored++;
            }
        }
    }

    std::cout << objects_restored << " Objects deserialized!" << std::endl;

    return 0;
}
