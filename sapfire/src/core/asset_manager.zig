const std = @import("std");
const zgui = @import("zgui");
const log = @import("logger.zig");
const crypto = std.crypto;
const json = std.json;
const jobs = @import("jobs.zig");
const nfd = @import("nfd");

const sf = struct {
    usingnamespace @import("../renderer/texture.zig");
    usingnamespace @import("../renderer/mesh.zig");
    usingnamespace @import("../renderer/material.zig");
    usingnamespace @import("../scene/scene.zig");
};

// TODO: add SceneAssetManager.
// The purpose is to separate runtime and edittime asset managers.
// Editor asset manager will load ALL assets to memory, while runtime asset manager will only load those used in the scene.
pub const AssetManager = struct {
    allocator: std.mem.Allocator,
    parse_arena: std.heap.ArenaAllocator,
    texture_manager: sf.TextureManager,
    material_manager: sf.MaterialManager,
    mesh_manager: sf.MeshManager,
    scene_manager: sf.SceneManager,

    pub fn texture_manager() *sf.TextureManager {
        return &instance.texture_manager;
    }

    pub fn material_manager() *sf.MaterialManager {
        return &instance.material_manager;
    }

    pub fn mesh_manager() *sf.MeshManager {
        return &instance.mesh_manager;
    }

    pub fn scene_manager() *sf.SceneManager {
        return &instance.scene_manager();
    }

    // TODO: takes in path to "project" serialization which contains paths to each subsystem's serialization file.
    // Each submodule will have its own file so that project's assets can be loaded in parallel
    pub fn init(
        allocator: std.mem.Allocator,
        project_config: []const u8,
    ) !void {
        instance.allocator = allocator;
        var arena = std.heap.ArenaAllocator.init(allocator);
        defer arena.deinit();
        const config_data = std.fs.cwd().readFileAlloc(arena.allocator(), project_config, 512 * 16) catch |e| {
            log.err("Failed to parse project config file.", .{});
            return e;
        };
        const Config = struct {
            texture_config: []const u8,
            material_config: []const u8,
            mesh_config: []const u8,
            scene_config: []const u8,
        };
        const config = try json.parseFromSliceLeaky(Config, arena.allocator(), config_data, .{});

        _ = try jobs.JobsManager.jobs().schedule(jobs.JobId.none, struct {
            allocator: std.mem.Allocator,
            path: []const u8,
            texman: *sf.TextureManager,
            pub fn exec(self: *@This()) void {
                self.texman.* = sf.TextureManager.init(self.allocator, self.path) catch {
                    return;
                };
            }
        }{
            .allocator = allocator,
            .path = config.texture_config,
            .texman = &instance.texture_manager,
        });
        jobs.JobsManager.jobs().start();
        const material_job = try jobs.JobsManager.jobs().schedule(jobs.JobId.none, struct {
            allocator: std.mem.Allocator,
            path: []const u8,
            matman: *sf.MaterialManager,
            pub fn exec(self: *@This()) void {
                self.matman.* = sf.MaterialManager.init(self.allocator, self.path) catch {
                    return;
                };
            }
        }{
            .allocator = allocator,
            .path = config.material_config,
            .matman = &instance.material_manager,
        });

        _ = try jobs.JobsManager.jobs().schedule(material_job, struct {
            allocator: std.mem.Allocator,
            path: []const u8,
            meshman: *sf.MeshManager,
            pub fn exec(self: *@This()) void {
                self.meshman.* = sf.MeshManager.init(self.allocator, self.path) catch {
                    return;
                };
            }
        }{
            .allocator = allocator,
            .path = config.mesh_config,
            .meshman = &instance.mesh_manager,
        });
        _ = try jobs.JobsManager.jobs().schedule(jobs.JobId.none, struct {
            pub fn exec(_: *@This()) void {
                jobs.JobsManager.jobs().stop();
            }
        }{});
        jobs.JobsManager.jobs().join();
        instance.scene_manager = try sf.SceneManager.init(allocator, config.scene_config);
        instance.parse_arena = std.heap.ArenaAllocator.init(allocator);
    }

    pub fn deinit() void {
        sf.MaterialManager.deinit(&instance.material_manager);
        sf.MeshManager.deinit(&instance.mesh_manager);
        sf.TextureManager.deinit(&instance.texture_manager);
        sf.SceneManager.deinit(&instance.scene_manager);
    }

    // TODO: this should be used to import raw files and generate .sf* format assets
    pub fn create_asset(
        path: []const u8,
        asset_type: AssetType,
    ) void {
        _ = asset_type;
        var guid: [64]u8 = undefined;
        crypto.hash.sha2.Sha512.hash(path, guid[0..], .{});
    }

    pub fn generate_guid(path: [:0]const u8) [64]u8 {
        var guid: [64]u8 = undefined;
        crypto.hash.sha2.Sha512.hash(path, &guid, .{});
        return guid;
    }

    var selected_import_asset_type: AssetType = .Mesh;
    var selected_texture_in_asset_creation: ?[64]u8 = null;
    var asset_modal_open: bool = false;
    var new_asset_name_buf = [_]u8{0} ** 128;

    var selected_explorer_category: AssetType = .Mesh;
    var selected_asset_guid: ?[64]u8 = null;

    pub fn draw_explorer(self: *AssetManager) !void {
        if (zgui.begin("Asset explorer", .{})) {
            if (zgui.button("Add", .{})) {
                asset_modal_open = true;
                selected_texture_in_asset_creation = null;
                new_asset_name_buf = [_]u8{0} ** 128;
                zgui.openPopup("Asset import modal", .{});
            }
            if (zgui.beginPopupModal("Asset import modal", .{ .popen = &asset_modal_open })) {
                if (zgui.comboFromEnum("Asset type", &selected_import_asset_type)) {}
                switch (selected_import_asset_type) {
                    .Mesh => {
                        if (zgui.button("Import", .{})) {
                            const open_path = try nfd.openFileDialog("gltf", null);
                            if (open_path) |path| {
                                try self.mesh_manager.import_mesh(path);
                                asset_modal_open = false;
                            }
                        }
                    },
                    .Texture => {
                        if (zgui.button("Import", .{})) {
                            const open_path = try nfd.openFileDialog("png", null);
                            if (open_path) |path| {
                                try self.texture_manager.import_texture_asset(path);
                                asset_modal_open = false;
                            }
                        }
                    },
                    .Material => {
                        if (zgui.inputText("Name: ", .{
                            .buf = &new_asset_name_buf,
                            .flags = .{
                                .enter_returns_true = true,
                            },
                        })) {}

                        if (zgui.beginListBox("Texture:", .{})) {
                            var it = self.texture_manager.texture_assets_map.iterator();
                            while (it.next()) |entry| {
                                if (zgui.selectable(entry.value_ptr.path, .{ .selected = selected_texture_in_asset_creation != null and std.mem.eql(u8, &entry.value_ptr.guid, &selected_texture_in_asset_creation.?) })) {
                                    selected_texture_in_asset_creation = entry.value_ptr.guid;
                                }
                            }
                            zgui.endListBox();
                        }
                        if (zgui.button("Add Material", .{})) {
                            var size: usize = 0;
                            for (0..128) |i| {
                                if (new_asset_name_buf[i] == 0) {
                                    size = i;
                                    break;
                                }
                            }
                            if (size > 0) {
                                const material_path_json = try std.mem.concatWithSentinel(self.parse_arena.allocator(), u8, &.{ "project/materials/", new_asset_name_buf[0..size], ".json" }, 0);
                                const guid = generate_guid(material_path_json);
                                var material_asset = sf.MaterialAsset{
                                    .texture_guid = selected_texture_in_asset_creation,
                                    .path = material_path_json,
                                    .guid = guid,
                                };
                                try self.material_manager.material_asset_map.put(guid, material_asset);
                                { // new material .json
                                    var file = try std.fs.cwd().createFile(material_path_json, .{});
                                    defer file.close();
                                    var writer = file.writer();
                                    try json.stringify(sf.MaterialAsset.Config{ .texture_guid = material_asset.texture_guid }, .{}, writer);
                                }
                                { // serialize material_config.json
                                    try self.material_manager.path_database.append(material_path_json);
                                    var file = try std.fs.cwd().createFile("project/material_config.json", .{});
                                    defer file.close();
                                    var writer = file.writer();
                                    try json.stringify(sf.MaterialManager.Config{ .database = self.material_manager.path_database.items }, .{}, writer);
                                }
                                asset_modal_open = false;
                            }
                        }
                    },
                    else => {},
                }
                zgui.endPopup();
            }

            if (zgui.beginListBox("Asset List", .{})) {
                { // Mesh
                    zgui.dummy(.{ .w = 0.0, .h = 5.0 });
                    zgui.text("Meshes:", .{});
                    zgui.separator();
                    var it = self.mesh_manager.mesh_assets_map.iterator();
                    while (it.next()) |entry| {
                        if (zgui.selectable(entry.value_ptr.path, .{ .selected = selected_asset_guid != null and std.mem.eql(u8, &entry.value_ptr.guid, &selected_asset_guid.?) })) {
                            selected_explorer_category = .Mesh;
                            selected_asset_guid = entry.value_ptr.guid;
                        }
                        if (zgui.isItemClicked(.right)) {
                            selected_explorer_category = .Mesh;
                            selected_asset_guid = entry.value_ptr.guid;
                        }
                    }
                }
                { // Materials
                    zgui.dummy(.{ .w = 0.0, .h = 5.0 });
                    zgui.text("Materials:", .{});
                    zgui.separator();
                    var it = self.material_manager.material_asset_map.iterator();
                    while (it.next()) |entry| {
                        if (zgui.selectable(entry.value_ptr.path, .{ .selected = selected_asset_guid != null and std.mem.eql(u8, &entry.value_ptr.guid, &selected_asset_guid.?) })) {
                            selected_explorer_category = .Material;
                            selected_asset_guid = entry.value_ptr.guid;
                        }
                        if (zgui.isItemClicked(.right)) {
                            selected_explorer_category = .Material;
                            selected_asset_guid = entry.value_ptr.guid;
                        }
                    }
                }
                { // Materials
                    zgui.dummy(.{ .w = 0.0, .h = 5.0 });
                    zgui.text("Textures:", .{});
                    zgui.separator();
                    var it = self.texture_manager.texture_assets_map.iterator();
                    while (it.next()) |entry| {
                        if (zgui.selectable(entry.value_ptr.path, .{ .selected = selected_asset_guid != null and std.mem.eql(u8, &entry.value_ptr.guid, &selected_asset_guid.?) })) {
                            selected_explorer_category = .Texture;
                            selected_asset_guid = entry.value_ptr.guid;
                        }
                        if (zgui.isItemClicked(.right)) {
                            selected_explorer_category = .Texture;
                            selected_asset_guid = entry.value_ptr.guid;
                        }
                    }
                }
                if (zgui.isMouseClicked(.right)) {
                    zgui.openPopup("Asset context menu", .{});
                }

                if (selected_asset_guid != null and zgui.isWindowFocused(.{ .root_window = true, .child_windows = true }) and zgui.beginPopup("Asset context menu", .{})) {
                    if (zgui.selectable("Delete", .{})) {
                        switch (selected_explorer_category) {
                            .Material => {
                                var mat = self.material_manager.material_asset_map.get(selected_asset_guid.?).?;
                                try std.fs.cwd().deleteFile(mat.path);
                                var index_to_remove: usize = 0;
                                for (self.material_manager.path_database.items, 0..) |path, index| {
                                    if (std.mem.eql(u8, path, mat.path)) {
                                        index_to_remove = index;
                                        break;
                                    }
                                }
                                _ = self.material_manager.material_asset_map.remove(selected_asset_guid.?);
                                _ = self.material_manager.path_database.swapRemove(index_to_remove);
                                { // serialize material_config.json
                                    var file = try std.fs.cwd().createFile("project/material_config.json", .{});
                                    defer file.close();
                                    var writer = file.writer();
                                    try json.stringify(sf.MaterialManager.Config{ .database = self.material_manager.path_database.items }, .{}, writer);
                                }
                            },
                            .Texture => { // TODO: If the texture is used, it'll crash
                                var tex = self.texture_manager.texture_assets_map.get(selected_asset_guid.?).?;
                                var index_to_remove: usize = 0;
                                for (self.texture_manager.path_database.items, 0..) |path, index| {
                                    if (std.mem.eql(u8, path, tex.path)) {
                                        index_to_remove = index;
                                        break;
                                    }
                                }
                                _ = self.texture_manager.texture_assets_map.remove(selected_asset_guid.?);
                                _ = self.texture_manager.path_database.swapRemove(index_to_remove);
                                { // serialize texture_config.json
                                    var file = try std.fs.cwd().createFile("project/texture_config.json", .{});
                                    defer file.close();
                                    var writer = file.writer();
                                    try json.stringify(sf.MaterialManager.Config{ .database = self.material_manager.path_database.items }, .{}, writer);
                                }
                            },
                            .Mesh => {
                                var mesh = self.mesh_manager.mesh_assets_map.get(selected_asset_guid.?).?;
                                try std.fs.cwd().deleteFile(mesh.path);
                                var index_to_remove: usize = 0;
                                for (self.mesh_manager.path_database.items, 0..) |path, index| {
                                    if (std.mem.eql(u8, path, mesh.path)) {
                                        index_to_remove = index;
                                        break;
                                    }
                                }
                                _ = self.mesh_manager.mesh_assets_map.remove(selected_asset_guid.?);
                                _ = self.mesh_manager.path_database.swapRemove(index_to_remove);
                                { // serialize mesh_config.json
                                    var file = try std.fs.cwd().createFile("project/mesh_config.json", .{});
                                    defer file.close();
                                    var writer = file.writer();
                                    try json.stringify(sf.MaterialManager.Config{ .database = self.material_manager.path_database.items }, .{}, writer);
                                }
                            },
                            else => {},
                        }
                        zgui.closeCurrentPopup();
                    }
                    zgui.endPopup();
                }
                zgui.endListBox();
            }
        }
        zgui.end();
    }
};

pub const AssetType = enum {
    Texture,
    Material,
    Mesh,
    Scene,
};

var instance: AssetManager = undefined;
pub fn asset_manager() *AssetManager {
    return &instance;
}
