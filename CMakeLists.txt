cmake_minimum_required(VERSION 3.15)
project(NewHanafuda)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(imgui)
include_directories(imgui-sfml-2.1)
include_directories(/usr/local/include)

# set the path to the library folder
link_directories(/usr/local/lib)

find_library(sfml-audio sfml-graphics sfml-window sfml-system)



add_executable(NewHanafuda
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        ai.cpp
        ai.h
        BaseUI.cpp
        BaseUI.h
        Card.cpp
        Card.h
        common_def.h
        ContentManager.cpp
        ContentManager.h
        external_declare.cpp
        external_declare.h
        Game.cpp
        Game.h
        imgui-SFML.cpp
        main.cpp
        Panel.cpp
        Panel.h
        PhaseState.cpp
        PhaseState.h
        Player.cpp
        Player.h
        utilities.cpp
        utilities.h)

# link the libraries to the executable
target_link_libraries (NewHanafuda
        sfml-audio
        sfml-graphics
        sfml-window
        sfml-system)

IF(APPLE)
    SET(CMAKE_CXX_LINK_FLAGS "-framework OpenGL -framework GLUT")
ENDIF(APPLE)

# Copy files from source directory to destination directory, substituting any
# variables.  Create destination directory if it does not exist.
macro(configure_files srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    #make_directory(${destDir})

    file(GLOB_RECURSE templateFiles RELATIVE ${srcDir} ${srcDir}/*)

    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        message(STATUS ${srcTemplatePath})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    COPYONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)

configure_files(${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res)
#configure_file(res bin/res COPYONLY)

#add_custom_target(copy-runtime-files ALL
#        COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res
#        DEPENDS ${NewHanafuda})
