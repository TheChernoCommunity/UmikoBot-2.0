# UmikoBot
Umiko is the bot used on [TheCherno's Discord Server](https://discord.gg/thecherno). This repository is what makes Umiko; it's the source code and it is completely open source!

## ❓ What can it do?
Umiko can do everything that a Discord bot can technically do. Of course it doesn't do all of it, but if you feel like there's something missing that would make Umiko better, contributions are always welcomed!

## 🙋‍♂️ Contributing
So you want to contribute? Awesome! Let's get you going. For starters, know that Umiko uses **[QDiscord](https://github.com/FancyKillerPanda/QDiscord)** to interface with the Discord API. This is already in the repo as a submodule, so you don't need to worry much.

There are some prerequisites you will need before you begin, so lets start with those:

### Prerequisites
#### Windows
- [Qt](https://www.qt.io/) (version **5.15.2** recommended)
- Visual Studio (recommended)

![Qt Required Components](images/Qt-Required-Components.png)

#### Linux (or WSL on Windows)
To build on linux, you first need to make sure you have your build tools ready to go (including a compiler (such as `g++`) and `cmake`). Then all you need to do to install Qt is

```bash
sudo apt install qt5-default libqt5websockets5-dev
```

### Building and running
After forking the main repository and cloning your fork to create a local copy, what next?

#### Windows
```bat
scripts\build.bat
```

This will create all the necessary Visual Studio files for you (they're located in the `sln` directory). Simply open up the solution with Visual Studio and provide the bot token (see below), then you're good to go!

Go to `Project Properties -> Debugging`:
![Visual Studio Command Args](images/Bot-Token-VS.png)

#### Linux
```bash
scripts/build.sh <token>
```

This will do all the necessary building steps (and even run the program for you). All you need to do is provide the bot authorisation token as a command line argument!

## ❓ Help and FAQ
If you weren't able to properly carry out the setup process, or if you just want to know more Umiko in general, considering checking these questions before opening an issue:

**Is Umiko going to be open for invites anytime soon?**

This hasn't been thought about much, but all of Umiko is developed keeping multiple servers in mind, so we're ready for that already!

**Is there some kind of a roadmap?**

The closest thing to that we currently have is [this](https://github.com/TheChernoCommunity/UmikoBot/projects/1).

**Is there some kind of a community where I can interact with fellow contributors?**

Glad you asked! We'll be happy to see you on our [Discord Server!](https://discord.gg/zrUpn7RG5k)
