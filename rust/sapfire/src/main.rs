use sapfire_renderer::*;
fn main() {
    pollster::block_on(SapfireRenderer::new()).run();
}
