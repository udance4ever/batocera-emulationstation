#include "GuiRetroAchievementsSettings.h"
#include "ThreadedHasher.h"
#include "GuiHashStart.h"
#include "SystemConf.h"
#include "ApiSystem.h"
#include "RetroAchievements.h"

#include "guis/GuiMsgBox.h"
#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"

GuiRetroAchievementsSettings::GuiRetroAchievementsSettings(Window* window) : GuiSettings(window, std::string("RETROACHIEVEMENT SETTINGS").c_str())
{
	addGroup(std::string("SETTINGS"));

	bool retroachievementsEnabled = SystemConf::getInstance()->getBool("global.retroachievements");
	std::string username = SystemConf::getInstance()->get("global.retroachievements.username");
	std::string password = SystemConf::getInstance()->get("global.retroachievements.password");

	// retroachievements_enable
	auto retroachievements_enabled = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_enabled->setState(retroachievementsEnabled);
	addWithLabel(std::string("RETROACHIEVEMENTS"), retroachievements_enabled);
	
	// retroachievements, username, password
	addInputTextRow(std::string("USERNAME"), "global.retroachievements.username", false);
	addInputTextRow(std::string("PASSWORD"), "global.retroachievements.password", true);

	addGroup(std::string("OPTIONS"));

	addSwitch(std::string("HARDCORE MODE"), std::string("Disable loading states, rewind and cheats for more points."), "global.retroachievements.hardcore", false, nullptr);
	addSwitch(std::string("LEADERBOARDS"), std::string("Compete in high-score and best time leaderboards (requires hardcore)."), "global.retroachievements.leaderboards", false, nullptr);
	addSwitch(std::string("VERBOSE MODE"), std::string("Show achievement progression on game launch and other notifications."), "global.retroachievements.verbose", false, nullptr);
	addSwitch(std::string("RICH PRESENCE"), "global.retroachievements.richpresence", false);
	addSwitch(std::string("ENCORE MODE"), std::string("Unlocked achievements can be earned again."), "global.retroachievements.encore", false, nullptr);
	addSwitch(std::string("AUTOMATIC SCREENSHOT"), std::string("Automatically take a screenshot when an achievement is earned."), "global.retroachievements.screenshot", false, nullptr);
	addSwitch(std::string("CHALLENGE INDICATORS"), std::string("Shows icons in the bottom right corner when eligible achievements can be earned."), "global.retroachievements.challenge_indicators", false, nullptr);

	// Unlock sound
	auto installedRSounds = ApiSystem::getInstance()->getRetroachievementsSoundsList();
	if (installedRSounds.size() > 0)
	{
		std::string currentSound = SystemConf::getInstance()->get("global.retroachievements.sound");

		auto rsounds_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, std::string("RETROACHIEVEMENT UNLOCK SOUND"), false);
		rsounds_choices->add(std::string("none"), "none", currentSound.empty() || currentSound == "none");

		for (auto snd : installedRSounds)
			rsounds_choices->add(std::string(Utils::String::toUpper(snd).c_str()), snd, currentSound == snd);

		if (!rsounds_choices->hasSelection())
			rsounds_choices->selectFirstItem();

		addWithLabel(std::string("UNLOCK SOUND"), rsounds_choices);
		addSaveFunc([rsounds_choices] { SystemConf::getInstance()->set("global.retroachievements.sound", rsounds_choices->getSelected()); });
	}

	// retroachievements_hardcore_mode
	addSwitch(std::string("SHOW RETROACHIEVEMENTS ENTRY IN MAIN MENU"), std::string("View your RetroAchievement stats right from the main menu!"), "RetroachievementsMenuitem", true, nullptr);

	addGroup(std::string("GAME INDEXES"));
	addSwitch(std::string("INDEX NEW GAMES AT STARTUP"), "CheevosCheckIndexesAtStart", true);
	addEntry(std::string("INDEX GAMES"), true, [this]
	{
		if (ThreadedHasher::checkCloseIfRunning(mWindow))
			mWindow->pushGui(new GuiHashStart(mWindow, ThreadedHasher::HASH_CHEEVOS_MD5));
	});

	addSaveFunc([retroachievementsEnabled, retroachievements_enabled, username, password, window]
	{
		bool newState = retroachievements_enabled->getState();
		std::string newUsername = SystemConf::getInstance()->get("global.retroachievements.username");
		std::string newPassword = SystemConf::getInstance()->get("global.retroachievements.password");
		std::string token = SystemConf::getInstance()->get("global.retroachievements.token");

		if (newState && (!retroachievementsEnabled || username != newUsername || password != newPassword || token.empty()))
		{
			std::string tokenOrError;
			if (RetroAchievements::testAccount(newUsername, newPassword, tokenOrError))
			{
				SystemConf::getInstance()->set("global.retroachievements.token", tokenOrError);
			}
			else
			{
				SystemConf::getInstance()->set("global.retroachievements.token", "");

				window->pushGui(new GuiMsgBox(window, std::string("UNABLE TO ACTIVATE RETROACHIEVEMENTS:") + "\n" + tokenOrError, std::string("OK"), nullptr, GuiMsgBoxIcon::ICON_ERROR));
				retroachievements_enabled->setState(false);
				newState = false;
			}
		}
		else if (!newState)
			SystemConf::getInstance()->set("global.retroachievements.token", "");

		if (SystemConf::getInstance()->setBool("global.retroachievements", newState))
			if (!ThreadedHasher::isRunning() && newState)
				ThreadedHasher::start(window, ThreadedHasher::HASH_CHEEVOS_MD5, false, true);
	});
}
