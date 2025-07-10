zephyr_library()
zephyr_library_sources(src/led_status.c)
target_sources(app PRIVATE src/led_status.c)
zephyr_include_directories(include)