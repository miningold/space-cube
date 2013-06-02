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

  int energies[kNumCubes];
  CLASS_TYPE crew[kNumCubes];
  OBSTACLE_TYPE obstacles[3];
  
  bool obstacleEncountered;
  bool disasterAvoided;

  float shieldCharge;

  OBSTACLE_TYPE currentObstacle;

  float obstacleTimer = 10.0f;
  float reactionTimer = 5.0f;

  void title();
  void waitForPlayers();
  void init();
  void run();
  void cleanup();

 private:
  Random rndm;
  void onConnect(unsigned id);
  void onNeighborAdd(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);
  void onNeighborRemove(unsigned firstID, unsigned firstSide,
                     unsigned secondID, unsigned secondSide);
  void onTap(unsigned id);

  void Update(TimeDelta timeStep);
  void FinishObstacle(void);

  bool shieldDrain;

  VideoBuffer vid[kNumCubes];
};

#endif
