use sapfire_app::*;
fn main() {
    let (app, renderer, event_loop) = pollster::block_on(App::new());
    App::run(app, renderer, event_loop);
}
