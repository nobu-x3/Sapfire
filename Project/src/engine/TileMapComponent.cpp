#include "TileMapComponent.h"
// #include "csv.hpp"
// #include "internal/csv_row.hpp"
#include <SDL_log.h>
#include <SDL_render.h>

void TileMapComponent::ReadCSV(std::string path, int height, int width)
{
	// mHeight = mTexHeight / height;
	// mWidth = mTexWidth / width;
	// csv::CSVReader reader(path); // lmao this library does not reset iterators or smth
	// int destOffsetY = 0;
	// for (csv::CSVRow &row : reader)
	// {
	// 	int destOffsetX = 0;
	// 	for (csv::CSVField &field : row)
	// 	{
	// 		int index = std::stoi(field.get());

	// 		int temp = 0;
	// 		int offsetY = index / mWidth;
	// 		int offsetX = index - (offsetY * mWidth);

	// 		SDL_Rect srcRect;
	// 		srcRect.x = offsetX * width;
	// 		srcRect.y = offsetY * height;
	// 		srcRect.w = width;
	// 		srcRect.h = height;

	// 		SDL_Rect destRect;
	// 		destRect.x = destOffsetX * width * mOwner->GetScale();
	// 		destRect.y = destOffsetY * height * mOwner->GetScale();
	// 		destRect.h = mOwner->GetScale() * height;
	// 		destRect.w = mOwner->GetScale() * width;

	// 		Tile tile;
	// 		tile.destrect = destRect;
	// 		tile.srcrect = srcRect;
	// 		mTiles.emplace_back(tile);

	// 		destOffsetX++;
	// 		SDL_Log("srcX - %d, srcY - %d, destX - %d, destY - %d", srcRect.x, srcRect.y, destRect.x,
	// 			destRect.y);
	// 	}
	// 	destOffsetY++;
	// }
}

/* void TileMapComponent::Draw(SDL_Renderer *renderer) */
/* { */
/* 	SDL_Rect rect; */
/* 	rect.w = static_cast<int>(mTexWidth); */
/* 	rect.h = static_cast<int>(mTexHeight); */
/* 	rect.x = static_cast<int>(mOwner->GetPosition().x - rect.w / 2.f); */
/* 	rect.y = static_cast<int>(mOwner->GetPosition().y - rect.h / 2.f); */

/* 	for (auto &tile : mTiles) */
/* 	{ */
/* 		SDL_RenderCopyEx(renderer, mTexture, &tile.srcrect, &tile.destrect, */
/* 				 -Math::ToDegrees(mOwner->GetRotation()), nullptr, SDL_FLIP_NONE); */
/* 	} */
/* } */
