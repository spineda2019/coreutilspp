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
    try trackCompilation(exels, &all_targets, b);

    for (all_targets.items) |each_target| {
        install_step.dependOn(&each_target.step);
    }
}

fn trackCompilation(
    exe: *std.Build.Step.Compile,
    tracked: *std.ArrayList(*std.Build.Step.Compile),
    b: *std.Build,
) !void {
    b.installArtifact(exe);
    try tracked.append(b.allocator, exe);

    var build_buf: std.ArrayList(u8) = .empty;
    try build_buf.appendSlice(b.allocator, "build ");
    try build_buf.appendSlice(b.allocator, exe.name);
    const ls_buildstep = b.step(exe.name, build_buf.items);
    ls_buildstep.dependOn(&exe.step);

    var name_buf: std.ArrayList(u8) = .empty;
    try name_buf.appendSlice(b.allocator, "run_");
    try name_buf.appendSlice(b.allocator, exe.name);
    var description_buf: std.ArrayList(u8) = .empty;
    try description_buf.appendSlice(b.allocator, "Run ");
    try description_buf.appendSlice(b.allocator, exe.name);
    const ls_runstep = b.step(name_buf.items, description_buf.items);
    const ls_runcmd = b.addRunArtifact(exe);
    ls_runcmd.step.dependOn(ls_runstep);
    if (b.args) |args| {
        ls_runcmd.addArgs(args);
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
