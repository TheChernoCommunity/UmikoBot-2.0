#pragma once

#include "core/Module.h"

class GlobalModule : public Module
{
public:
	GlobalModule();
	~GlobalModule();

	static void help(const Discord::Message&, const Discord::Channel&);
	static void echo(const Discord::Message&, const Discord::Channel&);
	static void setPrefix(const Discord::Message&, const Discord::Channel&);
};
