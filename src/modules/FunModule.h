#pragma once

#include "core/Core.h"
#include "modules/Module.h"

#include <QtNetwork>

class FunModule : public Module
{
public:
	FunModule();

	void roll(const Discord::Message&, const Discord::Channel&);
	void meme(const Discord::Message&, const Discord::Channel&);
	void github(const Discord::Message&, const Discord::Channel&);

private:
	void onMemeFinished(QNetworkReply* reply);
	void onGithubFinished(QNetworkReply* reply);
	
private:
	QNetworkAccessManager memeManager;
	QNetworkAccessManager githubManager;
	ChannelId memeChannel;
	ChannelId githubChannel;
};
