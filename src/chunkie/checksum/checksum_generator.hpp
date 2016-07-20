// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <vector>
#include <endian/big_endian.hpp>

#include "crc32_generator.hpp"

namespace score
{
    /// Example:
    /// @code
    ///  0                   1                   2                   3
    ///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                          checksum                             |
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                            data                               |
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                            ....                               |
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                            ....                               |
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                            ....                               |
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// The remaing space in the symbol after msg 3 is zero padded as it cannot
    /// fit another header. A new message will start in the following symbol.
    ///
    /// @endcode

    template<typename Super>
    class checksum_generator : public Super
    {
    public:

        /// Writes a complete atomic message to the bitstream and prepends a
        /// sha1 checksum to the back? of the buffer
        /// @param message the message to write to the bitstream
        /// @param cb the callback to call after data have been written
        void write_data(const std::vector<uint8_t>& message,
                                     std::function<void()> cb)
        {
            std::vector<uint8_t> checksum_message(message.size() + 4);
            uint32_t checksum = crc32_generator(message.data(), message.size());
            endian::big_endian::put32(checksum, checksum_message.data());
            std::copy(message.begin(), message.end(),
                      checksum_message.begin()+4);
            Super::write_data(checksum_message, cb);
        }

    private:

    };
}
