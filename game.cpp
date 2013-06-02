#include "game.h"

void Game::onConnect(unsigned id) {

  LOG("Cube %d connected\n", id);

  vid[id].initMode(BG0_SPR_BG1);
  vid[id].attach(id);

  vid[id].bg0.image(vec(0,0), Background);
}

void Game::onNeighborAdd(unsigned firstID, unsigned firstSide,
                         unsigned secondID, unsigned secondSide) {

}

void Game::onNeighborRemove(unsigned firstID, unsigned firstSide,
                   unsigned secondID, unsigned secondSide) {

}

void Game::onTap(unsigned id)
{
	CubeID cube(id);

	//LOG("Touch event on cube #%d, state=%d\n", id, cube.isTouching());
	

	if (cube.isTouching()) {
		switch(crew[id]) {
		case SHIP:
			LOG("Ship is being tapped. What does this do again?\n");
			break;
		case CAPTAIN:
			if (obstacleEncountered && currentObstacle == ASTEROID) {
				if (energies[id] >= 200){
					LOG("Firing lasers! Success! Asteroid destoyed!\n");
					energies[id] -= 200;
					FinishObstacle();
				}
				else {
					LOG("Not enough power to fire the lasers!");
				}
			}
			if (obstacleEncountered && currentObstacle == ALIEN) {
				if (energies[id] >= 500){
					LOG("Firing lasers! Success! Alien spacecraft destoyed!\n");
					energies[id] -= 500;
					FinishObstacle();
				}
				else {
					LOG("Not enough power to fire the lasers!");
				}
			}
			else {
				LOG("There's nothing to fire at!\n");
			}
			LOG("Current state: %d\n", energies[id]);
			break;
		case ENGINEER:
			LOG("Engineer generating power!\n");
			energies[id] += 5;
			LOG("Current state: %d\n", energies[id]);
			break;
		default:
			break;
		}
	}

	if (crew[id] == SCIENTIST) {
		if (cube.isTouching()) {
			shieldDrain = true;
		}
		else {
			shieldDrain = false;
		}
		LOG("Current state: %d\n", energies[id]);
	}	
}

void Game::title() {
  Events::cubeConnect.set(&Game::onConnect, this);
  Events::cubeTouch.set(&Game::onTap, this);
  Events::neighborAdd.set(&Game::onNeighborAdd, this);
  Events::neighborRemove.set(&Game::onNeighborRemove, this);


  for (CubeID cube : CubeSet::connected()) {
    onConnect(cube);
  }
  // TODO: show title screen, load assets in background
};

void Game::waitForPlayers() {
  // TODO: init cubes for wait for players
  // Do wait for players logic
}

void Game::init() {
  // TODO: init cubes for gameplay
}

void Game::run() {
	rndm = Random();

	for (int i = 0; i < kNumCubes; i++) {
		energies[i] = 1280;
		switch (i) {
		case 0:
			crew[i] = SHIP;
			obstacles[i] = ALIEN;
			break;
		case 1:
			crew[i] = CAPTAIN;
			obstacles[i] = ASTEROID;
			break;
		case 2:
			crew[i] = ENGINEER;
			obstacles[i] = IONSTORM;
			break;
		case 3:
			crew[i] = SCIENTIST;
			break;
		}
	}

	obstacleEncountered = false;
	disasterAvoided = false;
	shieldCharge = 0;

	TimeStep ts;

	// run gameplay code
	while(1) {
		Update(ts.delta());
		System::paint();
		ts.next();
	}
}

void Game::Update(TimeDelta timeStep){
	if (shieldDrain) {
		energies[3] -= 5;
		shieldCharge += 5;
		LOG("Shield Power: %d\n", energies[3]);
		if (currentObstacle == IONSTORM && shieldCharge >= 100) {
			LOG("ACTIVATING SHIELDS! You have passed through the ion cloud safely!");
			FinishObstacle();
		}

		if (currentObstacle == ALIEN && shieldCharge >= 200) {
			LOG("ACTIVATING SHIELDS! You were safely shielded from the alien's weapons!");
			FinishObstacle();
		}
	}

	if (!obstacleEncountered) {
		obstacleTimer -= timeStep.seconds();
	}

	else {
		reactionTimer -= timeStep.seconds();
	}

	if (obstacleTimer <= 0.0f) {
		obstacleTimer = timeBetweenObstacles;
		currentObstacle = obstacles[rndm.randint(0,2)];
		switch (currentObstacle) {
		case ALIEN:
			LOG("ALIEN ENCOUNTERED!\n");
			break;
		case ASTEROID:
			LOG("ASTEROID ENCOUNTERED!\n");
			break;
		case IONSTORM:
			LOG("ION STORM ENCOUNTERED!\n");
			break;
		default:
			break;
		}
		obstacleEncountered = true;
	}

	else if (reactionTimer <= 0.0f) {
		FinishObstacle();
		LOG("RESOLVED!\n");
	}
}

void Game::FinishObstacle() {
	shieldCharge = 0;
	obstacleEncountered = false;
	obstacleTimer = timeBetweenObstacles;
	reactionTimer = timeToReactToObstacle;
	currentObstacle = NONE;
}

void Game::cleanup() {

}
