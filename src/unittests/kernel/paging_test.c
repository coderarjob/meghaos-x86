#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_IMPLEMENTATION
#include <unittest/yukti.h>
#include <paging.h>

TEST (paging, pageframe_to_physical) {
    EQ_SCALAR(PAGEFRAME_TO_PHYSICAL(0x3), 0x3U * CONFIG_PAGE_FRAME_SIZE_BYTES);
    EQ_SCALAR(PAGEFRAME_TO_PHYSICAL(0x0), 0x0U);
    END();
}

TEST (paging, physical_to_pageframe) {
    EQ_SCALAR(PHYSICAL_TO_PAGEFRAME(0x0), 0x0U);
    EQ_SCALAR(PHYSICAL_TO_PAGEFRAME(4 * CONFIG_PAGE_FRAME_SIZE_BYTES), 0x4U);
    END();
}

void yt_reset() {
}

int main() {
    YT_INIT();
    pageframe_to_physical();
    physical_to_pageframe();
    RETURN_WITH_REPORT();
}
