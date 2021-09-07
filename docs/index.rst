Chunkie Documentation
=====================

Chunkie provides functionality for serializing an object into a stream and
deserializing a stream back into an object. This can be useful when sending big
objects such as video frames or files over a network.

The deserializer can reconstruct the original object when all the buffers are
inputted in order. If buffers are inputted out of order or some buffers are lost
the object cannot be reconstructed.



The Chunkie repository: https://github.com/steinwurf/chunkie

The ``examples`` folder contains some programs that demonstrate the usage
of the available API.

If you have any questions or suggestions about this library, please contact
us at our developer mailing list (hosted at Google Groups):

* http://groups.google.com/group/steinwurf-dev


Table of Contents
-----------------

.. toctree::
  :maxdepth: 2

  user_api/user_api
  examples/examples
  license


.. toctree::
  :maxdepth: 1

  news
