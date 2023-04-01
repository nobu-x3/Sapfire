use crate::resources;
use anyhow::*;
use image::GenericImageView;
use wgpu::{
    AddressMode, Device, Extent3d, FilterMode, ImageCopyTexture, ImageDataLayout, Origin3d, Queue,
    SamplerDescriptor, SurfaceConfiguration, TextureAspect, TextureDescriptor, TextureDimension,
    TextureFormat, TextureUsages, TextureViewDescriptor,
};
pub struct Texture {
    pub texture: wgpu::Texture,
    pub view: wgpu::TextureView,
    pub sampler: wgpu::Sampler,
}

impl Texture {
    pub const DEPTH_FORMAT: TextureFormat = TextureFormat::Depth32Float;

    pub fn new_depth_texture(
        device: &Device,
        config: &SurfaceConfiguration,
        label: &str,
    ) -> Texture {
        let size = Extent3d {
            width: config.width,
            height: config.height,
            depth_or_array_layers: 1,
        };
        let desc = &TextureDescriptor {
            label: Some(label),
            size,
            format: Texture::DEPTH_FORMAT,
            sample_count: 1,
            mip_level_count: 1,
            usage: TextureUsages::RENDER_ATTACHMENT | TextureUsages::TEXTURE_BINDING,
            dimension: TextureDimension::D2,
            view_formats: &[],
        };
        let texture = device.create_texture(&desc);
        let sampler = device.create_sampler(&SamplerDescriptor {
            address_mode_u: AddressMode::ClampToEdge,
            address_mode_v: AddressMode::ClampToEdge,
            address_mode_w: AddressMode::ClampToEdge,
            mag_filter: FilterMode::Linear,
            min_filter: FilterMode::Linear,
            mipmap_filter: FilterMode::Nearest,
            lod_min_clamp: 0.0,
            lod_max_clamp: 100.0,
            compare: Some(wgpu::CompareFunction::Less), // in case we want to render the depth texture
            ..Default::default()
        });
        let view = texture.create_view(&TextureViewDescriptor::default());
        Texture {
            texture,
            view,
            sampler,
        }
    }
}

pub fn from_image(
    device: &wgpu::Device,
    queue: &wgpu::Queue,
    img: &image::DynamicImage,
    label: &str,
) -> Result<Texture> {
    let diffuse_rgba = img.to_rgba8();
    let dimensions = img.dimensions();
    let texture_size = Extent3d {
        width: dimensions.0,
        height: dimensions.1,
        depth_or_array_layers: 1,
    };
    let texture = device.create_texture(&TextureDescriptor {
        label: Some(label),
        size: texture_size,
        mip_level_count: 1,
        sample_count: 1,
        dimension: TextureDimension::D2,
        format: TextureFormat::Rgba8UnormSrgb,
        usage: TextureUsages::TEXTURE_BINDING | TextureUsages::COPY_DST,
        view_formats: &[],
    });
    queue.write_texture(
        ImageCopyTexture {
            texture: &texture,
            mip_level: 0,
            origin: Origin3d::ZERO,
            aspect: TextureAspect::All,
        },
        &diffuse_rgba,
        ImageDataLayout {
            offset: 0,
            bytes_per_row: std::num::NonZeroU32::new(4 * dimensions.0),
            rows_per_image: std::num::NonZeroU32::new(dimensions.1),
        },
        texture_size,
    );
    let view = texture.create_view(&TextureViewDescriptor::default());
    let sampler = device.create_sampler(&SamplerDescriptor {
        address_mode_u: AddressMode::ClampToEdge,
        address_mode_v: AddressMode::ClampToEdge,
        address_mode_w: AddressMode::ClampToEdge,
        mag_filter: FilterMode::Linear,
        min_filter: FilterMode::Linear,
        mipmap_filter: FilterMode::Nearest,
        ..Default::default()
    });
    Ok(Texture {
        texture,
        sampler,
        view,
    })
}
pub fn from_bytes(
    device: &wgpu::Device,
    queue: &wgpu::Queue,
    diffuse_bytes: &[u8],
    label: &str,
) -> Result<Texture> {
    let diffuse_image = image::load_from_memory(diffuse_bytes).unwrap();
    from_image(device, queue, &diffuse_image, label)
}

pub fn load_texture(file_name: &str, device: &Device, queue: &Queue) -> anyhow::Result<Texture> {
    let data = resources::load_binary(file_name)?;
    from_bytes(device, queue, &data, file_name)
}
