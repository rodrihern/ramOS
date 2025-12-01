#include "usrlib.h"

int test_video(int argc, char * argv[]) {
    video_info_t info;
    sys_video_info(&info);
    int fb_size = info.height * info.pitch;
    uint32_t * framebuffer = sys_malloc(fb_size);
    uint32_t color;
    uint64_t start = sys_ms_elapsed();
    int fps = 0;

    while (sys_ms_elapsed() - start < 1000) {
        fps++;
        color = get_uint();
        for (int offset = 0; offset < fb_size/4; offset++) {
            framebuffer[offset] = color;
        }
        sys_present(framebuffer);
    }


    printf("w: %d\nh: %d\nbpp: %d\npitch: %d\n", info.width, info.height, info.bpp, info.pitch);
    printf("fps with for loop: %d\n", fps);

    start = sys_ms_elapsed();
    fps = 0;
    while (sys_ms_elapsed() - start < 1000) {
        fps++;
        color = get_uint();
        uint64_t to_fill = color | ((uint64_t)color << 32);
        memset64(framebuffer, to_fill, fb_size);
        sys_present(framebuffer);
    }
    printf("fps with memset: %d\n", fps);


    return 0;

}