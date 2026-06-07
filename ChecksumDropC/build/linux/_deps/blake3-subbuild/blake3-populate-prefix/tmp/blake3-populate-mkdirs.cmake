# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-src")
  file(MAKE_DIRECTORY "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-src")
endif()
file(MAKE_DIRECTORY
  "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-build"
  "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix"
  "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix/tmp"
  "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix/src/blake3-populate-stamp"
  "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix/src"
  "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix/src/blake3-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix/src/blake3-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/tony/Code/ChecksumDrop/ChecksumDropC/build/linux/_deps/blake3-subbuild/blake3-populate-prefix/src/blake3-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
