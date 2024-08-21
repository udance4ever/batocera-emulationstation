//  SPDX-License-Identifier: MIT
//
//  ES-DE Frontend
//  Renderer.cpp
//
//  Generic rendering functions.
//

#include "renderers/Renderer.h"

// $$ ES-DE Renderer doesn't link these
//#include "Renderer_GL21.h"
//#include "Renderer_GLES10.h"
//#include "Renderer_GLES20.h"

// $$ Font.h:57:49: error: unknown type name 'Transform4x4f'
#include "math/Transform4x4f.h"
#include "math/Vector2i.h"

#include "ImageIO.h"
#include "Log.h"
#include "Settings.h"
#include "renderers/RendererOpenGL.h"
#include "renderers/ShaderOpenGL.h"
#include "resources/ResourceManager.h"

#include "utils/StringUtil.h"

#include <SDL.h>
#include <stack>

#if defined(_WIN64)
#include <windows.h>

// $$ batocera version
//#if WIN32
//#include <Windows.h>
//#include <SDL_syswm.h>
//
//#include <dwmapi.h>
//#pragma comment(lib, "Dwmapi.lib")
//
//#include "utils/Platform.h"
//#endif

Renderer* Renderer::getInstance()
{
    static RendererOpenGL instance;
    return &instance;
}

	static std::stack<Rect> clipStack;
	static std::stack<Rect> nativeClipStack;

	static SDL_Window*      sdlWindow          = nullptr;
	static int              windowWidth        = 0;
	static int              windowHeight       = 0;
	static int              screenWidth        = 0;
	static int              screenHeight       = 0;
	static int              screenOffsetX      = 0;
	static int              screenOffsetY      = 0;
	static int              screenRotate       = 0;
	static bool             initialCursorState = 1;
	static Vector2i         screenMargin;
	static Rect				viewPort; // $$ used

	static Vector2i			sdlWindowPosition = Vector2i(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);

	static int              currentFrame = 0;

	int  getCurrentFrame() { return currentFrame; }



        static Rect screenToviewport(const Rect& rect)
        {
                Rect rc = rect;

                float dx = (screenWidth - 2 * screenMargin.x()) / (float)screenWidth;
                float dy = (screenHeight - 2 * screenMargin.y()) / (float)screenHeight;

                rc.x = screenMargin.x() + rc.x * dx;
                rc.y = screenMargin.y() + rc.y * dy;
                rc.w = (float) rc.w * dx;
                rc.h = (float) rc.h * dy;

                return rc;                                                                                          
        }

	// GuiComponent.cpp:108:24: error: no member named 'getScreenRect'
        Rect            getScreenRect(const Transform4x4f& transform, const Vector3f& size, bool viewPort)
        {
                return getScreenRect(transform, Vector2f(size.x(), size.y()), viewPort);
        }

	Rect		getScreenRect(const Transform4x4f& transform, const Vector2f& size, bool viewPort)
	{
		auto rc = Rect(
			(int)Math::round(transform.translation().x()),
			(int)Math::round(transform.translation().y()),
			(int)Math::round(size.x() * transform.r0().x()),
			(int)Math::round(size.y() * transform.r1().y()));

		if (viewPort && screenMargin.x() != 0 && screenMargin.y() != 0)
			return screenToviewport(rc);

		return rc;
	}

	// $$ used
	Vector2i  physicalScreenToRotatedScreen(int x, int y)
	{
		int mx = x;
		int my = y;

		switch (Renderer::getScreenRotate())
		{
		case 1:
			mx = Renderer::getScreenHeight() - mx;
			std::swap(mx, my);
			break;
		case 2:
			mx = Renderer::getScreenWidth() - mx;
			my = Renderer::getScreenHeight() - my;
			break;
		case 3:
			my = Renderer::getScreenWidth() - my;
			std::swap(mx, my);
			break;
		}

		return Vector2i(mx, my);
	}

// ES-DE version
void Renderer::setIcon()
{
    size_t width {0};
    size_t height {0};
    ResourceData resData {
        ResourceManager::getInstance().getFileData(":/graphics/window_icon_256.png")};
    std::vector<unsigned char> rawData {
        ImageIO::loadFromMemoryRGBA32(resData.ptr.get(), resData.length, width, height)};

    if (!rawData.empty()) {
        ImageIO::flipPixelsVert(rawData.data(), width, height);

        constexpr unsigned int bmask {0x00FF0000};
        constexpr unsigned int gmask {0x0000FF00};
        constexpr unsigned int rmask {0x000000FF};
        constexpr unsigned int amask {0xFF000000};

        // Try creating SDL surface from logo data.
        SDL_Surface* logoSurface {SDL_CreateRGBSurfaceFrom(
            static_cast<void*>(rawData.data()), static_cast<int>(width), static_cast<int>(height),
            32, static_cast<int>(width * 4), bmask, gmask, rmask, amask)};

        if (logoSurface != nullptr) {
            SDL_SetWindowIcon(mSDLWindow, logoSurface);
            SDL_FreeSurface(logoSurface);
        }
    }
}

