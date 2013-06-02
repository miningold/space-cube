#ifndef GAME_H
#define GAME_H

#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

class Game {
 public:
  static const unsigned kNumCubes = 4;

  void waitForPlayers();
  void init();
  void run();
  void cleanup();

  void draw();

 private:
  void onConnect(unsigned id);
  void onRefresh(unsigned id);
  void onNeighborAdd(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);
  void onNeighborRemove(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);

  void activateCube(CubeID cid);

  void checkConnection(unsigned firstID, unsigned firstSide,
                       unsigned secondID, unsigned secondSide);
  void showConnected(CubeID cid);
  void hideConnected(CubeID cid);

  bool isActive(NeighborID nid);

  AssetConfiguration<1> config;
  AssetLoader loader;

  VideoBuffer vid[kNumCubes];

  CubeSet newCubes; // new cubes as a result of paint()
  CubeSet lostCubes; // lost cubes as a result of paint()
  CubeSet reconnectedCubes; // reconnected (lost->new) cubes as a result of paint()
  CubeSet dirtyCubes; // dirty cubes as a result of paint()
  CubeSet activeCubes; // cubes showing the active scene

  int readyCubes = 0;
};

#endif
