=======
chunkie
=======

Quick Overview
--------------

The chunkie project provides tools for segmenting and reassembling buffers
of arbitrary size.
It also provides functionality for checksum verification of buffers.

File segmenter and reassembler splits up a file into smaller chunks and
reassembles these into the correct file again.

Message segmenter and reassembler splits up (or concatenates) buffers into
segments of a specified size - and reassembles the set of segments into the
same original buffers again.

For integrity tests of data, the checksum can be added using write_checksum
function. The checksum can then be verified with read_checksum function.
