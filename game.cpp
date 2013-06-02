#include "game.h"

static AssetSlot MainSlot = AssetSlot::allocate()
    .bootstrap(GameAssets);

void Game::onConnect(unsigned cid) {
  // this cube is either new or reconnected
  if (lostCubes.test(cid)) {
    // this is a reconnected cube since it was already lost this paint()
    lostCubes.clear(cid);
    reconnectedCubes.mark(cid);
  } else {
    // this is a brand-spanking new cube
    newCubes.mark(cid);
  }
  // begin showing some loading art (have to use BG0ROM since we don't have assets)
  dirtyCubes.mark(cid);
  auto& g = vid[cid];
  g.attach(cid);
  g.initMode(BG0_ROM);
  g.bg0rom.fill(vec(0,0), vec(16,16), BG0ROMDrawable::SOLID_BG);
  g.bg0rom.text(vec(1,1), "Hold on!", BG0ROMDrawable::BLUE);
  g.bg0rom.text(vec(1,14), "Adding Cube...", BG0ROMDrawable::BLUE);
}

void Game::onRefresh(unsigned cid) {
  LOG("refresh: %d\n", cid);
  dirtyCubes.mark(cid);
}

void Game::activateCube(CubeID cid) {
  // mark cube as active and render its canvas
  activeCubes.mark(cid);
  vid[cid].initMode(BG0_SPR_BG1);

  if (cid == 0) {
    vid[cid].bg0.image(vec(0,0), WaitingScreen);
  } else {
    vid[cid].bg0.image(vec(0,0), RoomBackground);

    auto &bg1 = vid[cid].bg1;

    bg1.setMask(BG1Mask::filled(vec(4, 4), vec(8, 8)));
    bg1.image(vec(4,4), CheckMark);
    bg1.setPanning(vec(96,0));
  }

  // TODO: we may or may not have to do this
  // auto neighbors = vid[cid].physicalNeighbors();
  // if (neighbors.hasNeighborAt(Side(0))) {
  //   showConnected(cid, Side(side));
  // }
}

void Game::checkConnection(unsigned firstID, unsigned firstSide,
                           unsigned secondID, unsigned secondSide) {
  if (firstID == 0 && firstSide != 2) {
    showConnected(secondID);
  } else if (secondID == 0 && secondSide != 2) {
    showConnected(firstID);
  }
}

void Game::showConnected(CubeID cid) {
  readyCubes++;
  vid[cid].bg1.setPanning(vec(4,4));
}

void Game::hideConnected(CubeID cid) {
  readyCubes--;
  vid[cid].bg1.setPanning(vec(96,0));
}

void Game::onNeighborAdd(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide) {
  if (isActive(firstID) && isActive(secondID)) {
    checkConnection(firstID, firstSide, secondID, secondSide);
  }
}

void Game::onNeighborRemove(unsigned firstID, unsigned firstSide,
                   unsigned secondID, unsigned secondSide) {
  if (firstID == 0 || secondID == 0) {
    unsigned id = firstID == 0 ? secondID : firstID;
    hideConnected(id);
  }
}

bool Game::isActive(NeighborID nid) {
  return nid.isCube() && activeCubes.test(nid);
}

void Game::waitForPlayers() {
  config.append(MainSlot, GameAssets);
    loader.init();

  Events::cubeConnect.set(&Game::onConnect, this);
  Events::cubeRefresh.set(&Game::onRefresh, this);

  Events::neighborAdd.set(&Game::onNeighborAdd, this);
  Events::neighborRemove.set(&Game::onNeighborRemove, this);

  for(CubeID cid : CubeSet::connected()) {
    vid[cid].attach(cid);
    activateCube(cid);
  }

  while (readyCubes < 3) {
    draw();
  }

  for (unsigned i = 0; i < kNumCubes; i++) {
    vid[i].bg1.eraseMask();
  }

  // TODO: unbind waiting stuff

  Events::cubeConnect.unset();
  Events::cubeRefresh.unset();

  Events::neighborAdd.unset();
  Events::neighborRemove.unset();
}

void Game::draw() {
  // clear the palette
  newCubes.clear();
  lostCubes.clear();
  reconnectedCubes.clear();
  dirtyCubes.clear();

  // fire events
  System::paint();

  // dynamically load assets just-in-time
  if (!(newCubes | reconnectedCubes).empty()) {
      // AudioTracker::pause();
      // playSfx(SfxConnect);
      loader.start(config);
      while(!loader.isComplete()) {
          for(CubeID cid : (newCubes | reconnectedCubes)) {
              vid[cid].bg0rom.hBargraph(
                  vec(0, 4), loader.cubeProgress(cid, 128), BG0ROMDrawable::ORANGE, 8
              );
          }
          // fire events while we wait
          System::paint();
      }
      loader.finish();
      // AudioTracker::resume();
  }

  // repaint cubes
  for(CubeID cid : dirtyCubes) {
      activateCube(cid);
  }

  // also, handle lost cubes, if you so desire :)
}

void Game::init() {
  // set background
  // set ship
  // setup obstacles?
  // setup projectiles?
  vid[0].bg0.image(vec(0, 0), Stars);
  vid[0].bg1.setMask(BG1Mask::filled(vec(0, 4), vec(8, 8)));
  vid[0].bg1.image(vec(0,4), Ship, 0);
}

void Game::run() {
  while(1) {
    System::paint();
  }
}

void Game::cleanup() {

}
