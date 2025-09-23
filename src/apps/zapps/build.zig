const std = @import("std");
const Model = std.Target.Cpu.Model;

pub fn build(b: *std.Build) !void {
    const linker_script_path = b.option([]const u8, "LinkerScriptPath", "Absolute path to linker script") orelse ""; // Must provide 'LinkerScriptPath'
    const libcm_path = b.option([]const u8, "LibCMPath", "Absolute path to libcm library") orelse ""; // Must provide 'LibCMPath'
    const crt_path = b.option([]const u8, "CRTPath", "Absolute path to crt.o") orelse ""; // Must provide 'CRTPath'
    const include_path = b.option([]const u8, "CInludePath", "Absolute path to C include root dir") orelse ""; // Must provide 'CInludePath'

    // TODO: Check if the options were provided. If not fail the build.

    const linker_script_rel_path = try std.fs.path.relative(b.allocator, ".", linker_script_path);
    const libcm_rel_path = try std.fs.path.relative(b.allocator, ".", libcm_path);
    const crt_rel_path = try std.fs.path.relative(b.allocator, ".", crt_path);
    const include_rel_path = try std.fs.path.relative(b.allocator, ".", include_path);

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

    hello_exe.root_module.addLibraryPath(b.path(libcm_rel_path));
    hello_exe.root_module.linkSystemLibrary("cm", .{});
    hello_exe.addObjectFile(b.path(crt_rel_path));
    hello_exe.addIncludePath(b.path(include_rel_path));

    hello_exe.linker_script = b.path(linker_script_rel_path);
    hello_exe.link_gc_sections = true;
    hello_exe.link_eh_frame_hdr = false;
    hello_exe.entry = .{ .symbol_name = "proc_start" };

    b.installArtifact(hello_exe);

    const output_path = try std.fs.path.join(b.allocator, &.{ b.install_path, "hello.flt" });
    defer b.allocator.free(output_path);

    const objcopy_run = b.addSystemCommand(&.{"objcopy"});
    objcopy_run.addArg("-O");
    objcopy_run.addArg("binary");
    objcopy_run.addArg(b.getInstallPath(.bin, hello_exe.name));
    objcopy_run.addArg(output_path);

    objcopy_run.step.dependOn(b.getInstallStep());
    b.default_step = &objcopy_run.step;
}
