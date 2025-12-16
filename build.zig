//!  build.zig - Build script for coreutilspp
//!
//!  Copyright (C) 2025  Sebastian Pineda (spineda.wpi.alum@gmail.com)
//!
//!  This program is free software; you can redistribute it and/or modify
//!  it under the terms of the GNU General Public License as published by
//!  the Free Software Foundation; either version 2 of the License, or
//!  (at your option) any later version.
//!
//!  This program is distributed in the hope that it will be useful,
//!  but WITHOUT ANY WARRANTY; without even the implied warranty of
//!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//!  GNU General Public License for more details.
//!
//!  You should have received a copy of the GNU General Public License along
//!  with this program. If not, see <https://www.gnu.org/licenses/>

const std = @import("std");
const builtin = @import("builtin");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // if other projects want to build this project on the fly, this ought
    // to be left to false as to not clobber their own compiledb.
    const create_compiledb: bool = b.option(
        bool,
        "compiledb",
        "Generate compile_commands.json for this project",
    ) orelse false;

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
    if (create_compiledb) {
        b.getInstallStep().dependOn(compile_db_step);
    }

    // coreutils
    const packages: []const PackageConfiguration = &.{
        .{
            .name = comptime "ls",
            .srcs = comptime &.{
                .{ .name = "main.cpp", .directory = "coreutils/ls/" },
            },
            .target = target,
            .optimize = optimize,
            .build_root = b,
        },
        .{
            .name = comptime "yes",
            .srcs = comptime &.{
                .{ .name = "main.cpp", .directory = "coreutils/yes/" },
            },
            .target = target,
            .optimize = optimize,
            .build_root = b,
        },
        .{
            .name = comptime "echo",
            .srcs = comptime &.{
                .{ .name = "main.cpp", .directory = "coreutils/echo/" },
            },
            .target = target,
            .optimize = optimize,
            .build_root = b,
        },
        .{
            .name = comptime "mkdir",
            .srcs = comptime &.{
                .{ .name = "main.cpp", .directory = "coreutils/mkdir/" },
            },
            .target = target,
            .optimize = optimize,
            .build_root = b,
        },
    };

    for (packages) |each_package| {
        const package = try createPackage(each_package);
        if (create_compiledb) {
            runcompiledb.step.dependOn(&package.step);
        }
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

    mod.addIncludePath(config.build_root.path(""));

    const comp = config.build_root.addExecutable(.{
        .name = config.name,
        .root_module = mod,
    });
    comp.lto = switch (config.optimize) {
        .Debug => .none,
        else => .full,
    };

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
