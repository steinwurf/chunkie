// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <chunkie/message/message_reassembler.hpp>
#include <chunkie/message/message_segmenter.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <fstream>

#include <boost/optional.hpp>

#include <stub/function.hpp>

// Test Base Fixture
template <typename TestType, typename Type>
class test_message_segment_reassemble : public TestType
{
public:
    virtual void SetUp()
    {
        // Make sure we dont create abnormally large messages
        m_max_message_size =
            std::min((uint64_t) ms.max_message_size, (uint64_t) 2500);

        m_segment_size = get_segment_size(m_max_message_size);
    }

    virtual uint64_t get_segment_size(uint64_t max_message_size)
    {
        uint64_t size = max_message_size / 4u;
        return size;
    }

    virtual void write_messages_to_segmenter(uint32_t messages)
    {
        ASSERT_GE(255u, messages);

        std::uniform_int_distribution<uint64_t> random_size(
            ms.min_message_size, m_max_message_size);

        /// Write messages to segmenter
        for (uint32_t index = 0; index < messages; ++index)
        {
            SCOPED_TRACE(::testing::Message() << "Running message " << index);

            ASSERT_LE(index, std::numeric_limits<uint8_t>::max())
                << "TestError: Index must be able to fit in a uint8_t";

            uint64_t message_size = random_size(m_random_engine);

            SCOPED_TRACE(::testing::Message() << "Message size "
                         << message_size);

            // Create a message of size message size and contents i
            std::vector<uint8_t> message(message_size, uint8_t(index));
            message.front() = (uint8_t) index;

            ms.write_message(message);

            // Track the message size
            segmenter_message_stub(index, message_size);
        }
    }

    virtual void read_segments_to_reassembler()
    {
        /// Pull out segments and put them into the reassembler:
        while (ms.segment_available(m_segment_size))
        {
            std::vector<uint8_t> seg = ms.get_segment(m_segment_size);

            EXPECT_EQ(m_segment_size, seg.size())
                << "The segment length must be equal to the segment size";
            mr.read_segment(seg);
        }

        // Empty the segmenter with flush if any data stored inside:
        if (ms.data_buffered() > 0)
        {
            SCOPED_TRACE(::testing::Message() << "Getting flush segment.");
            std::vector<uint8_t> flush_seg = ms.flush(m_segment_size);

            EXPECT_EQ(m_segment_size, flush_seg.size())
                << "The segment length must be equal to the segment size";

            mr.read_segment(flush_seg);
        }
    }

    virtual void get_messages_from_reassembler()
    {
        // uint32_t message_number = 0;
        // Pull reassembled messages from the reassemlber:
        while (mr.message_available())
        {
            std::vector<uint8_t> msg = mr.get_message();
            reassembler_message_stub(msg.front(), msg.size());
        }
    }

    virtual void verify_messages()
    {
        EXPECT_EQ(
            reassembler_message_stub.calls(),
            segmenter_message_stub.calls());
        for (uint32_t msgno = 0; msgno < segmenter_message_stub.calls();
             ++msgno)
        {
            auto segmenter_arg = segmenter_message_stub.call_arguments(msgno);
            auto segmenter_id = std::get<0>(segmenter_arg);
            auto segmenter_size = std::get<1>(segmenter_arg);

            ASSERT_EQ(msgno, segmenter_id)
                << "Segmenter message id not sequential";

            auto reassembler_arg =
                reassembler_message_stub.call_arguments(msgno);
            auto reassembler_id = std::get<0>(reassembler_arg);
            auto reassembler_size = std::get<1>(reassembler_arg);

            ASSERT_EQ(msgno, reassembler_id) << "All messages not reassembled";

            ASSERT_EQ(segmenter_size, reassembler_size)
                << "Reassembled message size not correct";
        }
    }

    virtual void run()
    {
        uint32_t messages = 100;

        SCOPED_TRACE(::testing::Message() << "Using segment size "
                     << m_segment_size);

        write_messages_to_segmenter(messages);

        EXPECT_TRUE(ms.segment_available(m_segment_size));

        read_segments_to_reassembler();

        get_messages_from_reassembler();

        verify_messages();
    }

protected:
    std::mt19937 m_random_engine = std::mt19937(0);

    chunkie::message_segmenter<Type> ms;
    chunkie::message_reassembler<Type> mr;

    uint64_t m_max_message_size = 0;
    uint64_t m_segment_size = 0;

    // keep track of message number and size in two stubs:
    stub::function<void(uint8_t, Type)> segmenter_message_stub;
    stub::function<void(uint8_t, Type)> reassembler_message_stub;
};

// Test fixture for Typed Tests:
template <class Type>
class test_message_segment_reassemble_header_type
    : public test_message_segment_reassemble<::testing::Test, Type>
{
};

