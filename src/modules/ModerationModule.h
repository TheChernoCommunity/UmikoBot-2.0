#pragma once

#include "core/Core.h"
#include "modules/Module.h"

class ModerationModule : public Module
{
public:
	ModerationModule();

	void moderateInvitations(const Discord::Message&, const Discord::Channel&);

protected:
	void onMessage(const Discord::Message&, const Discord::Channel&) override;
	
private:
	bool isModeratingInvitations = false;
};
