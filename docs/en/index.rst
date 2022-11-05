USB library
==================
This is project i decided to start some time ago to learn USB peripheral on esp32-S2. First library is working, but its not the best, so i decided to rewrite it completely to make it compatible with arduino and esp-idf this time.

Although it is new library it is still reusing part of old code and has similar API, so it should be fairly easy to migrate code to new library for users. Both libraries, old and new one, should work with S2 and S3, but old library wont be supported when this one is finished.



.. toctree::
    :hidden:

    USB device      <device/index>
    USB host        <host/index>
