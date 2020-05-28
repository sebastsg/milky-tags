# Milky Tags
File tagging software focused on speed, ease of use, and tag portability.

# Why?
Other tagging software seem to either be very slow, or have inconvenient user interfaces.
This project focuses on high performance, quick and easy tagging and untagging, and uses the file renaming approach to mark files with tags. This allows you to have full control over your tagging work. 

# Tags and groups
Tags exist inside groups, to make it easy to find the tag you want to use.
Both groups and tags must have a unique name, as the tag is not bound to groups in any way.
It's just a matter of UI convenience.

Each tag can have its own background and text color, to make them easily distinguishable.
You can also write a "pretty name" for a tag, as well as a description.
The pretty name will only be used in the UI.

# How to build
This project uses [CMake](https://cmake.org). Run `project/vs16.bat` to generate the solution file.

It's also required that you clone and build [nfwk](https://github.com/sebastsg/nfwk) in the same parent directory.

If you are building a release, make sure ``DEV_VERSION`` in ``start.cpp`` is defined to ``0``. I'll fix that quirk eventually.

# The milky.tags file
This file contains the groups and tags that are recognized.
Currently, it only lets you define your own tags, but I'll add some "import tags" feature later.
It's currently a binary file, but I will convert it to a text format later.