class test_message_segment_reassemble_segment_size : public
    test_message_segment_reassemble<
    ::testing::TestWithParam<uint32_t>, uint32_t>
{
    virtual uint64_t get_segment_size(uint64_t /*max_message_size*/)
    {
        return GetParam();
    }
};

using HeaderTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;

TYPED_TEST_CASE(test_message_segment_reassemble_header_type, HeaderTypes);

TYPED_TEST(test_message_segment_reassemble_header_type, run)
{
    this->run();
}

// Test for different segment sizes
INSTANTIATE_TEST_CASE_P(/* segment size */,
                        test_message_segment_reassemble_segment_size,
                        ::testing::Values(100U, 250U, 500U, 1500U));

TEST_P(test_message_segment_reassemble_segment_size, run)
{
    this->run();
}

class test_message_segment_reassemble_under_loss
    : public test_message_segment_reassemble<
      ::testing::TestWithParam<double /*loss rate*/>, uint32_t>
{
    // Redefine function reading segments to throw some away
    virtual void read_segments_to_reassembler()
    {
        std::bernoulli_distribution packet_loss(GetParam());

        // Pull out segments and put them into the reassembler:
        while (ms.segment_available(m_segment_size))
        {
            std::vector<uint8_t> seg = ms.get_segment(m_segment_size);

            EXPECT_EQ(m_segment_size, seg.size())
                << "The segment length must be equal to the segment size";

            // Simulate a lost packet
            if (packet_loss(m_random_engine))
            {
                continue;
            }

            mr.read_segment(seg);
        }

        // Empty the segmenter with flush if any data stored inside:
        if (ms.data_buffered() > 0)
        {
            SCOPED_TRACE(::testing::Message() << "Getting flush segment.");
            std::vector<uint8_t> flush_seg = ms.flush(m_segment_size);

            EXPECT_EQ(m_segment_size, flush_seg.size())
                << "The segment length must be equal to the segment size";

            mr.read_segment(flush_seg);
        }
    };

    // Redefine verification function:
    virtual void verify_messages()
    {
        boost::optional<uint8_t> last_reassembled_msgno;

        for (uint32_t index = 0; index < reassembler_message_stub.calls();
             ++index)
        {
            auto reassembler_arg =
                reassembler_message_stub.call_arguments(index);
            auto reassembler_id = std::get<0>(reassembler_arg);
            auto reassembler_size = std::get<1>(reassembler_arg);

            SCOPED_TRACE(::testing::Message()
                         << "Verifying message with sequence number "
                         << reassembler_id);

            if (!last_reassembled_msgno)
            {
                last_reassembled_msgno = reassembler_id;
            }
            else
            {
                ASSERT_LT(*last_reassembled_msgno, reassembler_id)
                    << "Messages reassembled out of order!";
            }

            ASSERT_LT(reassembler_id, segmenter_message_stub.calls())
                << "Reassembled a message with sequence number larger than "
                "number of transmitted messages";

            auto segmenter_arg =
                segmenter_message_stub.call_arguments(reassembler_id);
            auto segmenter_size = std::get<1>(segmenter_arg);

            ASSERT_EQ(segmenter_size, reassembler_size)
                << "Reassembled message size not correct";
        }
    }
};

// Test for different segment sizes
INSTANTIATE_TEST_CASE_P(/* loss rate */,
                        test_message_segment_reassemble_under_loss,
                        ::testing::Values(0.0, 0.01, 0.05, 0.1, 0.2, 0.3));

TEST_P(test_message_segment_reassemble_under_loss, run)
{
    this->run();
}

TEST(test_wire_format, dump)
{
    auto error_message =
        "If this failed, the wire format might have changed. \n"
        "This should be addressed in the news file. \n"
        "To fix the issue created a new test_dump file with "
        "example/create_test_file_dump example\n";

    chunkie::message_reassembler<uint8_t> message_reassembler;

    std::ifstream file("test_dump", std::ios::binary|std::ios::ate);
    ASSERT_TRUE(file.is_open());

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto segment_size = 50;
    std::vector<uint8_t> segment(segment_size);


    for (uint32_t i = 0; i < size / segment.size(); ++i)
    {
        file.read((char*)segment.data(), segment.size());
        message_reassembler.read_segment(segment);
    }

    uint32_t number_of_segments = 0;
    while (message_reassembler.message_available())
    {
        std::vector<uint8_t> message = message_reassembler.get_message();
        std::vector<uint8_t> expected_message(message.size(), message.size());
        EXPECT_EQ(expected_message, message) << error_message;
        number_of_segments++;
    }
    EXPECT_EQ(3U, number_of_segments) << error_message;
    file.close();
}
