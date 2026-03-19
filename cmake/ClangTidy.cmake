function(tatnez_enable_clang_tidy target_name enable_clang_tidy)
  if(NOT enable_clang_tidy)
    return()
  endif()

  find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
  if(CLANG_TIDY_EXECUTABLE)
    file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" TATNEZ_CLANG_TIDY_SOURCE_DIRECTORY)
    set(TATNEZ_CLANG_TIDY_COMMAND
        "${CLANG_TIDY_EXECUTABLE}"
        "--header-filter=^(${TATNEZ_CLANG_TIDY_SOURCE_DIRECTORY}/include/tatnez|${TATNEZ_CLANG_TIDY_SOURCE_DIRECTORY}/src)")
    set_property(TARGET "${target_name}" PROPERTY CXX_CLANG_TIDY "${TATNEZ_CLANG_TIDY_COMMAND}")
  endif()
endfunction()
