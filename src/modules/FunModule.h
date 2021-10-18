#pragma once

#include "core/Core.h"
#include "modules/Module.h"

class FunModule : public Module
{
public:
	FunModule();

	void roll(const Discord::Message&, const Discord::Channel&);
};
