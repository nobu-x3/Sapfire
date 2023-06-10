use legion::{world::SubWorld, *};
use sapfire_renderer::VirtualKeyCode;

use crate::{
    components::{self, CameraComponent, RenderComponent, TransformComponent},
    CameraSpeed,
};

#[system]
#[write_component(components::TransformComponent)]
#[read_component(components::CameraComponent)]
pub fn input(
    ecs: &mut SubWorld,
    #[resource] key: &Option<VirtualKeyCode>,
    #[resource] camera_speed: &CameraSpeed,
) {
    if let Some(key) = key {
        let mut delta_pos = glam::Vec3::ZERO;
        match key {
            VirtualKeyCode::W | VirtualKeyCode::Up => {
                delta_pos += camera_speed.speed * glam::Vec3::NEG_Z
            }
            VirtualKeyCode::A | VirtualKeyCode::Left => {
                delta_pos += camera_speed.speed * glam::Vec3::X
            }
            VirtualKeyCode::S | VirtualKeyCode::Down => {
                delta_pos += camera_speed.speed * glam::Vec3::Z
            }
            VirtualKeyCode::D | VirtualKeyCode::Right => {
                delta_pos += camera_speed.speed * glam::Vec3::NEG_X
            }
            VirtualKeyCode::E => delta_pos += camera_speed.speed * glam::Vec3::NEG_Y,
            VirtualKeyCode::Q => delta_pos += camera_speed.speed * glam::Vec3::Y,
            _ => {}
        }
        if delta_pos != glam::Vec3::ZERO {
            let mut cameras =
                <&mut TransformComponent>::query().filter(component::<CameraComponent>());
            if let Some(camera_transform) = cameras.iter_mut(ecs).next() {
                // camera_transform.translation = glam::vec3(camera_transform.translation.x + deltaPos.x, camera_transform.translation.y + deltaPos.y, camera_transform.translation.z + deltaPos.z);
                camera_transform.translation += delta_pos;
            }
        }
    };
}

#[system]
#[read_component(components::RenderComponent)]
#[read_component(components::TransformComponent)]
pub fn render(
    ecs: &mut SubWorld,
    #[resource] render_pipeline: &sapfire_renderer::wgpu_wrapper::RenderPipeline,
    #[resource] render_pass: &mut sapfire_renderer::wgpu_wrapper::RenderPass,
) {
    let mut query = <(&RenderComponent, &TransformComponent)>::query();
    query.iter_mut(ecs).for_each(|res| {
        render_pass.set_pipeline(render_pipeline);
    })
}
