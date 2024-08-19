#include "GuiBluetoothDeviceOptions.h"
#include "guis/GuiMsgBox.h"
#include "GuiLoading.h"
#include "ApiSystem.h"

GuiBluetoothDeviceOptions::GuiBluetoothDeviceOptions(Window* window, const std::string& id, const std::string& name, bool isConnected, std::function<void()> onComplete)
    : GuiComponent(window), mMenu(window, std::string("DEVICE OPTIONS").c_str()), mId(id), mName(name), mIsConnected(isConnected), mWaitingLoad(false), mOnComplete(onComplete)
{
    addChild(&mMenu);

    mMenu.setSubTitle(name);

    if (mIsConnected)
    {
        mMenu.addButton(std::string("DISCONNECT"), "disconnect", [&] { onDisconnectDevice(); });
    }
    else
    {
        mMenu.addButton(std::string("CONNECT"), "connect", [&] { onConnectDevice(); });
    }
    mMenu.addButton(std::string("FORGET"), "forget", [&] { onForgetDevice(); });
    mMenu.addButton(std::string("BACK"), "back", [&] { delete this; });

    mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
}

bool GuiBluetoothDeviceOptions::input(InputConfig* config, Input input)
{
    if (GuiComponent::input(config, input))
        return true;

    if (input.value != 0 && config->isMappedTo(BUTTON_BACK, input))
    {
        if (!mWaitingLoad)
            delete this;

        return true;
    }

    return false;
}

std::vector<HelpPrompt> GuiBluetoothDeviceOptions::getHelpPrompts()
{
    std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
    prompts.push_back(HelpPrompt(BUTTON_BACK, std::string("BACK")));
    return prompts;
}

void GuiBluetoothDeviceOptions::onForgetDevice()
{
    if (mWaitingLoad)
        return;

    Window* window = mWindow;
    window->pushGui(new GuiMsgBox(window, Utils::String::format(std::string("ARE YOU SURE YOU WANT TO FORGET '%s' ?").c_str(), mName.c_str()),
        std::string("YES"), [this, window]
        {             
            window->pushGui(new GuiLoading<bool>(window, std::string("PLEASE WAIT"),
                [this, window](auto gui)
                {
                    mWaitingLoad = true;
                    return ApiSystem::getInstance()->removeBluetoothDevice(mId);
                },
                [this, window](bool ret)
                {
                    mWaitingLoad = false;
                    if (mOnComplete) mOnComplete();
                    delete this;
                }));
        },
        std::string("NO"), nullptr));
}

void GuiBluetoothDeviceOptions::onConnectDevice()
{
    if (mWaitingLoad)
        return;

    Window* window = mWindow;
    window->pushGui(new GuiMsgBox(window, Utils::String::format(std::string("ARE YOU SURE YOU WANT TO CONNECT TO '%s' ?").c_str(), mName.c_str()),
        std::string("YES"), [this, window]
        {             
            window->pushGui(new GuiLoading<bool>(window, std::string("PLEASE WAIT"),
                [this, window](auto gui)
                {
                    mWaitingLoad = true;
                    return ApiSystem::getInstance()->connectBluetoothDevice(mId);
                },
                [this, window](bool ret)
                {
                    mWaitingLoad = false;
                    if (mOnComplete) mOnComplete();
                    delete this;
                }));
        },
        std::string("NO"), nullptr));
}

void GuiBluetoothDeviceOptions::onDisconnectDevice()
{
    if (mWaitingLoad)
        return;

    Window* window = mWindow;
    window->pushGui(new GuiMsgBox(window, Utils::String::format(std::string("ARE YOU SURE YOU WANT TO DISCONNECT '%s' ?").c_str(), mName.c_str()),
        std::string("YES"), [this, window]
        {             
            window->pushGui(new GuiLoading<bool>(window, std::string("PLEASE WAIT"),
                [this, window](auto gui)
                {
                    mWaitingLoad = true;
                    return ApiSystem::getInstance()->disconnectBluetoothDevice(mId);
                },
                [this, window](bool ret)
                {
                    mWaitingLoad = false;
                    if (mOnComplete) mOnComplete();
                    delete this;
                }));
        },
        std::string("NO"), nullptr));
}
