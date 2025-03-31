cmake_minimum_required(VERSION 3.26)

file(GLOB sources "*.c" "*.cpp" "${INCLUDE_DIR}/*.h")

add_executable(${PROJECT_NAME} ${sources})

# Зависимости
target_link_libraries(${PROJECT_NAME} PRIVATE
        Threads::Threads
        fmt::fmt
        nlohmann_json::nlohmann_json
        ${ImageMagick_LIBRARIES}
        ${VIPS_LIBRARIES}       # Directly link Vips libraries
        ${GLIB_LIBRARIES}       # GLib is required by Vips
        ${GOBJECT_LIBRARIES}    # GObject is required by Vips
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${INCLUDE_DIR}
        ${ImageMagick_INCLUDE_DIRS}
        ${VIPS_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
        ${GOBJECT_INCLUDE_DIRS}
)

# Add linker flags if needed
target_link_options(${PROJECT_NAME} PRIVATE ${VIPS_LDFLAGS})