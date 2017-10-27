// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <chunkie/file/file_reassembler.hpp>
#include <chunkie/file/file_segmenter.hpp>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include <iostream>


int main(int argc, char* argv[])
{
    // Check cmdline argument [input file] [output path]
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " [path/to/input/file] "
                  << "[path/to/output/directory]" << std::endl;
        return 1;
    }
    if (!boost::filesystem::exists(argv[1]))
    {
        std::cout << "Path " << argv[1] << " does not exist." << std::endl;
        return 2;
    }
    if (!boost::filesystem::is_regular_file(argv[1]))
    {
        std::cout << "Path " << argv[1] << " is not a file. " << std::endl;
        return 3;
    }

    if (!boost::filesystem::exists(argv[2]))
    {
        std::cout << "Path " << argv[2] << " does not exist." << std::endl;
        return 4;
    }

    if (!boost::filesystem::is_directory(argv[2]))
    {
        std::cout << "Path " << argv[2] << " is not a directory." << std::endl;
        return 5;
    }

    boost::filesystem::path path_in(argv[1]);
    boost::filesystem::path path_out(argv[2]);

    // Paths OK

    // Open the input file for reading
    boost::iostreams::mapped_file_source input_file(path_in);

    using segmenter_type =
        chunkie::file_segmenter<boost::iostreams::mapped_file_source>;
    using reassembler_type =
        chunkie::file_reassembler<boost::iostreams::mapped_file_sink>;

    // Construct the segmenter
    auto segmenter = std::make_unique<segmenter_type>(
        input_file, path_in.filename().string(), 5'000'000);

    // Prepare the reassembler and the output file
    std::unique_ptr<reassembler_type> reassembler;
    boost::iostreams::mapped_file_sink output_file;

    // allocate buffer for serialized segments
    std::vector<uint8_t> buffer;

    for (uint32_t sid = 0; sid < segmenter->segments(); ++sid)
    {
        // scope for generated segment
        {
            chunkie::file_segment segment = segmenter->read_segment(sid);

            buffer.resize(segment.size_serialized(), 0);
            segment.serialize(buffer.data(), buffer.size());
        } // segment goes out of scope

        // scope for reconstructed segment
        {
            auto segment = chunkie::file_segment::from_buffer(buffer);
            // Use the first segment to construct the reassembler
            if (!reassembler)
            {
                // Prepare the file object using info in segment
                boost::iostreams::mapped_file_params params{};
                params.path = (path_out / segment.filename()).string();
                params.new_file_size = segment.file_size();
                output_file.open(params);

                // Construct reassembler
                reassembler = std::make_unique<reassembler_type>(
                    output_file, segment.filename());
            }

            if (!reassembler->has_segment(segment))
                reassembler->write_segment(segment);
        }
    }

    std::cout << "Segmented file of " << segmenter->file_size() << "bytes."
              << std::endl;
    std::cout << "Reassembled " << reassembler->reassembled_bytes()
              << " bytes into file " << argv[2] << "/"
              << reassembler->filename() << std::endl;
    return 0;
}
