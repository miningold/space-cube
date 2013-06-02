#include "game.h"

static AssetSlot MainSlot = AssetSlot::allocate()
	.bootstrap(GameAssets);

void Game::onConnect(unsigned cid) {
	if (ready) {
		return;
	}
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
	if (ready) {
		return;
	}
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
	LOG("Connected and ready\n");
	readyCubes++;
	vid[cid].bg1.setPanning(vec(4,4));
}

void Game::hideConnected(CubeID cid) {
	readyCubes--;
	vid[cid].bg1.setPanning(vec(96,0));
}

void Game::onNeighborAdd(unsigned firstID, unsigned firstSide,
	unsigned secondID, unsigned secondSide) {
		if (ready) {
			return;
		}
		if (isActive(firstID) && isActive(secondID)) {
			checkConnection(firstID, firstSide, secondID, secondSide);
		}
}

void Game::onNeighborRemove(unsigned firstID, unsigned firstSide,
	unsigned secondID, unsigned secondSide) {
		if (ready) {
			return;
		}
		if (firstID == 0 || secondID == 0) {
			unsigned id = firstID == 0 ? secondID : firstID;
			hideConnected(id);
		}
}

bool Game::isActive(NeighborID nid) {
	return nid.isCube() && activeCubes.test(nid);
}

void Game::onLink(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide) {
	if (firstID != 0 && secondID != 0) {
		if (firstSide == 0) {
			if (secondSide == 0) {
				LOG("INCOMPATIBLE CONNECTION\n");
				return;
			}
			else {
				LOG("#%d is connected at side #%d to #%d at side #%d\n", firstID, firstSide, secondID, secondSide);
				connectedIDs[firstID] = secondID;
				if (firstID == 1 && secondID == 3) {
					assistingShields = true;
				}
				return;
			}
		}

		else if (secondSide == 0) {
			if (firstSide == 0) {
				LOG("INCOMPATIBLE CONNECTION\n");
				return;
			}
			else {
				LOG("#%d is connected at side #%d to #%d at side #%d\n", firstID, firstSide, secondID, secondSide);
				connectedIDs[secondID] = firstID;
				if (secondID == 1 && firstID == 3) {
					assistingShields = true;
				}
				return;
			}
		}
	}
}

void Game::onUnlink(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide) {
	if (firstID != 0 && secondID != 0) {
		if (firstSide == 0) {
			if (secondSide == 0) {
				LOG("INCOMPATIBLE CONNECTION\n");
				return;
			}
			else {
				LOG("#%d disconnected at side #%d from #%d at side #%d\n", firstID, firstSide, secondID, secondSide);
				connectedIDs[firstID] = 0;
				if (firstID == 1 && secondID == 3) {
					assistingShields = false;
				}
				return;
			}
		}

		else if (secondSide == 0) {
			if (firstSide == 0) {
				LOG("INCOMPATIBLE CONNECTION\n");
				return;
			}
			else {
				LOG("#%d disconnected at side #%d from #%d at side #%d\n", firstID, firstSide, secondID, secondSide);
				connectedIDs[secondID] = 0;
				if (secondID == 1 && firstID == 3) {
					assistingShields = false;
				}
				return;
			}
		}
	}
}

void Game::onTap(unsigned id)
{
	bool acting = false;
	CubeID cube(id);

	if (id == 0) return;

	if (cube.isTouching()) {
		switch(crew[id]) {
		case SHIP:
			return;
			break;
		case CAPTAIN:
			if (functioning[id]) {
				if (connectedIDs[id] == 0) {
					if (obstacleEncountered && currentObstacle == ASTEROID) {
						if (energies[id] >= 200) {
							acting = true;
							firing = true;
							LOG("Firing lasers! Success! Asteroid destoyed!\n");
							energies[id] -= 200;
							FinishObstacle();
						}
						else {
							LOG("Not enough power to fire the lasers!");
						}
					}
					else if (obstacleEncountered && currentObstacle == ALIEN) {
						if (energies[id] >= 500) {
							acting = true;
							firing = true;
							LOG("Firing lasers! Success! Alien spacecraft destoyed!\n");
							energies[id] -= 500;
							FinishObstacle();
						}
					}
					else {
						LOG("There's nothing to fire at!\n");
					}
				}
				else {
					if (connectedIDs[id] == 2) {
						LOG("ASSISTING ENGINEER: Powering up!\n");
						energies[connectedIDs[id]] += 5;
					}
				}
			}
			LOG("Current state: %d\n", energies[id]);
			break;
		case ENGINEER:
			if (functioning[id]) {
				if (connectedIDs[id] == 0) {
					LOG("Engineer generating power!\n");
					acting = true;
					energies[id] += 5;
				}
				LOG("Current state: %d\n", energies[id]);
				break;
			}
		default:
			break;
		}
	}

	if (crew[id] == SCIENTIST) {
		if (functioning[id]) {
			if (connectedIDs[id] == 0) {
				if (cube.isTouching()) {
					acting = true;
					shieldDrain = true;
				} else {
					shieldDrain = false;
				}
			} else {
				if (!functioning[connectedIDs[id]]) {
					if (repairs[connectedIDs[id]] > 0) {
						repairs[connectedIDs[id]]--;
					}
				}
				else {
					LOG("Nothing to repair!\n");
				}
			}
		}
		LOG("Current state: %d\n", energies[id]);
	}

	if (firing) {
		vid[0].sprites[1].setImage(Bullet, 0);
	}

  if (shieldDrain) {
    showShield();
  } else {
    hideShield();
  }

  if (id != 0) {
	  if (cube.isTouching() && acting) {
		  LOG("character: %d is acting", id);
		  characterActing[crew[id]] = true;
		  showCharacter(id, vec(76, 64), 3);
	  } else {
		  characterActing[crew[id]] = false;
		  showCharacter(id, vec(76, 64), 2);
	  }
  }
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

	ready = true;

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


	Events::cubeTouch.set(&Game::onTap, this);
	Events::neighborAdd.set(&Game::onLink, this);
	Events::neighborRemove.set(&Game::onUnlink, this);
}