// $$ ES-DE version
bool Renderer::createWindow()
{
    LOG(LogInfo) << "Creating window...";

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        LOG(LogError) << "Couldn't initialize SDL: " << SDL_GetError();
        return false;
    }

    mInitialCursorState = (SDL_ShowCursor(0) != 0);

    int displayIndex {Settings::getInstance()->getInt("DisplayIndex")};
    // Check that an invalid value has not been manually entered in the es_settings.xml file.
    if (displayIndex != 1 && displayIndex != 2 && displayIndex != 3 && displayIndex != 4) {
        Settings::getInstance()->setInt("DisplayIndex", 1);
        displayIndex = 0;
    }
    else {
        --displayIndex;
    }

    int availableDisplays = SDL_GetNumVideoDisplays();
    if (displayIndex > availableDisplays - 1) {
        LOG(LogWarning) << "Requested display " << std::to_string(displayIndex + 1)
                        << " does not exist, changing to display 1";
        displayIndex = 0;
    }
    else {
        LOG(LogInfo) << "Using display: " << std::to_string(displayIndex + 1);
    }

    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(displayIndex, &displayMode);

#if defined(_WIN64)
    // Tell Windows that we're DPI aware so that we can set a physical resolution and
    // avoid any automatic DPI scaling.
    SetProcessDPIAware();
    // We need to set the resolution based on the actual display bounds as the numbers
    // returned by SDL_GetDesktopDisplayMode are calculated based on DPI scaling and
    // therefore do not necessarily reflect the physical display resolution.
    SDL_Rect displayBounds;
    SDL_GetDisplayBounds(displayIndex, &displayBounds);
    displayMode.w = displayBounds.w;
    displayMode.h = displayBounds.h;
#endif

    sScreenWidth = Settings::getInstance()->getInt("ScreenWidth") ?
                       Settings::getInstance()->getInt("ScreenWidth") :
                       displayMode.w;
    sScreenHeight = Settings::getInstance()->getInt("ScreenHeight") ?
                        Settings::getInstance()->getInt("ScreenHeight") :
                        displayMode.h;
    mScreenOffsetX = glm::clamp((Settings::getInstance()->getInt("ScreenOffsetX") ?
                                     Settings::getInstance()->getInt("ScreenOffsetX") :
                                     0),
                                -(displayMode.w / 2), displayMode.w / 2);
    mScreenOffsetY = glm::clamp((Settings::getInstance()->getInt("ScreenOffsetY") ?
                                     Settings::getInstance()->getInt("ScreenOffsetY") :
                                     0),
                                -(displayMode.w / 2), displayMode.h / 2);
    mScreenRotation = Settings::getInstance()->getInt("ScreenRotate");

    if (mScreenOffsetX != 0 || mScreenOffsetY != 0) {
        LOG(LogInfo) << "Screen offset: " << mScreenOffsetX << " horizontal, " << mScreenOffsetY
                     << " vertical";
    }
    else {
        LOG(LogInfo) << "Screen offset: disabled";
    }

    mPaddingWidth = 0;
    mPaddingHeight = 0;
    bool fullscreenPadding {false};

    if (Settings::getInstance()->getBool("FullscreenPadding") && sScreenWidth <= displayMode.w &&
        sScreenHeight <= displayMode.h) {
        mWindowWidth = displayMode.w;
        mWindowHeight = displayMode.h;
        mPaddingWidth = displayMode.w - sScreenWidth;
        mPaddingHeight = displayMode.h - sScreenHeight;
        mScreenOffsetX -= mPaddingWidth / 2;
        mScreenOffsetY -= mPaddingHeight / 2;
        fullscreenPadding = true;
    }

    if (!fullscreenPadding) {
        mWindowWidth = sScreenWidth;
        mWindowHeight = sScreenHeight;
    }

    // In case someone manually added an invalid value to es_settings.xml.
    if (mScreenRotation != 0 && mScreenRotation != 90 && mScreenRotation != 180 &&
        mScreenRotation != 270) {
        LOG(LogWarning) << "Invalid screen rotation value " << mScreenRotation
                        << " defined, changing it to 0/disabled";
        mScreenRotation = 0;
    }

    LOG(LogInfo) << "Screen rotation: "
                 << (mScreenRotation == 0 ? "disabled" :
                                            std::to_string(mScreenRotation) + " degrees");

    if (mScreenRotation == 90 || mScreenRotation == 270) {
        const int tempVal {sScreenWidth};
        sScreenWidth = sScreenHeight;
        sScreenHeight = tempVal;
    }

    if (sScreenHeight >= sScreenWidth)
        sIsVerticalOrientation = true;
    else
        sIsVerticalOrientation = false;

    // Prevent the application window from minimizing when switching windows (when launching
    // games or when manually switching windows using the task switcher).
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

#if defined(__unix__) && !defined(__ANDROID__)
    // Disabling desktop composition can lead to better framerates and a more fluid user
    // interface, but with some drivers it can cause strange behaviors when returning to
    // the desktop.
    if (Settings::getInstance()->getBool("DisableComposition"))
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "1");
    else
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

    bool userResolution {false};
    // Check if the user has changed the resolution from the command line.
    if (mWindowWidth != displayMode.w || mWindowHeight != displayMode.h)
        userResolution = true;

    unsigned int windowFlags {0};
    setup();

#if defined(_WIN64)
    // For Windows we use SDL_WINDOW_BORDERLESS as "real" full screen doesn't work properly.
    // The borderless mode seems to behave well and it's almost completely seamless, especially
    // with a hidden taskbar.
    if (!userResolution)
        windowFlags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL;
    else
        // If the resolution has been manually set from the command line, then keep the border.
        windowFlags = SDL_WINDOW_OPENGL;
#elif defined(__APPLE__)
    // Not sure if this could be a useful setting.
    //        SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "0");

    // The SDL_WINDOW_BORDERLESS mode seems to be the only mode that somehow works on macOS
    // as a real fullscreen mode will do lots of weird stuff like preventing window switching
    // or refusing to let emulators run at all. SDL_WINDOW_FULLSCREEN_DESKTOP almost works, but
    // it "shuffles" windows when starting the emulator and won't return properly when the game
    // has exited. With SDL_WINDOW_BORDERLESS some emulators (like RetroArch) have to be
    // configured to run in fullscreen mode or switching to its window will not work, but
    // apart from that this mode works fine.
    if (!userResolution)
        windowFlags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;
    else
        windowFlags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;
