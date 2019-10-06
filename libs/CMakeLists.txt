add_subdirectory(catch)

option(BUILD_SHARE_LIBS "Build shared libs" OFF)
set(XXHASH_BUILD_ENABLE_INLINE_API OFF)
set(XXHASH_BUILD_XXHSUM OFF)
add_subdirectory(xxhash/cmake_unofficial/ xxhash/build/> EXCLUDE_FROM_ALL)
