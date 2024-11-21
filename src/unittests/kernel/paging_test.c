#include <paging.h>
#include <unittest/unittest.h>

TEST (paging, pageframe_to_physical) {
    EQ_SCALAR(PAGEFRAME_TO_PHYSICAL(0x3), 0x3 * CONFIG_PAGE_FRAME_SIZE_BYTES);
    EQ_SCALAR(PAGEFRAME_TO_PHYSICAL(0x0), 0x0);
    END();
}

TEST (paging, physical_to_pageframe) {
    EQ_SCALAR(PHYSICAL_TO_PAGEFRAME(0x0), 0x0);
    EQ_SCALAR(PHYSICAL_TO_PAGEFRAME(4 * CONFIG_PAGE_FRAME_SIZE_BYTES), 0x4);
    END();
}

void reset() {
}

int main() {
    pageframe_to_physical();
    physical_to_pageframe();
    return 0;
}
