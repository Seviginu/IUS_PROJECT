cmake_minimum_required(VERSION 3.15)

project(posix_demo LANGUAGES C)

# Общие настройки компиляции
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -D_WINDOWS_
    -DTRACE_ON_ENTER=$<IF:$<BOOL:${TRACE_ON_ENTER}>,1,0>
    -DprojCOVERAGE_TEST=$<IF:$<BOOL:${COVERAGE_TEST}>,1,0>
    -DprojENABLE_TRACING=$<IF:$<BOOL:${NO_TRACING}>,0,1>
)

# Настройки сборки
if(PROFILE)
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
else()
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
endif()

# Опции санитайзеров
if(SANITIZE_ADDRESS)
    add_compile_options(-fsanitize=address -fsanitize=alignment)
endif()

if(SANITIZE_LEAK)
    add_compile_options(-fsanitize=leak)
endif()

# Пути к FreeRTOS
set(FREERTOS_KERNEL_PATH "../../Source")
set(FREERTOS_PLUS_TRACE_PATH "../../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace")

# Конфигурация FreeRTOS
add_library(freertos_config INTERFACE)

target_include_directories(freertos_config INTERFACE
    ./
    ./Trace_Recorder_Configuration
    ${FREERTOS_PLUS_TRACE_PATH}/include
    ${FREERTOS_PLUS_TRACE_PATH}/kernelports/FreeRTOS/include
    ${CMAKE_CURRENT_SOURCE_DIR}/logger  # Добавляем путь к логгеру
)

# Настройки порта и кучи
set(FREERTOS_HEAP "3" CACHE STRING "Selected heap implementation" FORCE)
set(FREERTOS_PORT "GCC_POSIX" CACHE STRING "Selected FreeRTOS port" FORCE)

# Подключаем ядро FreeRTOS
add_subdirectory(${FREERTOS_KERNEL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS-Kernel)

# Настройки компиляции для ядра
target_compile_options(freertos_kernel PRIVATE
    $<IF:$<BOOL:${NO_TRACING}>,, -Wno-pointer-to-int-cast>
)

# Исходные файлы демо-приложения
file(GLOB FREERTOS_PLUS_TRACE_SOURCES 
    ${FREERTOS_PLUS_TRACE_PATH}/*.c 
    ${FREERTOS_PLUS_TRACE_PATH}/kernelports/FreeRTOS/*.c
)

file(GLOB COMMON_MINIMAL_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../Common/Minimal/*.c
)

# Исключаем ненужные файлы
list(REMOVE_ITEM COMMON_MINIMAL_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../Common/Minimal/main.c
)

# Основной исполняемый файл
add_executable(posix_demo
    code_coverage_additions.c
    console.c
    main.c
    main_blinky.c
    main_full.c
    run-time-stats-utils.c
    logger/logger.c  # Добавляем ваш логгер
    $<$<NOT:$<BOOL:${NO_TRACING}>>:${FREERTOS_PLUS_TRACE_SOURCES}>
    ${COMMON_MINIMAL_SOURCES}
)

# Настройки include для демо
target_include_directories(posix_demo PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../Common/include
    ${FREERTOS_PLUS_TRACE_PATH}/Include
    ${FREERTOS_PLUS_TRACE_PATH}/streamports/File/include
    ${FREERTOS_PLUS_TRACE_PATH}/streamports/File/config
    ${CMAKE_CURRENT_SOURCE_DIR}/logger  # Путь к заголовкам логгера
)

# Определения для демо
target_compile_definitions(posix_demo PRIVATE
    $<$<STREQUAL:${USER_DEMO},BLINKY_DEMO>:USER_DEMO=0>
    $<$<STREQUAL:${USER_DEMO},FULL_DEMO>:USER_DEMO=1>
)

# Линковка
target_link_libraries(posix_demo 
    freertos_kernel 
    freertos_config
    pthread
    $<$<BOOL:${SANITIZE_ADDRESS}>:-fsanitize=address>
    $<$<BOOL:${SANITIZE_LEAK}>:-fsanitize=leak>
)

# Дополнительные настройки для логгера
if(UNIX)
    target_link_libraries(posix_demo m rt)
endif()