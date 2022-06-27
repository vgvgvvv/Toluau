cmake_minimum_required(VERSION 3.17)

project(ToLuau)

include(InitProject.cmake)
InitProject()

ReMake_AddSubDirsRec(src)
ReMake_AddSubDirsRec(test)