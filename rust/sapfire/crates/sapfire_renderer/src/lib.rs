extern crate glfw;
use glfw::{Context, Key, WindowEvent};
extern crate gl;
use std::sync::mpsc::Receiver;


pub struct Renderer {
    window: glfw::Window,
    glfw: glfw::Glfw,
    events: Receiver<(f64, WindowEvent)>,
}

impl Renderer {
    pub fn new() -> Renderer {
        let mut glfw = glfw::init(glfw::FAIL_ON_ERRORS).unwrap();
        glfw.window_hint(glfw::WindowHint::OpenGlProfile(glfw::OpenGlProfileHint::Any));
        let (mut window, events) = glfw
            .create_window(800, 600, "Sapfire", glfw::WindowMode::Windowed)
            .expect("Failed to open glfw window");
        gl::load_with(|s| window.get_proc_address(s));
        glfw.set_swap_interval(glfw::SwapInterval::Sync(1));
        window.set_key_polling(true);
        window.make_current();
        Renderer {
            window: window,
            glfw: glfw,
            events: events,
        }
    }
    pub fn run(&mut self) {
        while !self.window.should_close() {
            self.glfw.poll_events();
            for (_, event) in glfw::flush_messages(&self.events) {
                Renderer::on_input(&mut self.window, event);
            }
            unsafe
            {
                gl::ClearColor(1.0, 0.3, 0.3, 1.0);
                gl::Clear(gl::COLOR_BUFFER_BIT);
                self.window.swap_buffers();
            }
        }
    }

    fn on_input(window: &mut glfw::Window, event: glfw::WindowEvent) {
        match event {
            glfw::WindowEvent::Key(Key::Escape, _, glfw::Action::Press, _) => {
                println!("Exiting...");
                window.set_should_close(true)
            }
            _ => {}
        }
    }
}
