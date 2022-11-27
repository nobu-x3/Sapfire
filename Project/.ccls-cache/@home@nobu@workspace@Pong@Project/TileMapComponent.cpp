#include "TileMapComponent.h"
#include "Math.h"
#include "csv.hpp"
#include "internal/csv_row.hpp"
#include <SDL_log.h>
#include <SDL_render.h>

/*TileMapComponent::~TileMapComponent()
{
    while(!mTiles.empty())
    {
	delete mTiles.back();
    }
}*/

void TileMapComponent::ReadCSV(std::string path)
{
	csv::CSVReader reader1(path);
	int tempHeight = 0;
	for (csv::CSVRow row : reader1)
	{
		int tempWidth = 0;
		for (csv::CSVField &field : row)
		{
			tempWidth++;
		}
		mWidth = tempWidth;
		tempHeight++;
    }
    mHeight = tempHeight;
	float tileWidth = mTexWidth / mWidth;
	float tileHeight = mTexHeight / mHeight;

	tempHeight = 0;
	csv::CSVReader reader(path); // lmao this library does not reset iterators or smth
	for (csv::CSVRow &row : reader)
	{

		for (csv::CSVField &field : row)
		{
			int index = std::stoi(field.get());
			int tempWidth = 0;
			Tile tile;
			SDL_Rect destrect;
			destrect.h = tileHeight;
			destrect.w = tileWidth;
			destrect.x = static_cast<int>((tempWidth * tileWidth + tileWidth / 2.f) * mOwner->GetScale());
			destrect.y =
			    static_cast<int>((tempHeight * tileHeight + tileHeight / 2.f) * mOwner->GetScale());
			if (index >= 0)
			{
				int y = index / mWidth;
				int x = index - y * mWidth;
                SDL_Log("index %s: y - %d, x - %d", field.get().c_str(), y, x);
		SDL_Rect srcrect;
		srcrect.y = static_cast<int>(y * tileHeight + tileHeight / 2.f);
		srcrect.x = static_cast<int>(x * tileWidth + tileWidth / 2.f);
		srcrect.w = static_cast<int>(tileWidth);
		srcrect.h = static_cast<int>(tileHeight);
		tile.srcrect = srcrect;
			}
		tile.destrect = destrect;
		mTiles.emplace_back(tile);
		tempWidth++;
		}
		tempHeight++;
	}
}

void TileMapComponent::Draw(SDL_Renderer *renderer)
{
	SDL_Rect rect;
	rect.w = static_cast<int>(mTexWidth);
	rect.h = static_cast<int>(mTexHeight);
	rect.x = static_cast<int>(mOwner->GetPosition().x - rect.w / 2.f);
	rect.y = static_cast<int>(mOwner->GetPosition().y - rect.h / 2.f);

	for (auto &tile : mTiles)
	{
		SDL_RenderCopyEx(renderer, mTexture, &tile.srcrect, &tile.destrect,
				 -Math::ToDegrees(mOwner->GetRotation()), nullptr, SDL_FLIP_NONE);
	}
}
