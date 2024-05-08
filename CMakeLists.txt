cmake_minimum_required(VERSION 3.13...3.27)

include(vendor/pico_sdk_import.cmake)

project(picow-usb-failures C CXX ASM)
set(FAMILY rp2040)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${CMAKE_PROJECT_NAME} main.c)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE pico_stdlib)
if(PICO_BOARD STREQUAL "pico_w")
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE pico_cyw43_arch_none)
endif()

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})


find_program(OPENOCD openocd)
if(OPENOCD)
  add_custom_target(flash
    COMMAND ${OPENOCD} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${CMAKE_PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${CMAKE_PROJECT_NAME}
  )
  add_custom_target(reset COMMAND ${OPENOCD} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c init -c reset -c exit)
endif()
