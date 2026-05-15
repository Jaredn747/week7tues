
cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-toolchain.cmake)

project(lecture_homework_week07-t LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker-script.ld")
set(TARGET_NAME "uart-int")

add_executable(${TARGET_NAME}.elf
    src/start.s
    src/main.c
    src/timer.c
    src/vid.c
    src/exceptions.c
    src/font.c
    src/string.c
    src/kbd.c
    src/uart_int.c
    src/log.c
)

add_custom_command(
    TARGET ${TARGET_NAME}.elf
    PRE_LINK
    COMMAND ${CMAKE_OBJCOPY} ARGS -I binary -O elf32-littlearm -B arm ../image0.bmp image0.o
)

add_custom_command(TARGET ${TARGET_NAME}.elf
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${TARGET_NAME}.elf ${TARGET_NAME}.bin
)

set_target_properties(${TARGET_NAME}.elf PROPERTIES
    ADDITIONAL_CLEAN_FILES ${TARGET_NAME}.bin
)

target_include_directories(${TARGET_NAME}.elf PRIVATE ${PROJECT_SOURCE_DIR}/include include/)
# Add the linker flag to the target

target_link_options(${TARGET_NAME}.elf PRIVATE 
    "-T${LINKER_SCRIPT}" # Linker script path
    "--specs=nano.specs"                 # Optimized C library
    "-Wl,--gc-sections"                  # Remove unused code
)

set_target_properties(${TARGET_NAME}.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