#else
    if (!userResolution)
        windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL;
    else
        windowFlags = SDL_WINDOW_OPENGL;
#endif

    if ((mSDLWindow = SDL_CreateWindow("ES-DE", SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayIndex),
                                       SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayIndex), mWindowWidth,
                                       mWindowHeight, windowFlags)) == nullptr) {
        LOG(LogError) << "Couldn't create SDL window. " << SDL_GetError();
        return false;
    }

#if defined(__APPLE__)
    // The code below is required as the high DPI scaling on macOS is very bizarre and is
    // measured in "points" rather than pixels (even though the naming convention sure looks
    // like pixels). For example there could be a 1920x1080 entry in the OS display settings
    // that actually corresponds to something like 3840x2160 pixels while at the same time
    // there is a separate 1080p entry which corresponds to a "real" 1920x1080 resolution.
    // Therefore the --resolution flag results in different things depending on whether a high
    // DPI screen is used. E.g. 1280x720 on a 4K display would actually end up as 2560x1440
    // which is incredibly strange. No point in struggling with this strangeness though,
    // instead we simply indicate the physical pixel dimensions in parenthesis in the log
    // file and make sure to double the window and screen sizes in case of a high DPI
    // display so that the full application window is used for rendering.
    int width {0};
    SDL_GL_GetDrawableSize(mSDLWindow, &width, nullptr);
    int scaleFactor {static_cast<int>(width / mWindowWidth)};

    LOG(LogInfo) << "Display resolution: " << std::to_string(displayMode.w) << "x"
                 << std::to_string(displayMode.h) << " (physical resolution "
                 << std::to_string(displayMode.w * scaleFactor) << "x"
                 << std::to_string(displayMode.h * scaleFactor) << ")";
    LOG(LogInfo) << "Display refresh rate: " << std::to_string(displayMode.refresh_rate) << " Hz";
    LOG(LogInfo) << "Application resolution: " << std::to_string(sScreenWidth) << "x"
                 << std::to_string(sScreenHeight) << " (physical resolution "
                 << std::to_string(sScreenWidth * scaleFactor) << "x"
                 << std::to_string(sScreenHeight * scaleFactor) << ")";

    mWindowWidth *= scaleFactor;
    mWindowHeight *= scaleFactor;
    sScreenWidth *= scaleFactor;
    sScreenHeight *= scaleFactor;
    mPaddingWidth *= scaleFactor;
    mPaddingHeight *= scaleFactor;
    mScreenOffsetX *= scaleFactor;
    mScreenOffsetY *= scaleFactor;

#else
    LOG(LogInfo) << "Display resolution: " << std::to_string(displayMode.w) << "x"
                 << std::to_string(displayMode.h);
    LOG(LogInfo) << "Display refresh rate: " << std::to_string(displayMode.refresh_rate) << " Hz";
    LOG(LogInfo) << "Application resolution: " << std::to_string(sScreenWidth) << "x"
                 << std::to_string(sScreenHeight);
#endif

    sScreenHeightModifier = static_cast<float>(sScreenHeight) / 1080.0f;
    sScreenWidthModifier = static_cast<float>(sScreenWidth) / 1920.0f;
    sScreenAspectRatio = static_cast<float>(sScreenWidth) / static_cast<float>(sScreenHeight);

    if (sIsVerticalOrientation)
        sScreenResolutionModifier = sScreenWidth / 1080.0f;
    else
        sScreenResolutionModifier = sScreenHeight / 1080.0f;

    if (Settings::getInstance()->getBool("FullscreenPadding")) {
        if (!fullscreenPadding) {
            LOG(LogWarning) << "Fullscreen padding can't be applied when --resolution is set "
                               "higher than the display resolution";
            LOG(LogInfo) << "Screen mode: windowed";
        }
        else {
            LOG(LogInfo) << "Screen mode: fullscreen padding";
        }
    }
    else if (userResolution) {
        LOG(LogInfo) << "Screen mode: windowed";
    }
    else {
        LOG(LogInfo) << "Screen mode: fullscreen";
    }

    LOG(LogInfo) << "Setting up OpenGL...";

    if (!createContext())
        return false;

    setIcon();
    setSwapInterval();

#if defined(_WIN64) || defined(__ANDROID__)
    // It seems as if Windows needs this to avoid a brief white screen flash on startup.
    // Possibly this is driver-specific rather than OS-specific. There is additional code
    // in init() to work around the white screen flash issue on all operating systems.
    // On Android the swap is also necessary to avoid displaying random garbage when
    // the rendering starts.
    swapBuffers();
#endif

    return loadShaders();
}

