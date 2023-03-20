extern crate glfw;
use glfw::{Context, Key, WindowEvent};
extern crate gl;
use gl::*;
use std::{
    mem::{size_of, size_of_val},
    sync::mpsc::Receiver,
};

pub struct Renderer {
    window: glfw::Window,
    glfw: glfw::Glfw,
    events: Receiver<(f64, WindowEvent)>,
    vao: u32,
    vbo: u32,
    shader_program: u32,
}

type Vertex = [f32; 3];
const VERTICES: [Vertex; 3] = [[-0.5, -0.5, 0.0], [0.5, -0.5, 0.0], [0.0, 0.5, 0.0]];
const VERT_SHADER: &str = r#"#version 330 core
  layout (location = 0) in vec3 pos;
  void main() {
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
  }
"#;
const FRAG_SHADER: &str = r#"#version 330 core
  out vec4 final_color;

  void main() {
    final_color = vec4(1.0, 1.0, 1.0, 1.0);
  }
"#;
impl Renderer {
    pub fn new() -> Renderer {
        let mut glfw = glfw::init(glfw::FAIL_ON_ERRORS).unwrap();
        glfw.window_hint(glfw::WindowHint::OpenGlProfile(
            glfw::OpenGlProfileHint::Any,
        ));
        let (mut window, events) = glfw
            .create_window(800, 600, "Sapfire", glfw::WindowMode::Windowed)
            .expect("Failed to open glfw window");
        gl::load_with(|s| window.get_proc_address(s));
        glfw.set_swap_interval(glfw::SwapInterval::Sync(1));
        window.set_key_polling(true);
        window.make_current();
        unsafe {
            let mut vao = 0;
            gl::GenVertexArrays(1, &mut vao);
            assert_ne!(vao, 0);
            let mut vbo = 0;
            gl::GenBuffers(1, &mut vbo);
            assert_ne!(vbo, 0);
            gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
            gl::BufferData(
                gl::ARRAY_BUFFER,
                size_of_val(&VERTICES) as isize,
                VERTICES.as_ptr().cast(),
                gl::STATIC_DRAW,
            );
            gl::VertexAttribPointer(
                0,
                3,
                FLOAT,
                FALSE,
                size_of::<Vertex>().try_into().unwrap(),
                0 as *const _,
            );
            gl::EnableVertexAttribArray(0);
            let vertex_shader = gl::CreateShader(VERTEX_SHADER);
            assert_ne!(vertex_shader, 0);
            gl::ShaderSource(
                vertex_shader,
                1,
                &(VERT_SHADER.as_bytes().as_ptr().cast()),
                &(VERT_SHADER.len().try_into().unwrap()),
            );
            gl::CompileShader(vertex_shader);
            {
                let mut success = 0;
                GetShaderiv(vertex_shader, COMPILE_STATUS, &mut success);
                if success == 0 {
                    let mut v: Vec<u8> = Vec::with_capacity(1024);
                    let mut log_len = 0_i32;
                    GetShaderInfoLog(vertex_shader, 1024, &mut log_len, v.as_mut_ptr().cast());
                    v.set_len(log_len.try_into().unwrap());
                    panic!("Vertex compile error: {}", String::from_utf8_lossy(&v));
                }
            }
            let fragment_shader = CreateShader(FRAGMENT_SHADER);
            assert_ne!(fragment_shader, 0);
            ShaderSource(
                fragment_shader,
                1,
                &(FRAG_SHADER.as_bytes().as_ptr().cast()),
                &(FRAG_SHADER.len().try_into().unwrap()),
            );
            CompileShader(fragment_shader);
            {
                let mut success = 0;
                GetShaderiv(fragment_shader, COMPILE_STATUS, &mut success);
                if success == 0 {
                    let mut v: Vec<u8> = Vec::with_capacity(1024);
                    let mut log_len = 0_i32;
                    GetShaderInfoLog(vertex_shader, 1024, &mut log_len, v.as_mut_ptr().cast());
                    v.set_len(log_len.try_into().unwrap());
                    panic!("Fragment compile error: {}", String::from_utf8_lossy(&v));
                }
            }
            let shader_program = CreateProgram();
            AttachShader(shader_program, vertex_shader);
            AttachShader(shader_program, fragment_shader);
            LinkProgram(shader_program);
            {
                let mut success = 0;
                GetProgramiv(shader_program, LINK_STATUS, &mut success);
                if success == 0 {
                    let mut v: Vec<u8> = Vec::with_capacity(1024);
                    let mut log_len = 0_i32;
                    GetProgramInfoLog(shader_program, 1024, &mut log_len, v.as_mut_ptr().cast());
                    v.set_len(log_len.try_into().unwrap());
                    panic!("Program Link Error: {}", String::from_utf8_lossy(&v));
                }
            }
            DeleteShader(vertex_shader);
            DeleteShader(fragment_shader);
            UseProgram(shader_program);
            Renderer {
                window,
                glfw,
                events,
                vao,
                vbo,
                shader_program,
            }
        }
    }

    pub fn run(&mut self) {
        while !self.window.should_close() {
            self.glfw.poll_events();
            for (_, event) in glfw::flush_messages(&self.events) {
                Renderer::on_input(&mut self.window, event);
            }
            unsafe {
                gl::Clear(gl::COLOR_BUFFER_BIT);
                DrawArrays(TRIANGLES, 0, 3);
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
