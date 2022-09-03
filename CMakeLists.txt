cmake_minimum_required(VERSION 3.17)

project(ToLuau)

include(InitProject.cmake)
InitProject()

ReMake_AddSubDirsRec(src)

set(IsRoot "")
IsRootProject(IsRoot)
if(${IsRoot})
    message(STATUS "root project add test targets")
    ReMake_AddSubDirsRec(test)
    ReMake_AddSubDirsRec(tools)
endif()