void Game::run() {
	rndm = Random();

	vid[0].bg0.image(vec(0, 0), Stars);

	for (int i = 0; i < kNumCubes; i++) {
		energies[i] = 1280;
		switch (i) {
		case 0:
			crew[i] = SHIP;
			// characterImages[i] = Ship;
			obstacles[i] = ALIEN;
			connectedIDs[i] = 0;
			functioning[i] = true;
			repairs[i] = tapsNeededToRepair;
			break;
		case 1:
			crew[i] = CAPTAIN;
			characterImages[i] = Captain;
			obstacles[i] = ASTEROID;
			connectedIDs[i] = 0;
			functioning[i] = true;
			repairs[i] = tapsNeededToRepair;
			break;
		case 2:
			crew[i] = ENGINEER;
			characterImages[i] = Engineer;
			obstacles[i] = IONSTORM;
			connectedIDs[i] = 0;
			functioning[i] = true;
			repairs[i] = tapsNeededToRepair;
			break;
		case 3:
			crew[i] = SCIENTIST;
			connectedIDs[i] = 0;
			characterImages[i] = Scientist;
			functioning[i] = true;
			repairs[i] = tapsNeededToRepair;
			break;
		}

		if (i == 0) {
			vid[i].bg1.setMask(BG1Mask::filled(vec(0, 4), vec(8, 8)));
			vid[i].bg1.image(vec(0,4), Ship, 0);
		} else {
      // set energy mask
      vid[i].bg1.setMask(BG1Mask::filled(vec(0, 14), vec(4, 2)));
      vid[i].bg1.setPanning(vec(-11, 5));

      showCharacter(i, vec(56, 64), 0);
		}
	}

	obstacleEncountered = false;
	disasterAvoided = false;
	shieldCharge = 0;
	assistingShields = false;
	currentObstacle = NONE;

	bullet.set(52, 60);
	bulletTarget.set(76, 60);

	vid[0].sprites[5].move(64, 32);

	TimeStep ts;

	// run gameplay code
	while(1) {
		Update(ts.delta());
		System::paint();
		ts.next();
	}
}

