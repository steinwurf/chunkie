// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <iostream>
#include <fstream>
#include <ostream>

#include <chunkie/message/message_segmenter.hpp>

int main(int argc, char* argv[])
{
    if (argc != 2 || std::string(argv[1]) == "--help")
    {
        std::cout << "Usage: " << argv[0] << " OUTPUT_DIR" << std::endl;
        return 0;
    }

    auto filename = std::string(argv[1]);
    std::ofstream output_file(filename, std::ios::binary);

    chunkie::message_segmenter<uint8_t> ms;
    for (const auto& size : {127, 64, 38})
    {
        std::cout << "Creating message of size " << size << std::endl;
        ms.write_message(std::vector<uint8_t>(size, size));
    }

    const uint32_t segment_size = 50;
    uint32_t number_of_segments = 0;
    // Pull out the segments and save them in a file
    while (ms.segment_available(segment_size))
    {
        std::vector<uint8_t> segment = ms.get_segment(segment_size);
        output_file.write((char*)segment.data(), segment.size());
        number_of_segments++;
        std::cout << "Handled segment of size " << segment.size() << std::endl;
    }
    if (ms.data_buffered() > 0)
    {
        std::vector<uint8_t> segment = ms.flush(segment_size);
        output_file.write((char*)segment.data(), segment.size());
        number_of_segments++;
        std::cout << "Handled flushed segment of size "
                  << segment.size() << std::endl;
    }

    output_file.close();

    std::cout << "Created " << number_of_segments << " segments" << std::endl;
    std::cout << "Dump file written to " << filename << std::endl;
    return 0;
}
