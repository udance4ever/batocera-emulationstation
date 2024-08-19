#include "GuiScraperSettings.h"

#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiScraperStart.h"
#include "scrapers/ThreadedScraper.h"

GuiScraperSettings::GuiScraperSettings(Window* window) : GuiSettings(window, std::string("SCRAPER SETTINGS").c_str())
{
	auto scrap = Scraper::getScraper();
	if (scrap == nullptr)
		return;

	std::string scraper = Scraper::getScraperName(scrap);

	std::string imageSourceName = Settings::getInstance()->getString("ScrapperImageSrc");
	auto imageSource = std::make_shared< OptionListComponent<std::string> >(mWindow, std::string("IMAGE SOURCE"), false);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Screenshot) ||
		scrap->isMediaSupported(Scraper::ScraperMediaSource::Box2d) ||
		scrap->isMediaSupported(Scraper::ScraperMediaSource::Box3d) ||
		scrap->isMediaSupported(Scraper::ScraperMediaSource::Mix) ||
		scrap->isMediaSupported(Scraper::ScraperMediaSource::FanArt) ||
		scrap->isMediaSupported(Scraper::ScraperMediaSource::TitleShot))
	{
		// Image source : <image> tag

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Screenshot))
			imageSource->add(std::string("SCREENSHOT"), "ss", imageSourceName == "ss");

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::TitleShot))
			imageSource->add(std::string("TITLE SCREENSHOT"), "sstitle", imageSourceName == "sstitle");

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Mix))
		{
			imageSource->add(std::string("MIX V1"), "mixrbv1", imageSourceName == "mixrbv1");
			imageSource->add(std::string("MIX V2"), "mixrbv2", imageSourceName == "mixrbv2");
		}

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box2d))
			imageSource->add(std::string("BOX 2D"), "box-2D", imageSourceName == "box-2D");

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box3d))
			imageSource->add(std::string("BOX 3D"), "box-3D", imageSourceName == "box-3D");

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::FanArt))
			imageSource->add(std::string("FAN ART"), "fanart", imageSourceName == "fanart");

		imageSource->add(std::string("NONE"), "", imageSourceName.empty());

		if (!imageSource->hasSelection())
			imageSource->selectFirstItem();

		addWithLabel(std::string("IMAGE SOURCE"), imageSource);
		addSaveFunc([imageSource] { Settings::getInstance()->setString("ScrapperImageSrc", imageSource->getSelected()); });
	}

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box2d) || scrap->isMediaSupported(Scraper::ScraperMediaSource::Box3d))
	{
		// Box source : <thumbnail> tag
		std::string thumbSourceName = Settings::getInstance()->getString("ScrapperThumbSrc");
		auto thumbSource = std::make_shared< OptionListComponent<std::string> >(mWindow, std::string("BOX SOURCE"), false);
		thumbSource->add(std::string("NONE"), "", thumbSourceName.empty());

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box2d))
			thumbSource->add(std::string("BOX 2D"), "box-2D", thumbSourceName == "box-2D");

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box3d))//if (scraper == "HfsDB")
			thumbSource->add(std::string("BOX 3D"), "box-3D", thumbSourceName == "box-3D");

		if (!thumbSource->hasSelection())
			thumbSource->selectFirstItem();

		addWithLabel(std::string("BOX SOURCE"), thumbSource);
		addSaveFunc([thumbSource] { Settings::getInstance()->setString("ScrapperThumbSrc", thumbSource->getSelected()); });

		imageSource->setSelectedChangedCallback([this, scrap, thumbSource](std::string value)
		{
			if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box2d))
			{
				if (value == "box-2D")
					thumbSource->remove(std::string("BOX 2D"));
				else
					thumbSource->add(std::string("BOX 2D"), "box-2D", false);
			}

			if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Box3d))
			{
				if (value == "box-3D")
					thumbSource->remove(std::string("BOX 3D"));
				else
					thumbSource->add(std::string("BOX 3D"), "box-3D", false);
			}
		});
	}

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Marquee) || scrap->isMediaSupported(Scraper::ScraperMediaSource::Wheel))
	{
		// Logo source : <marquee> tag
		std::string logoSourceName = Settings::getInstance()->getString("ScrapperLogoSrc");

		auto logoSource = std::make_shared< OptionListComponent<std::string> >(mWindow, std::string("LOGO SOURCE"), false);
		logoSource->add(std::string("NONE"), "", logoSourceName.empty());

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Wheel))
			logoSource->add(std::string("WHEEL"), "wheel", logoSourceName == "wheel");

		if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Marquee)) // if (scraper == "HfsDB")
			logoSource->add(std::string("MARQUEE"), "marquee", logoSourceName == "marquee");

		if (!logoSource->hasSelection())
			logoSource->selectFirstItem();

		addWithLabel(std::string("LOGO SOURCE"), logoSource);
		addSaveFunc([logoSource] { Settings::getInstance()->setString("ScrapperLogoSrc", logoSource->getSelected()); });
	}

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Region))
	{
		std::string region = Settings::getInstance()->getString("ScraperRegion");
		auto regionCtrl = std::make_shared< OptionListComponent<std::string> >(mWindow, std::string("PREFERED REGION"), false);
		regionCtrl->addRange({ { std::string("AUTO"), "" }, { "EUROPE", "eu" },  { "USA", "us" }, { "JAPAN", "jp" } , { "WORLD", "wor" } }, region);

		if (!regionCtrl->hasSelection())
			regionCtrl->selectFirstItem();

		addWithLabel(std::string("PREFERED REGION"), regionCtrl);
		addSaveFunc([regionCtrl] { Settings::getInstance()->setString("ScraperRegion", regionCtrl->getSelected()); });
	}

	addSwitch(std::string("OVERWRITE NAMES"), "ScrapeNames", true);
	addSwitch(std::string("OVERWRITE DESCRIPTIONS"), "ScrapeDescription", true);
	addSwitch(std::string("OVERWRITE MEDIAS"), "ScrapeOverWrite", true);

	addGroup(std::string("SCRAPE FOR"));

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::ShortTitle))
		addSwitch(std::string("SHORT NAME"), "ScrapeShortTitle", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Ratings))
		addSwitch(std::string("COMMUNITY RATING"), "ScrapeRatings", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Video))
		addSwitch(std::string("VIDEO"), "ScrapeVideos", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::FanArt))
		addSwitch(std::string("FANART"), "ScrapeFanart", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Bezel_16_9))
		addSwitch(std::string("BEZEL (16:9)"), "ScrapeBezel", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::BoxBack))
		addSwitch(std::string("BOX BACKSIDE"), "ScrapeBoxBack", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Map))
		addSwitch(std::string("MAP"), "ScrapeMap", true);


	/*
	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::TitleShot))
	addSwitch(std::string("SCRAPE TITLESHOT"), "ScrapeTitleShot", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Cartridge))
	addSwitch(std::string("SCRAPE CARTRIDGE"), "ScrapeCartridge", true);
	*/

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::Manual))
		addSwitch(std::string("MANUAL"), "ScrapeManual", true);

	if (scrap->isMediaSupported(Scraper::ScraperMediaSource::PadToKey))
		addSwitch(std::string("PADTOKEY SETTINGS"), "ScrapePadToKey", true);

	// ScreenScraper Account		
	if (scraper == "ScreenScraper")
	{
		addGroup(std::string("ACCOUNT"));
		addInputTextRow(std::string("USERNAME"), "ScreenScraperUser", false, true);
		addInputTextRow(std::string("PASSWORD"), "ScreenScraperPass", true, true);
	}
}