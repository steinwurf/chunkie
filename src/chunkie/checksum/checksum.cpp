// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include "checksum.hpp"

namespace chunkie
{
    void write_checksum(std::vector<uint8_t>& message)
    {
        const uint32_t length = message.size();

        // std::vector<uint8_t> checksum_message(message.size() + 4);
        uint32_t checksum = detail::crc32(message.data(), length);

        message.resize(length+sizeof(checksum));

        endian::big_endian::put32(checksum, message.data()+length);
    }

    bool read_checksum(std::vector<uint8_t>& message)
    {
        // Generate checksum from buffer
        uint32_t generated = 
            detail::crc32(message.data(), message.size() - sizeof(uint32_t));

        // Fetch embeeded checksum
        const uint32_t pos = message.size() - sizeof(uint32_t);
        uint32_t embedded = endian::big_endian::get32(&message.at(pos));

        // Compare checksum
        if (generated == embedded)
        {
            message.resize(pos);
            return true;
        }
        return false;
    }
}