// $$ ED-DE version
void Renderer::destroyWindow()
{
    destroyContext();
    SDL_DestroyWindow(mSDLWindow);

    mSDLWindow = nullptr;

    SDL_ShowCursor(mInitialCursorState);
    SDL_Quit();
}

	void activateWindow()
	{
		SDL_RestoreWindow(sdlWindow);
		SDL_RaiseWindow(sdlWindow);

		if (Settings::getInstance()->getBool("Windowed"))
		{
			int h; int w;
			SDL_GetWindowSize(sdlWindow, &w, &h);

			SDL_DisplayMode DM;
			SDL_GetCurrentDisplayMode(0, &DM);

			if (w == DM.w && h == DM.h)
				SDL_SetWindowPosition(sdlWindow, 0, 0);
		}
		
		SDL_SetWindowInputFocus(sdlWindow);		
	}


	void updateProjection()
	{
		Transform4x4f projection = Transform4x4f::Identity();
		Rect          viewport;

		switch(screenRotate)
		{
			case 0:
			{
				viewport.x = screenOffsetX + screenMargin.x();
				viewport.y = screenOffsetY + screenMargin.y();
				viewport.w = screenWidth - 2 * screenMargin.x();
				viewport.h = screenHeight - 2 * screenMargin.y();

				projection.orthoProjection(0, screenWidth, screenHeight, 0, -1.0, 1.0);
			}
			break;

			case 1:
			{
				viewport.x = windowWidth - screenOffsetY - screenHeight;
				viewport.y = screenOffsetX;
				viewport.w = screenHeight;
				viewport.h = screenWidth;

				projection.orthoProjection(0, screenHeight, screenWidth, 0, -1.0, 1.0);
				projection.rotate((float)ES_DEG_TO_RAD(90), {0, 0, 1});
				projection.translate({0, screenHeight * -1.0f, 0});
			}
			break;

			case 2:
			{
				viewport.x = windowWidth  - screenOffsetX - screenWidth;
				viewport.y = windowHeight - screenOffsetY - screenHeight;
				viewport.w = screenWidth;
				viewport.h = screenHeight;

				projection.orthoProjection(0, screenWidth, screenHeight, 0, -1.0, 1.0);
				projection.rotate((float)ES_DEG_TO_RAD(180), {0, 0, 1});
				projection.translate({screenWidth * -1.0f, screenHeight * -1.0f, 0});
			}
			break;

			case 3:
			{
				viewport.x = screenOffsetY;
				viewport.y = windowHeight - screenOffsetX - screenWidth;
				viewport.w = screenHeight;
				viewport.h = screenWidth;

				projection.orthoProjection(0, screenHeight, screenWidth, 0, -1.0, 1.0);
				projection.rotate((float)ES_DEG_TO_RAD(270), {0, 0, 1});
				projection.translate({screenWidth * -1.0f, 0, 0});
			}
			break;
		}

		setViewport(viewport);
		setProjection(projection);
	}

	// $$ used
	Vector2i	setScreenMargin(int marginX, int marginY)
	{
		auto oldMargin = screenMargin;

		if (screenMargin.x() == marginX && screenMargin.y() == marginY)
			return oldMargin;
		
		screenMargin = Vector2i(marginX, marginY);

		Rect          viewport;
		viewport.x = screenOffsetX + screenMargin.x();
		viewport.y = screenOffsetY + screenMargin.y();
		viewport.w = screenWidth - 2 * screenMargin.x();
		viewport.h = screenHeight - 2 * screenMargin.y();

		setViewport(viewport);
		return oldMargin;
	}





bool Renderer::init()
{
    if (!createWindow())
        return false;

    glm::mat4 projection {getIdentity()};

    if (mScreenRotation == 0) {
        mViewport.x = mWindowWidth + mScreenOffsetX - sScreenWidth;
        mViewport.y = mWindowHeight + mScreenOffsetY - sScreenHeight;
        mViewport.w = sScreenWidth;
        mViewport.h = sScreenHeight;
        mProjectionMatrix = glm::ortho(0.0f, static_cast<float>(sScreenWidth),
                                       static_cast<float>(sScreenHeight), 0.0f, -1.0f, 1.0f);
    }
    else if (mScreenRotation == 90) {
        mViewport.x = mWindowWidth + mScreenOffsetX - sScreenHeight;
        mViewport.y = mWindowHeight + mScreenOffsetY - sScreenWidth;
        mViewport.w = sScreenHeight;
        mViewport.h = sScreenWidth;
        projection = glm::ortho(0.0f, static_cast<float>(sScreenHeight),
                                static_cast<float>(sScreenWidth), 0.0f, -1.0f, 1.0f);
        projection = glm::rotate(projection, glm::radians(90.0f), {0.0f, 0.0f, 1.0f});
        mProjectionMatrix = glm::translate(projection, {0.0f, sScreenHeight * -1.0f, 0.0f});
    }
    else if (mScreenRotation == 180) {
        mViewport.x = mWindowWidth + mScreenOffsetX - sScreenWidth;
        mViewport.y = mWindowHeight + mScreenOffsetY - sScreenHeight;
        mViewport.w = sScreenWidth;
        mViewport.h = sScreenHeight;
        projection = glm::ortho(0.0f, static_cast<float>(sScreenWidth),
                                static_cast<float>(sScreenHeight), 0.0f, -1.0f, 1.0f);
        projection = glm::rotate(projection, glm::radians(180.0f), {0.0f, 0.0f, 1.0f});
        mProjectionMatrix =
            glm::translate(projection, {sScreenWidth * -1.0f, sScreenHeight * -1.0f, 0.0f});
    }
    else if (mScreenRotation == 270) {
        mViewport.x = mWindowWidth + mScreenOffsetX - sScreenHeight;
        mViewport.y = mWindowHeight + mScreenOffsetY - sScreenWidth;
        mViewport.w = sScreenHeight;
        mViewport.h = sScreenWidth;
        projection = glm::ortho(0.0f, static_cast<float>(sScreenHeight),
                                static_cast<float>(sScreenWidth), 0.0f, -1.0f, 1.0f);
        projection = glm::rotate(projection, glm::radians(270.0f), {0.0f, 0.0f, 1.0f});
        mProjectionMatrix = glm::translate(projection, {sScreenWidth * -1.0f, 0.0f, 0.0f});
    }

    mProjectionMatrixNormal = glm::ortho(0.0f, static_cast<float>(sScreenWidth),
                                         static_cast<float>(sScreenHeight), 0.0f, -1.0f, 1.0f);
    setViewport(mViewport);

    // This is required to avoid a brief white screen flash during startup on some systems.
    drawRect(0.0f, 0.0f, static_cast<float>(getScreenWidth()),
             static_cast<float>(getScreenHeight()), 0x000000FF, 0x000000FF);
    swapBuffers();

    return true;
}

