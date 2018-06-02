cmake_minimum_required (VERSION 3.0)
set (PROJECT_NAME vRt)
set (APPLICATION_NAME vRt-dev)
project (${PROJECT_NAME})

if (COMPILER_ID MATCHES "MSVC")
  set(MSVC_${lang}_ARCHITECTURE_ID "${ARCHITECTURE_ID}")
endif()

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
    
    "./include/imgui/*.c"
    "./include/imgui/*.cpp"
    "./include/imgui/*/*.c"
    "./include/imgui/*/*.cpp"
    "./include/vulkan/*.c"
    "./include/vulkan/*.cpp"
    "./include/vulkan/*/*.c"
    "./include/vulkan/*/*.cpp"
    #"./include/tinyrender/*.c"
    #"./include/tinyrender/*.cpp"
    #"./include/tinyrender/*/*.c"
    #"./include/tinyrender/*/*.cpp"
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

set (DEFINES
    -DNOMINMAX
    -DGLM_ENABLE_EXPERIMENTAL
    -DGLFW_INCLUDE_NONE
    -DUSE_CIMG
    -DEXPERIMENTAL_GLTF
    -DNSM=vt
)

if(MSVC OR MSVS OR MSYS OR MINGW)
    set(DEFINES ${DEFINES} -DOS_WIN) 
endif()

if(UNIX AND NOT APPLE)
    set(DEFINES ${DEFINES} -DOS_LNX) 
endif()


set (CMAKE_MODULE_PATH 
    "${DEP_DIR}/modules" 
    "${CMAKE_MODULE_PATH}/modules" 
    "${PROJECT_SOURCE_DIR}/modules/" )

find_package(Vulkan QUIET)
get_filename_component(VULKAN_DIR ${Vulkan_LIBRARY} DIRECTORY )

set (DEP_DIR "${PROJECT_SOURCE_DIR}")
set (LIB_DIR "${DEP_DIR}/lib"     "${DEP_DIR}/testapps/lib" "${VULKAN_DIR}")
set (INC_DIR "${DEP_DIR}/include" "${DEP_DIR}/testapps/include" "${DEP_DIR}/testapps/" "${DEP_DIR}/testapps/imgui" "${PROJECT_SOURCE_DIR}")
set (LIBS glfw3)

# if OptiX was has
if (OptiX_INSTALL_DIR AND ENABLE_OPTIX_DENOISER) 
set (INC_DIR "${INC_DIR}" "${OptiX_INCLUDE}" ${CUDA_INCLUDE_DIRS})
set (LIB_DIR "${LIB_DIR}" "${OptiX_INSTALL_DIR}/lib")
set (LIBS ${LIBS} ${optix_LIBRARY} ${optix_prime_LIBRARY} ${optixu_LIBRARY} ${CUDA_LIBRARIES})
if (ENABLE_OPTIX_DENOISER) 
set (DEFINES ${DEFINES} -DOPTIX_DENOISER_HACK)
endif()
endif()


#set (CMAKE_C_EXTENSIONS ON)
#set (CMAKE_CXX_EXTENSIONS ON)
#set (CMAKE_C_STANDARD 11)
#set (CMAKE_CXX_STANDARD 20)
#set (CMAKE_CXX_EXTENSIONS ON)
#set (CMAKE_CXX_STANDARD 17)

if (MSVC)
    set (CMAKE_CXX_FLAGS "/W1 /EHa /openmp /std:c++latest /Zc:rvalueCast /arch:AVX2 /Oi /Ot /Oy /GT /GL /Gy /GR")
else()
    set (CMAKE_CXX_FLAGS "-W1 -std=c++2a -march=skylake -mtune=skylake -Ox")
    set (CMAKE_C_FLAGS "-W1 -std=c1x")
endif()


set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS}")

set (INCLUDE_LIST 
    ${INC_DIR}
    ${Vulkan_INCLUDE_DIRS}
)

add_definitions(${DEFINES})
link_directories(${LIB_DIR})
include_directories(${INCLUDE_LIST})
file (GLOB RSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES_LIST})

add_executable(${APPLICATION_NAME} src/examples/helloTriangle.cpp ${RSOURCES})
target_link_libraries(${APPLICATION_NAME} ${LIBS})

foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
