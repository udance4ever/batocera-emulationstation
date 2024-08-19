#include "GuiSystemInformation.h"
#include "SystemConf.h"
#include "components/SwitchComponent.h"
#include "ThemeData.h"
#include "ApiSystem.h"
#include "views/UIModeController.h"

GuiSystemInformation::GuiSystemInformation(Window* window) : GuiSettings(window, std::string("INFORMATION").c_str())
{
	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	bool warning = ApiSystem::getInstance()->isFreeSpaceLimit();

	addGroup(std::string("INFORMATION"));

	addWithLabel(std::string("VERSION"), std::make_shared<TextComponent>(window, ApiSystem::getInstance()->getVersion(), font, color));
	addWithLabel(std::string("USER DISK USAGE"), std::make_shared<TextComponent>(window, ApiSystem::getInstance()->getFreeSpaceUserInfo(), font, warning ? 0xFF0000FF : color));
	addWithLabel(std::string("SYSTEM DISK USAGE"), std::make_shared<TextComponent>(window, ApiSystem::getInstance()->getFreeSpaceSystemInfo(), font, color));
	
	std::vector<std::string> infos = ApiSystem::getInstance()->getSystemInformations();
	if (infos.size() > 0)
	{
		addGroup(std::string("SYSTEM"));

		for (auto info : infos)
		{
			std::vector<std::string> tokens = Utils::String::split(info, ':');
			if (tokens.size() >= 2)
			{
				// concatenat the ending words
				std::string vname;
				for (unsigned int i = 1; i < tokens.size(); i++)
				{
					if (i > 1) vname += " ";
					vname += tokens.at(i);
				}

				addWithLabel(std::string(tokens.at(0).c_str()), std::make_shared<TextComponent>(window, vname, font, color));
			}
		}
	}

	addGroup(std::string("VIDEO DRIVER"));
	for (auto info : Renderer::getDriverInformation())
		addWithLabel(std::string(info.first.c_str()), std::make_shared<TextComponent>(window, info.second, font, color));
}
