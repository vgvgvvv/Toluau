

macro(InitProject)

    message(STATUS "fetch re-make library")
    include(FetchContent)
    FetchContent_Declare(
            re-cmake
            GIT_REPOSITORY https://github.com/vgvgvvv/re-cmake
            GIT_TAG        "test-1.2"
    )
    FetchContent_MakeAvailable(re-cmake)

    message(STATUS "fetch re-third-part library")
    include(FetchContent)
    FetchContent_Declare(
            re-third-part
            GIT_REPOSITORY https://github.com/vgvgvvv/re-third-part
            GIT_TAG        "v1.0"
    )
    FetchContent_MakeAvailable(re-third-part)

    message(STATUS "re-cmake_SOURCE_DIR ${re-cmake_SOURCE_DIR}")
    list(APPEND CMAKE_MODULE_PATH "${re-cmake_SOURCE_DIR}")
    include(Init)

    ReMake_InitProject()

endmacro()