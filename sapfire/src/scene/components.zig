const zm = @import("zmath");
const zgui = @import("zgui");
const std = @import("std");
const zgpu = @import("zgpu");
const ecs = @import("zflecs");
const asset = @import("../core/asset_manager.zig");
const AssetManager = asset.AssetManager;
const generate_guid = AssetManager.generate_guid;
const rendering = @import("../rendering.zig");
const Material = rendering.Material;

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

    pub fn draw_inspect(self: *Transform, world: *ecs.world_t, entity: ecs.entity_t, asset_manager: *AssetManager) void {
        _ = asset_manager;
        var to_set = false;
        {
            zgui.text("Position:", .{});
            if (zgui.button("Transform Options", .{})) {
                zgui.openPopup("Transform Component Context", .{});
            }
            if (zgui.beginPopup("Transform Component Context", .{})) {
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
        zgui.dummy(.{ .h = 5, .w = 0 });
    }

    pub fn calculate_local(self: *Transform) void {
        const scaling: zm.Mat = zm.scaling(self.scale[0], self.scale[1], self.scale[2]);
        if (self.rot_dirty) {
            self.rotation = zm.quatFromRollPitchYaw(std.math.degreesToRadians(f32, self.euler_angles[0]), std.math.degreesToRadians(f32, self.euler_angles[1]), std.math.degreesToRadians(f32, self.euler_angles[2]));
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

    pub fn draw_inspect(self: *Mesh, world: *ecs.world_t, entity: ecs.entity_t, asset_manager: *AssetManager) void {
        zgui.text("Mesh:", .{});
        if (zgui.button("Mesh Options", .{})) {
            zgui.openPopup("Mesh Component Context", .{});
        }
        if (zgui.beginPopup("Mesh Component Context", .{})) {
            if (zgui.selectable("Delete", .{})) {
                ecs.remove(world, entity, Mesh);
                zgui.closeCurrentPopup();
            }
            zgui.endPopup();
        }
        const maybe_scene = @import("scene.zig").Scene.scene;
        if (maybe_scene) |scene| {
            for (scene.asset.geometry_paths.items) |path| {
                if (std.mem.eql(u8, &self.guid, &generate_guid(path))) {
                    if (zgui.button(path, .{})) {
                        zgui.openPopup("Mesh menu", .{});
                    }
                }
            }
            if (zgui.beginPopup("Mesh menu", .{})) {
                var it = asset_manager.mesh_manager.mesh_assets_map.iterator();
                while (it.next()) |entry| {
                    if (zgui.selectable(entry.value_ptr.path, .{})) {
                        if (!scene.mesh_manager.mesh_map.contains(entry.value_ptr.guid)) {
                            const mesh = asset_manager.mesh_manager.mesh_map.get(entry.value_ptr.guid) orelse val: {
                                const mesh = rendering.MeshAsset.load_mesh(entry.value_ptr.path, &asset_manager.mesh_manager, null, &scene.vertices, &scene.indices) catch |e| {
                                    std.log.err("Failed to add mesh asset to the editor manager. {s}.", .{@typeName(@TypeOf(e))});
                                    zgui.endPopup();
                                    return;
                                };
                                break :val mesh;
                            };
                            scene.mesh_manager.mesh_map.put(mesh.guid, mesh) catch |e| {
                                std.log.err("Failed to add a new mesh to scene asset manager, {s}.", .{@typeName(@TypeOf(e))});
                                zgui.endPopup();
                                return;
                            };
                            scene.recreate_buffers();
                        }
                        _ = ecs.set(world, entity, Mesh, scene.mesh_manager.mesh_map.get(entry.value_ptr.guid).?);
                    }
                }
                zgui.endPopup();
            }
        }
        zgui.spacing();
        zgui.separator();
        zgui.dummy(.{ .h = 5, .w = 0 });
    }
};

// This must be populated with all inspectable components
pub fn inspect_entity_components(world: *ecs.world_t, entity: ecs.entity_t, asset_manager: *AssetManager) void {
    inspect_components(Transform, world, entity, asset_manager);
    inspect_components(Mesh, world, entity, asset_manager);
    inspect_components(Material, world, entity, asset_manager);
}

fn inspect_components(comptime T: anytype, world: *ecs.world_t, entity: ecs.entity_t, asset_manager: *AssetManager) void {
    if (ecs.get(world, entity, T)) |val| {
        if (!ecs.is_valid(world, entity) or !ecs.is_alive(world, entity)) return;
        var val_copy = val.*;
        val_copy.draw_inspect(world, entity, asset_manager);
    }
}
