use sapfire_renderer::*;
fn main() {
    let mut window = Window::new(RenderingAPI::OpenGL);
    let context = window.create_context();
    window.run();
}
