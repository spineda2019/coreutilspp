const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const install_step = b.getInstallStep();

    var all_targets: std.ArrayList(*std.Build.Step.Compile) = .empty;

    const sourcels: []const SourceFile = comptime &.{
        .{ .name = "main.cpp", .directory = "ls/" },
    };
    const modls = try createCppModule(sourcels, target, optimize, b);
    const exels = b.addExecutable(.{
        .name = "ls",
        .root_module = modls,
    });
    b.installArtifact(exels);
    try all_targets.append(b.allocator, exels);
    const ls_buildstep = b.step("ls", "build ls");
    ls_buildstep.dependOn(&exels.step);
    const ls_runstep = b.step("run_ls", "Run ls");
    const ls_runcmd = b.addRunArtifact(exels);
    ls_runcmd.step.dependOn(ls_runstep);
    if (b.args) |args| {
        ls_runcmd.addArgs(args);
    }

    for (all_targets.items) |each_target| {
        install_step.dependOn(&each_target.step);
    }
}

const SourceFile = struct {
    name: []const u8,
    directory: []const u8,
};

fn createCppModule(
    comptime files: []const SourceFile,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    b: *std.Build,
) !*std.Build.Module {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = false,
        .link_libcpp = true,
    });

    inline for (files) |cppfile| {
        const path = calc_path: {
            var buf: std.ArrayList(u8) = .empty;
            try buf.appendSlice(b.allocator, cppfile.directory);
            try buf.appendSlice(b.allocator, cppfile.name);
            break :calc_path b.path(buf.items);
        };

        const compiledb_tmp_path = calc_tmp: {
            var buf: std.ArrayList(u8) = .empty;
            for (cppfile.directory) |letter| {
                try buf.append(b.allocator, switch (letter) {
                    '/' => '.',
                    else => |other| other,
                });
            }
            try buf.appendSlice(b.allocator, cppfile.name);
            try buf.appendSlice(b.allocator, ".json.tmp");
            break :calc_tmp buf.items;
        };

        mod.addCSourceFile(.{
            .file = path,
            .flags = &.{
                "-std=c++23",
                "-Wall",
                "-Wextra",
                "-Wpedantic",
                "-Wshadow",
                "-Wconversion",
                "-Werror",
                "-MJ",
                compiledb_tmp_path,
            },
            .language = .cpp,
        });
    }

    return mod;
}
