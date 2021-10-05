#pragma once

#include "modules/Module.h"

// For use when registering commands
#define OPTIONAL(x) "(" x ")?"
#define IDENTIFIER "\\s\\S+"
#define TEXT "\\s.+"

class GlobalModule : public Module
{
public:
	GlobalModule();
	~GlobalModule();

	static void help(Module*, const Discord::Message&, const Discord::Channel&);
	static void echo(Module*, const Discord::Message&, const Discord::Channel&);
	static void setPrefix(Module*, const Discord::Message&, const Discord::Channel&);
	static void enable(Module*, const Discord::Message&, const Discord::Channel&);
	static void disable(Module*, const Discord::Message&, const Discord::Channel&);

private:
	static void enableDisableImpl(Module*, const Discord::Message&, const Discord::Channel&, bool enable);
};
