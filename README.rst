=======
chunkie
=======

|Linux make-specs| |Windows make-specs| |MacOS make-specs| |Linux CMake| |Windows CMake| |MacOS CMake| |Valgrind| |No Assertions| |Clang Format| |Cppcheck|

.. |Linux make-specs| image:: https://github.com/steinwurf/chunkie/actions/workflows/linux_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/linux_mkspecs.yml
   
.. |Windows make-specs| image:: https://github.com/steinwurf/chunkie/actions/workflows/windows_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/windows_mkspecs.yml

.. |MacOS make-specs| image:: https://github.com/steinwurf/chunkie/actions/workflows/macos_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/macos_mkspecs.yml
   
.. |Linux CMake| image:: https://github.com/steinwurf/chunkie/actions/workflows/linux_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/linux_cmake.yml

.. |Windows CMake| image:: https://github.com/steinwurf/chunkie/actions/workflows/windows_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/windows_cmake.yml
   
.. |MacOS CMake| image:: https://github.com/steinwurf/chunkie/actions/workflows/macos_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/macos_cmake.yml

.. |Clang Format| image:: https://github.com/steinwurf/chunkie/actions/workflows/clang-format.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/clang-format.yml

.. |No Assertions| image:: https://github.com/steinwurf/chunkie/actions/workflows/nodebug.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/nodebug.yml

.. |Valgrind| image:: https://github.com/steinwurf/chunkie/actions/workflows/valgrind.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/valgrind.yml

.. |Cppcheck| image:: https://github.com/steinwurf/chunkie/actions/workflows/cppcheck.yml/badge.svg
   :target: https://github.com/steinwurf/chunkie/actions/workflows/cppcheck.yml

Chunkie provides functionality for serializing an object into a stream and
deserializing a stream back into an object. This can be useful when sending big
objects such as video frames or files over a network.

The deserializer can reconstruct the original object when all the buffers are
inputted in order. If buffers are inputted out of order or some buffers are lost
the object cannot be reconstructed.

*Note* that there is no explicit integrity check before an object is outputted.
So if it is critical that erroneous objects are detected an integrity check
should be made on outputted symbols. It is possible that an erroneous object
can be outputted in circumstances where buffers are lost in a very particular
way. Specifically if the tail buffer(s) of an object is lost and the head
buffer(s) of the subsequent object is lost, and the remaining size of the
subsequent object matches exactly the expected remaining bytes from the first
object, an object will be outputted where the head is from the first object and
the tail from the second object.

Below two examples of the output when serializing some objects to buffers using
chunkie.


Example 1
.........

An object of 20 bytes is serialized into 3 buffers.

Buffer 1

::

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |1|                        size = 20                            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 1 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 1 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


Buffer 2

::

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |0|                        size = 12                            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 1 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 1 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Buffer 3

::

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |0|                        size = 4                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 1 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |1|                        size = ...                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 2 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


Example 2
.........

Objects of 4 and 7 bytes respectively serialized into two buffers.

Buffer 1

::

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |1|                        size = 4                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 3 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |1|                        size = 7                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 4 data                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


Buffer 2

::

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |0|                        size = 3                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 4 data            |1|           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                          size = 6               |             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        object 5 data                          |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 |
    +-+-+-+-+-+-+-+-+-+

Use as Dependency in CMake
--------------------------

To depend on this project when using the CMake build system, add the following
in your CMake build script::

   add_subdirectory("/path/to/chunkie" chunkie)
   target_link_libraries(<my_target> steinwurf::chunkie)

Where ``<my_target>`` is replaced by your target.

