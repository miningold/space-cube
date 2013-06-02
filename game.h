#ifndef GAME_H
#define GAME_H

#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

class Game {
 public:
	 enum CLASS_TYPE { SHIP, CAPTAIN, ENGINEER, SCIENTIST };
	 enum OBSTACLE_TYPE { ALIEN, ASTEROID, IONSTORM, NONE };

  static const unsigned kNumCubes = 4;
  static const float timeBetweenObstacles = 5.0f;
  static const float timeToReactToObstacle = 2.5f;
  static const float tapsNeededToRepair = 15;

  int energies[kNumCubes];
  CLASS_TYPE crew[kNumCubes];
  int connectedIDs[kNumCubes];
  bool functioning[kNumCubes];
  int repairs[kNumCubes];

  OBSTACLE_TYPE obstacles[3];

  bool obstacleEncountered;
  bool disasterAvoided;

  int shieldCharge;
  bool transferringPower;
  bool assistingShields;

  OBSTACLE_TYPE currentObstacle;

  float obstacleTimer = 10.0f;
  float reactionTimer = 5.0f;

  void waitForPlayers();
  void init();
  void run();
  void cleanup();

  void draw();

 private:
  Random rndm;
  void onConnect(unsigned id);
  void onRefresh(unsigned id);
  void onNeighborAdd(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);
  void onNeighborRemove(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);
  void onTap(unsigned id);

  void onLink(unsigned firstID, unsigned firstSide,
	  unsigned secondID, unsigned secondSide);
  void onUnlink(unsigned firstID, unsigned firstSide,
	  unsigned secondID, unsigned secondSide);

  void Update(TimeDelta timeStep);
  void FinishObstacle(void);
  void DisableCrewMember(void);

  bool shieldDrain;

  void activateCube(CubeID cid);

  void checkConnection(unsigned firstID, unsigned firstSide,
                       unsigned secondID, unsigned secondSide);
  void showConnected(CubeID cid);
  void hideConnected(CubeID cid);

  bool isActive(NeighborID nid);

  // BG1Mask mask;

  Float2 bullet, bulletTarget;
  float bulletSpeed = 0.2f;
  bool firing;

  AssetConfiguration<1> config;
  AssetLoader loader;

  VideoBuffer vid[kNumCubes];

  CubeSet newCubes; // new cubes as a result of paint()
  CubeSet lostCubes; // lost cubes as a result of paint()
  CubeSet reconnectedCubes; // reconnected (lost->new) cubes as a result of paint()
  CubeSet dirtyCubes; // dirty cubes as a result of paint()
  CubeSet activeCubes; // cubes showing the active scene

  float characterDuration = 1;
  float characterTimer = 0;

  AssetImage characterImages[kNumCubes];
  bool characterActing[kNumCubes];
  int characterFrame = 0;

  int readyCubes = 0;

  bool ready = false;
};

#endif
