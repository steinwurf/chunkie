// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <array>
#include <algorithm>
#include <deque>
#include <vector>

#include <endian/stream_writer.hpp>
#include <endian/big_endian.hpp>

namespace chunkie
{
    /// The message segmenter will cut messages into segments of a specific size
    /// The segmenter will append headers to the messages and the segments.
    /// Once header a the start of each segment, and one header for each
    /// additional message within a segment.
    /// The length of these can be adjusted with the HeaderType template
    /// parameter, providing maximum messages lengths given the following rules:
    /// HeaderType = uint8_t  ->  Message sizes must be smaller than 128 bytes
    /// HeaderType = uint16_t ->  Message sizes must be smaller than 32768 bytes
    /// HeaderType = uint32_t ->  Message sizes must be smaller than 2^31 bytes
    /// HeaderType = uint64_t ->  Message sizes must be smaller than 2^63 bytes
    /// A HeaderType of uint32_t is default is this in general allows
    /// problem-free use. Overhead optimisations may be done by using smaller
    /// header types, but this will limit the lengths of the messages sent.
    ///
    /// Below is a few examples of how the message serializer operates:
    ///
    /// Example 1:
    /// 3 symbols of size 20 bytes containing one full message of len 40 bytes.
    /// In the middle of symbol no. 3 a new message starts
    /// @code
    ///
    /// Symbol 1:
    ///  0                   1                   2                   3
    ///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |1|                        size = 40                            | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// Symbol 2:
    ///  0                   1                   2                   3
    ///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |0|                        size = 24                            | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// Symbol 3:
    ///  0                   1                   2                   3
    ///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |0|                        size = 8                             | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |1|                        size = ...                           | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// @endcode
    ///
    ///
    /// Example 2:
    /// 2 symbols of size 20 bytes containing three full messages of 8, 12 and
    /// 5 bytes respectively
    /// @code
    ///
    /// Symbol 1:
    ///  0                   1                   2                   3
    ///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |1|                        size = 8                             | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |1|                        size = 12                            | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// Symbol 2:
    ///  0                   1                   2                   3
    ///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |0|                        size = 8                             | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |                                                               | <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |1|                        size = 5                             | <- hdr
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /// |         |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0| <- msg
    /// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// The remaining space in the symbol after msg 3 is zero padded as it can
    /// not fit another header. A new message will start in the
    /// following symbol.
    ///
    /// @endcode

    template<typename HeaderType>
    class message_segmenter
    {
        using header = std::array<uint8_t, sizeof(HeaderType)>;

        using queue_item = std::pair<bool, std::vector<uint8_t>>;

        using message_queue = std::deque<queue_item>;

    public:

        static const HeaderType min_message_size = sizeof(HeaderType) + 1;

        static const HeaderType max_message_size =
            std::numeric_limits<HeaderType>::max() / 2;


        // returns the amount of message data in bytes stored in the segmenter.
        uint64_t data_buffered()
        {
            uint64_t bytes = 0;

            for (const auto& item : m_message_queue)
            {
                bytes += item.second.size();
            }

            return bytes;
        }

        // param: buffer containing message to write
        void write_message(const std::vector<uint8_t>& message)
        {
            assert(message.size() <= max_message_size);
            queue_item item(true, message);
            m_message_queue.push_back(std::move(item));
        }
        void write_message(std::vector<uint8_t>&& message)
        {
            assert(message.size() <= max_message_size);
            queue_item item(true, message);
            m_message_queue.push_back(std::move(item));
        }

        // param: size of segment to write
        // return: true if a segment of size 'segment_size' can be written.
        bool segment_available(HeaderType segment_size)
        {
            uint64_t bytes_available = 0;

            for (const auto& item : m_message_queue)
            {
                bytes_available += sizeof(HeaderType) + item.second.size();
            }

            return bytes_available >= segment_size;
        }

        // param: write a segment of size segment size
        // return: the segment
        std::vector<uint8_t> get_segment(HeaderType segment_size)
        {
            assert(segment_available(segment_size) &&
                   "Attempting to getting segment when none is available. "
                   "Always use segment_available() before fetching segment.");

            std::vector<uint8_t> segment(segment_size);

            if (!segment_available(segment_size))
            {
                segment.clear();
                return segment;
            }

            
            write_segment(segment);

            return segment;
        }

        // If any partial message contained inside the segmenter,
        // flush zeropads this partial message to fill a segment
        // of size 'segment_size'. If the remaining message data fills more
        // than a segment size, an assertion is thrown.
        std::vector<uint8_t> flush(HeaderType segment_size)
        {
            assert(!segment_available(segment_size) &&
                   "Flushing when full segment is available!");

            assert(data_buffered() != 0 &&
                   "Flushing when no data is stored internally in segmenter");
         
            std::vector<uint8_t> segment(segment_size, 0);

            write_segment(segment);

            return segment;
        }

    private:

        header make_header(bool start,
                                                            HeaderType size)
        {
            header hdr;
            endian::big_endian::put<HeaderType>(size, hdr.data());
            hdr.front() |= start << 7;
            return hdr;
        }

        void write_segment(std::vector<uint8_t>& segment)
        {
            assert(segment.size() > sizeof(HeaderType) &&
                   "Segment size must be bigger than the size of HeaderType");

            endian::stream_writer<endian::big_endian> writer(segment.data(),
                                                             segment.size());

            for (auto it = m_message_queue.begin();
                 it != m_message_queue.end();
                 it = m_message_queue.erase(it))
            {
                auto& start = it->first;
                auto& message = it->second;

                if (message.size() == 0)
                    continue;

                auto hdr = make_header(start, message.size());
                writer.write(hdr.data(), hdr.size());

                uint32_t bytes = std::min(writer.remaining_size(), 
                                          (uint32_t)message.size());

                writer.write(message.data(), bytes);

                start = false;
                message.erase(message.begin(), message.begin() + bytes);

                if (writer.remaining_size() <= sizeof(HeaderType))
                    break;
            }
        }

    private:

        message_queue m_message_queue;

    };
}
