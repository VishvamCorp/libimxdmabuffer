cmake_minimum_required(VERSION 3.5)

project(imxDMAbuffer LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
set(IMXDMABUFFER_VERSION "1.1")
set(IMXDMABUFFER_PATCH_VERSION "3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -fPIC -DPIC")

if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "The project requires gcc to be used as C compiler")
endif()

set(LIB_NAME imxdmabuffer)
set(LIB_SRC_PATH imxdmabuffer)

file(GLOB LIB_SRC ${LIB_SRC_PATH}/imxdmabuffer.c)
file(GLOB LIB_HEADERS
        ${LIB_SRC_PATH}/imxdmabuffer_physaddr.h
        ${CMAKE_CURRENT_BINARY_DIR}/imxdmabuffer_config.h)

# OPTIONS
option(ENABLE_ALLOC_STATS_ENABLE "Enable allocation statistics" OFF)
option(IMX_LINUX_HEADERS_PATH "Path to i.MX linux headers" "")
option(WITH_DMA_HEAP_ALLOCATOR "Build with dma-heap allocator support" ON)
option(DMA_HEAP_UNCACHED_MEMORY "dma-heap allocator allocates uncached DMA memory" OFF)
option(WITH_G2D_ALLOCATOR "Build with G2D allocator support" ON)
option(G2D_INCLUDES "Path to the directory where the g2d.h header is" "/usr/include")
option(G2D_LIBS "Path to the directory where the g2d library is" "/usr/lib")

# Parse OPTIONS
if(ENABLE_ALLOC_STATS_ENABLE)
    set(IMXDMABUFFER_ALLOC_STATS_ENABLED TRUE)
    message(STATUS "Allocation statistics are enabled")
else()
    set(IMXDMABUFFER_ALLOC_STATS_ENABLED FALSE)
endif()

# Universal function to check if a file is in the paths.
function(check_file_in_paths var_name file_name search_path)
    if(search_path)
        find_path(FOUND_PATH
            NAMES ${file_name}
            NO_DEFAULT_PATH
            PATHS ${search_path}
        )

        if(NOT FOUND_PATH)
            message(FATAL_ERROR "Could not find ${file_name} in ${search_path}")
        endif()
    else()
        find_path(FOUND_PATH NAMES ${file_name})
        if(NOT FOUND_PATH)
            message(FATAL_ERROR "${file_name} could not be found in system paths")
        endif()
    endif()

    set(${var_name} ${FOUND_PATH} PARENT_SCOPE)
endfunction()

# Check headers path.
# IMX_SYS_ROOT env var may be set, where the /usr/include is located.
if (DEFINED ENV{IMX_SYS_ROOT})
    set(IMX_SYS_ROOT $ENV{IMX_SYS_ROOT})
    set(IMX_OS_HEADERS_PATH ${IMX_SYS_ROOT}/usr/include)

    set(G2D_INCLUDES ${IMX_SYS_ROOT}/usr/include)
    set(G2D_LIBS ${IMX_SYS_ROOT}/usr/lib)

    include_directories(${IMX_OS_HEADERS_PATH})
    link_directories(${IG2D_LIBSMX_SYS_ROOT}/usr/lib)

    message(STATUS "IMX_OS_HEADERS_PATH is set to ${IMX_OS_HEADERS_PATH}")
endif()

# IMX_KERNEL_SRC env var may be set, where the kernel source is located.
if (DEFINED ENV{IMX_KERNEL_SRC})
    set(IMX_KERNEL_SRC $ENV{IMX_KERNEL_SRC})
    set(IMX_LINUX_HEADERS_PATH ${IMX_KERNEL_SRC}/include)

    include_directories(${IMX_KERNEL_SRC})
    include_directories(${IMX_LINUX_HEADERS_PATH})

    message(STATUS "IMX_LINUX_HEADERS_PATH is set to ${IMX_LINUX_HEADERS_PATH}")
endif()

