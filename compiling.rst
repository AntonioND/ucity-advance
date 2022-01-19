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

Finally, convert the assets of the game:

.. code:: bash

   bash assets.sh

Linux
=====

This should be enough:

.. code:: bash

    mkdir build
    cd build
    cmake ..
    make -j`nproc` install

The output is in the folder ``build/install``.

Game Boy Advance
================

With **devkitPro**
^^^^^^^^^^^^^^^^^^

First, you need to install devkitPro. Follow the instructions in the following
link to install it in your system: https://devkitpro.org/wiki/Getting_Started

The Linux and Game Boy Advance binaries are built at the same time:

.. code:: bash

    mkdir build
    cd build
    cmake .. -DBUILD_GBA=ON -DCMAKE_BUILD_TYPE=Release
    make -j`nproc` install

The output is in the folder ``build/install``.

Without **devkitPro**
^^^^^^^^^^^^^^^^^^^^^

Install ``gcc-arm-none-eabi`` toolchain: You can get it from your package
manager, or from `Arm's GNU toolchain downloads website`_. In Ubuntu you can
just do:

.. code:: bash

    sudo apt install gcc-arm-none-eabi

The Linux and Game Boy Advance binaries are built at the same time:

.. code:: bash

    mkdir build
    cd build
    cmake .. -DBUILD_GBA=ON -DCMAKE_BUILD_TYPE=Release -DUSE_DEVKITARM=OFF
    make -j`nproc` install

The output is in the folder ``build/install``.

Regenerate assets
=================

If a tileset is modified, for example, it is needed to regenerate assets.

Install `Tiled <https://www.mapeditor.org/>`_. Then run:

.. code:: bash

   bash gen_maps.sh

It isn't needed to do this as part of the build process, the resulting files are
included in the repository.

.. _Arm's GNU toolchain downloads website: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
