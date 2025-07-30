#include <libdragon.h>
#include "gdb.hpp"

int main()
{
    debug_init_isviewer();
    debug_init_usblog();

    asset_init_compression(2);
    joypad_init();

    vi_init();
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
    rdpq_init();

    while (true) {
        surface_t* disp = display_get();
        rdpq_attach_clear(disp, NULL);
        rdpq_detach();

        uint64_t modified = GDB::fileLastModified("hello.txt");
        char msg[100];
        snprintf(msg, sizeof(msg), "Last modified: %llu", modified);
        graphics_draw_text(disp, 16, 16, msg);

        uint8_t contents[100];
        int size = GDB::readFile("hello.txt", sizeof(contents)-1, contents);

        if (size > -1) {
            contents[size] = '\0'; // Terminate the string because GDB just copies bytes 
            graphics_draw_text(disp, 16, 32, (const char*)contents);
        }

        display_show(disp);

    }
}