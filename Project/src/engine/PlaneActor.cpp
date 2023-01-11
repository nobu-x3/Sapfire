#include "PlaneActor.h"
#include "Game.h"
#include "MeshComponent.h"
#include "engine/renderer/Renderer.h"

PlaneActor::PlaneActor(Game *game) : Actor(game)
{
	SetScale(10.0f);
	MeshComponent *mc = new MeshComponent(this);
	mc->SetMesh(GetGame()->GetRenderer()->GetMesh("../Assets/Plane.sfmesh"));
}
