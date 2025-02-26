cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

# include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
FILE(GLOB app_sources src/*.c src/tasks/*.c)
target_sources(app PRIVATE ${app_sources})

zephyr_cc_option(-Wfatal-errors)
