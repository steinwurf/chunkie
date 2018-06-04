News for chunkie
================

This file lists the major changes between versions. For a more detailed list of
every change, see the Git log.

Latest
------
* tbd

8.0.0
-----
* Major: Complete re-implementation see new classes and example, wireformat
  remains the same.
* Major: Checksum functionality removed.
* Major: Removed boost dependency.
* Minor: Removed internal dependency stub

7.0.0
-----
* Major: Upgrade to endian 9

6.1.0
-----
* Minor: Expose ``message_size`` on ``message_reassembler``.

6.0.0
-----
* Major: Upgrade to endian 8
* Major: Upgrade to bitter 4

5.0.0
-----
* Major: Upgrade to endian 5

4.0.1
-----
* Patch: Made ``message_reassembler::message_available()`` ``const``.

4.0.0
-----
* Minor: Exposed ``header_type`` type definition.
* Major: Added bitter dependency.
* Minor: Added error code API to ``file_segment::from_buffer``. Functions
  ``static file_segment file_segment::from_buffer(const uint8_t*, uint32_t, std::error&) noexcept``
  and
  ``static file_segment file_segment::from_buffer(const std::vector<uint8_t>&, std::error&) noexcept``
  now available.

3.0.0
-----
* Major: Major overhaul of file_segmenter and file_reassembler. New API.

2.0.0
-----
* Major: Upgrade to waf-tools 4
* Major: Upgrade to endian 4
* Major: Upgrade to boost 3
* Minor: Upgrade to gtest 4
* Minor: Upgrade to stub 6

1.0.2
-----
* Patch: Fixed message reassembler occasional crash when segments are lost.

1.0.1
-----
* Patch: Fixed minimum message size. Messages can now be down to 1 byte.

1.0.0
------
* First stable version.
* Message segmenting and reassembly
* File segmenting and reassembly
* Checksum write and parse
