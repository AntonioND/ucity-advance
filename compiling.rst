=========================
µCity Advance - Compiling
=========================

Requirements
============

Before building for either GBA or Linux, you need to follow the instructions
here:

First, you need CMake 3.15 or newer, make, and libpng 1.6. For example:

.. code:: bash

   sudo apt install cmake build-essential libpng-dev

Then, clone the repository of this game with all its submodules:

.. code:: bash

    git clone --recurse-submodules https://github.com/AntonioND/ucity-advance
    cd ucity-advance

Build **SuperFamiconv**:

.. code:: bash

    cd SuperFamiconv
    mkdir build && cd build && cmake .. && make -j`nproc`
    cd ..

Build the packer used by **UMOD Player**:

.. code:: bash

    cd umod-player
    mkdir build && cd build && cmake .. && make -j`nproc`
    cd ../..

Finally, convert the assets of the game:

.. code:: bash

   bash assets.sh

Game Boy Advance
================

First, you need to install devkitPro. Follow the instructions in the following
link to install it in your system: https://devkitpro.org/wiki/Getting_Started

Build the library files of **libugba** and the **UMOD Player**:

.. code:: bash

    cd libugba && make && cd ..
    cd umod-player/player && make && cd ../..

Finally, build the game:

.. code:: bash

    make -j`nproc`

Linux
=====

If you have **devkitPro** with **grit** in your system, it should be enough to
do this:

.. code:: bash

    mkdir build && cd build && cmake .. && make -j`nproc`

If you don't install devkitPro because you only want to build the PC
executables, you still need to get Grit to convert the graphics into the right
format for the GBA: https://github.com/devkitPro/grit/releases

You'll need to make sure that CMake can find it by adding it to your system's
``PATH`` environment variable. If you have installed devkitPro, the build system
should be able to find the Grit executable installed by it. If you don't want to
have it in your ``PATH``, you can also set the ``GRIT_PATH`` variable when
invoking cmake: ``cmake .. -DGRIT_PATH=/path/to/grit/folder/``

Regenerate assets
=================

If a tileset is modified, for example, it is needed to regenerate assets.

Install `Tiled <https://www.mapeditor.org/>`_. Then run:

.. code:: bash

   bash gen_maps.sh

It isn't needed to do this as part of the build process, the resulting files are
included in the repository.
