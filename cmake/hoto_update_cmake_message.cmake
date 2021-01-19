if(${CMAKE_VERSION} VERSION_LESS 3.14)
  message(
    "_____________________________________________________________________")
  message("to update Cmake on linux:")
  message("https://github.com/Kitware/CMake/")
  message("linux => cmake-3.19.1-Linux-x86_64.sh")
  message("         sudo ./cmake.sh --prefix=/usr/local/ --exclude-subdir")
  message("windows good luck :)")
endif()
