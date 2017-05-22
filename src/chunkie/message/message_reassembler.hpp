// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <algorithm>
#include <array>
#include <deque>
#include <vector>

#include <endian/big_endian.hpp>
#include <endian/stream_reader.hpp>
#include <bitter/reader.hpp>

namespace chunkie
{
template <typename HeaderType>
class message_reassembler
{
public:

    using header_type = HeaderType;

    // The header consists of a size and a start bit
    using header_reader =
        bitter::reader<header_type, (sizeof(header_type) * 8) - 1, 1>;

private:

    class message
    {
    public:

        /// Resets the message and sets the size to the given size.
        void reset(header_type size)
        {
            m_offset = 0;
            m_data.resize(size);
        }

        /// Inserts data from an endian_stream reader.
        void copy_from_reader(
            endian::stream_reader<endian::big_endian>& source, uint32_t bytes)
        {
            assert(m_offset + bytes <= m_data.size());
            source.read(m_data.data() + m_offset, bytes);
            m_offset += bytes;
        }

        /// Extracts the internal data buffer and replaces it with a new one.
        std::vector<uint8_t> extract_data()
        {
            auto data = std::move(m_data);
            m_data = std::vector<uint8_t>(0);
            m_offset = 0;
            return data;
        }

        /// Returns true if the message is complete
        bool is_complete() const
        {
            return m_data.size() != 0 && m_offset == m_data.size();
        }

        /// Returns the remaining number of bytes before the message is complete
        header_type remaining_size() const
        {
            return m_data.size() - m_offset;
        }

    private:

        header_type m_offset = 0;
        std::vector<uint8_t> m_data;
    };

public:

    // Read a segment. The buffer MUST start with a segment start,
    // but does not need to be a full segment. If a partial segment is
    // used, the remaining segment MUST be discarded if available.
    // Segments must be read in-order,
    void read_segment(const std::vector<uint8_t>& data_buffer)
    {
        auto reader = endian::stream_reader<endian::big_endian>(data_buffer);

        while (reader.remaining_size() > sizeof(header_type))
        {
            header_type header_data;
            reader.read(header_data);

            auto header = header_reader(header_data);

            auto remaining_size =
                header.template field<0>().template read_as<header_type>();

            auto start = header.template field<1>().template read_as<bool>();

            // If remaining size is zero we have hit a flush-segment
            if (remaining_size == 0)
            {
                break;
            }

            // If we get a message start, then the full segment size is
            // stored and the data should be cleared. It is possible that
            // the previous message was not finished due to packet loss.
            if (start)
            {
                m_message.reset(remaining_size);
            }

            auto bytes =
                std::min((header_type)reader.remaining_size(), remaining_size);

            if (remaining_size != m_message.remaining_size())
            {
                // header mismatch, message lost - clear data and skip.
                m_message.reset(0);
                reader.seek(reader.position() + bytes);
            }
            else
            {
                // Read data
                m_message.copy_from_reader(reader, bytes);
            }

            // If message is complete, move to queue
            if (m_message.is_complete())
            {
                m_message_queue.push_back(std::move(m_message.extract_data()));
            }
        }
    }

    // returns true if a message can be extracted
    bool message_available() const
    {
        return !m_message_queue.empty();
    }

    // returns a message
    std::vector<uint8_t> get_message()
    {
        assert(message_available());

        std::vector<uint8_t> message(std::move(m_message_queue.front()));
        m_message_queue.pop_front();

        return message;
    }

private:

    // The current message being reassembled
    message m_message;

    // The queue of reassembled messages, front being the oldest
    std::deque<std::vector<uint8_t>> m_message_queue;
};
}
