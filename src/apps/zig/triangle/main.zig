const std = @import("std");
const zm = @import("zm");
const Canvas = @import("Canvas.zig");
const process = zm.process;
const graphics = zm.graphics;
const win = zm.window;

pub const panic = zm.MosApplicationPanic;

export fn proc_main() void {
    zm.debug.log(.INFO, @src(), "Starting application...", .{});

    _ = process.create_thread(&process1, false);
    _ = process.create_thread(&process2, false);

    _ = process.register_event_handler(.PROCCESS_YIELD_REQ, &repaint_on_yield);

    while (true) {
        _ = process.handle_events();
    }

    zm.process.exit(3);
}

fn repaint_on_yield(_: *const process.ProcessEvent) callconv(.c) void {
    win.flush();
}

fn process1() callconv(.c) void {
    const h = win.create("Window 1 - Hello from Zig") catch {
        @panic("window creation failed");
    };

    const fb = win.frameBuffer(h) catch {
        @panic("Could not get Framebuffer for window");
    };

    const width = fb.width_px;
    const height = fb.height_px;

    const canvas: Canvas = .{
        .g = &fb,
        .height = height,
        .width = width,
    };

    const count = 400;
    var color1:u32 = 0xFFB;
    var color2:u32 = 0xFF0050;

    while (true) {
        for (0..count) |i| {
            canvas.draw_line(.{ width / 2, 0 }, .{ (width * i) / count, height / 2 }, color1);
            canvas.draw_line(.{ width / 2, height - 1 }, .{ (width * i) / count, height / 2 }, color2);
            process.delay(1);
        }
        const temp = color1;
        color1 = color2;
        color2 = temp;
    }
}

fn process2() callconv(.c) void {
    const h = win.create("Window 2 - Hello from Zig") catch {
        @panic("window creation failed");
    };

    const fb = win.frameBuffer(h) catch {
        @panic("Could not get Framebuffer for window");
    };

    const width = fb.width_px;
    const height = fb.height_px;

    var t: u32 = 50;
    while (true) {
        for (1..width) |x| {
            for (1..height) |y| {
                const color: u32 = if (((y & (t+x)) % t) == 0) 0xffee00 else 0x202020;
                graphics.putPixel(&fb, x, y, color);
            }
        }
        t = if (t > 500) 50 else t + 10;
        process.delay(50);
    }
}
