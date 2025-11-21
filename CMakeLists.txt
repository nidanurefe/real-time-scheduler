cmake_minimum_required(VERSION 3.10)
project(RTSchedulerCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rt_scheduler
    src/main.cpp
    src/models.hpp
    src/parser.hpp
    src/policies.hpp
    src/sched_base.hpp
    src/sched_servers.hpp
    src/factory.hpp
)