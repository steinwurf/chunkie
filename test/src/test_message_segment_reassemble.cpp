// Copyright (c) 2015 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <iostream>
#include <random>

#include <stub/call.hpp>

#include <chunkie/message/message_segmenter.hpp>
#include <chunkie/message/message_reassembler.hpp>


// Test Base Fixture
template <typename TestType, typename Type>
class test_message_segment_reassemble : public TestType
{
public:

    virtual uint64_t get_segment_size()
    {
        return ms.max_message_size / 4u;
    }

    void run()
    {
        uint32_t messages = 50;

        std::mt19937 random_engine(0);

        // Make sure we dont create abnormally large messages, max 5000 bytes
        uint32_t max_message_size = std::min((uint64_t)ms.max_message_size,
                                             (uint64_t) 1000u); 

        std::uniform_int_distribution<Type> random_size(ms.min_message_size,
                                                        max_message_size);

        uint32_t segment_size = get_segment_size();
        SCOPED_TRACE(::testing::Message() << "Using segment size "
                                          << segment_size); 

        /// Write messages to segmenter
        for (uint32_t index = 0; index < messages; ++index)
        {
            SCOPED_TRACE(::testing::Message() << "Running message " << index);
            
            ASSERT_LE(index, std::numeric_limits<uint8_t>::max())
                << "TestError: Index must be able to fit in a uint8_t";

            uint32_t message_size = random_size(random_engine);

            SCOPED_TRACE(::testing::Message() << "Message size "
                                              << message_size);

            // Create a message of size message size and contents i
            std::vector<uint8_t> message(message_size, uint8_t(index));

            ms.write_message(message);

            // Track the message size
            message_size_stub(message_size);
        }

        ASSERT_EQ(messages, message_size_stub.calls());

        /// Pull out segments and put them into the reassembler:
        while (ms.segment_available(segment_size))
        {
            std::vector<uint8_t> seg = ms.get_segment(segment_size);

            EXPECT_EQ(segment_size, seg.size()) << "The segment length must be "
                                                   "equal to the segment size";
            mr.read_segment(seg);
        }

        // Empty the segmenter with flush if any data stored inside:
        if (ms.data_buffered() > 0)
        {
            SCOPED_TRACE(::testing::Message() << "Getting flush segment.");
            std::vector<uint8_t> flush_seg = ms.flush(segment_size);
            
            EXPECT_EQ(segment_size, flush_seg.size()) 
                << "The segment length must be equal to the segment size";

            mr.read_segment(flush_seg);
        }

        uint32_t message_number = 0;
        // Pull reassembled messages from the reassemlber:
        while (mr.message_available())
        {
            SCOPED_TRACE(::testing::Message() << "Pulling out message no "
                                              << message_number);
            
            std::vector<uint8_t> msg = mr.get_message();

            auto arg = message_size_stub.call_arguments(message_number);
            uint32_t expected_size = std::get<0>(arg);

            ASSERT_EQ(expected_size, msg.size());

            message_number += 1;
        }
    }

protected:

    chunkie::message_segmenter<Type> ms;
    chunkie::message_reassembler<Type> mr;

    stub::call<void(Type)> message_size_stub;
};

// Test fixture for Typed Tests:
template<class Type>
class test_message_segment_reassemble_header_type :
    public test_message_segment_reassemble<::testing::Test, Type>
{};

class test_message_segment_reassemble_segment_size :
    public test_message_segment_reassemble<::testing::TestWithParam<uint32_t>,
                                            uint32_t>
{
    virtual uint64_t get_segment_size()
    {
        return GetParam();
    }
};

using HeaderTypes = ::testing::Types<
                    uint8_t,
                    uint16_t,
                    uint32_t,
                    uint64_t
                    >;

TYPED_TEST_CASE(test_message_segment_reassemble_header_type, HeaderTypes);

TYPED_TEST(test_message_segment_reassemble_header_type, run)
{
    this->run();
}

// Test for different segment sizes
INSTANTIATE_TEST_CASE_P(/* segment size */,
    test_message_segment_reassemble_segment_size, 
    ::testing::Values(100U, 250U, 500U, 1500U)
);

TEST_P(test_message_segment_reassemble_segment_size, run)
{
    this->run();
}
