cmake_minimum_required(VERSION 3.4)

# findGLFW helper function
function(_findGLFW3_vsbinary target)

    FILE(GLOB GLFW_VC_LIB_DIRS "${GLFW_DIR}/lib-vc*")
    
    if(NOT GLFW_VC_LIB_DIRS)
        message(FATAL_ERROR "GLFW_DIR contains neither a CMakeLists.txt nor pre-compiled libraries for visual studio")
    endif()

    set(GLFW_INCLUDE_DIRS "${GLFW_DIR}/include/")

    function(addMSVCPreCompiled version)
        if(NOT EXISTS "${GLFW_DIR}/lib-vc${version}/glfw3.lib")
        message(FATAL_ERROR "Missing required visual studio pre-compiled library!")
        endif()
        set(GLFW_LIBRARIES "${GLFW_DIR}/lib-vc${version}/glfw3.lib" PARENT_SCOPE)
    endfunction()

    if(MSVC_VERSION GREATER_EQUAL 1920)
        addMSVCPreCompiled("2019")
    elseif(MSVC_VERSION GREATER_EQUAL 1910)
        addMSVCPreCompiled("2017")
    elseif(MSVC_VERSION GREATER_EQUAL 1900)
        addMSVCPreCompiled("2015")
    elseif(MSVC_VERSION LESS 1900)
        message(FATAL_ERROR "Visual Studio version is less than minimum (VS 2015)")
    endif()

    set(GLFW_LIBRARIES ${GLFW_LIBRARIES} PARENT_SCOPE)
    message(STATUS "Set GLFW_LIBRARIES: ${GLFW_LIBRARIES}")

endfunction(_findGLFW3_vsbinary)

# findGLFW helper function
function(_findGLFW3_sourcepkg target)

    option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
    option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
    option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)

    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
    else()
        add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
    endif()

    set(GLFW_LIBRARIES glfw PARENT_SCOPE)

endfunction(_findGLFW3_sourcepkg)


# Find and add GLFW3 using find_package or environment variable 
function(findGLFW3 target)

    find_package(glfw3 QUIET)

    if(glfw3_FOUND)

        # Include paths are added automatically by the glfw3 find_package
        target_link_libraries(${CMAKE_PROJECT_NAME} glfw)

    elseif(DEFINED ENV{GLFW_DIR})

        set(GLFW_DIR "$ENV{GLFW_DIR}")
        message(STATUS "GLFW environment variable found. Attempting use...")

        if(NOT EXISTS "${GLFW_DIR}/CMakeLists.txt" AND WIN32)
            _findGLFW3_vsbinary(target) 
        elseif(EXISTS "${GLFW_DIR}/CMakeLists.txt")
            _findGLFW3_sourcepkg(target)
        else()
            message(FATAL_ERROR "GLFW environment variable 'GLFW_DIR' found, but points to a directory which is not a source package containing 'CMakeLists.txt'.")
        endif()

        if(GLFW_LIBRARIES)
            target_include_directories(${target} PUBLIC "${GLFW_DIR}/include")
            target_link_libraries(${target} "${GLFW_LIBRARIES}")
        else()
            message(FATAL_ERROR "Internal Error! GLFW_LIBRARIES variable did not get set! Contact your TA, this is their fault.")
        endif()

    else()
        message(FATAL_ERROR "glfw3 could not be found through find_package or environment varaible 'GLFW_DIR'! glfw3 must be installed!")
    endif()

endfunction(findGLFW3)

# Find and add GLM using find_package or environment variable 
function(findGLM target)

    find_package(glm QUIET)

    if(NOT glm_FOUND)
        set(GLM_INCLUDE_DIRS "$ENV{GLM_INCLUDE_DIR}")
        if(NOT GLM_INCLUDE_DIRS)
            message(WARNING "glm could not be found through find_package or environment variable 'GLM_INCLUDE_DIR'! glm must be installed!")
            return()
        endif()
    endif()

    target_include_directories(${target} PUBLIC "${GLM_INCLUDE_DIRS}")
    
endfunction(findGLM)


# findAssimp helper function
function(_findAssimp_sourcepkg target)

    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_subdirectory(${ASSIMP_DIR} ${ASSIMP_DIR}/build/include/assimp)
    else()
        add_subdirectory(${ASSIMP_DIR} ${ASSIMP_DIR}/debug)
    endif()

    set(ASSIMP_LIBRARIES assimp PARENT_SCOPE)

endfunction(_findAssimp_sourcepkg)

function(_findAssimp_vsbinary target)
    set(ASSIMP_INCLUDE_DIRS "${ASSIMP_DIR}/include/")
    if(NOT EXISTS "${ASSIMP_DIR}/build/code/Debug/assimp-vc140-mt.lib")
#        message(FATAL_ERROR "Missing required visual studio pre-compiled library!")
    endif()
    set(ASSIMP_LIBRARIES "${ASSIMP_DIR}/build/code/Debug/assimp-vc140-mt.lib" PARENT_SCOPE)
    message(STATUS "Set ASSIMP_LIBRARIES: ${ASSIMP_LIBRARIES}")
endfunction(_findAssimp_vsbinary)

#find and add assimp dir using environment variable
function(findAssimp target)

    find_package(assimp QUIET)

    if(DEFINED ENV{ASSIMP_DIR})

        set(ASSIMP_DIR "$ENV{ASSIMP_DIR}")
        message(STATUS "Assimp environment variable found. Attempting use...")

        if(WIN32)
            _findAssimp_vsbinary(target) 
        else()
            message(FATAL_ERROR "Assimp environment variable 'ASSIMP_DIR' found, but points to a directory which is not a source package containing 'CMakeLists.txt'.")
        endif()

        if(ASSIMP_LIBRARIES)
            target_include_directories(${target} PUBLIC "${ASSIMP_DIR}/include")
            target_include_directories(${target} PUBLIC "${ASSIMP_DIR}/build/include")
#            target_link_libraries(${target} "${ASSIMP_LIBRARIES}")
            target_link_libraries(${target} "C:/bins/assimp-4.1.0/build/code/libassimp.dll")
            message(STATUS "${ASSIMP_LIBRARIES}")
        else()
            message(FATAL_ERROR "Internal Error! ASSIMP_LIBRARIES variable did not get set!")
        endif()

    else()
        message(FATAL_ERROR "Assimp could not be found through find_package or environment varaible 'ASSIMP_DIR'! Assimp must be installed!")
    endif()

endfunction(findAssimp)

function(findBullet target)
#    find_package(bullet)
    include_directories("C:/bins/vcpkg-master/packages/bullet3_x86-windows/include")
    include_directories("C:/bins/vcpkg-master/packages/bullet3_x86-windows/include/bullet")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/lib/Bullet3Common.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/lib/BulletCollision.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/lib/BulletDynamics.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/lib/BulletInverseDynamics.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/lib/BulletSoftBody.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/lib/LinearMath.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/debug/lib/Bullet3Common_Debug.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/debug/lib/BulletCollision_Debug.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/debug/lib/BulletDynamics_Debug.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/debug/lib/BulletInverseDynamics_Debug.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/debug/lib/BulletSoftBody_Debug.lib")
    target_link_libraries(${target} "C:/bins/vcpkg-master/packages/bullet3_x86-windows/debug/lib/LinearMath_Debug.lib")
endfunction(findBullet)