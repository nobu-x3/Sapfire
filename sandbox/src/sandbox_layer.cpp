#include "sandbox_layer.h"

SandboxLayer::SandboxLayer() : Sapfire::Layer("Sandbox Layer") { CLIENT_TRACE("HELLO"); }

void SandboxLayer::on_attach() {}

void SandboxLayer::on_detach() {}

void SandboxLayer::on_update(Sapfire::f32 delta_time) {}

void SandboxLayer::on_event(Sapfire::Event& e) {}
