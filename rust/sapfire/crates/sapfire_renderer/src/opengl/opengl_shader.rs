pub mod shader;
pub mod opengl {
    pub struct OpenGLShader {
        shader_program: u32,
    }

    impl Shader for OpenGLShader {
        fn new(path: &str) -> OpenGLShader {
            unsafe {
                let vertex_shader = CreateShader(VERTEX_SHADER);
                assert_ne!(vertex_shader, 0);
                ShaderSource(
                    vertex_shader,
                    1,
                    &(VERT_SHADER.as_bytes().as_ptr().cast()),
                    &(VERT_SHADER.len().try_into().unwrap()),
                );
                CompileShader(vertex_shader);
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
                        GetProgramInfoLog(
                            shader_program,
                            1024,
                            &mut log_len,
                            v.as_mut_ptr().cast(),
                        );
                        v.set_len(log_len.try_into().unwrap());
                        panic!("Program Link Error: {}", String::from_utf8_lossy(&v));
                    }
                }
                DeleteShader(vertex_shader);
                DeleteShader(fragment_shader);
                UseProgram(shader_program);
                return OpenGLShader { shader_program };
            }
        }
        fn bind();
        fn set_matrix_uniform(name: &str, matrix: &glam::mat4);
        fn set_vector_uniform(name: &str, vector: &glam::vec3);
        fn get_program_id() -> u32;
    }
}
