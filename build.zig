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

const Config = struct {
    b: *std.Build,
    name: []const u8,
    root_source_file: []const u8,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    compiledb: bool,
};

const common_cpp_flags = [_][]const u8{
    "-std=c++26",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wshadow",
    "-Wconversion",
    "-Werror",
};

fn tmpJsonPath(
    buf: *std.ArrayList([]const u8),
    allocator: std.mem.Allocator,
    file_name: []const u8,
) std.mem.Allocator.Error!void {
    try buf.append(allocator, "-MJ");
    try buf.append(allocator, name: {
        var name: std.ArrayList(u8) = .empty;
        for (file_name) |letter| {
            try name.append(allocator, switch (letter) {
                '/' => '.',
                else => |other| other,
            });
        }
        try name.appendSlice(allocator, ".json.tmp");
        break :name name.items;
    });
}

const CommonModule = struct {
    name: []const u8,
    module: *std.Build.Module,

    pub fn create(config: Config) std.mem.Allocator.Error!CommonModule {
        const module = config.b.createModule(.{
            .target = config.target,
            .optimize = config.optimize,
            .link_libcpp = true,
        });

        const allocator = config.b.allocator;
        var flags: std.ArrayList([]const u8) = .empty;
        for (common_cpp_flags) |flag| {
            try flags.append(allocator, flag);
        }
        if (config.compiledb) {
            try tmpJsonPath(&flags, config.b.allocator, config.root_source_file);
        }

        module.addCSourceFile(.{
            .file = config.b.path(config.root_source_file),
            .flags = flags.items,
            .language = .cpp,
        });
        module.addIncludePath(config.b.path(""));

        return .{
            .name = config.name,
            .module = module,
        };
    }
};

pub fn build(b: *std.Build) std.mem.Allocator.Error!void {
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
        .target = b.resolveTargetQuery(std.Target.Query.fromTarget(&builtin.target)),
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

    const CoreUtils = struct {
        ls: CommonModule,
        yes: CommonModule,
        echo: CommonModule,
        mkdir: CommonModule,
    };

    const modules: CoreUtils = .{
        .ls = try .create(.{
            .b = b,
            .name = "ls",
            .root_source_file = "coreutils/ls/main.cpp",
            .target = target,
            .optimize = optimize,
            .compiledb = create_compiledb,
        }),
        .yes = try .create(.{
            .b = b,
            .name = "yes",
            .root_source_file = "coreutils/yes/main.cpp",
            .target = target,
            .optimize = optimize,
            .compiledb = create_compiledb,
        }),
        .echo = try .create(.{
            .b = b,
            .name = "echo",
            .root_source_file = "coreutils/echo/main.cpp",
            .target = target,
            .optimize = optimize,
            .compiledb = create_compiledb,
        }),
        .mkdir = try .create(.{
            .b = b,
            .name = "mkdir",
            .root_source_file = "coreutils/mkdir/main.cpp",
            .target = target,
            .optimize = optimize,
            .compiledb = create_compiledb,
        }),
    };

    inline for (comptime std.meta.fieldNames(CoreUtils)) |field| {
        const coreutil: CommonModule = @field(modules, field);
        const exe = b.addExecutable(.{
            .name = coreutil.name,
            .root_module = coreutil.module,
        });
        exe.lto = switch (optimize) {
            .Debug => .none,
            else => .full,
        };
        b.installArtifact(exe);
        const run_cmd = b.addRunArtifact(exe);
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }

        const single_build = b.step("build_" ++ field, "Build " ++ field);
        single_build.dependOn(&exe.step);

        const single_run = b.step("run_" ++ field, "Run " ++ field);
        single_run.dependOn(&run_cmd.step);

        if (create_compiledb) {
            runcompiledb.step.dependOn(&exe.step);
        }
    }

    const test_step = b.step("test", "Run unit tests");

    const cpp_test_files = [_][]const u8{
        "tests/ArgumentParser/tests.cpp",
    };

    const test_mod = b.createModule(.{
        .root_source_file = b.path("tests/root.zig"),
        .link_libcpp = true,
        .optimize = optimize,
        .target = target,
        .sanitize_c = .full,
        .sanitize_thread = true,
    });

    for (cpp_test_files) |file| {
        var flags: std.ArrayList([]const u8) = .empty;
        for (common_cpp_flags) |common_flag| {
            try flags.append(b.allocator, common_flag);
        }
        if (create_compiledb) {
            try tmpJsonPath(&flags, b.allocator, file);
        }
        test_mod.addCSourceFile(.{
            .file = b.path(file),
            .language = .cpp,
        });
    }

    const test_compilation = b.addTest(.{
        .root_module = test_mod,
    });
    const run_test = b.addRunArtifact(test_compilation);

    if (create_compiledb) {
        execompiledb.step.dependOn(&test_compilation.step);
        run_test.step.dependOn(&execompiledb.step);
    }

    test_step.dependOn(&run_test.step);
}
