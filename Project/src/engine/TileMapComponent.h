#include "SpriteComponent.h"
#include <SDL_rect.h>
#include <string>
class TileMapComponent : public SpriteComponent
{
	public:
		struct Tile
		{
				SDL_Rect srcrect;
				SDL_Rect destrect;
		};
		TileMapComponent(class Actor *owner, int drawOrder = 100)
		    : SpriteComponent(owner, drawOrder), mWidth(0), mHeight(0)
		{
		}
		~TileMapComponent() = default;
		void ReadCSV(std::string path, int height, int width);

		/* virtual void Draw(class SDL_Renderer *renderer) override; */

		private:
		std::vector<Tile> mTiles;
		int mWidth, mHeight;
};
