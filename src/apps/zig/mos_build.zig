const std = @import("std");
const Build = std.Build;
const Step = Build.Step;
const Target = std.Target;

const CMakeBuildModes = enum {
    DEBUG,
    NDEBUG,

    fn from(mode: []const u8) @This() {
        return std.meta.stringToEnum(CMakeBuildModes, mode) orelse @panic("Invalid Build Mode");
    }
};
const CMakeBoolean = enum {
    ON,
    OFF,

    fn from(b: []const u8) @This() {
        return std.meta.stringToEnum(CMakeBoolean, b) orelse @panic("Invalid Boolean");
    }
};

const CMakeBuildVariables = struct {
    build_mode: CMakeBuildModes,
    graphics_enabled: CMakeBoolean,
    port_e9_enabled: CMakeBoolean,
};

const CMakeExports = struct {
    linker_script_rel_path: []const u8,
    libcm_rel_path: []const u8,
    crta_rel_path: []const u8,
    include_rel_path: []const u8,
    entry_point: []const u8,
    variables: CMakeBuildVariables,
};

pub const i686_target_query = Target.Query{
    .abi = .none,
    .cpu_arch = .x86,
    .cpu_model = .{
        .explicit = &.{
            .name = "i686",
            .llvm_name = "",
            .features = .empty,
        },
    },
    .os_tag = .freestanding,
};

const BuildOptions = struct {
    options: CMakeExports,
    root_src_file: []const u8,
    target: Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
};

const BuildSteps = struct {
    root_step: *Step,
    compilation_step: *Step.Compile,
};

pub fn addCMakeExportOptions(b: *Build) !CMakeExports {
    const linker_script_path = b.option(
        []const u8,
        "LinkerScriptPath",
        "Absolute path to linker script",
    ) orelse ""; // Must provide 'LinkerScriptPath'
    const libcm_path = b.option(
        []const u8,
        "LibCMPath",
        "Absolute path to libcm library",
    ) orelse ""; // Must provide 'LibCMPath'
    const crt_path = b.option(
        []const u8,
        "CRTPath",
        "Absolute path to crt.o",
    ) orelse ""; // Must provide 'CRTPath'
    const include_path = b.option(
        []const u8,
        "CInludePath",
        "Absolute path to C include root dir",
    ) orelse ""; // Must provide 'CInludePath'
    const entry_point = b.option(
        []const u8,
        "EntryPoint",
        "Entry point of user application",
    ) orelse ""; // Must provide 'EntryPoint'

    const build_mode = b.option(
        []const u8,
        "BuildMode",
        "Build mode (DEBUG, NDEBUG)",
    ) orelse "DEBUG";
    const graphics_enabled = b.option(
        []const u8,
        "GraphicsEnabled",
        "Is graphics enabled",
    ) orelse "OFF";
    const port_e9_enabled = b.option(
        []const u8,
        "PortE9Enabled",
        "Is port 0xE9 printing enabled",
    ) orelse "OFF";

    return .{
        .crta_rel_path = try std.fs.path.relative(b.allocator, ".", crt_path),
        .include_rel_path = try std.fs.path.relative(b.allocator, ".", include_path),
        .libcm_rel_path = try std.fs.path.relative(b.allocator, ".", libcm_path),
        .linker_script_rel_path = try std.fs.path.relative(b.allocator, ".", linker_script_path),
        .entry_point = entry_point,
        .variables = .{
            .build_mode = .from(build_mode),
            .graphics_enabled = .from(graphics_enabled),
            .port_e9_enabled = .from(port_e9_enabled),
        },
    };
}

