const std = @import("std");
const Build = std.Build;
const Step = Build.Step;

const Self = @This();

const BuildOptions = struct {
    linker_script_rel_path: []const u8,
    libcm_rel_path: []const u8,
    crta_rel_path: []const u8,
    include_rel_path: []const u8,
    entry_point: []const u8,
};

const i686_cpu_model = std.Target.Cpu.Model{
    .name = "i686",
    .llvm_name = "",
    .features = .empty,
};

options: BuildOptions,
b: *Build,
optimize: std.builtin.OptimizeMode,
target: std.Build.ResolvedTarget,

pub fn init(b: *std.Build) !Self {
    return .{
        .target = b.resolveTargetQuery(.{
            .abi = .none,
            .cpu_arch = .x86,
            .cpu_model = .{ .explicit = &i686_cpu_model },
            .os_tag = .freestanding,
        }),
        .optimize = b.standardOptimizeOption(.{}),
        .b = b,
        .options = try getMosBuildOptions(b),
    };
}

fn getMosBuildOptions(b: *Build) !BuildOptions {
    const linker_script_path = b.option([]const u8, "LinkerScriptPath", "Absolute path to linker script") orelse ""; // Must provide 'LinkerScriptPath'
    const libcm_path = b.option([]const u8, "LibCMPath", "Absolute path to libcm library") orelse ""; // Must provide 'LibCMPath'
    const crt_path = b.option([]const u8, "CRTPath", "Absolute path to crt.o") orelse ""; // Must provide 'CRTPath'
    const include_path = b.option([]const u8, "CInludePath", "Absolute path to C include root dir") orelse ""; // Must provide 'CInludePath'
    const entry_point = b.option([]const u8, "EntryPoint", "Entry point of user application") orelse ""; // Must provide 'EntryPoint'

    return .{
        .crta_rel_path = try std.fs.path.relative(b.allocator, ".", crt_path),
        .include_rel_path = try std.fs.path.relative(b.allocator, ".", include_path),
        .libcm_rel_path = try std.fs.path.relative(b.allocator, ".", libcm_path),
        .linker_script_rel_path = try std.fs.path.relative(b.allocator, ".", linker_script_path),
        .entry_point = entry_point,
    };
}

pub fn addExecutable(self: Self, comptime name: []const u8, comptime root_src_file: []const u8) *Step {
    const b = self.b;
    const exe = self.elf_compilation(name, root_src_file);
    const exe_install = b.addInstallArtifact(exe, .{});

    const flatten_output_file = name ++ ".flt";
    const objcopy_run = b.addSystemCommand(&.{ "objcopy", "-O", "binary" });
    objcopy_run.addArg(b.getInstallPath(.bin, exe.name));
    const flatten_output_path = objcopy_run.addOutputFileArg(flatten_output_file);

    const flatten_install = b.addInstallFileWithDir(flatten_output_path, .bin, flatten_output_file);

    objcopy_run.step.dependOn(&exe_install.step);
    return &flatten_install.step;
}

fn elf_compilation(self: Self, name: []const u8, root_src_file: []const u8) *Step.Compile {
    const b = self.b;
    const exe = b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{
            .root_source_file = b.path(root_src_file),
            .target = self.target,
            .optimize = self.optimize,
            .omit_frame_pointer = false,
            .red_zone = false,
            .unwind_tables = .none,
            .stack_protector = false,
            .single_threaded = true,
            .pic = false,
        }),
    });

    exe.root_module.addLibraryPath(b.path(self.options.libcm_rel_path));
    exe.root_module.linkSystemLibrary("cm", .{});
    exe.addObjectFile(b.path(self.options.crta_rel_path));
    exe.addIncludePath(b.path(self.options.include_rel_path));

    exe.linker_script = b.path(self.options.linker_script_rel_path);
    exe.link_gc_sections = true;
    exe.link_eh_frame_hdr = false;
    exe.entry = .{ .symbol_name = self.options.entry_point };

    return exe;
}
