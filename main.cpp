#include <sifteo.h>
#include "assets.gen.h"
using namespace Sifteo;

static const unsigned kNumCubes = 4;

// static AssetSlot MainSlot = AssetSlot::allocate()
//     .bootstrap(GameAssets);

static Metadata M = Metadata()
    .title("Space")
    .package("com.spaceteam.space", "1.0")
    .icon(Icon)
    .cubeRange(0, kNumCubes);

static AssetSlot MainSlot = AssetSlot::allocate()
    .bootstrap(GameAssets);

static int count = 0;

static VideoBuffer vid[kNumCubes];

// static char **roles = { "ship", "captain", "engineer", "scientist" };


class Space {
 public:

  void install() {

    // Events::neighborAdd.set(&Space::onNeighborAdd, this);
    // Events::neighborRemove.set(&Space::onNeighborRemove, this);
    Events::cubeConnect.set(&Space::onConnect, this);

    for (CubeID cube : CubeSet::connected()) {
      onConnect(cube);
    }
  }

 private:
  void onConnect(unsigned id) {
    CubeID cube(id);
    uint64_t hwid = cube.hwID();

    LOG("Cube %d connected\n", id);

    vid[id].initMode(BG0_SPR_BG1);
    vid[id].attach(id);

    vid[id].bg0.image(vec(0,0), Background);

    // String<24> str;
    // str << "I am cube #" << cube << "\n";

    // vid[cube].bg0rom.text(vec(1, 2), str);

    // str.clear();

    // if (CubeSet::connected().count() < 4) {
    //   str << "Waiting...\n";
    //   vid[cube].bg0rom.text(vec(1, 3), str);
    // } else {

    //   int index = 0;
    //   for (CubeID cubeID : CubeSet::connected()) {
    //     str.clear();
    //     str << "YAY!!!!\n";
    //     vid[cubeID].bg0rom.text(vec(1, 3), str);
    //   }
    // }
  }

  void onNeighborAdd(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide) {
    LOG("Neighbor Add: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

    if (count < 3) {
      String<24> str;

      str << "Waiting...\n";

      if (firstID == 0 || secondID == 0) {
        unsigned id = firstID == 0 ? secondID : firstID;

        vid[id].bg0rom.text(vec(1, 3), str);
        count++;
      }
    }
  }

  void onNeighborRemove(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide) {
    LOG("Neighbor Remove: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);
  }
};

void main() {

  static Space space;

  space.install();

  while (1) {
    System::paint();
  }
}