pub fn addExecutable(b: *Build, comptime name: []const u8, options: BuildOptions) BuildSteps {
    // 1. Compilation and installation of the generated binary
    const exe = elf_executable(b, name, &options);
    const exe_install = b.addInstallArtifact(exe, .{});

    // 2. Run objcopy on the compiler output (generated from above step) and install the output into
    // the 'bin' folder.
    const objcopy_run = b.addSystemCommand(&.{ "objcopy", "-O", "binary" });
    objcopy_run.step.dependOn(&exe_install.step);

    const flatten_output_file = name ++ ".flt";
    objcopy_run.addArg(b.getInstallPath(.bin, exe.name));
    const flatten_output_path = objcopy_run.addOutputFileArg(flatten_output_file);

    const flatten_install = b.addInstallFileWithDir(flatten_output_path, .bin, flatten_output_file);

    // 3. A Root target for the above steps. This was added so that 'zig build <name>' can be used
    // to build one particular target.
    const root = b.step(name, "Builds '" ++ name ++ "' target.");
    root.dependOn(&flatten_install.step);

    return .{
        .root_step = root,
        .compilation_step = exe,
    };
}

fn MosModuleCreateOptions(b: *Build, options: *const BuildOptions) Build.Module.CreateOptions {
    return .{
        .root_source_file = b.path(options.root_src_file),
        .target = options.target,
        .optimize = options.optimize,
        .omit_frame_pointer = false,
        .red_zone = false,
        .unwind_tables = .none,
        .stack_protector = false,
        .single_threaded = true,
        .pic = false,
    };
}

fn elf_executable(b: *Build, comptime name: []const u8, options: *const BuildOptions) *Step.Compile {
    const exe = b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(MosModuleCreateOptions(b, options)),
    });

    exe.root_module.addLibraryPath(b.path(options.options.libcm_rel_path));
    exe.root_module.linkSystemLibrary("cm", .{});
    exe.addObjectFile(b.path(options.options.crta_rel_path));
    exe.addIncludePath(b.path(options.options.include_rel_path));

    exe.linker_script = b.path(options.options.linker_script_rel_path);
    exe.link_gc_sections = true;
    exe.link_eh_frame_hdr = false;
    exe.entry = .{ .symbol_name = options.options.entry_point };

    return exe;
}

pub fn addLibrary(b: *Build, comptime name: []const u8, options: BuildOptions) BuildSteps {
    // 1. Compilation and installation of the generated binary
    const lib = elf_library(b, name, &options);
    const lib_install = b.addInstallArtifact(lib, .{});

    // 3. A Root target for the above steps. This was added so that 'zig build <name>' can be used
    // to build one particular target.
    const root = b.step(name, "Builds '" ++ name ++ "' target.");
    root.dependOn(&lib_install.step);

    return .{
        .root_step = root,
        .compilation_step = lib,
    };
}

fn elf_library(b: *Build, comptime name: []const u8, options: *const BuildOptions) *Step.Compile {
    const lib = b.addLibrary(.{
        .name = name,
        .root_module = b.createModule(MosModuleCreateOptions(b, options)),
    });

    lib.root_module.addLibraryPath(b.path(options.options.libcm_rel_path));
    lib.root_module.linkSystemLibrary("cm", .{});
    lib.addObjectFile(b.path(options.options.crta_rel_path));
    lib.addIncludePath(b.path(options.options.include_rel_path));

    return lib;
}

pub fn addModule(b: *Build, comptime name: []const u8, options: BuildOptions) *Build.Module {
    const mod = b.addModule(name, MosModuleCreateOptions(b, &options));
    return mod;
}

pub fn setCMakeExportOptions(b: *Build, mod: *Build.Module, options: CMakeExports) void {
    mod.addLibraryPath(b.path(options.libcm_rel_path));
    mod.linkSystemLibrary("cm", .{});
    mod.addIncludePath(b.path(options.include_rel_path));

    const cmake_build_variables = b.addOptions();
    cmake_build_variables.addOption(CMakeBuildModes, "build_mode", options.variables.build_mode);
    cmake_build_variables.addOption(CMakeBoolean, "graphics_enabled", options.variables.graphics_enabled);
    cmake_build_variables.addOption(CMakeBoolean, "port_e9_enabled", options.variables.port_e9_enabled);

    mod.addOptions("config", cmake_build_variables);
}
