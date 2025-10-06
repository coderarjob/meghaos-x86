const assert = @import("std").debug.assert;
const zm = @import("zm");
const WindowFrameBufferInfo = zm.window.WindowFrameBufferInfo;
const graphics = zm.graphics;

pub const Point = struct { usize, usize };

g: *const WindowFrameBufferInfo,
width: usize,
height: usize,

pub fn point_destruct(T: type, p: Point) struct { T, T } {
    const x = @as(T, @intCast(p[0]));
    const y = @as(T, @intCast(p[1]));

    return .{ x, y };
}

pub fn draw_line(self: @This(), p1: Point, p2: Point, color: u32) void {
    const x1, const y1 = point_destruct(isize, p1);
    const x2, const y2 = point_destruct(isize, p2);

    const dx = @abs(x2 - x1);
    const dy = @abs(y2 - y1);

    if (dy > dx) {
        // Line is more vertical. Here we increment y and calculate x.
        // m = (y2 - y1)/(x2 - x1)
        // x = (y-y1)/m + x1
        // x = (x2 - x1)(y-y1)/(y2 - y1) + x1
        const pa, const pb = if (p1[1] > p2[1]) .{ p2, p1 } else .{ p1, p2 };
        const pa_x, const pa_y = point_destruct(isize, pa);
        const pb_x, const pb_y = point_destruct(isize, pb);

        assert(pa_y <= pb_y);

        for (pa[1]..pb[1] + 1) |y| {
            const x: usize = @intCast(
                @divFloor(
                    (pb_x - pa_x) * (@as(isize, @intCast(y)) - pa_y),
                    pb_y - pa_y,
                ) + pa_x,
            );
            assert(y >= 0 and y < self.height);
            assert(x >= 0 and x < self.width);
            graphics.putPixel(self.g, x, y, color);
        }
    } else {
        // Line is more horizontal. Here we increment x and calculate y.
        // m = (y2 - y1)/(x2 - x1)
        // y = m(x - x1) + y1
        // y = (y2 - y1)(x - x1)/(x2 - x1) + y1
        const pa, const pb = if (p1[0] > p2[0]) .{ p2, p1 } else .{ p1, p2 };
        const pa_x, const pa_y = point_destruct(isize, pa);
        const pb_x, const pb_y = point_destruct(isize, pb);

        assert(pa_x <= pb_x);

        for (pa[0]..pb[0] + 1) |x| {
            const y: usize = @intCast(
                @divFloor(
                    (pb_y - pa_y) * (@as(isize, @intCast(x)) - pa_x),
                    pb_x - pa_x,
                ) + pa_y,
            );
            assert(y >= 0 and y < self.height);
            assert(x >= 0 and x < self.width);
            graphics.putPixel(self.g, x, y, color);
        }
    }
}
