cmake_minimum_required(VERSION 3.24)
project(emulator)

set(CMAKE_C_STANDARD 23)

add_executable(emulator src/main.c
        src/emulator.c
        src/emulator.h
        src/utils/util_time.c
        src/utils/util_time.h
        src/utils/util_bits.c
        src/utils/util_bits.h
        src/utils/util_types.h
        src/graphics.c
        src/graphics.h
        src/input.c
        src/input.h
)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(emulator
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)