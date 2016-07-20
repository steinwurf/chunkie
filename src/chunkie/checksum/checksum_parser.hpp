// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <vector>
#include <functional>

#include <endian/big_endian.hpp>

#include "crc32_generator.hpp"

namespace score
{
    template<typename Super>
    class checksum_parser : public Super
    {

    public:

        using typename Super::data_ready_callback;

        /// Set the data ready callback that should be called when a full
        /// message is recovered. This overrides Super::set_data_ready_callback.
        /// The Super will call the parse_data function of the message_parser
        /// when a symbol is decoded.
        /// @param cb the callback function
        void set_data_ready_callback(const data_ready_callback& cb)
        {
            m_data_ready_callback = cb;

            Super::set_data_ready_callback(
                std::bind(&checksum_parser::verify_checksum,
                          this, std::placeholders::_1));
        }

        // Read a complete atomic message from a bitstream.
        // It is expected that the data arrives in order
        void verify_checksum(const std::vector<uint8_t>& message)
        {
            std::vector<uint8_t> checked_message(message.begin() + 4,
                                                 message.end());

            uint32_t generated = crc32_generator(checked_message.data(),
                                                 checked_message.size());
            uint32_t embedded = endian::big_endian::get32(message.data());

            if (generated == embedded)
            {
                if (m_data_ready_callback)
                    m_data_ready_callback(checked_message);
            }
            else
            {
                m_checksum_errors++;
            }
        }

        /// Get the number of times this parser has experienced checksum errors
        /// return the uint32_t checksum
        uint32_t checksum_errors() const
        {
            return m_checksum_errors;
        }

    private:

        std::function<void(std::vector<uint8_t>&)> m_data_ready_callback;

        uint32_t m_checksum_errors = 0;

    };
}
