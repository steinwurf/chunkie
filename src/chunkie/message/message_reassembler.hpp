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

namespace chunkie
{
    template <typename HeaderType>
    class message_reassembler
    {
    public:
        using header = std::array<uint8_t, sizeof(HeaderType)>;

        using message_queue = std::deque<std::vector<uint8_t>>;

        struct message
        {
            HeaderType m_size;
            std::vector<uint8_t> m_data;
        };

    public:
        // Read a segment. The buffer MUST start with a segment start,
        // but does not need to be a full segment. If a partial segment is
        // used, the remaining segment MUST be discarded if available.
        // Segments must be read in-order,
        void read_segment(const std::vector<uint8_t>& segment)
        {
            uint64_t consumed = 0;

            while (consumed < segment.size() - sizeof(HeaderType))
            {
                header hdr;
                std::copy(segment.begin() + consumed,
                          segment.begin() + consumed + hdr.size(), hdr.begin());

                consumed += hdr.size();

                const bool start = message_start(hdr);
                const uint64_t remaining_size = message_size(hdr);

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
                    m_message.m_size = remaining_size;
                    m_message.m_data.clear();
                    m_message.m_data.reserve(remaining_size);
                }

                if (remaining_size !=
                    m_message.m_size - m_message.m_data.size())
                {
                    // header mismatch, message lost
                    m_message.m_size = 0;
                    m_message.m_data.clear();
                }

                uint32_t bytes = std::min(segment.size() - consumed,
                                          (std::size_t) remaining_size);

                m_message.m_data.insert(m_message.m_data.end(),
                                        segment.begin() + consumed,
                                        segment.begin() + consumed + bytes);

                consumed += bytes;

                // If message is complete, move to queue
                if (m_message.m_size == m_message.m_data.size() &&
                    m_message.m_size > 0)
                {
                    m_message_queue.push_back(std::move(m_message.m_data));

                    m_message.m_data.clear();
                    m_message.m_size = 0;
                }
            }
        }

        // returns true if a message can be extracted
        bool message_available()
        {
            return !m_message_queue.empty();
        }

        // returns a message
        std::vector<uint8_t> get_message()
        {
            assert(message_available());

            std::vector<uint8_t> msg(std::move(m_message_queue.front()));
            m_message_queue.pop_front();

            return msg;
        }

    private:
        bool message_start(header hdr)
        {
            return (hdr.front() >> 7) & 1;
        }

        HeaderType message_size(header hdr)
        {
            hdr.front() &= ~(1 << 7);
            return endian::big_endian::get<HeaderType>(hdr.data());
        }

    private:
        // The current message being reassembled
        message m_message;

        // The queue of reassembled messages, front being the oldest
        message_queue m_message_queue;
    };
}
