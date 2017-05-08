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

#include <bitter/writer.hpp>



#include <iostream>





namespace chunkie
{
/// The message segmenter will cut messages into segments of a specific size
/// The segmenter will append headers to the messages and the segments.
/// One header at the start of each segment, and one header for each
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
/// 3 Segment of size 20 bytes containing one full message of len 40 bytes.
/// In the middle of segment no. 3 a new message starts
/// @code
///
/// Segment 1:
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
/// Segment 2:
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
/// Segment 3:
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
/// 2 segments of size 20 bytes containing three full messages of 8, 12 and
/// 5 bytes respectively
/// @code
///
/// Segment 1:
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
/// Segment 2:
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
/// The remaining space in the segment after msg 3 is zero padded as it can
/// not fit another header. A new message will start in the
/// following segment.
///
/// @endcode

template<typename HeaderType>
class message_segmenter
{
public:

    using header_type = HeaderType;

    // The header consists of a size and a start bit
    using header_writer =
        bitter::writer<header_type, (sizeof(header_type) * 8) - 1, 1>;

    static const uint64_t min_message_size = 1;

    static const uint64_t max_message_size =
        std::numeric_limits<header_type>::max() / 2;

private:

    struct message
    {
        message(const std::vector<uint8_t>& data) :
            m_start(true),
            m_data(data)
        { }

        header_type header() const
        {
            assert(m_data.size() >= 1);
            assert(m_data.size() <= max_message_size);

            auto writer = header_writer();
            writer.template field<0>(m_data.size());
            writer.template field<1>(m_start);
            return writer.data();
        }

        void move_to_writer(
            endian::stream_writer<endian::big_endian>& writer, uint32_t bytes)
        {
            m_start = false;
            writer.write(m_data.data(), bytes);
            m_data.erase(m_data.begin(), m_data.begin() + bytes);
        }

        uint32_t size() const
        {
            return m_data.size();
        }

    private:

        bool m_start;
        std::vector<uint8_t> m_data;
    };

public:

    // returns the amount of message data in bytes stored in the segmenter.
    uint64_t data_buffered() const
    {
        uint64_t bytes = 0;

        for (const auto& item : m_message_queue)
        {
            bytes += item.size();
        }

        return bytes;
    }

    // param: buffer containing message to write
    void write_message(const std::vector<uint8_t>& data)
    {
        assert(data.size() >= min_message_size);
        assert(data.size() <= max_message_size);
        message item(data);
        m_message_queue.push_back(std::move(item));
    }

    void write_message(std::vector<uint8_t>&& data)
    {
        assert(data.size() >= min_message_size);
        assert(data.size() <= max_message_size);
        message item(data);
        m_message_queue.push_back(std::move(item));
    }

    // param: size of segment to write
    // return: true if a segment of size 'segment_size' can be written.
    bool segment_available(uint32_t segment_size) const
    {
        uint64_t bytes_available = data_buffered() +
                                   sizeof(HeaderType) * m_message_queue.size();

        return bytes_available >= segment_size;
    }

    // param: write a segment of size segment size
    // return: the segment
    std::vector<uint8_t> get_segment(uint32_t segment_size)
    {
        assert(segment_available(segment_size) &&
               "Attempting to getting segment when none is available. "
               "Always use segment_available() before fetching segment.");

        std::vector<uint8_t> segment(segment_size);
        write_segment(segment);

        return segment;
    }

    // If any partial message contained inside the segmenter,
    // flush zeropads this partial message to fill a segment
    // of size 'segment_size'. If the remaining message data fills more
    // than a segment size, an assertion is thrown.
    std::vector<uint8_t> flush(uint32_t segment_size)
    {
        assert(segment_available(segment_size) == 0 &&
               "Flushing when full segment is available!");

        assert(data_buffered() != 0 &&
               "Flushing when no data is stored internally in segmenter");

        std::vector<uint8_t> segment(segment_size, 0);

        write_segment(segment);

        return segment;
    }

private:

    void write_segment(std::vector<uint8_t>& segment)
    {
        assert(segment.size() > sizeof(HeaderType) &&
               "Segment size must be bigger than the size of HeaderType");

        endian::stream_writer<endian::big_endian> writer(segment);

        auto message = m_message_queue.begin();
        while (message != m_message_queue.end() &&
               writer.remaining_size() > sizeof(header_type))
        {
            writer.write(message->header());
            uint32_t bytes = std::min(writer.remaining_size(), message->size());
            message->move_to_writer(writer, bytes);

            // If there's no more data in the message remove it.
            if (message->size() == 0)
                message = m_message_queue.erase(message);
        }
    }

private:

    std::deque<message> m_message_queue;

};
}
