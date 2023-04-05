use legion::{*, world::SubWorld};
use sapfire_renderer::{VirtualKeyCode};

use crate::{components::{self, TransformComponent, CameraComponent}, CameraSpeed};

#[system]
#[write_component(components::TransformComponent)]
#[read_component(components::CameraComponent)]
pub fn input(ecs: &mut SubWorld,
#[resource] key: &Option<VirtualKeyCode>,
#[resource] camera_speed : &CameraSpeed){
    if let Some(key) = key{
        let mut deltaPos = glam::Vec3::ZERO;
                match key {
                    VirtualKeyCode::W | VirtualKeyCode::Up => {
                        deltaPos += camera_speed.speed * glam::Vec3::NEG_Z
                    }
                    VirtualKeyCode::A | VirtualKeyCode::Left => {
                        deltaPos += camera_speed.speed * glam::Vec3::NEG_X
                    }
                    VirtualKeyCode::S | VirtualKeyCode::Down => {
                        deltaPos += camera_speed.speed * glam::Vec3::Z
                    }
                    VirtualKeyCode::D | VirtualKeyCode::Right => {
                        deltaPos += camera_speed.speed * glam::Vec3::X
                    }
                    VirtualKeyCode::E => {
                        deltaPos += camera_speed.speed * glam::Vec3::Y
                    }
                    VirtualKeyCode::Q => {
                        deltaPos += camera_speed.speed * glam::Vec3::NEG_Y
                    }
                    _ => {}
                }
                if deltaPos != glam::Vec3::ZERO{
                    let mut cameras = <&mut TransformComponent>::query().filter(component::<CameraComponent>());
                    if let Some(camera_transform) = cameras.iter_mut(ecs).next(){
                        // camera_transform.translation = glam::vec3(camera_transform.translation.x + deltaPos.x, camera_transform.translation.y + deltaPos.y, camera_transform.translation.z + deltaPos.z);
                        camera_transform.translation += deltaPos;
                    }
                }
    };
}