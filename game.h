#ifndef GAME_H
#define GAME_H

#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

class Game {
 public:
  static const unsigned kNumCubes = 4;

  void title();
  void init();
  void run();
  void cleanup();

 private:
  void onConnect(unsigned id);
  void onNeighborAdd(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);
  void onNeighborRemove(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);

  VideoBuffer vid[kNumCubes];
};

#endif