void Renderer::deinit()
{
    // Destroy the window.
    destroyWindow();
}

void Renderer::pushClipRect(const glm::ivec2& pos, const glm::ivec2& size)
{
    Rect box {pos.x, pos.y, size.x, size.y};

    if (box.w == 0)
        box.w = sScreenWidth - box.x;
    if (box.h == 0)
        box.h = sScreenHeight - box.y;

    if (mScreenRotation == 0) {
        box = {mScreenOffsetX + box.x + mPaddingWidth, mScreenOffsetY + box.y + mPaddingHeight,
               box.w, box.h};
    }
    else if (mScreenRotation == 90) {
        box = {mScreenOffsetX + mWindowWidth - (box.y + box.h), mScreenOffsetY + box.x, box.h,
               box.w + mPaddingHeight};
    }
    else if (mScreenRotation == 270) {
        box = {mScreenOffsetX + box.y + mPaddingWidth,
               mScreenOffsetY + mWindowHeight - (box.x + box.w), box.h, box.w};
    }
    else if (mScreenRotation == 180) {
        box = {mWindowWidth + mScreenOffsetX - box.x - box.w,
               mWindowHeight + mScreenOffsetY - box.y - box.h, box.w, box.h};
    }

    // Make sure the box fits within mClipStack.top(), and clip further accordingly.
    if (mClipStack.size()) {
        const Rect& top {mClipStack.top()};
        if (top.x > box.x)
            box.x = top.x;
        if (top.y > box.y)
            box.y = top.y;
        if ((top.x + top.w) < (box.x + box.w))
            box.w = (top.x + top.w) - box.x;
        if ((top.y + top.h) < (box.y + box.h))
            box.h = (top.y + top.h) - box.y;
    }

    if (box.w < 0)
        box.w = 0;
    if (box.h < 0)
        box.h = 0;

    mClipStack.push(box);
    setScissor(box);
}

	// $$ batocera
	void pushClipRect(const Vector2i& _pos, const Vector2i& _size)
	{
		pushClipRect(_pos.x(), _pos.y(), _size.x(), _size.y());
	}

	void pushClipRect(Rect rect)
	{
		pushClipRect(rect.x, rect.y, rect.w, rect.h);
	}

	void pushClipRect(int x, int y, int w, int h)	
	{
		Rect box(x, y, w, h);

		if(box.w == 0) box.w = screenWidth  - box.x;
		if(box.h == 0) box.h = screenHeight - box.y;

		switch(screenRotate)
		{
			case 0: { box = Rect(screenOffsetX + box.x,                       screenOffsetY + box.y,                        box.w, box.h); } break;
			case 1: { box = Rect(windowWidth - screenOffsetY - box.y - box.h, screenOffsetX + box.x,                        box.h, box.w); } break;
			case 2: { box = Rect(windowWidth - screenOffsetX - box.x - box.w, windowHeight - screenOffsetY - box.y - box.h, box.w, box.h); } break;
			case 3: { box = Rect(screenOffsetY + box.y,                       windowHeight - screenOffsetX - box.x - box.w, box.h, box.w); } break;
		}

		// make sure the box fits within clipStack.top(), and clip further accordingly
		if(clipStack.size())
		{
			const Rect& top = clipStack.top();
			if (top.x > box.x)
			{
				box.w += (box.x - top.x);
				box.x = top.x;
			}
			if (top.y > box.y)
			{
				box.h += (box.y - top.y);
				box.y = top.y;				
			}
			if((top.x + top.w) < (box.x + box.w)) 
				box.w = (top.x + top.w) - box.x;
			if((top.y + top.h) < (box.y + box.h)) 
				box.h = (top.y + top.h) - box.y;
		}

		if(box.w < 0) box.w = 0;
		if(box.h < 0) box.h = 0;

		clipStack.push(box);
		nativeClipStack.push(Rect(x, y, w, h));

		if (screenMargin.x() != 0 && screenMargin.y() != 0)
			box = screenToviewport(box);

		setScissor(box);

	} // pushClipRect


void Renderer::popClipRect()
{
    if (mClipStack.empty()) {
        LOG(LogError) << "Tried to popClipRect while the stack was empty";
        return;
    }

    mClipStack.pop();

    if (mClipStack.empty())
        setScissor(Rect());
    else
        setScissor(mClipStack.top());
}

