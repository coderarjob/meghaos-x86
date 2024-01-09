#include <paging.h>
#include <unittest/unittest.h>

TEST (paging, pagefrmae_to_physical) {
    EQ_SCALAR(PAGEFRAME_TO_PHYSICAL(0x3), 0x3 * CONFIG_PAGE_FRAME_SIZE_BYTES);
    EQ_SCALAR(PAGEFRAME_TO_PHYSICAL(0x0), 0x0);
    END();
}

TEST (paging, bytes_to_pageframes_floor) {
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(0x1), 0x0);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(4095), 0x0);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(4096), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(4097), 0x1);
    END();
}

TEST (paging, bytes_to_pageframes_ceiling) {
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(0x1), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(4095), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(4096), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(4097), 0x2);

    END();
}

void reset() {
}

int main() {
    pagefrmae_to_physical();
    bytes_to_pageframes_ceiling();
    bytes_to_pageframes_floor();
    return 0;
}
