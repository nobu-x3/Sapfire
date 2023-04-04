mod components;
use legion::*;
use sapfire_renderer::*;

struct App {
    ecs: World,
    resources: Resources,
    systems: Schedule,
    renderer: SapfireRenderer,
}
