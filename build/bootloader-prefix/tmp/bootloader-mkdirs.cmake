# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/micah/esp/esp-idf/components/bootloader/subproject"
  "/home/micah/src/my_project/build/bootloader"
  "/home/micah/src/my_project/build/bootloader-prefix"
  "/home/micah/src/my_project/build/bootloader-prefix/tmp"
  "/home/micah/src/my_project/build/bootloader-prefix/src/bootloader-stamp"
  "/home/micah/src/my_project/build/bootloader-prefix/src"
  "/home/micah/src/my_project/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/micah/src/my_project/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