void Renderer::drawRect(const float x,
                        const float y,
                        const float w,
                        const float h,
                        const unsigned int color,
                        const unsigned int colorEnd,
                        const bool horizontalGradient,
                        const float opacity,
                        const float dimming,
                        const BlendFactor srcBlendFactor,
                        const BlendFactor dstBlendFactor,
                        const float cornerRadius)
{
    Vertex vertices[4];

    float wL {w};
    float hL {h};

    // If the width or height was scaled down to less than 1 pixel, then set it to
    // 1 pixel so that it will still render on lower resolutions.
    if (wL > 0.0f && wL < 1.0f)
        wL = 1.0f;
    if (hL > 0.0f && hL < 1.0f)
        hL = 1.0f;

    // clang-format off
    vertices[0] = {{x,      y     }, {0.0f, 0.0f}, color};
    vertices[1] = {{x,      y + hL}, {0.0f, 0.0f}, horizontalGradient ? color : colorEnd};
    vertices[2] = {{x + wL, y     }, {0.0f, 0.0f}, horizontalGradient ? colorEnd : color};
    vertices[3] = {{x + wL, y + hL}, {0.0f, 0.0f}, colorEnd};
    // clang-format on

    // Round vertices.
    for (int i {0}; i < 4; ++i)
        vertices[i].position = glm::round(vertices[i].position);

    vertices->opacity = opacity;
    vertices->dimming = dimming;

    if (cornerRadius > 0.0f) {
        vertices->shaderFlags = vertices->shaderFlags | Renderer::ShaderFlags::ROUNDED_CORNERS;
        vertices->cornerRadius = cornerRadius;
    }

    bindTexture(0, 0);
    drawTriangleStrips(vertices, 4, srcBlendFactor, dstBlendFactor);
}

	void drawRect(const float _x, const float _y, const float _w, const float _h, const unsigned int _color, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		drawRect(_x, _y, _w, _h, _color, _color, true, _srcBlendFactor, _dstBlendFactor);
	} // drawRect

	void drawRect(const float _x, const float _y, const float _w, const float _h, const unsigned int _color, const unsigned int _colorEnd, bool horizontalGradient, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		const unsigned int color    = convertColor(_color);
		const unsigned int colorEnd = convertColor(_colorEnd);
		Vertex             vertices[4];

		vertices[0] = { { _x     ,_y      }, { 0.0f, 0.0f }, color };
		vertices[1] = { { _x     ,_y + _h }, { 0.0f, 0.0f }, horizontalGradient ? colorEnd : color };
		vertices[2] = { { _x + _w,_y      }, { 0.0f, 0.0f }, horizontalGradient ? color : colorEnd };
		vertices[3] = { { _x + _w,_y + _h }, { 0.0f, 0.0f }, colorEnd };

		// round vertices
		//for(int i = 0; i < 4; ++i)
		//	vertices[i].pos.round();

		bindTexture(0);
		drawTriangleStrips(vertices, 4, _srcBlendFactor, _dstBlendFactor);

	} // drawRect

	SDL_Window* getSDLWindow()     { return sdlWindow; }
	int         getWindowWidth()   { return windowWidth; }
	int         getWindowHeight()  { return windowHeight; }
	int         getScreenWidth()   { return screenWidth; }
	int         getScreenHeight()  { return screenHeight; }
	int         getScreenOffsetX() { return screenOffsetX; }
	int         getScreenOffsetY() { return screenOffsetY; }
	int         getScreenRotate()  { return screenRotate; }
// $$ used - static
static	bool		isVerticalScreen() { return screenHeight > screenWidth; }

	float		getScreenProportion() 
	{ 
		if (screenHeight == 0)
			return 1.0;

		return (float) screenWidth / (float) screenHeight;
	}

	std::map<std::string, float> ratios =
	{
		{ "4/3",		   4.0f / 3.0f },
		{ "16/9",          16.0f / 9.0f },
		{ "16/10",         16.0f / 10.0f },
		{ "16/15",         16.0f / 15.0f },
		{ "21/9",          21.0f / 9.0f },
		{ "1/1",           1 / 1 },
		{ "2/1",           2.0f / 1.0f },
		{ "3/2",           3.0f / 2.0f },
		{ "3/4",           3.0f / 4.0f },
		{ "4/1",           4.0f / 1.0f },
		{ "9/16",          9.0f / 16.0f },
		{ "5/4",           5.0f / 4.0f },
		{ "6/5",           6.0f / 5.0f },
		{ "7/9",           7.0f / 9.0f },
		{ "8/3",           8.0f / 3.0f },
		{ "8/7",           8.0f / 7.0f },
		{ "19/12",         19.0f / 12.0f },
		{ "19/14",         19.0f / 14.0f },
		{ "30/17",         30.0f / 17.0f },
		{ "32/9",          32.0f / 9.0f }
	};

	// $$ used
	std::string  getAspectRatio()
	{
		float nearDist = 9999999;
		std::string nearName = "";

		float prop = Renderer::getScreenProportion();

		for (auto ratio : ratios)
		{
			float dist = abs(prop - ratio.second);
			if (dist < nearDist)
			{
				nearDist = dist;
				nearName = ratio.first;
			}
		}

		return nearName;
	}

	// $$ used - made static
