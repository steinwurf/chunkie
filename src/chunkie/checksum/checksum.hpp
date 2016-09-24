// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <vector>
#include <endian/big_endian.hpp>

#include <boost/crc.hpp>

namespace chunkie
{
namespace detail
{
/// Returns the sha1 checksum of the input buffer
/// @param data the input buffer
/// @param size the input buffer size
/// @return the sha1 checksum of buffer
inline uint32_t crc32(const uint8_t* data, uint32_t size)
{
    boost::crc_32_type result;
    result.process_bytes(data, size);
    return result.checksum();
}
}

/// Appends a checksum to the back of a buffer
/// @param the buffer to append checksum of
/// Example:
/// @code
///  0                   1                   2                   3
///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                            data                               |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                            ....                               |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                            ....                               |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                            ....                               |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                          checksum                             |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///
/// @endcode
void write_checksum(std::vector<uint8_t>& message);

/// Read the checksum (prepended) from a buffer.
/// If it matches, the checksum is stripped from the buffer
/// Otherwise, the buffer is untouched
/// @param message the buffer with a checksum appended
/// @return true if checksum is correct
bool read_checksum(std::vector<uint8_t>& message);
}
