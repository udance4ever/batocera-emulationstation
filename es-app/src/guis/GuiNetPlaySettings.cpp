#include "GuiNetPlaySettings.h"
#include "SystemConf.h"
#include "ThreadedHasher.h"
#include "components/SwitchComponent.h"
#include "GuiHashStart.h"

GuiNetPlaySettings::GuiNetPlaySettings(Window* window) : GuiSettings(window, std::string("NETPLAY SETTINGS").c_str())
{
	std::string port = SystemConf::getInstance()->get("global.netplay.port");
	if (port.empty())
		SystemConf::getInstance()->set("global.netplay.port", "55435");

	addGroup(std::string("SETTINGS"));

	auto enableNetplay = std::make_shared<SwitchComponent>(mWindow);
	enableNetplay->setState(SystemConf::getInstance()->getBool("global.netplay"));
	addWithLabel(std::string("ENABLE NETPLAY"), enableNetplay);
	addInputTextRow(std::string("NICKNAME"), "global.netplay.nickname", false);

	addGroup(std::string("OPTIONS"));

	addInputTextRow(std::string("PORT"), "global.netplay.port", false);
	addOptionList(std::string("USE RELAY SERVER"), { { std::string("NONE"), "" },{ std::string("NEW YORK") , "nyc" },{ std::string("MADRID") , "madrid" },{ std::string("MONTREAL") , "montreal" },{ std::string("SAO PAULO") , "saopaulo" } }, "global.netplay.relay", false);
	addSwitch(std::string("SHOW UNAVAILABLE GAMES"), std::string("Show rooms for games not present on this machine."), "NetPlayShowMissingGames", true, nullptr);

	addGroup(std::string("GAME INDEXES"));

	addSwitch(std::string("INDEX NEW GAMES AT STARTUP"), "NetPlayCheckIndexesAtStart", true);
	addEntry(std::string("INDEX GAMES"), true, [this]
	{
		if (ThreadedHasher::checkCloseIfRunning(mWindow))
			mWindow->pushGui(new GuiHashStart(mWindow, ThreadedHasher::HASH_NETPLAY_CRC));
	});

	Window* wnd = mWindow;
	addSaveFunc([wnd, enableNetplay]
	{
		if (SystemConf::getInstance()->setBool("global.netplay", enableNetplay->getState()))
		{
			if (!ThreadedHasher::isRunning() && enableNetplay->getState())
			{
				ThreadedHasher::start(wnd, ThreadedHasher::HASH_NETPLAY_CRC, false, true);
			}
		}
	});
}
