function(tatnez_enable_sanitizers target_name enable_sanitizers)
  if(NOT enable_sanitizers)
    return()
  endif()

  if(MSVC)
    return()
  endif()

  target_compile_options("${target_name}" PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options("${target_name}" PRIVATE -fsanitize=address,undefined)
endfunction()
