# Generate VERSION file in package — called during "cmake --build build --target package"
execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git log -1 --format=%ci
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")

file(WRITE "${PACKAGE_DIR}/plugins/VERSION"
    "${GIT_VERSION}\n"
    "commit: ${GIT_COMMIT}\n"
    "commit date: ${GIT_DATE}\n"
    "build time: ${BUILD_TIME}\n"
)
