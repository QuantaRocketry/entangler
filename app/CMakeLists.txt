cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../modules/fs/littlefs)
FILE(GLOB_RECURSE app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

zephyr_cc_option(-Wfatal-errors)
zephyr_cc_option(-Wall)
