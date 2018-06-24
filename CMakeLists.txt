cmake_minimum_required (VERSION 3.0)
set (PROJECT_NAME vRt)

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
    #"./include/vulkan/*/*.c"
    #"./include/vulkan/*/*.cpp"
    #"./include/tinyrender/*.c"
    #"./include/tinyrender/*.cpp"
    #"./include/tinyrender/*/*.c"
    #"./include/tinyrender/*/*.cpp"
    "./include/vRt/*.c"
    "./include/vRt/*.cpp"
    "./include/vRt/*/*.c"
    #"./include/vRt/*/*.cpp"
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

if(MSVC OR MSVS OR MSYS OR MINGW)
    set(DEFINES ${DEFINES} -DOS_WIN -DWIN32) 
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
set (LIB_DIR "${DEP_DIR}/lib"     "${DEP_DIR}/src/lib"     "${VULKAN_DIR}")
set (INC_DIR "${DEP_DIR}/include" "${DEP_DIR}/src/include" "${DEP_DIR}/src/" "${DEP_DIR}/src/base/" "${DEP_DIR}/src/base/imgui" "${PROJECT_SOURCE_DIR}")
#set (LIBS glfw3 ${Vulkan_LIBRARY})
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

if (MSVC)
    set (CMAKE_CXX_FLAGS "/std:c++latest /EHsc")
else()
    set (CMAKE_CXX_FLAGS "-std=c++2a")
    set (CMAKE_C_FLAGS "-std=c1x")
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

