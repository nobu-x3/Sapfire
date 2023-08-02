const zm = @import("zmath");
const zgui = @import("zgui");
const std = @import("std");
const zgpu = @import("zgpu");
const ecs = @import("zflecs");

pub const ComponentTypes = enum {
    transform,
    mesh,
    material,
};

pub const name_type_map = std.ComptimeStringMap(ComponentTypes, .{
    .{ "scene.components.Transform", .transform },
    .{ "scene.components.Mesh", .mesh },
    .{ "renderer.material.Material", .material },
});

pub const Transform = extern struct {
    position: @Vector(4, f32) = .{ 0.0, 0.0, 0.0, 0.0 },
    rotation: zm.Quat = zm.qidentity(),
    euler_angles: @Vector(3, f32) = .{ 0.0, 0.0, 0.0 },
    scale: @Vector(4, f32) = .{ 1.0, 1.0, 1.0, 0.0 },
    rot_dirty: bool = false,
    local: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),
    world: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),

    pub fn draw_inspect(self: *Transform, world: *ecs.world_t, entity: ecs.entity_t) void {
        var to_set = false;
        {
            zgui.text("Position:", .{});
            zgui.sameLine(.{ .spacing = 200.0 });
            if (zgui.button("...", .{})) {
                zgui.openPopup("Component Context", .{});
            }
            if (zgui.beginPopup("Component Context", .{})) {
                if (zgui.selectable("Reset", .{})) {
                    _ = ecs.set(world, entity, Transform, .{});
                    zgui.closeCurrentPopup();
                }
                if (zgui.selectable("Delete", .{})) {
                    ecs.remove(world, entity, Transform);
                    zgui.closeCurrentPopup();
                }
                zgui.endPopup();
            }
            zgui.indent(.{});
            if (zgui.dragFloat("Pos X", .{ .v = &self.position[0] })) {
                to_set = true;
            }
            if (zgui.dragFloat("Pos Y", .{ .v = &self.position[1] })) {
                to_set = true;
            }
            if (zgui.dragFloat("Pos Z", .{ .v = &self.position[2] })) {
                to_set = true;
            }
            zgui.unindent(.{});
        }
        {
            zgui.text("Scale:", .{});
            zgui.indent(.{});
            if (zgui.dragFloat("Scale X", .{ .v = &self.scale[0] })) {
                to_set = true;
            }
            if (zgui.dragFloat("Scale Y", .{ .v = &self.scale[1] })) {
                to_set = true;
            }
            if (zgui.dragFloat("Scale Z", .{ .v = &self.scale[2] })) {
                to_set = true;
            }
            zgui.unindent(.{});
        }
        {
            zgui.text("Rotation:", .{});
            zgui.indent(.{});
            if (zgui.dragFloat("Pitch", .{ .v = &self.euler_angles[0], .min = -360.0, .max = 360.0 })) {
                to_set = true;
                self.rot_dirty = true;
            }
            if (zgui.dragFloat("Yaw", .{ .v = &self.euler_angles[1], .min = -360.0, .max = 360.0 })) {
                to_set = true;
                self.rot_dirty = true;
            }
            if (zgui.dragFloat("Roll", .{ .v = &self.euler_angles[2], .min = -360.0, .max = 360.0 })) {
                to_set = true;
                self.rot_dirty = true;
            }
            zgui.unindent(.{});
        }
        if (to_set) {
            self.calculate_local();
            _ = ecs.set(world, entity, Transform, self.*);
        }
        zgui.spacing();
        zgui.separator();
    }

    pub fn calculate_local(self: *Transform) void {
        const scaling: zm.Mat = zm.scaling(self.scale[0], self.scale[1], self.scale[2]);
        if (self.rot_dirty) {
            self.rotation = zm.quatFromRollPitchYaw(self.euler_angles[0], self.euler_angles[1], self.euler_angles[2]);
        }
        const matFromQuat: zm.Mat = zm.matFromQuat(self.rotation);
        const rotation: zm.Mat = zm.mul(scaling, matFromQuat);
        self.local = zm.mul(rotation, zm.translationV(self.position));
    }
};

pub const Mesh = struct {
    guid: [64]u8,
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
};

// This must be populated with all inspectable components
pub fn inspect_entity_components(world: *ecs.world_t, entity: ecs.entity_t) void {
    inspect_components(Transform, world, entity);
}

fn inspect_components(comptime T: anytype, world: *ecs.world_t, entity: ecs.entity_t) void {
    if (ecs.get(world, entity, T)) |val| {
        if (!ecs.is_valid(world, entity) or !ecs.is_alive(world, entity)) return;
        var val_copy = val.*;
        val_copy.draw_inspect(world, entity);
    }
}
