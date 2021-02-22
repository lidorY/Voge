#pragma once
#include "camera.h"

class Game {
public:
	Game() {}

	Camera& GameCamera() { return camera_; };
private:

	Camera camera_;
};