use crate::shader::Shader;
use gl::*;
use std::{fs::File, io::Read};
pub struct OpenGLShader {
    pub shader_program: u32,
}

impl OpenGLShader {
    // pub fn new(path_vertex: &str, path_fragment: &str) -> OpenGLShader {
    //     unsafe {
    //         // TODO: make this not panic but read a magenta shader
    //         let mut file_vertex = File::open(path_vertex)
    //             .unwrap_or_else(|_| panic!("Failed to open vertex shader {}", path_vertex));
    //         let mut file_frag = File::open(path_fragment)
    //             .unwrap_or_else(|_| panic!("Failed to open {}", path_fragment));
    //         let mut source_vertex = String::from("");
    //         let mut source_frag = String::from("");
    //         file_vertex
    //             .read_to_string(&mut source_vertex)
    //             .expect("Failed to read vertex shader source");
    //         file_frag
    //             .read_to_string(&mut source_frag)
    //             .expect("Failed to read fragment shader source");
    //         let vertex_shader = CreateShader(VERTEX_SHADER);
    //         assert_ne!(vertex_shader, 0);
    //         ShaderSource(
    //             vertex_shader,
    //             1,
    //             &(source_vertex.as_bytes().as_ptr().cast()),
    //             &(source_vertex.len().try_into().unwrap()),
    //         );
    //         CompileShader(vertex_shader);
    //         {
    //             let mut success = 0;
    //             GetShaderiv(vertex_shader, COMPILE_STATUS, &mut success);
    //             if success == 0 {
    //                 let mut v: Vec<u8> = Vec::with_capacity(1024);
    //                 let mut log_len = 0_i32;
    //                 GetShaderInfoLog(vertex_shader, 1024, &mut log_len, v.as_mut_ptr().cast());
    //                 v.set_len(log_len.try_into().unwrap());
    //                 panic!("Vertex compile error: {}", String::from_utf8_lossy(&v));
    //             }
    //         }
    //         let fragment_shader = CreateShader(FRAGMENT_SHADER);
    //         assert_ne!(fragment_shader, 0);
    //         ShaderSource(
    //             fragment_shader,
    //             1,
    //             &(source_frag.as_bytes().as_ptr().cast()),
    //             &(source_frag.len().try_into().unwrap()),
    //         );
    //         CompileShader(fragment_shader);
    //         {
    //             let mut success = 0;
    //             GetShaderiv(fragment_shader, COMPILE_STATUS, &mut success);
    //             if success == 0 {
    //                 let mut v: Vec<u8> = Vec::with_capacity(1024);
    //                 let mut log_len = 0_i32;
    //                 GetShaderInfoLog(vertex_shader, 1024, &mut log_len, v.as_mut_ptr().cast());
    //                 v.set_len(log_len.try_into().unwrap());
    //                 panic!("Fragment compile error: {}", String::from_utf8_lossy(&v));
    //             }
    //         }
    //         let shader_program = CreateProgram();
    //         AttachShader(shader_program, vertex_shader);
    //         AttachShader(shader_program, fragment_shader);
    //         LinkProgram(shader_program);
    //         {
    //             let mut success = 0;
    //             GetProgramiv(shader_program, LINK_STATUS, &mut success);
    //             if success == 0 {
    //                 let mut v: Vec<u8> = Vec::with_capacity(1024);
    //                 let mut log_len = 0_i32;
    //                 GetProgramInfoLog(shader_program, 1024, &mut log_len, v.as_mut_ptr().cast());
    //                 v.set_len(log_len.try_into().unwrap());
    //                 panic!("Program Link Error: {}", String::from_utf8_lossy(&v));
    //             }
    //         }
    //         DeleteShader(vertex_shader);
    //         DeleteShader(fragment_shader);
    //         UseProgram(shader_program);
    //         return OpenGLShader { shader_program };
    //     }
    // }

