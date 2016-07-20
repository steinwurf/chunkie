// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <algorithm>
#include <vector>

#include <boost/crc.hpp>

namespace score
{
    /// Returns the sha1 checksum of the input buffer
    /// @param data the input buffer
    /// @param size the input buffer size
    /// @return the sha1 checksum of buffer
    inline uint32_t crc32_generator(const uint8_t* data, uint32_t size)
    {
        boost::crc_32_type result;
        result.process_bytes(data, size);
        return result.checksum();
    }
}
