###############################################################################
# Copyright (c) 2018-2024 LunarG, Inc.
# All rights reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Author: LunarG Team
# Description: CMake script for gfxrecon tests
###############################################################################


cmake_minimum_required(VERSION 3.24.0)

if (NOT DEFINED ENV{GFXRECON_NO_TEST_APPS})
    set(GFXRECON_INSTALL_TESTDIR ${CMAKE_INSTALL_PREFIX}/test)
    set(GFXRECON_INSTALL_LAYER_INSTALL_DIR ${GFXRECON_INSTALL_TESTDIR}/vulkan/explicit_layer.d/)

    add_subdirectory(icd)
    if(NOT CMAKE_SYSTEM_NAME MATCHES "Android")
        add_subdirectory(android_mock)
    endif()
    add_subdirectory(test_apps)

    enable_testing()
    set(INSTALL_GTEST OFF)
    find_package(GoogleTest REQUIRED)
    include(GoogleTest)

    set(GFXRECON_TESTCASES
        test_cases/ahb.cpp
        test_cases/debug-utils.cpp
        test_cases/isolate-render-passes.cpp
        test_cases/serialize-compute-and-transfer.cpp
        test_cases/triangle.cpp
        test_cases/multisample-depth.cpp
        test_cases/shader-objects.cpp
        test_cases/pipeline-binaries.cpp
        test_cases/acquired-image.cpp
        test_cases/set-environment.cpp
        test_cases/deep-pnext-chain.cpp
        test_cases/host-image-copy.cpp
        test_cases/sparse-resources.cpp
        test_cases/triangle-extra-device.cpp
        test_cases/json-normalizer.cpp
        test_cases/reruns.cpp
        test_cases/screenshots.cpp
        test_cases/cross-driver.cpp
        test_cases/memory-translation.cpp
        test_cases/bad-files.cpp
        test_cases/malformed-content.cpp
        test_cases/replay-command-line.cpp
        test_cases/layer-faults.cpp
        capture_mutations.cpp
        verify-gfxr.cpp)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
        list(APPEND GFXRECON_TESTCASES test_cases/external-memory-fd.cpp)
        list(APPEND GFXRECON_TESTCASES test_cases/wait-for-present.cpp)
        list(APPEND GFXRECON_TESTCASES test_cases/trigger-trimming.cpp)
    endif()

    add_executable(gfxrecon-testapp-runner ${GFXRECON_TESTCASES})

    # A command-line front for the capture mutations, so a developer can make the file that a
    # case made and run a tool on it by hand.
    add_executable(gfxrecon-capture-mutate capture_mutate_main.cpp capture_mutations.cpp)
    target_include_directories(gfxrecon-capture-mutate PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(gfxrecon-capture-mutate gfxrecon_format gfxrecon_util)
    install(TARGETS gfxrecon-capture-mutate RUNTIME DESTINATION ${GFXRECON_INSTALL_TESTDIR})

    add_dependencies(gfxrecon-testapp-runner gfxrecon-test-launcher VkICD_mock_icd gfxrecon-convert gfxrecon-info gfxrecon-replay VkLayer_gfxreconstruct)
    # stb_image decodes the PNG screenshots for the pixel comparison.
    target_include_directories(gfxrecon-testapp-runner PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${GFXRECON_SOURCE_DIR}/external/stb)
    target_link_libraries(gfxrecon-testapp-runner
            gfxrecon_format
            gfxrecon_util
            GTest::gtest GTest::gtest_main)
    # ctest starts each test case from the install test directory. The harness finds the launcher,
    # the tools, the resources and the known-good files relative to the current directory, so
    # install before you run ctest. Discovery also runs the binary in that directory, so it must
    # wait until ctest runs (PRE_TEST) and not happen at build time, when the install directory
    # does not exist yet. Every case gets the "smoke" label. Later labels select tests by driver
    # and by cost. TEST_LIST names the variable that holds the discovered cases at ctest time.
    # NO_PRETTY_VALUES keeps the name that a parameterized test gives its parameter, such as
    # "Serialized.CorrectGFXR/triangle", instead of the quoted parameter value.
    gtest_discover_tests(gfxrecon-testapp-runner
        WORKING_DIRECTORY ${GFXRECON_INSTALL_TESTDIR}
        DISCOVERY_MODE PRE_TEST
        DISCOVERY_TIMEOUT 60
        NO_PRETTY_VALUES
        TEST_LIST GFXRECON_TESTAPP_TESTS
        PROPERTIES
            LABELS "smoke"
            TIMEOUT 120)

    # One loader environment for every test case. Each entry is NAME=value. The common list selects
    # the capture layer and makes the test apps headless. One list per driver selects the driver.
    # ctest applies the common list and the list for the driver that GFXRECON_TEST_DRIVER names,
    # through test_environment.cmake, so a plain ctest run works and a switch of driver needs no
    # reconfigure. The run-tests scripts export the same lists, so a single app run gets the same
    # environment.
    set(GFXRECON_TEST_APPS_DIR ${GFXRECON_INSTALL_TESTDIR}/test_apps)
    if (WIN32)
        set(GFXRECON_TEST_LAYER_DIR ${GFXRECON_INSTALL_TESTDIR})
        set(GFXRECON_TEST_MOCK_ICD_LIBRARY ${GFXRECON_TEST_APPS_DIR}/VkICD_mock_icd.dll)
    elseif (APPLE)
        set(GFXRECON_TEST_LAYER_DIR ${GFXRECON_INSTALL_LAYER_INSTALL_DIR})
        set(GFXRECON_TEST_MOCK_ICD_LIBRARY ${GFXRECON_TEST_APPS_DIR}/libVkICD_mock_icd.dylib)
    else ()
        set(GFXRECON_TEST_LAYER_DIR ${GFXRECON_INSTALL_LAYER_INSTALL_DIR})
        set(GFXRECON_TEST_MOCK_ICD_LIBRARY ${GFXRECON_TEST_APPS_DIR}/libVkICD_mock_icd.so)
    endif ()
    set(GFXRECON_TEST_ENVIRONMENT
        "VK_LAYER_PATH=${GFXRECON_TEST_LAYER_DIR}"
        "VK_IMPLICIT_LAYER_PATH="
        "VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_gfxreconstruct"
        "GFXRECON_CAPTURE_FILE_TIMESTAMP=false"
        "GFXRECON_TESTAPP_HEADLESS=true")
    if (APPLE)
        list(APPEND GFXRECON_TEST_ENVIRONMENT
            "DYLD_LIBRARY_PATH=${GFXRECON_TEST_LAYER_DIR}:${GFXRECON_TEST_APPS_DIR}"
            "VK_LOADER_LAYERS_DISABLE=~implicit~"
            "VK_LOADER_LAYERS_ENABLE=VK_LAYER_LUNARG_gfxreconstruct"
            "DISPLAY=:0")
    elseif (UNIX)
        list(APPEND GFXRECON_TEST_ENVIRONMENT
            "LD_LIBRARY_PATH=${GFXRECON_TEST_LAYER_DIR}:${GFXRECON_TEST_APPS_DIR}"
            "VK_LOADER_LAYERS_DISABLE=~all~"
            "VK_LOADER_LAYERS_ENABLE=*gfxreconstruct*"
            "DISPLAY=:0")
    else ()
        list(APPEND GFXRECON_TEST_ENVIRONMENT
            "VK_LOADER_LAYERS_DISABLE=~all~"
            "VK_LOADER_LAYERS_ENABLE=*gfxreconstruct*")
    endif ()

    # The mock driver. GFXRECON_TESTAPP_MOCK_ICD lets a test app open the mock library and change
    # its test configuration. A test app treats a missing variable as "no mock".
    # Each driver list also names its driver, so the harness finds known_good/<driver>/.
    set(GFXRECON_TEST_ENVIRONMENT_MOCK
        "GFXRECON_TEST_DRIVER=mock"
        "VK_DRIVER_FILES=${GFXRECON_TEST_APPS_DIR}/VkICD_mock_icd.json"
        "VK_ICD_FILENAMES=${GFXRECON_TEST_APPS_DIR}/VkICD_mock_icd.json"
        "GFXRECON_TESTAPP_MOCK_ICD=${GFXRECON_TEST_MOCK_ICD_LIBRARY}")

    # Lavapipe, the Mesa software rasterizer. The manifest comes from the mesa-vulkan-drivers
    # package on Linux. Another platform, or another install location, sets the cache variable.
    # An empty value makes the lavapipe driver unavailable, and the switch reports that.
    if (CMAKE_SYSTEM_NAME MATCHES "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(GFXRECON_TEST_LAVAPIPE_ICD_JSON_DEFAULT /usr/share/vulkan/icd.d/lvp_icd.aarch64.json)
    elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(GFXRECON_TEST_LAVAPIPE_ICD_JSON_DEFAULT /usr/share/vulkan/icd.d/lvp_icd.x86_64.json)
    else ()
        set(GFXRECON_TEST_LAVAPIPE_ICD_JSON_DEFAULT "")
    endif ()
    set(GFXRECON_TEST_LAVAPIPE_ICD_JSON ${GFXRECON_TEST_LAVAPIPE_ICD_JSON_DEFAULT} CACHE FILEPATH
        "The lavapipe ICD manifest that GFXRECON_TEST_DRIVER=lavapipe selects")
    set(GFXRECON_TEST_ENVIRONMENT_LAVAPIPE
        "GFXRECON_TEST_DRIVER=lavapipe"
        "VK_DRIVER_FILES=${GFXRECON_TEST_LAVAPIPE_ICD_JSON}"
        "VK_ICD_FILENAMES=${GFXRECON_TEST_LAVAPIPE_ICD_JSON}")

    # A cross-driver case captures on one driver and replays on the other, so the harness needs
    # both manifests whatever the current driver is. An empty lavapipe manifest skips those cases.
    list(APPEND GFXRECON_TEST_ENVIRONMENT
        "GFXRECON_TEST_MOCK_ICD_JSON=${GFXRECON_TEST_APPS_DIR}/VkICD_mock_icd.json"
        "GFXRECON_TEST_MOCK_ICD_LIBRARY=${GFXRECON_TEST_MOCK_ICD_LIBRARY}"
        "GFXRECON_TEST_LAVAPIPE_ICD_JSON=${GFXRECON_TEST_LAVAPIPE_ICD_JSON}")

    # ctest reads this file after the discovery file, so it sees GFXRECON_TESTAPP_TESTS.
    configure_file(test_environment.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/test_environment.cmake @ONLY)
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES ${CMAKE_CURRENT_BINARY_DIR}/test_environment.cmake)

    # The same lists as export lines for the run-tests scripts.
    function(gfxrecon_test_environment_exports out_var)
        set(exports "")
        foreach (entry IN LISTS ARGN)
            string(FIND "${entry}" "=" separator)
            string(SUBSTRING "${entry}" 0 ${separator} name)
            math(EXPR value_start "${separator} + 1")
            string(SUBSTRING "${entry}" ${value_start} -1 value)
            if (WIN32)
                string(APPEND exports "$env:${name}=\"${value}\"\n")
            else ()
                string(APPEND exports "export ${name}=\"${value}\"\n")
            endif ()
        endforeach ()
        set(${out_var} "${exports}" PARENT_SCOPE)
    endfunction()
    gfxrecon_test_environment_exports(GFXRECON_TEST_ENVIRONMENT_EXPORTS ${GFXRECON_TEST_ENVIRONMENT})
    gfxrecon_test_environment_exports(GFXRECON_TEST_ENVIRONMENT_MOCK_EXPORTS ${GFXRECON_TEST_ENVIRONMENT_MOCK})
    gfxrecon_test_environment_exports(GFXRECON_TEST_ENVIRONMENT_LAVAPIPE_EXPORTS ${GFXRECON_TEST_ENVIRONMENT_LAVAPIPE})

    # The run-tests script exports the environment and then calls ctest. It needs this build
    # directory and, for a multi-config generator, the configuration. configure_file fills the
    # directory, the ctest path and the exports. file(GENERATE) fills the configuration.
    set(GFXRECON_TEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
    if (APPLE)
        set(GFXRECON_RUN_TESTS_SCRIPT run-tests_macos.sh)
    elseif (UNIX)
        set(GFXRECON_RUN_TESTS_SCRIPT run-tests.sh)
    else ()
        set(GFXRECON_RUN_TESTS_SCRIPT run-tests.ps1)
    endif ()
    configure_file(${GFXRECON_RUN_TESTS_SCRIPT}.in
                   ${CMAKE_CURRENT_BINARY_DIR}/scripts/${GFXRECON_RUN_TESTS_SCRIPT}.configured
                   @ONLY)
    file(GENERATE
         OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/scripts/$<CONFIG>/${GFXRECON_RUN_TESTS_SCRIPT}
         INPUT ${CMAKE_CURRENT_BINARY_DIR}/scripts/${GFXRECON_RUN_TESTS_SCRIPT}.configured)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/scripts/$<CONFIG>/${GFXRECON_RUN_TESTS_SCRIPT}
            DESTINATION ${GFXRECON_INSTALL_TESTDIR})

    install(TARGETS gfxrecon-testapp-runner RUNTIME DESTINATION ${GFXRECON_INSTALL_TESTDIR})
    # The reference files are binaries that change with every header update, and every image
    # golden adds to them. test/README.md states the budget. The check runs at configure time, so a
    # file that breaks the budget fails the build before it reaches a pull request.
    set(GFXRECON_KNOWN_GOOD_BUDGET_BYTES 5242880)
    file(GLOB_RECURSE known_good_files LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/known_good/*)
    set(known_good_bytes 0)
    foreach (known_good_file IN LISTS known_good_files)
        file(SIZE ${known_good_file} known_good_file_bytes)
        math(EXPR known_good_bytes "${known_good_bytes} + ${known_good_file_bytes}")
    endforeach ()
    if (known_good_bytes GREATER GFXRECON_KNOWN_GOOD_BUDGET_BYTES)
        message(FATAL_ERROR "test/known_good is ${known_good_bytes} bytes, over the budget of "
                            "${GFXRECON_KNOWN_GOOD_BUDGET_BYTES} bytes. See test/README.md.")
    endif ()
    install(DIRECTORY known_good DESTINATION ${GFXRECON_INSTALL_TESTDIR})
    install(TARGETS gfxrecon-convert RUNTIME DESTINATION ${GFXRECON_INSTALL_TESTDIR})
    install(TARGETS gfxrecon-info RUNTIME DESTINATION ${GFXRECON_INSTALL_TESTDIR})
    install(TARGETS gfxrecon-replay RUNTIME DESTINATION ${GFXRECON_INSTALL_TESTDIR})
    if (UNIX)
        install(FILES $<TARGET_FILE_DIR:VkLayer_gfxreconstruct>/VkLayer_gfxreconstruct.json DESTINATION ${GFXRECON_INSTALL_LAYER_INSTALL_DIR})
        install(TARGETS VkLayer_gfxreconstruct RUNTIME DESTINATION ${GFXRECON_INSTALL_LAYER_INSTALL_DIR} LIBRARY DESTINATION ${GFXRECON_INSTALL_LAYER_INSTALL_DIR})
    else ()
        install(FILES $<TARGET_FILE_DIR:VkLayer_gfxreconstruct>/VkLayer_gfxreconstruct.json DESTINATION ${GFXRECON_INSTALL_TESTDIR})
        install(TARGETS VkLayer_gfxreconstruct RUNTIME DESTINATION ${GFXRECON_INSTALL_TESTDIR} LIBRARY DESTINATION ${GFXRECON_INSTALL_TESTDIR})
    endif ()

endif ()
