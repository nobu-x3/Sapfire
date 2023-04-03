use sapfire_renderer::*;
fn main() {
    let res = pollster::block_on(SapfireRenderer::new());
    SapfireRenderer::run(res);
}
