const std = @import("std");
const builtin = @import("builtin");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // compiledb
    const compile_db_step = b.step(
        "compiledb",
        "Build & format compile_commands.json",
    );
    const modcompiledb = b.createModule(.{
        .root_source_file = b.path("cleandb/main.zig"),
        .target = std.Build.resolveTargetQuery(b, std.Target.Query.fromTarget(&builtin.target)),
        .optimize = optimize,
    });
    const execompiledb = b.addExecutable(.{
        .name = "compiledb",
        .root_module = modcompiledb,
    });
    const runcompiledb = b.addRunArtifact(execompiledb);
    runcompiledb.addArg(
        std.process.getCwdAlloc(b.allocator) catch unreachable,
    );
    compile_db_step.dependOn(&runcompiledb.step);
    // TODO: make this optional of this is a package
    b.getInstallStep().dependOn(compile_db_step);

    // coreutils
    const packages: []const PackageConfiguration = &.{
        .{
            .name = comptime "ls",
            .srcs = comptime &.{
                .{ .name = "main.cpp", .directory = "ls/" },
            },
            .target = target,
            .optimize = optimize,
            .build_root = b,
        },
        .{
            .name = comptime "yes",
            .srcs = comptime &.{
                .{ .name = "main.cpp", .directory = "yes/" },
            },
            .target = target,
            .optimize = optimize,
            .build_root = b,
        },
    };

    for (packages) |each_package| {
        const package = try createPackage(each_package);
        runcompiledb.step.dependOn(&package.step);
    }
}

fn createPackage(config: PackageConfiguration) !*std.Build.Step.Compile {
    const mod = config.build_root.createModule(.{
        .target = config.target,
        .optimize = config.optimize,
        .link_libc = false,
        .link_libcpp = true,
    });

    for (config.srcs) |cppfile| {
        const path = calc_path: {
            var buf: std.ArrayList(u8) = .empty;
            try buf.appendSlice(config.build_root.allocator, cppfile.directory);
            try buf.appendSlice(config.build_root.allocator, cppfile.name);
            break :calc_path config.build_root.path(buf.items);
        };

        const compiledb_tmp_path = calc_tmp: {
            var buf: std.ArrayList(u8) = .empty;
            for (cppfile.directory) |letter| {
                try buf.append(config.build_root.allocator, switch (letter) {
                    '/' => '.',
                    else => |other| other,
                });
            }
            try buf.appendSlice(config.build_root.allocator, cppfile.name);
            try buf.appendSlice(config.build_root.allocator, ".json.tmp");
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

    const comp = config.build_root.addExecutable(.{
        .name = config.name,
        .root_module = mod,
    });

    config.build_root.installArtifact(comp);

    var build_buf: std.ArrayList(u8) = .empty;
    try build_buf.appendSlice(config.build_root.allocator, "build ");
    try build_buf.appendSlice(config.build_root.allocator, config.name);
    const buildstep = config.build_root.step(config.name, build_buf.items);
    buildstep.dependOn(&comp.step);

    var name_buf: std.ArrayList(u8) = .empty;
    try name_buf.appendSlice(config.build_root.allocator, "run_");
    try name_buf.appendSlice(config.build_root.allocator, config.name);
    var description_buf: std.ArrayList(u8) = .empty;
    try description_buf.appendSlice(config.build_root.allocator, "Run ");
    try description_buf.appendSlice(config.build_root.allocator, config.name);
    const runstep = config.build_root.step(
        name_buf.items,
        description_buf.items,
    );
    const runcmd = config.build_root.addRunArtifact(comp);
    runstep.dependOn(&runcmd.step);
    if (config.build_root.args) |args| {
        runcmd.addArgs(args);
    }

    config.build_root.getInstallStep().dependOn(&comp.step);

    return comp;
}

const PackageConfiguration = struct {
    name: []const u8,
    srcs: []const SourceFile,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    build_root: *std.Build,
};

const SourceFile = struct {
    name: []const u8,
    directory: []const u8,
};
