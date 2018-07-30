cmake_minimum_required (VERSION 3.10)
set (PROJECT_NAME vRt)
project (${PROJECT_NAME})


# Architecture specific
if (COMPILER_ID MATCHES "MSVC")
  set(MSVC_${lang}_ARCHITECTURE_ID "${ARCHITECTURE_ID}")
endif()


# Set source directories
set(SOURCES_LIST 
    "./src/include/*/*.hpp"
    "./src/include/*/*.hh"
    "./src/include/*/*.h"
    "./src/include/*/*/*.hpp"
    "./src/include/*/*/*.hh"
    "./src/include/*/*/*.h"
    "./src/include/*.hpp"
    "./src/include/*.hh"
    "./src/include/*.h"
    "./src/imgui/*.hpp"
    "./src/imgui/*.hh"
    "./src/imgui/*.h"
    "./src/imgui/imgui_draw.cpp"
    "./src/imgui/imgui.cpp"
    "./src/examples/*.hpp"
    
    "./src/base/*.hpp"
    "./src/base/*.cpp"
    "./src/base/*.c"
    "./src/base/*.h"
    
    "./src/*/*/*.hpp"
    "./src/*/*/*.cpp"
    "./src/*/*/*.c"
    "./src/*/*/*.h"
    
    "./include/imgui/*.c"
    "./include/imgui/*.cpp"
    "./include/imgui/*/*.c"
    "./include/imgui/*/*.cpp"
    "./include/vulkan/*.c"
    "./include/vulkan/*.cpp"
    "./include/vulkan/*/*.c"
    "./include/vulkan/*/*.cpp"

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
    "./include/*/*/*/*/*/*.h"
    "./include/*/*/*/*/*/*.hpp"
    "./include/*/*/*/*/*/*.inl"
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
    -DVRT_ENABLE_VEZ_INTEROP
)


# Set MSVC/Windows specific 
# Linux TODO 
if (MSVC OR MSVS OR MSYS OR MINGW)
    set(DEFINES ${DEFINES} -DOS_WIN -DWIN32) 
endif()

# Set Linux specific
if (UNIX OR LINUX)
    set(DEFINES ${DEFINES} -DOS_LNX) 
endif()


# set libs and include dirs of projects
set (DEP_DIR "${PROJECT_SOURCE_DIR}")
set (LIB_DIR "${DEP_DIR}/lib"     "${DEP_DIR}/src/lib")
set (INC_DIR "${DEP_DIR}/include" "${DEP_DIR}/src/include" "${DEP_DIR}/src/" "${DEP_DIR}/src/base/" "${DEP_DIR}/src/base/imgui" "${PROJECT_SOURCE_DIR}")

# set required libs
#set (LIBS glfw3 VEZ)
set (LIBS glfw3 VEZ vulkan-1)

# set GCC8 (Linux) libs
# MingW-W64 TODO
if (UNIX OR LINUX)
find_package(X11 QUIET)
    if (X11_FOUND)
        set (LIBS ${LIBS} ${X11_LIBRARIES})
        set (INC_DIR ${INC_DIR} ${X11_INCLUDE_DIR})
    endif()
    #set (LIBS ${LIBS} -static-libgcc -static-libstdc++ -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl)
    set (LIBS ${LIBS} -static-libgcc -static-libstdc++ -lglfw3 -lX11 -lXrandr -lpthread -ldl)
endif()


# set language specific
set (CMAKE_CXX_EXTENSIONS ON)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_C_STANDARD 11)


# compiler specific options
if (MSVC)
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -march=skylake -fpermissive -lpthread -w") # Clang in Visual Studio 2017
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /arch:AVX2 /permissive- /Ot /W3 /NODEFAULTLIB:library") # Native compiler of Visual Studio 2017
    #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}"")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -march=skylake -fpermissive -lpthread -w")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c1x -march=skylake -fpermissive -lpthread -w")
endif()


# set optimizations for build types
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS} -O0")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS}")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS} -O0")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS}")


# set default compiler options
add_definitions(${DEFINES})
link_directories(${LIB_DIR})
include_directories(${INC_DIR})
file (GLOB RSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES_LIST})


# make helloTriangle
set (APPLICATION_NAME helloTriangle)
add_executable(${APPLICATION_NAME} src/examples/helloTriangle.cpp ${RSOURCES})
target_link_libraries(${APPLICATION_NAME} ${LIBS})
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()


# make gltfBasic
set (APPLICATION_NAME gltfBasic)
add_executable(${APPLICATION_NAME} src/examples/gltfBasic.cpp ${RSOURCES})
target_link_libraries(${APPLICATION_NAME} ${LIBS})
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()


# make static library
set (LIBRARY_NAME vRt)
add_library(${LIBRARY_NAME} include/vRt/vRt_lib.cpp ${RSOURCES})
target_link_libraries(${LIBRARY_NAME} ${LIBS})
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