static	bool        isSmallScreen()    
	{ 		
		return ScreenSettings::isSmallScreen();
		//return screenWidth <= 480 || screenHeight <= 480; 
	};

	bool isClippingEnabled() { return !clipStack.empty(); }

	inline bool valueInRange(int value, int min, int max)
	{
		return (value >= min) && (value <= max);
	}

	bool rectOverlap(Rect &A, Rect &B)
	{
		bool xOverlap = valueInRange(A.x, B.x, B.x + B.w) ||
			valueInRange(B.x, A.x, A.x + A.w);

		bool yOverlap = valueInRange(A.y, B.y, B.y + B.h) ||
			valueInRange(B.y, A.y, A.y + A.h);

		return xOverlap && yOverlap;
	}

	// $$ used
	bool isVisibleOnScreen(float x, float y, float w, float h)
	{
		static Rect screen = Rect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight());

		if (w > 0 && x + w <= 0)
			return false;

		if (h > 0 && y + h <= 0)
			return false;
		
		if (x == screen.w || y == screen.h)
			return false;

		Rect box = Rect(x, y, w, h);

		if (!rectOverlap(box, screen))
			return false;
			
		if (clipStack.empty())
			return true;

		if (nativeClipStack.empty())
		{
			LOG(LogDebug) << "Renderer::isVisibleOnScreen used without any clip stack!";
			return true;
		}

		return rectOverlap(nativeClipStack.top(), box);
	}

        // $$ ThemeAnimation.h:74:20: error: no member named 'mixColors'                
	unsigned int mixColors(unsigned int first, unsigned int second, float percent)
	{
		unsigned char alpha0 = (first >> 24) & 0xFF;
		unsigned char blue0 = (first >> 16) & 0xFF;
		unsigned char green0 = (first >> 8) & 0xFF;
		unsigned char red0 = first & 0xFF;

		unsigned char alpha1 = (second >> 24) & 0xFF;
		unsigned char blue1 = (second >> 16) & 0xFF;
		unsigned char green1 = (second >> 8) & 0xFF;
		unsigned char red1 = second & 0xFF;

		unsigned char alpha = (unsigned char)(alpha0 * (1.0 - percent) + alpha1 * percent);
		unsigned char blue = (unsigned char)(blue0 * (1.0 - percent) + blue1 * percent);
		unsigned char green = (unsigned char)(green0 * (1.0 - percent) + green1 * percent);
		unsigned char red = (unsigned char)(red0 * (1.0 - percent) + red1 * percent);

		return (alpha << 24) | (blue << 16) | (green << 8) | red;
	}

