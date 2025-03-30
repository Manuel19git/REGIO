cmake_minimum_required(VERSION 3.24)

# Project name and C++ standard settings
project(REGIO VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(W_ARCH "x86")
endif()
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(W_ARCH "x64")
endif()

# Function to search for fxc.exe recursively
function(find_fxc_exe)
    # Set the initial search directory to the Windows Kits folder
    set(search_dir "C:/Program Files (x86)/Windows Kits")

    # Use GLOB_RECURSE to find all instances of fxc.exe
    file(GLOB_RECURSE fxc_paths "${search_dir}/*/bin/*/${W_ARCH}/fxc.exe")

    # Check if we found any paths
    if (fxc_paths)
        list(GET fxc_paths 0 fxc_path)
        set(FXC_PATH "${fxc_path}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "fxc.exe not found. Please ensure the Windows SDK is installed.")
    endif()
endfunction()

find_fxc_exe()

# Output the found path
message(STATUS "Found fxc.exe at ${FXC_PATH}")

# Note from cmake creators:
# "We do not recommend using GLOB to collect a list of source files from your source tree. If no 
# CMakeLists.txt file changes when a source is added or removed then the generated build system 
# cannot know when to ask CMake to regenerate"
# https://cmake.org/cmake/help/latest/command/file.html#glob

# Add executable target
file(GLOB SOURCES "REGIO/*.cpp") # find files matching the GLOBbing-expression and store it 
file(GLOB HEADERS "REGIO/*.h")
file(GLOB EFFECTS "REGIO/*.fx")

# Configure shaders
set(SHADERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/REGIO/VertexShader.hlsl"
    "${CMAKE_CURRENT_SOURCE_DIR}/REGIO/PixelShader.hlsl"
    "${CMAKE_CURRENT_SOURCE_DIR}/REGIO/SimplePixelShader.hlsl"
    "${CMAKE_CURRENT_SOURCE_DIR}/REGIO/SkyPixelShader.hlsl"
    "${CMAKE_CURRENT_SOURCE_DIR}/REGIO/common.hlsl"
)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/REGIO/VertexShader.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/REGIO/PixelShader.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/REGIO/SimplePixelShader.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/REGIO/SkyPixelShader.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/REGIO/common.hlsl PROPERTIES ShaderType "ignore")
set_source_files_properties(${SHADERS} PROPERTIES ShaderModel "4_0")

# This option tells Visual Studio to treat this file as header, thus not compiling it
set_source_files_properties(${SHADERS} PROPERTIES HEADER_FILE_ONLY ON)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${EFFECTS} ${SHADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add the submodule directory to the CMake build
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Assimp as a static library")
add_subdirectory(external/assimp)
add_subdirectory(external/DirectXTK)
add_subdirectory(external/FX11)

# Link the library to your target
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE DirectXTK)
target_link_libraries(${PROJECT_NAME} PRIVATE Effects11) # name defined in their own CMakeLists.txt in add_library

# Optional, but include if needed
target_include_directories(${PROJECT_NAME} PRIVATE external/assimp/include)
target_include_directories(${PROJECT_NAME} PRIVATE external/DirectXTK/Inc)
target_include_directories(${PROJECT_NAME} PRIVATE external/FX11/inc)


# Check the generator used
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    # Add Visual Studio specific options or configurations
    message(STATUS "Configuring for Visual Studio")

    # Define custom filters
    source_group("Shaders" FILES ${SHADERS})
    source_group("Effects" FILES ${EFFECTS})

    # Set the shader output directory
    set(EFFECT_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/effects)
    set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

    # Create the output directory
    file(MAKE_DIRECTORY ${EFFECT_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    # Compile each effect file
    foreach(EFFECT ${EFFECTS})
        get_filename_component(EFFECT_NAME ${EFFECT} NAME_WE) # Get the effect base name without extension

        # Define output file name
        set(OUTPUT_FILE ${EFFECT_OUTPUT_DIR}/${EFFECT_NAME}.fxo)
 
        # Add a custom command to compile the effect file
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${FXC_PATH} /T fx_5_0 /E Simple /Fo "${OUTPUT_FILE}" "${EFFECT}"
            DEPENDS "${EFFECT}" # Depend on the original shader file
            COMMENT "Compiling effect file ${EFFECT} to ${OUTPUT_FILE}"
        )

        # Create a custom target that depends on the output file
        add_custom_target(compile_${EFFECT_NAME} ALL DEPENDS ${OUTPUT_FILE})
        add_dependencies(${PROJECT_NAME} compile_${EFFECT_NAME})
    endforeach()

    # Compile each shader file
    foreach(SHADER ${SHADERS})
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE) # Get the shader base name without extension

        # Define output file name
        set(OUTPUT_FILE ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.cso)

        # Get shader type and model
        get_source_file_property(shadertype ${SHADER} ShaderType)
        get_source_file_property(shadermodel ${SHADER} ShaderModel)

	if (${shadertype} STREQUAL "ignore")
	    continue()
	endif()
        
        # Add a custom command to compile the shader
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${FXC_PATH} /nologo /Emain /T ${shadertype}_${shadermodel} /Zi /Fo "${OUTPUT_FILE}" "${SHADER}"
            #COMMAND ${FXC_PATH} /T fx_5_0 /E Simple /Fo "${OUTPUT_FILE}" "${SHADER}"
            DEPENDS "${SHADER}" # Depend on the original shader file
            COMMENT "Compiling HLSL shader ${SHADER} to ${OUTPUT_FILE}"
        )

        # Create a custom target that depends on the output file
        add_custom_target(compile_${SHADER_NAME} ALL DEPENDS ${OUTPUT_FILE})
        add_dependencies(${PROJECT_NAME} compile_${SHADER_NAME})
    endforeach()
    
    # Example: Set compiler options for Visual Studio
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)  # Set warning level to 4

    # For it to find the windows main
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

    # You can also define any Visual Studio specific flags or options here
    add_definitions(-DUNICODE -D_UNICODE)
    
endif()

# Install
install(TARGETS ${PROJECT_NAME}
    DESTINATION ${PROJECT_NAME}
)

# Move shaders and assets to specific folder
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/
	DESTINATION shaders
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/output/
	DESTINATION output
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data/
	DESTINATION Data
)


