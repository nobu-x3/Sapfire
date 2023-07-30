const zm = @import("zmath");
const zgui = @import("zgui");
const std = @import("std");
const zgpu = @import("zgpu");
const ecs = @import("zflecs");

pub const ComponentTypes = enum {
    transform,
    scale,
    mesh,
    material,
};

pub const name_type_map = std.ComptimeStringMap(ComponentTypes, .{
    .{ "scene.components.Transform", .transform },
    .{ "scene.components.Scale", .scale },
    .{ "scene.components.Mesh", .mesh },
    .{ "renderer.material.Material", .material },
});

pub const Transform = extern struct {
    local: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),
    world: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),

    pub fn draw_inspect(self: *Transform, world: *ecs.world_t, entity: ecs.entity_t) void {
        {
            zgui.text("Position:", .{});
            zgui.sameLine(.{ .spacing = 200.0 });
            if (zgui.button("...", .{})) {
                zgui.openPopup("Component Context", .{});
            }
            if (zgui.beginPopup("Component Context", .{})) {
                if (zgui.selectable("Reset", .{})) {
                    _ = ecs.set(world, entity, Transform, .{});
                    _ = ecs.set(world, entity, Scale, .{});
                    zgui.closeCurrentPopup();
                }
                if (zgui.selectable("Delete", .{})) {
                    ecs.remove(world, entity, Transform);
                    ecs.remove(world, entity, Scale);
                    zgui.closeCurrentPopup();
                }
                zgui.endPopup();
            }
            zgui.indent(.{});
            if (zgui.dragFloat("Pos X", .{ .v = &self.local[3][0] })) {
                _ = ecs.set(world, entity, Transform, self.*);
            }
            if (zgui.dragFloat("Pos Y", .{ .v = &self.local[3][1] })) {
                _ = ecs.set(world, entity, Transform, self.*);
            }
            if (zgui.dragFloat("Pos Z", .{ .v = &self.local[3][2] })) {
                _ = ecs.set(world, entity, Transform, self.*);
            }
            zgui.unindent(.{});
        }
        {
            zgui.text("Scale:", .{});
            zgui.indent(.{});
            if (ecs.get(world, entity, Scale)) |ro_scale| {
                var scale = ro_scale.*;
                if (zgui.dragFloat("Scale X", .{ .v = &scale.scale[0] })) {
                    _ = ecs.set(world, entity, Scale, scale);
                    var scale_mat: zm.Mat = zm.scaling(scale.scale[0], scale.scale[1], scale.scale[2]);
                    var quat: zm.Quat = zm.quatFromMat(self.local);
                    var rot: zm.Mat = zm.matFromQuat(quat);
                    var rot_mat: zm.Mat = zm.mul(scale_mat, rot);
                    var translate: zm.Mat = zm.translation(self.local[3][0], self.local[3][1], self.local[3][2]);
                    self.local = zm.mul(rot_mat, translate);
                    _ = ecs.set(world, entity, Transform, self.*);
                }
                if (zgui.dragFloat("Scale Y", .{ .v = &scale.scale[1] })) {
                    _ = ecs.set(world, entity, Scale, scale);
                    var scale_mat: zm.Mat = zm.scaling(scale.scale[0], scale.scale[1], scale.scale[2]);
                    var quat: zm.Quat = zm.quatFromMat(self.local);
                    var rot: zm.Mat = zm.matFromQuat(quat);
                    var rot_mat: zm.Mat = zm.mul(scale_mat, rot);
                    var translate: zm.Mat = zm.translation(self.local[3][0], self.local[3][1], self.local[3][2]);
                    self.local = zm.mul(rot_mat, translate);
                    _ = ecs.set(world, entity, Transform, self.*);
                }
                if (zgui.dragFloat("Scale Z", .{ .v = &scale.scale[2] })) {
                    _ = ecs.set(world, entity, Scale, scale);
                    var scale_mat: zm.Mat = zm.scaling(scale.scale[0], scale.scale[1], scale.scale[2]);
                    var quat: zm.Quat = zm.quatFromMat(self.local);
                    var rot: zm.Mat = zm.matFromQuat(quat);
                    var rot_mat: zm.Mat = zm.mul(scale_mat, rot);
                    var translate: zm.Mat = zm.translation(self.local[3][0], self.local[3][1], self.local[3][2]);
                    self.local = zm.mul(rot_mat, translate);
                    _ = ecs.set(world, entity, Transform, self.*);
                }
                zgui.unindent(.{});
            } else return;
        }
        {
            zgui.text("Rotation:", .{});
            zgui.indent(.{});
            var quat = zm.quatFromMat(self.local);
            const rpy = zm.quatToRollPitchYaw(quat);
            var x_angle_deg = std.math.radiansToDegrees(f32, rpy[0]);
            var y_angle_deg = std.math.radiansToDegrees(f32, rpy[1]);
            var z_angle_deg = std.math.radiansToDegrees(f32, rpy[2]);
            const scale = ecs.get(world, entity, Scale).?.scale;
            if (zgui.dragFloat("Pitch", .{ .v = &x_angle_deg })) {
                var new_rot: zm.Quat = zm.quatFromRollPitchYaw(std.math.degreesToRadians(f32, x_angle_deg), std.math.degreesToRadians(f32, y_angle_deg), std.math.degreesToRadians(f32, z_angle_deg));
                var rot_mat: zm.Mat = zm.matFromQuat(new_rot);
                const scaling: zm.Mat = zm.scaling(scale[0], scale[1], scale[2]);
                const rotation: zm.Mat = zm.mul(scaling, rot_mat);
                const pos = [3]f32{ self.local[3][0], self.local[3][1], self.local[3][2] };
                self.local = zm.mul(rotation, zm.translationV(zm.loadArr3(pos)));
                _ = ecs.set(world, entity, Transform, self.*);
            }
            if (zgui.dragFloat("Yaw", .{ .v = &y_angle_deg })) {
                var new_rot: zm.Quat = zm.quatFromRollPitchYaw(std.math.degreesToRadians(f32, x_angle_deg), std.math.degreesToRadians(f32, y_angle_deg), std.math.degreesToRadians(f32, z_angle_deg));
                var rot_mat: zm.Mat = zm.matFromQuat(new_rot);
                const scaling: zm.Mat = zm.scaling(scale[0], scale[1], scale[2]);
                const rotation: zm.Mat = zm.mul(scaling, rot_mat);
                const pos = [3]f32{ self.local[3][0], self.local[3][1], self.local[3][2] };
                self.local = zm.mul(rotation, zm.translationV(zm.loadArr3(pos)));
                _ = ecs.set(world, entity, Transform, self.*);
            }
            if (zgui.dragFloat("Roll", .{ .v = &z_angle_deg })) {
                var new_rot: zm.Quat = zm.quatFromRollPitchYaw(std.math.degreesToRadians(f32, x_angle_deg), std.math.degreesToRadians(f32, y_angle_deg), std.math.degreesToRadians(f32, z_angle_deg));
                var rot_mat: zm.Mat = zm.matFromQuat(new_rot);
                const scaling: zm.Mat = zm.scaling(scale[0], scale[1], scale[2]);
                const rotation: zm.Mat = zm.mul(scaling, rot_mat);
                const pos = [3]f32{ self.local[3][0], self.local[3][1], self.local[3][2] };
                self.local = zm.mul(rotation, zm.translationV(zm.loadArr3(pos)));
                _ = ecs.set(world, entity, Transform, self.*);
            }
            zgui.unindent(.{});
        }

        zgui.spacing();
        zgui.separator();
    }
};

pub const Scale = extern struct {
    scale: @Vector(3, f32) = .{ 1.0, 1.0, 1.0 },
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
