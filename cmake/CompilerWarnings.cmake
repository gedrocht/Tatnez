function(tatnez_enable_project_warnings target_name enable_warnings_as_errors)
  if(MSVC)
    target_compile_options(
      "${target_name}"
      PRIVATE
        /W4
        /permissive-
        /EHsc
        /Zc:__cplusplus
        /wd4702
    )

    if(enable_warnings_as_errors)
      target_compile_options("${target_name}" PRIVATE /WX)
    endif()
  else()
    target_compile_options(
      "${target_name}"
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wdouble-promotion
    )

    if(enable_warnings_as_errors)
      target_compile_options("${target_name}" PRIVATE -Werror)
    endif()
  endif()
endfunction()