#define ROUNDING_PIECES 8.0f

	static void addRoundCorner(float x, float y, double sa, double arc, float r, unsigned int color, float pieces, std::vector<Vertex> &vertex)
	{
		// centre of the arc, for clockwise sense
		float cent_x = x + r * Math::cosf(sa + ES_PI / 2.0f);
		float cent_y = y + r * Math::sinf(sa + ES_PI / 2.0f);

		// build up piecemeal including end of the arc
		int n = ceil(pieces * arc / ES_PI * 2.0f);

		float step = arc / (float)n;

		Vertex vx;
		vx.tex = Vector2f::Zero();
		vx.col = color;

		for (int i = 0; i <= n; i++)
		{
			float ang = sa + step * (float)i;

			// compute the next point
			float next_x = cent_x + r * Math::sinf(ang);
			float next_y = cent_y - r * Math::cosf(ang);

			vx.pos[0] = next_x;
			vx.pos[1] = next_y;
			vertex.push_back(vx);
		}
	}

	std::vector<Vertex> createRoundRect(float x, float y, float width, float height, float radius, unsigned int color)
	{
		auto finalColor = convertColor(color);
		float pieces = Math::min(3.0f, Math::max(radius / 3.0f, ROUNDING_PIECES));

		std::vector<Vertex> vertex;
		addRoundCorner(x, y + radius, 3.0f * ES_PI / 2.0f, ES_PI / 2.0f, radius, finalColor, pieces, vertex);
		addRoundCorner(x + width - radius, y, 0.0, ES_PI / 2.0f, radius, finalColor, pieces, vertex);
		addRoundCorner(x + width, y + height - radius, ES_PI / 2.0f, ES_PI / 2.0f, radius, finalColor, pieces, vertex);
		addRoundCorner(x + radius, y + height, ES_PI, ES_PI / 2.0f, radius, finalColor, pieces, vertex);
		return vertex;
	}
	
	void drawRoundRect(float x, float y, float width, float height, float radius, unsigned int color, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		bindTexture(0);

		std::vector<Vertex> vertex = createRoundRect(x, y, width, height, radius, color);
		drawTriangleFan(vertex.data(), vertex.size(), _srcBlendFactor, _dstBlendFactor);
	}

	// $$ used
	void enableRoundCornerStencil(float x, float y, float width, float height, float radius)
	{
		std::vector<Vertex> vertex = createRoundRect(x, y, width, height, radius);
		setStencil(vertex.data(), vertex.size());
	}


	//////////////////////////////////////////////////////////////////////////

	std::vector<std::string> getRendererNames()
	{
		std::vector<std::string> ret;
	
#ifdef RENDERER_GLES_20
		{
			GLES20Renderer rd;				
			ret.push_back(rd.getDriverName());
		}
#endif

#ifdef RENDERER_OPENGL_21
		{
			OpenGL21Renderer rd;
			ret.push_back(rd.getDriverName());
		}
#endif

#ifdef RENDERER_OPENGLES_10
		{
			GLES10Renderer rd;
			ret.push_back(rd.getDriverName());
		}
#endif
		return ret;
	}

	IRenderer* getRendererFromName(const std::string& name)
	{
		if (name.empty())
			return nullptr;

#ifdef RENDERER_GLES_20
		{
			GLES20Renderer rd;
			if (rd.getDriverName() == name)
				return new GLES20Renderer();
		}
#endif

#ifdef RENDERER_OPENGL_21
		{
			OpenGL21Renderer rd;
			if (rd.getDriverName() == name)
				return new OpenGL21Renderer();
		}
#endif

#ifdef RENDERER_OPENGLES_10
		{
			GLES10Renderer rd;
			if (rd.getDriverName() == name)
				return new GLES10Renderer();
		}
#endif

		return nullptr;
	}

	static IRenderer* createRenderer()
	{
		IRenderer* instance = getRendererFromName(Settings::getInstance()->getString("Renderer"));
		if (instance == nullptr)
		{
#ifdef RENDERER_GLES_20
			instance = new GLES20Renderer();
#elif RENDERER_OPENGL_21
			instance = new OpenGL21Renderer();
#elif RENDERER_OPENGLES_10
			instance = new GLES10Renderer();
#endif
		}

		return instance;
	}

	static IRenderer* _instance = nullptr;

	static inline IRenderer* Instance()
	{
		if (_instance == nullptr)
			_instance = createRenderer();

		return _instance;
	}

	//////////////////////////////////////////////////////////////////////////
	std::string getDriverName()
	{
		return Instance()->getDriverName();
	}

	std::vector<std::pair<std::string, std::string>> getDriverInformation()
	{
		return Instance()->getDriverInformation();
	}

	unsigned int getWindowFlags()
	{
		return Instance()->getWindowFlags();
	}

	void setupWindow()
	{
		return Instance()->setupWindow();
	}

	void createContext() 
	{
		Instance()->createContext();
	}

	void resetCache()
	{
		Instance()->resetCache();
	}
	
	void destroyContext()
	{
		Instance()->destroyContext();
	}

	unsigned int createTexture(const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data)
	{
		return Instance()->createTexture(_type, _linear, _repeat, _width, _height, _data);
	}

	void  destroyTexture(const unsigned int _texture)
	{
		Instance()->destroyTexture(_texture);
	}

	void updateTexture(const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data)
	{
		Instance()->updateTexture(_texture, _type, _x, _y, _width, _height, _data);
	}

	void bindTexture(const unsigned int _texture)
	{
		Instance()->bindTexture(_texture);
	}

	void drawLines(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		Instance()->drawLines(_vertices, _numVertices, _srcBlendFactor, _dstBlendFactor);
	}

	// $$ used
	void drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor, bool verticesChanged)
	{
		Instance()->drawTriangleStrips(_vertices, _numVertices, _srcBlendFactor, _dstBlendFactor, verticesChanged);
	}

	// $$ used
	void drawSolidRectangle(const float _x, const float _y, const float _w, const float _h, const unsigned int _fillColor, const unsigned int _borderColor, float borderWidth, float cornerRadius)
	{
		Instance()->drawSolidRectangle(_x, _y, _w, _h, _fillColor, _borderColor, borderWidth, cornerRadius);
	}

	void drawTriangleFan(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		Instance()->drawTriangleFan(_vertices, _numVertices, _srcBlendFactor, _dstBlendFactor);
	}

	bool shaderSupportsCornerSize(const std::string& shader)
	{
		return Instance()->shaderSupportsCornerSize(shader);
	}

	bool supportShaders()
	{
		return Instance()->supportShaders();
	}

	void setProjection(const Transform4x4f& _projection)
	{
		Instance()->setProjection(_projection);
	}

	// $$ used
	void setMatrix(const Transform4x4f& _matrix)
	{
		Instance()->setMatrix(_matrix);
	}

	void blurBehind(const float _x, const float _y, const float _w, const float _h, const float blurSize)
	{
		std::map<std::string, std::string> map;
		map["blur"] = std::to_string(blurSize);
		Instance()->postProcessShader(":/shaders/blur.glsl", _x, _y, _w, _h, map);		
	}

	// $$ used
	void postProcessShader(const std::string& path, const float _x, const float _y, const float _w, const float _h, const std::map<std::string, std::string>& parameters, unsigned int* data)
	{
		Instance()->postProcessShader(path, _x, _y, _w, _h, parameters, data);
	}

	Rect& getViewport()
	{
		return viewPort;
	}

	void setViewport(const Rect& _viewport)
	{
		viewPort = _viewport;
		Instance()->setViewport(_viewport);
	}

	void setScissor(const Rect& _scissor)
	{
		Instance()->setScissor(_scissor);
	}

	// $$ used
	void setStencil(const Vertex* _vertices, const unsigned int _numVertices)
	{
		Instance()->setStencil(_vertices, _numVertices);
	}

	// $$ used
	void disableStencil()
	{
		Instance()->disableStencil();
	}

	void setSwapInterval()
	{
		Instance()->setSwapInterval();
	}

	void swapBuffers() 
	{
		currentFrame++;
		Instance()->swapBuffers();
	}

	size_t getTotalMemUsage()
	{
		return Instance()->getTotalMemUsage();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool  ScreenSettings::isSmallScreen()
	{
		auto menus = Settings::getInstance()->getString("ForceSmallScreen");
		if (!menus.empty())
			return menus == "true";

		return screenWidth <= 480 || screenHeight <= 480;
	}

	bool  ScreenSettings::fullScreenMenus()
	{
		auto menus = Settings::getInstance()->getString("FullScreenMenu");
		if (!menus.empty())
			return menus == "true";

		//return true;
		return isSmallScreen();
	}

	float ScreenSettings::menuFontScale()
	{
		auto scale = Settings::getInstance()->getString("MenuFontScale");
		if (!scale.empty())
		{
			auto val = Utils::String::toFloat(scale);
			if (val > 0 && val < 4)
				return val;
		}

		float sz = Math::min(Renderer::getScreenWidth(), Renderer::getScreenHeight());
		if (sz < 320)
			return 1.5f;  // GPI 320x240

		if (sz >= 320 && sz < 720) // ODROID 480x320;
			return 1.31f;

		return 1.0f;
	}

	float ScreenSettings::fontScale()
	{
		auto scale = Settings::getInstance()->getString("FontScale");
		if (!scale.empty())
		{
			auto val = Utils::String::toFloat(scale);
			if (val > 0 && val < 4)
				return val;
		}

		float sz = Math::min(Renderer::getScreenWidth(), Renderer::getScreenHeight());
		if (sz < 320) 
			return 1.5f;  // GPI 320x240

		if (sz >= 320 && sz < 720) // ODROID 480x320;
			return 1.31f;

		return 1.0f;
	}
