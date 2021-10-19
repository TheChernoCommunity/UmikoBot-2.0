#include "FunModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <random>

#include <QJsonDocument>
#include <QJsonObject>

using namespace Discord;

FunModule::FunModule()
	: Module("Fun")
{
	QObject::connect(&memeManager, &QNetworkAccessManager::finished, [this](QNetworkReply* reply) { onMemeFinished(reply); });
	QObject::connect(&githubManager, &QNetworkAccessManager::finished, [this](QNetworkReply* reply) { onGithubFinished(reply); });
	
	namespace CP = CommandPermission;
	registerCommand(Commands::Roll, "roll" INTEGER OPTIONAL(INTEGER), CP::User, CALLBACK(roll));
	registerCommand(Commands::Meme, "meme", CP::User, CALLBACK(meme));
	registerCommand(Commands::Github, "github", CP::User, CALLBACK(github));
}

void FunModule::onMemeFinished(QNetworkReply* reply)
{
	if (reply->error())
	{
		UmikoBot::get().createMessage(memeChannel, reply->errorString());
		return;
	}

	QJsonDocument document = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8());
	QJsonObject mainObject = document.object();

	bool isNsfw = mainObject["nsfw"].toBool();
	if (isNsfw)
	{
		// Tries again
		memeManager.get(QNetworkRequest(QUrl("https://meme-api.herokuapp.com/gimme")));
		return;
	}

	EmbedImage image;
	image.setUrl(mainObject["url"].toString());

	EmbedFooter footer;
	footer.setText(QString("Post was made by u/%1 on r/%2.\nSee the actual post at %3").arg(mainObject["author"].toString(),
																							mainObject["subreddit"].toString(),
																							mainObject["postLink"].toString()));
	
	Embed embed;
	embed.setTitle(mainObject["title"].toString());
	embed.setImage(image);
	embed.setFooter(footer);
	UmikoBot::get().createMessage(memeChannel, embed);
}

void FunModule::onGithubFinished(QNetworkReply* reply)
{
	if (reply->error())
	{
		UmikoBot::get().createMessage(githubChannel, reply->errorString());
		return;
	}

	QJsonDocument document = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8());
	QJsonObject mainObject = document.object();

	QJsonArray items = mainObject["items"].toArray();
	if (items.size() == 0)
	{
		UmikoBot::get().createMessage(githubChannel, "No github repositories found!");
		return;
	}
	
	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };
	std::uniform_int_distribution<> distribution(0, items.size() - 1);

	QJsonObject repoObject = items[distribution(prng)].toObject();

	Embed embed;
	embed.setTitle(repoObject["full_name"].toString());
	embed.setDescription(QString("Stars: **%1**\nLanguage: **%2**\n%3").arg(QString::number(repoObject["stargazers_count"].toInt()),
																			repoObject["language"].toString(), repoObject["html_url"].toString()));
	UmikoBot::get().createMessage(githubChannel, embed);
}

void FunModule::roll(const Message& message, const Channel& channel)
{
	(void) channel;
	QStringList args = message.content().split(QRegularExpression(SPACE));
	int min = 0;
	int max = 0;
	bool okMin = true;
	bool okMax = true;

	if (args.size() == 2)
	{
		max = args[1].toInt(&okMax);
	}
	else if (args.size() == 3)
	{
		min = args[1].toInt(&okMin);
		max = args[2].toInt(&okMax);
	}

	if (!okMin || !okMax)
	{
		SEND_MESSAGE("One of your bounds is invalid!");
		return;
	}

	if (min > max)
	{
		std::swap(min, max);
	}

	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };
	std::uniform_int_distribution<> distribution(min, max);
	SEND_MESSAGE(QString("My value was **%1**!").arg(distribution(prng)));
}

void FunModule::meme(const Message& message, const Channel& channel)
{
	(void) message;
	memeChannel = channel.id();
	memeManager.get(QNetworkRequest(QUrl("https://meme-api.herokuapp.com/gimme")));
}

void FunModule::github(const Message& message, const Channel& channel)
{
	(void) message;
	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };
	std::uniform_int_distribution<> distribution('A', 'Z');
	QChar character = QChar((char) distribution(prng));

	githubChannel = channel.id();
	githubManager.get(QNetworkRequest(QUrl("https://api.github.com/search/repositories?q=" + QString(character))));
}
