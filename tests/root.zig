const std = @import("std");

extern "c" fn test_argparser() bool;

test test_argparser {
    try std.testing.expect(test_argparser());
}
