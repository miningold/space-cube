#include "game.h"

#include <sifteo.h>
#include "assets.gen.h"

// static AssetSlot MainSlot = AssetSlot::allocate()
//     .bootstrap(GameAssets);

static Metadata M = Metadata()
    .title("Super Space Cubed")
    .package("com.spaceteam6.ss3", "1.0")
    .icon(Icon)
    .cubeRange(0, Game::kNumCubes);

void main() {
  static Game game;

  while (1) {
    game.waitForPlayers();
    game.init();
    game.run();
    game.cleanup();
  }
}
