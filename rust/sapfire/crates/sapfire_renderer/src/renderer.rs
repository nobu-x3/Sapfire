pub trait Renderer {
    fn add_shader(&mut self, vertex_path: &str, frag_path: &str);
}
