const cm = @cImport({
    @cInclude("cm.h");
});

export fn proc_main() void {
    cm.cm_process_kill(0x1B);
}