void Game::Update(TimeDelta timeStep) {

	characterTimer += timeStep.seconds();

	// bullet animation
	if (firing) {
		Float2 difference = bulletTarget - bullet;
		bullet += difference * bulletSpeed;


		if (difference.len() < 0.7f) {
			firing = false;
			bullet.set(52, 60);
			bulletTarget.set(76, 60);
			vid[0].sprites[1].hide();
		} else if (difference.len() < 1.5f) {
			vid[0].sprites[1].setImage(Bullet, 1);

      showCheckmark();
		}

		vid[0].sprites[1].move(bullet);
	}

	// character animation
	if (characterTimer >= characterDuration) {
		characterTimer = 0;

		characterFrame = characterFrame == 0 ? 1 : 0;

		for (unsigned i = 1; i < kNumCubes; i++) {
			// if (!characterActing[i]) {
        showCharacter(i, vec(56, 64), characterFrame);
			// }
		}
	}

  // Checkmark
  if (showCheck) {
    checkTimer += timeStep.seconds();

    if (checkTimer > checkDuration) {
      checkTimer = 0;
      vid[0].sprites[2].hide();
    }
  }

	for (unsigned i = 1; i < kNumCubes; i++) {
		if (repairs[i] <= 0) {
			functioning[i] = true;
      vid[i].bg0.image(vec(0,0), RoomBackground, 0);
			repairs[i] = tapsNeededToRepair;
		}
	}

	if (shieldDrain) {
		if (!assistingShields) {
			if (energies[3] >= 5) {
				energies[3] -= 5;
				shieldCharge += 5;
			}
		}
		else {
			if (energies[3] >= 2 && energies[1] >= 3) {
				energies[3] -= 2;
				energies[1] -= 3;
				shieldCharge += 5;
			}
			else {
				if (energies[3] < 2) {
					if (energies[1] >= 5) {
						energies[1] -= 5;
						shieldCharge += 5;
					}
				}
				else if (energies[1] < 3) {
					if (energies[3] >= 5) {
						energies[3] -= 5;
						shieldCharge += 5;
					}
				}
			}
		}
		LOG("Shield Charge: %d\n", shieldCharge);
		if (currentObstacle == IONSTORM && shieldCharge >= 100) {
			LOG("ACTIVATING SHIELDS! You have passed through the ion cloud safely!");
			showCheckmark();
			FinishObstacle();
		}

		if (currentObstacle == ALIEN && shieldCharge >= 200) {
			LOG("ACTIVATING SHIELDS! You were safely shielded from the alien's weapons!");
			showCheckmark();
			FinishObstacle();
		}

		if (currentObstacle == ASTEROID && shieldCharge >= 150) {
			LOG("ACTIVATING SHIELDS! You safely deflected the Asteroid!");
			showCheckmark();
			FinishObstacle();
		}
	}

	if (connectedIDs[2] != 0 && energies[2] >= 5) {
		energies[2] -= 5;
		energies[connectedIDs[2]] += 5;
		LOG("ENERGY TRANSFER: ENGINEER: %d, #%d: %d\n", energies[2], connectedIDs[2], energies[connectedIDs[2]]);
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

		int obstacleIndex = -1;
		switch (currentObstacle) {
		case ALIEN:
			LOG("ALIEN ENCOUNTERED!\n");
			obstacleIndex = 1;
			break;
		case ASTEROID:
			LOG("ASTEROID ENCOUNTERED!\n");
			obstacleIndex = 2;
			break;
		case IONSTORM:
			LOG("ION STORM ENCOUNTERED!\n");
			obstacleIndex = 0;
			break;
		default:
			break;
		}

		vid[0].sprites[5].setImage(Obstacles, obstacleIndex);
		obstacleEncountered = true;
	}

	else if (reactionTimer <= 0.0f) {
    vid[0].sprites[5].hide();
		DisableCrewMember();
		FinishObstacle();
		LOG("RESOLVED!\n");
	}

  updateEnergy();
  updateDamaged();
}

void Game::FinishObstacle() {
	shieldCharge = 0;
	obstacleEncountered = false;
	obstacleTimer = timeBetweenObstacles;
	reactionTimer = timeToReactToObstacle;
	currentObstacle = NONE;
}

void Game::DisableCrewMember() {
	if (functioning[1] || functioning[2] || functioning[3]) {
		int selection = rndm.randint(1,3);
		if (!functioning[1] && !functioning[2]) {
			functioning[3] = false;
			return;
		}
		else {
			while (selection == 3 || !functioning[selection]) {
				selection = rndm.randint(1,3);
			}
		}

		functioning[selection] = false;
	}
	return;
}

void Game::showCheckmark() {
  showCheck = true;
  checkTimer = 0;
  vid[0].sprites[2].setImage(CheckMark);
  vid[0].sprites[2].move(64, 32);
  vid[0].sprites[5].hide();
}

void Game::showShield() {
  vid[0].bg1.image(vec(0,4), Ship, 1);
}

void Game::hideShield() {
  vid[0].bg1.image(vec(0,4), Ship, 0);
}

void Game::updateEnergy() {
  String<8> energy;
  for (unsigned i = 1; i < kNumCubes; i++) {
    energy.clear();

    if (energies[i] < 1000) {
      energy << " ";
    }

    if (energies[i] < 100) {
      energy << " ";
    }

    if (energies[i] < 10) {
      energy << " ";
    }

    energy << energies[i];

    vid[i].bg1.text(vec(0, 14), Font, energy);
  }
}

void Game::updateDamaged() {
  damageFrame += 0.2f;

  if (damageFrame > 4) {
    damageFrame -= 4;
  }

  int temp = ((int)damageFrame) % RoomBackground.numFrames();

  if (temp != curDamageFrame) {
    curDamageFrame = temp;

    for (unsigned i = 0; i < kNumCubes; i++) {
      if (!functioning[i]) {
        vid[i].bg0.image(vec(0,0), RoomBackground, curDamageFrame);
      }
    }
  }
}

void Game::showCharacter(unsigned id, Int2 pos, int frame) {
  vid[id].sprites[2].setImage(characterImages[id], frame);
  vid[id].sprites[2].move(pos);
  // vid[id].bg1.setPanning(pos);
}

void Game::cleanup() {

}
