// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <chunkie/file/file_segmenter.hpp>
#include <chunkie/file/file_reassembler.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    // Check cmdline argument [input file] [output path]
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " [path/to/input/file] " 
                  << "[path/to/output/directory]"<< std::endl;
        return 1;
    }
    if(!boost::filesystem::exists(argv[1]))
    {
        std::cout << "Path " << argv[1] << " does not exist." << std::endl;
        return 2;
    }
    if(!boost::filesystem::is_regular_file(argv[1]))
    {
        std::cout << "Path " << argv[1] << " is not a file. " << std::endl;
        return 3;
    }

    if(!boost::filesystem::exists(argv[2]))
    {
        std::cout << "Path " << argv[2] << " does not exist." << std::endl;
        return 4;
    }

    if(!boost::filesystem::is_directory(argv[2]))
    {
        std::cout << "Path " << argv[2] << " is not a directory." << std::endl;
        return 5;
    }

    // Paths OK

    // Open the file with the file segmenter
    // We load in smaller chunks of up to 5MB and "send" these
    chunkie::file_segmenter fs(argv[1], (uint32_t) 5e6);

    // Prepare the file reassembler
    chunkie::file_reassembler fr(argv[2]);

    while (!fs.end_of_file())
    {
        // Load a chunk from the file
        std::vector<uint8_t> buffer = fs.load();

        // Send the buffer over the network or similar here
        std::cout << "Processing file chunk of size "
                  << buffer.size() << std::endl;

        // Save the file chunk into the output file
        fr.save(buffer);
    }

    if(!fr.end_of_file())
    {
        std::cerr << "An error occurred, file " << fr.name()
                  << " not reassembled correctly. Offset: " << fr.offset() 
                  << std::endl;
        return 6;
    }

    std::cout << "Saved file " << fr.name() << "." << std::endl;

    return 0;
}