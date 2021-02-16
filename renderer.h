#pragma once

#include "device_resources.h"

class Renderer :public std::enable_shared_from_this<Renderer> {
public:
	Renderer(std::shared_ptr<DeviceResources> const& dr) {}
};
