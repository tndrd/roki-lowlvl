cmake_minimum_required(VERSION 3.8)

include(Misc/config)

set(MODULE_NAME "rokilowlvl")

function(ASSERT_VAR VAR)
  if (NOT DEFINED ${VAR})
    message(FATAL_ERROR ${VAR} is not defined)
  endif()
endfunction()

function(PROPAGATE_VAR VAR)
  ASSERT_VAR(${VAR})
  add_compile_definitions(${VAR}=${${VAR}})
endfunction()

ASSERT_VAR(MODULE_NAME)
ASSERT_VAR(TEST_EXECUTABLE)
ASSERT_VAR(CURRENT_VERSION)

PROPAGATE_VAR(SERIAL_BAUDRATE_ENV)
PROPAGATE_VAR(SERIAL_STOPBITS_ENV)
PROPAGATE_VAR(SERIAL_PARITY_ENV)
PROPAGATE_VAR(SERIAL_TIMEOUT_ENV)
PROPAGATE_VAR(PYTHON_MODULE_NAME)

set(DIAG_SCRIPTS_DIR /usr/lib/${MODULE_NAME}/diagnostic/Scripts)
add_compile_definitions(DIAG_SCRIPTS_DIR=${DIAG_SCRIPTS_DIR})

set(DIAG_SUITES_DIR /usr/lib/${MODULE_NAME}/diagnostic/Suites)
add_compile_definitions(DIAG_SUITES_DIR=${DIAG_SUITES_DIR})

# Required to build shared python library
add_compile_options(-fPIC -g)

project(roki-lowlvl)

add_subdirectory(Deps/pybind11)
add_subdirectory(Deps/googletest)
add_subdirectory(Deps/roki-mb-daemon)
add_subdirectory(Deps/roki-mb-interface)

include_directories(Inc/)
add_library(CheckDefs1 Src/CheckDefs.cpp)

# Motherboard factory
add_library(MbFactory Src/MbFactory.cpp)
target_link_libraries(MbFactory PUBLIC MotherboardAdapter DaemonTools CheckDefs1)
target_include_directories(MbFactory PUBLIC Inc/)

# Tests
add_executable(${TEST_EXECUTABLE} Src/MbDiagEntry.cpp)
target_link_libraries(${TEST_EXECUTABLE} PUBLIC MbFactory ZubrAdapter RokiRcb4Adapter SKServoAdapter GTest::gtest_main)
if(DEFINED ENV{ROKI_USE_HARDWARE_MOCKS})
    target_compile_definitions(${TEST_EXECUTABLE} PRIVATE USE_MOCKS)
endif()

install(TARGETS ${TEST_EXECUTABLE} RUNTIME DESTINATION /usr/bin/)
install(DIRECTORY Diagnostic/Scripts/ DESTINATION ${DIAG_SCRIPTS_DIR})
install(DIRECTORY Diagnostic/Suites/ DESTINATION ${DIAG_SUITES_DIR})

# Bindings
pybind11_add_module(${PYTHON_MODULE_NAME} Src/PyBinding.cpp)
target_link_libraries(${PYTHON_MODULE_NAME} PUBLIC MbFactory RokiRcb4Adapter ZubrAdapter)

execute_process(COMMAND python3 -c "import site; print(site.getsitepackages()[0])" OUTPUT_VARIABLE PYTHON3_SITEDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
message(INFO " Python site-packages directory: ${PYTHON3_SITEDIR}")
install(TARGETS ${PYTHON_MODULE_NAME} DESTINATION ${PYTHON3_SITEDIR})

# Bundling library

# Thanks Christian Adam for the solution
# Credits: https://cristianadam.eu/20190501/bundling-together-static-libraries-with-cmake/
function(bundle_static_library tgt_name bundled_tgt_name)
  list(APPEND static_libs ${tgt_name})

  function(_recursively_collect_dependencies input_target)
    set(_input_link_libraries LINK_LIBRARIES)
    get_target_property(_input_type ${input_target} TYPE)
    if (${_input_type} STREQUAL "INTERFACE_LIBRARY")
      set(_input_link_libraries INTERFACE_LINK_LIBRARIES)
    endif()
    get_target_property(public_dependencies ${input_target} ${_input_link_libraries})
    foreach(dependency IN LISTS public_dependencies)
      if(TARGET ${dependency})
        get_target_property(alias ${dependency} ALIASED_TARGET)
        if (TARGET ${alias})
          set(dependency ${alias})
        endif()
        get_target_property(_type ${dependency} TYPE)
        if (${_type} STREQUAL "STATIC_LIBRARY")
          list(APPEND static_libs ${dependency})
        endif()

        get_property(library_already_added
          GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency})
        if (NOT library_already_added)
          set_property(GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency} ON)
          _recursively_collect_dependencies(${dependency})
        endif()
      endif()
    endforeach()
    set(static_libs ${static_libs} PARENT_SCOPE)
  endfunction()

  _recursively_collect_dependencies(${tgt_name})

  list(REMOVE_DUPLICATES static_libs)

  set(bundled_tgt_full_name 
    ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${bundled_tgt_name}${CMAKE_STATIC_LIBRARY_SUFFIX})

  if (CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    file(WRITE ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
      "CREATE ${bundled_tgt_full_name}\n" )
        
    foreach(tgt IN LISTS static_libs)
      file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
        "ADDLIB $<TARGET_FILE:${tgt}>\n")
    endforeach()
    
    file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "SAVE\n")
    file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "END\n")

    file(GENERATE
      OUTPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      INPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in)

    set(ar_tool ${CMAKE_AR})
    if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      set(ar_tool ${CMAKE_CXX_COMPILER_AR})
    endif()

  else()
    message(FATAL_ERROR "Unknown bundle scenario!")
  endif()

  add_library(${bundled_tgt_name} STATIC Src/CheckDefs.cpp)
  set_target_properties(${bundled_tgt_name}
    PROPERTIES 
      INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${tgt_name},INTERFACE_INCLUDE_DIRECTORIES>
      INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${tgt_name},INCLUDE_DIRECTORIES>)
  add_dependencies(${bundled_tgt_name} ${tgt_name})
  
  add_custom_command(TARGET ${bundled_tgt_name} POST_BUILD
    COMMAND ${ar_tool} -M < ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
    COMMENT "Bundling ${bundled_tgt_name}"
    VERBATIM)
endfunction()

add_library(rokilowlvl-export STATIC Src/CheckDefs.cpp)
target_link_libraries(rokilowlvl-export PUBLIC MotherboardAdapter ZubrAdapter RokiRcb4Adapter SKServoAdapter DaemonTools CLI)

bundle_static_library(rokilowlvl-export rokilowlvl)

install(TARGETS rokilowlvl
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            
       )

install(DIRECTORY $<TARGET_PROPERTY:rokilowlvl,INTERFACE_INCLUDE_DIRECTORIES>
        DESTINATION /usr/local/include
        )

install(DIRECTORY Inc/
DESTINATION /usr/local/include
)
