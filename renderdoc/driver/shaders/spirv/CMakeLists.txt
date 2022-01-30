set(sources
    glslang_compile.cpp
    glslang_compile.h
    spirv_common.cpp
    spirv_common.h
    spirv_editor.cpp
    spirv_editor.h
    spirv_gen.cpp
    spirv_gen.h
    spirv_op_helpers.h
    spirv_compile.cpp
    spirv_compile.h
    spirv_debug_setup.cpp
    spirv_debug_glsl450.cpp
    spirv_debug.cpp
    spirv_debug.h
    spirv_reflect.cpp
    spirv_reflect.h
    spirv_processor.cpp
    spirv_processor.h
    spirv_disassemble.cpp
    spirv_stringise.cpp
    var_dispatch_helpers.h)

add_definitions(-DAMD_EXTENSIONS)
add_definitions(-DNV_EXTENSIONS)

set_property(SOURCE ${glslang_sources}
    PROPERTY COMPILE_FLAGS "-Wno-ignored-qualifiers -Wno-strict-aliasing -Wno-unreachable-code-break")

# GCC 7.0 and above needs -Wno-implicit-fallthrough on these files
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.9)
    set_property(SOURCE
        ${glslang_dir}/glslang/MachineIndependent/reflection.cpp
        ${glslang_dir}/glslang/MachineIndependent/Intermediate.cpp
        ${glslang_dir}/glslang/MachineIndependent/ParseHelper.cpp
        APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-implicit-fallthrough")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_property(SOURCE ${glslang_sources}
        APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-unknown-warning-option")

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.9)
        set_property(SOURCE ${glslang_dir}/glslang/MachineIndependent/iomapper.cpp
            APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-deprecated-copy")
    endif()
endif()

add_library(rdoc_spirv OBJECT ${sources})
target_compile_definitions(rdoc_spirv ${RDOC_DEFINITIONS})
target_include_directories(rdoc_spirv ${RDOC_INCLUDES} ${glslang_dir})
