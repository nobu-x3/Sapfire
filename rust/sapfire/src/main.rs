extern crate glfw;
use glfw::{Action, Context, Key};
use glium::glutin::event::Event;

fn main() {
    let mut glfw = glfw::init(glfw::FAIL_ON_ERRORS).unwrap();
    let (mut window, events) = glfw.create_window(800, 600, "Sapfire", glfw::WindowMode::Windowed)
        .expect("Failed to open glfw window");
    window.set_key_polling(true);
    window.make_current();
    while !window.should_close()
    {
        glfw.poll_events();
        for(_, event) in glfw::flush_messages(&events)
        {
            on_input(&mut window, event);
        }
    }
}

fn on_input(window: &mut glfw::Window, event: glfw::WindowEvent)
{
    match event
    {
        glfw::WindowEvent::Key(Key::Escape, _, glfw::Action::Press, _) =>
        {
            window.set_should_close(true)
        }
        _ => {}
    }
}