# Setup DMA-Heap allocator
if(WITH_DMA_HEAP_ALLOCATOR)
    check_file_in_paths(DMA_HEAP_PATH linux/dma-heap.h ${IMX_LINUX_HEADERS_PATH})
    message(STATUS "DMA-Heap allocator is enabled")
    set(IMXDMABUFFER_DMA_HEAP_ALLOCATOR_ENABLED TRUE)

    if (NOT DMA_HEAP_UNCACHED_MEMORY)
        add_definitions(-DIMXDMABUFFER_DMA_HEAP_ALLOCATES_UNCACHED_MEMORY=0)
        set(DMA_HEAP_DEVICE_NODE_PATH "/dev/dma_heap/linux,cma")
    else()
        add_definitions(-DIMXDMABUFFER_DMA_HEAP_ALLOCATES_UNCACHED_MEMORY=1)
        set(DMA_HEAP_DEVICE_NODE_PATH "/dev/dma_heap/linux,cma-uncached")
    endif()

    list(APPEND LIB_SRC ${LIB_SRC_PATH}/imxdmabuffer_dma_heap_allocator.c)
    list(APPEND LIB_HEADERS ${LIB_SRC_PATH}/imxdmabuffer_dma_heap_allocator.h)
else()
    message(STATUS "DMA-Heap allocator is disabled")
    set(IMXDMABUFFER_DMA_HEAP_ALLOCATOR_ENABLED FALSE)
endif()

# Setup G2D allocator
if(WITH_G2D_ALLOCATOR)
    check_file_in_paths(G2D_PATH g2d.h ${G2D_INCLUDES})
    check_file_in_paths(G2D_LIB_PATH libg2d.so ${G2D_LIBS})

    message(STATUS "G2D allocator is enabled")
    set(IMXDMABUFFER_G2D_ALLOCATOR_ENABLED TRUE)

    list(APPEND LIB_SRC ${LIB_SRC_PATH}/imxdmabuffer_g2d_allocator.c)
    list(APPEND LIB_HEADERS ${LIB_SRC_PATH}/imxdmabuffer_g2d_allocator.h)
else()
    message(STATUS "G2D allocator is disabled")
    set(IMXDMABUFFER_G2D_ALLOCATOR_ENABLED FALSE)
endif()

# Build
add_library(${LIB_NAME} SHARED ${LIB_SRC})

set_target_properties(${LIB_NAME} PROPERTIES
    C_STANDARD 99
    C_EXTENSIONS ON
    C_STANDARD_REQUIRED ON
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/imxdmabuffer_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/imxdmabuffer_config.h"
    @ONLY
)

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(${LIB_NAME} PROPERTIES
    VERSION ${IMXDMABUFFER_VERSION}
    SOVERSION ${IMXDMABUFFER_PATCH_VERSION}
    PUBLIC_HEADER ${LIB_SRC_PATH}/imxdmabuffer.h
)

# Install options
install(TARGETS ${LIB_NAME}
    LIBRARY DESTINATION /usr/local/lib
    PUBLIC_HEADER DESTINATION /usr/local/include/imxdmabuffer
)

install(FILES
            ${LIB_HEADERS}
        DESTINATION
            /usr/local/include/imxdmabuffer
)

# DEB Package generation
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
elseif(CPACK_BUILD_CONFIG)
    string(TOUPPER "${CPACK_BUILD_CONFIG}" BUILD_TYPE)
else()
    set(BUILD_TYPE "UNKNOWN")
endif()

set(CPAK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/VishvamCorp/libimxdmabuffer")
set(CPACK_PACKAGE_NAME "libimxdmabuffer")
set(CPACK_PACKAGE_VERSION "${IMXDMABUFFER_VERSION}.${IMXDMABUFFER_PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "Nikita Bulaev <nikita.bulaev@vishwamcorp.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nikita Bulaev <nikita.bulaev@vishwamcorp.com>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

if (NOT ENABLE_ALLOC_STATS_ENABLE)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${BUILD_TYPE}")
else()
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${BUILD_TYPE}-alloc-stats")
endif()

include(CPack)

add_custom_target(deb
    COMMAND ${CMAKE_MAKE_PROGRAM} -G DEB
    DEPENDS ${LIB_NAME}
    COMMENT "Creating deb package"
    VERBATIM
)
