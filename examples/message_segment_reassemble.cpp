// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <chunkie/message/message_segmenter.hpp>
#include <chunkie/message/message_reassembler.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;
    // Specify the semgent size we want:
    uint32_t segment_size = 845;

    // build the message segmenter
    // chunkie::message_segmenter<uint8_t> ms;
    chunkie::message_segmenter<uint32_t> ms;

    // Prepare the message reassembler
    // chunkie::message_reassembler<uint8_t> mr;
    chunkie::message_reassembler<uint32_t> mr;

    // Load 4 "random sized" messages into the segmenter:
    for (const auto& size : {1337, 208, 5681, 540}) // total 7766
    // for (const auto& size : {127, 64, 38, 24})
    {
        std::cout << "'Sending' message of size " << size << std::endl;
        ms.write_message(std::vector<uint8_t>(size));
    }

    // Pull out the segments and load them into the reassembler
    while (ms.segment_available(segment_size))
    {
        std::vector<uint8_t> seg = ms.get_segment(segment_size);

        // This is where the segment should be sent / saved / dealt with
        std::cout << "Handling segment of size " << seg.size() << std::endl;

        // load in the segment at the receiving end
        mr.read_segment(seg);
    }

    // make sure to empty the segmenter for any partial data (if any):
    if (ms.data_buffered() > 0)
    {
        std::vector<uint8_t> seg = ms.flush(segment_size);
        std::cout << "Handling flushed segment of size "
                  << seg.size() << std::endl;

        mr.read_segment(seg);
    }

    // Pull out the messages from the reassembler:
    while (mr.message_available())
    {
        std::vector<uint8_t> msg = mr.get_message();
        std::cout << "'Received' message of size " << msg.size() << std::endl;
    }

    return 0;
}