    pub fn new(path: &str) -> OpenGLShader {
        let (vertex, fragment) = OpenGLShader::parse(path);
        unsafe {
            let shader_program = CreateProgram();
            AttachShader(shader_program, vertex);
            AttachShader(shader_program, fragment);
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
            DeleteShader(vertex);
            DeleteShader(fragment);
            UseProgram(shader_program);
            return OpenGLShader { shader_program };
        }
    }

    const TYPE_TOKEN: &str = "#type";

    pub fn parse(path: &str) -> (u32, u32) {
        let mut file = File::open(path).expect("File does not exist");
        let mut parsed_file = String::from("");
        file.read_to_string(&mut parsed_file).unwrap();
        let mut vertex_shader: u32 = 0;
        let mut fragment_shader: u32 = 0;
        for shader_src in parsed_file.split(OpenGLShader::TYPE_TOKEN) {
            match shader_src
                .lines()
                .find(|&x| x.contains("vert") || x.contains("vertex"))
            {
                Some(x) => {
                    let parsed = shader_src.replacen(x, "\n\r", 1);
                    unsafe {
                        vertex_shader = CreateShader(VERTEX_SHADER);
                        assert_ne!(vertex_shader, 0);
                        ShaderSource(
                            vertex_shader,
                            1,
                            &(parsed.as_bytes().as_ptr().cast()),
                            &(parsed.len().try_into().unwrap()),
                        );
                        CompileShader(vertex_shader);
                        {
                            let mut success = 0;
                            GetShaderiv(vertex_shader, COMPILE_STATUS, &mut success);
                            if success == 0 {
                                let mut v: Vec<u8> = Vec::with_capacity(1024);
                                let mut log_len = 0_i32;
                                GetShaderInfoLog(
                                    vertex_shader,
                                    1024,
                                    &mut log_len,
                                    v.as_mut_ptr().cast(),
                                );
                                v.set_len(log_len.try_into().unwrap());
                                panic!("Vertex compile error: {}", String::from_utf8_lossy(&v));
                            }
                        }
                    }
                }
                None => {}
            };
            match shader_src
                .lines()
                .find(|&x| x.contains("frag") || x.contains("fragment"))
            {
                Some(x) => {
                    let parsed = shader_src.replacen(x, "\n\r", 1);
                    unsafe {
                        fragment_shader = CreateShader(FRAGMENT_SHADER);
                        assert_ne!(fragment_shader, 0);
                        ShaderSource(
                            fragment_shader,
                            1,
                            &(parsed.as_bytes().as_ptr().cast()),
                            &(parsed.len().try_into().unwrap()),
                        );
                        CompileShader(fragment_shader);
                        {
                            let mut success = 0;
                            GetShaderiv(fragment_shader, COMPILE_STATUS, &mut success);
                            if success == 0 {
                                let mut v: Vec<u8> = Vec::with_capacity(1024);
                                let mut log_len = 0_i32;
                                GetShaderInfoLog(
                                    vertex_shader,
                                    1024,
                                    &mut log_len,
                                    v.as_mut_ptr().cast(),
                                );
                                v.set_len(log_len.try_into().unwrap());
                                panic!("Fragment compile error: {}", String::from_utf8_lossy(&v));
                            }
                        }
                    }
                }
                None => {}
            }
        }
        return (vertex_shader, fragment_shader);
    }
}

impl Shader for OpenGLShader {
    fn bind(&self) {
        unsafe {
            UseProgram(self.shader_program);
        }
    }

    fn set_matrix_uniform(&self, name: &str, matrix: &glam::Mat4) {
        unsafe {
            let loc = GetUniformLocation(self.shader_program, name.as_ptr().cast());
            UniformMatrix4fv(loc, 1, TRUE, matrix.as_ref().as_ptr());
        }
    }
    fn set_vector_uniform(&self, name: &str, vector: &glam::Vec3) {
        unsafe {
            let loc = GetUniformLocation(self.shader_program, name.as_ptr().cast());
            Uniform3fv(loc, 1, vector.as_ref().as_ptr());
        }
    }
}
