const std = @import("std");
const Model = std.Target.Cpu.Model;

const linker_script_path = "../../../src/kernel/x86/process.ld";
const libcm_path = "../../../build-os/src/cm";
const include_path = "../../../include/cm";

pub fn build(b: *std.Build) void {
    const i686_cpu_model = Model{
        .name = "i686",
        .llvm_name = "",
        .features = .empty,
    };

    const target = b.resolveTargetQuery(.{
        .abi = .none,
        .cpu_arch = .x86,
        .cpu_model = .{ .explicit = &i686_cpu_model },
        .os_tag = .freestanding,
    });

    const hello_exe = b.addExecutable(.{
        .name = "hello",
        .root_module = b.createModule(.{
            .root_source_file = b.path("hello.zig"),
            .target = target,
            .optimize = b.standardOptimizeOption(.{}),
            .omit_frame_pointer = false,
            .red_zone = false,
            .unwind_tables = .none,
            .stack_protector = false,
            .single_threaded = true,
            .pic = false,
        }),
    });

    hello_exe.root_module.addLibraryPath(b.path(libcm_path));
    hello_exe.root_module.linkSystemLibrary("cm", .{});
    hello_exe.addObjectFile(b.path("crta.o"));
    hello_exe.addIncludePath(b.path(include_path));

    hello_exe.linker_script = b.path(linker_script_path);
    hello_exe.link_gc_sections = true;
    hello_exe.link_eh_frame_hdr = false;
    hello_exe.entry = .{ .symbol_name = "proc_start" };

    b.installArtifact(hello_exe);
}
