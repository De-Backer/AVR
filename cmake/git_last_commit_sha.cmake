cmake_minimum_required(VERSION 3.14...3.19)

message(" - git_last_commit_sha")

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  #get GIT COMMIT SHA
  execute_process(
    COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} rev-parse --verify HEAD
    OUTPUT_VARIABLE  GIT_COMMIT_SHA
    RESULT_VARIABLE GIT_COMMIT_RESULT)

  # This strips terminating newline in the variable
  string(REGEX REPLACE "\n$" "" GIT_COMMIT_SHA "${GIT_COMMIT_SHA}")

  # This adds to definitions => .cpp
  add_definitions(-DGIT_COMMIT_SHA="${GIT_COMMIT_SHA}")

  if(NOT GIT_COMMIT_RESULT EQUAL "0")
    message(
      FATAL_ERROR
      "git rev-parse --verify HEAD failed with "
      ${GIT_COMMIT_RESULT}
      ", please check")
  endif()
endif()
