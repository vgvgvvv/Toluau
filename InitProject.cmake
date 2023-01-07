

macro(InitProject)

    message(STATUS "fetch re-make library")
    include(FetchContent)
    FetchContent_Declare(
            re-cmake
            GIT_REPOSITORY https://github.com/vgvgvvv/re-cmake
            GIT_TAG        "test-1.2"
    )
    FetchContent_MakeAvailable(re-cmake)

    message(STATUS "re-cmake_SOURCE_DIR ${re-cmake_SOURCE_DIR}")
    list(APPEND CMAKE_MODULE_PATH "${re-cmake_SOURCE_DIR}")
    include(Init)

    ReMake_InitProject()

endmacro()