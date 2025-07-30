#include <libdragon.h>
#include "gdbfile.h"

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

        uint64_t modified = gdbfile_last_modified("hello.txt");
        char msg[100];
        snprintf(msg, sizeof(msg), "Last modified: %llu", modified);
        graphics_draw_text(disp, 16, 16, msg);

        uint8_t contents[100];
        int size = gdbfile_read("hello.txt", sizeof(contents)-1, contents);

        if (size > -1) {
            contents[size] = '\0'; // Terminate the string because GDB just copies bytes 
            graphics_draw_text(disp, 16, 32, (const char*)contents);

            // Write back an answer
            gdbfile_write_string("output.txt", "It works!");
        }


        display_show(disp);

    }
}
