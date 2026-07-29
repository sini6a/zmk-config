# SPDX-License-Identifier: MIT

target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM app PRIVATE
    src/pocketboard_status_screen.c
)
