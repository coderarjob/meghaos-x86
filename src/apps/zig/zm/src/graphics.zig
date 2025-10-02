const WindowFrameBufferInfo = @import("window.zig").WindowFrameBufferInfo;
const graphics = @cImport({
    @cInclude("graphics.h");
});

pub const Color = graphics.Color;

pub inline fn putPixel(
    g: *const WindowFrameBufferInfo,
    x: u32,
    y: u32,
    color: Color,
) void {
    graphics.graphics_putpixel(@ptrCast(g), x, y, color);
}

// TODO: graphics_rect is not drawing rectangles correctly and causing page fault if run for some
// time. Below function is disabled due to this reason.
//pub inline fn rect(
//    g: *const WindowFrameBufferInfo,
//    x: u32,
//    y: u32,
//    w: u32,
//    h: u32,
//    color: Color,
//) void {
//    graphics.graphics_rect(@ptrCast(g), x, y, w, h, color);
//}
