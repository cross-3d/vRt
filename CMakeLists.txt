cmake_minimum_required (VERSION 3.10)
set (PROJECT_NAME vRt)
project (${PROJECT_NAME})

# Architecture specific
if (COMPILER_ID MATCHES "MSVC")
  set(MSVC_${lang}_ARCHITECTURE_ID "${ARCHITECTURE_ID}")
endif()

# Import modules 
set (DEP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEP_DIR}" "${DEP_DIR}/lib/cmake/")
find_package (Vulkan REQUIRED)
find_package (glfw3  REQUIRED)

# Set source directories
set(SOURCES_LIST 
    "./src/*/*.hpp"
    "./src/*/*.hxx"
    "./src/*/*.inl"
    
    "./src/*/*/*.hpp"
    "./src/*/*/*.hxx"
    "./src/*/*/*.inl"
    
    "./include/vulkan/*.c"
    "./include/vulkan/*.cpp"
    "./include/vulkan/*/*.c"
    "./include/vulkan/*/*.cpp"
    "./include/vRt/*.c"
    "./include/vRt/*.cpp"

    "./include/*.h"
    "./include/*.hpp"
    "./include/*.inl"
    
    "./include/*/*.h"
    "./include/*/*.hpp"
    "./include/*/*.inl"
    
    "./include/*/*/*.h"
    "./include/*/*/*.hpp"
    "./include/*/*/*.inl"
    
    "./include/*/*/*/*.h"
    "./include/*/*/*/*.hpp"
    "./include/*/*/*/*.inl"
    
    "./include/*/*/*/*/*.h"
    "./include/*/*/*/*/*.hpp"
    "./include/*/*/*/*/*.inl"
)

# Set definition
set (DEFINES
    -DNOMINMAX
    -D_CRT_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -DGLM_FORCE_SWIZZLE
    -DGLM_ENABLE_EXPERIMENTAL
    -DGLFW_INCLUDE_NONE
    -DEXPERIMENTAL_GLTF
    -DNSM=vte
    -DTINY_RENDERER_VK
)

# Set MSVC/Windows specific 
if (MSVC OR MSVS OR MSYS OR MINGW)
    set(DEFINES ${DEFINES} -DOS_WIN -DWIN32) 
endif()

# Set Linux specific
if (UNIX OR LINUX)
    set(DEFINES ${DEFINES} -DOS_LNX) 
endif()

# set libs and include dirs of projects
set (LIB_DIR "${DEP_DIR}/lib")
set (INC_DIR ${Vulkan_INCLUDE_DIR}
    "${DEP_DIR}/include" 
    "${DEP_DIR}/src/" 
    "${DEP_DIR}/src/base/" 
    "${DEP_DIR}")

# set required libs
set (LIBS glfw3 ${Vulkan_LIBRARY})

# set GCC8 (Linux) libs
# MingW-W64 TODO
if (UNIX OR LINUX)
find_package(X11 QUIET)
    if (X11_FOUND)
        set (LIBS ${LIBS} ${X11_LIBRARIES})
        set (INC_DIR ${INC_DIR} ${X11_INCLUDE_DIR})
    endif()
    set (LIBS ${LIBS} -static-libgcc -static-libstdc++ -lglfw3 -lX11 -lXrandr -lpthread -ldl)
endif()

# set language specific
set (CMAKE_CXX_EXTENSIONS 1)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_STANDARD 11)

# compiler specific options
if (MSVC) # Clang in Visual Studio 2017
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /arch:AVX2 /fp:strict /permissive- /Ot /W3 /NODEFAULTLIB:library") # Native compiler of Visual Studio 2017
    set (CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -march=skylake -fpermissive -lpthread -w -mfpmath=sse,387 -mpc32 -mavx2 -mrecip=all -msse2avx")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c1x   -march=skylake -fpermissive -lpthread -w -mfpmath=sse,387 -mpc32 -mavx2 -mrecip=all -msse2avx")
    if (WIN32)
        set (CMAKE_CXX_FLAGS "${CMAK_CXX_FLAGS} -Wa,-mbig-obj")
        set (CMAKE_C_FLAGS "${CMAK_C_FLAGS} -Wa,-mbig-obj")
    endif()
endif()

# set optimizations for build types
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# set default compiler options
add_definitions(${DEFINES})
link_directories(${LIB_DIR})
include_directories(${INC_DIR})
file (GLOB RSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES_LIST})

# make gltfPBR
set (APPLICATION_NAME gltfPBR)
add_executable(${APPLICATION_NAME} src/examples/gltfPBR.cpp src/examples/main.cpp ${RSOURCES})
target_link_libraries(${APPLICATION_NAME} ${LIBS})

# add includes to MSVC explorer 
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
