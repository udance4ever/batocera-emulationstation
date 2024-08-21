//  SPDX-License-Identifier: MIT
//
//  ES-DE Frontend
//  Renderer.h
//
//  Generic rendering functions.
//

#ifndef ES_CORE_RENDERER_RENDERER_H
#define ES_CORE_RENDERER_RENDERER_H

#include "Log.h"
#include "utils/MathUtil.h"

#include <stack>
#include <string>
#include <vector>
// $$ error: no template named 'map' in namespace 'std'
#include <map>
// $$ batocera
#include <cstring>
#include "math/Vector2f.h"
#include "math/Vector3f.h"

// $$ Font.h:57:49: error: unknown type name 'Transform4x4f'
class  Transform4x4f;
class  Vector2i;
struct SDL_Window;

class Renderer
{
public:
    enum class TextureType {
        RGBA,
        BGRA,
        RED
    };

	// $$ batocera version (different class name)
	namespace Texture
	{
		enum Type
		{
			RGBA  = 0,
			ALPHA = 1

		}; // Type

	} // Texture::

    enum class BlendFactor {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA
    };

	// $$ batocera
//	namespace Blend
//	{
//		enum Factor
//		{
//			ZERO                = 0,
//			ONE                 = 1,
//			SRC_COLOR           = 2,
//			ONE_MINUS_SRC_COLOR = 3,
//			SRC_ALPHA           = 4,
//			ONE_MINUS_SRC_ALPHA = 5,
//			DST_COLOR           = 6,
//			ONE_MINUS_DST_COLOR = 7,
//			DST_ALPHA           = 8,
//			ONE_MINUS_DST_ALPHA = 9
//
//		}; // Factor
//
//	} // Blend::

    // clang-format off
    enum Shader {
        CORE            = 0x00000001,
        BLUR_HORIZONTAL = 0x00000002,
        BLUR_VERTICAL   = 0x00000004,
        SCANLINES       = 0x00000008
    };

    enum ShaderFlags {
        PREMULTIPLIED         = 0x00000001,
        FONT_TEXTURE          = 0x00000002,
        POST_PROCESSING       = 0x00000004,
        CLIPPING              = 0x00000008,
        ROTATED               = 0x00000010, // Screen rotated 90 or 270 degrees.
        ROUNDED_CORNERS       = 0x00000020,
        ROUNDED_CORNERS_NO_AA = 0x00000040,
        CONVERT_PIXEL_FORMAT  = 0x00000080
    };
    // clang-format on

    struct Vertex {
        glm::vec2 position;
        glm::vec2 texcoord;
        unsigned int color;
        glm::vec4 clipRegion;
        float brightness;
        float opacity;
        float saturation;
        float dimming;
        float cornerRadius;
        float reflectionsFalloff;
        float blurStrength;
        unsigned int shaders;
        unsigned int shaderFlags;

        Vertex()
            : position {0.0f, 0.0f}
            , texcoord {0.0f, 0.0f}
            , color {0x00000000}
            , clipRegion {0.0f, 0.0f, 0.0f, 0.0f}
            , brightness {0.0f}
            , opacity {1.0f}
            , saturation {1.0f}
            , dimming {1.0f}
            , cornerRadius {0.0f}
            , reflectionsFalloff {0.0f}
            , blurStrength {0.0f}
            , shaders {0}
            , shaderFlags {0}
        {
        }

        Vertex(const glm::vec2& positionArg,
               const glm::vec2& texcoordArg,
               const unsigned int colorArg)
            : position {positionArg}
            , texcoord {texcoordArg}
            , color {colorArg}
            , clipRegion {0.0f, 0.0f, 0.0f, 0.0f}
            , brightness {0.0f}
            , opacity {1.0f}
            , saturation {1.0f}
            , dimming {1.0f}
            , cornerRadius {0.0f}
            , reflectionsFalloff {0.0f}
            , blurStrength {0.0f}
            , shaders {0}
            , shaderFlags {0}
        {
        }
    };

// $$ batocera version
//	struct Vertex
//	{
//		Vertex() 
//			: col(0)			
//			, saturation(1.0f)
//			, cornerRadius(0.0f)
//			, customShader(nullptr)
//		{
//
//		}
//
//		Vertex(const Vector2f& _pos, const Vector2f& _tex, const unsigned int _col) 
//			: pos(_pos)
//			, tex(_tex)
//			, col(_col) 
//			, saturation(1.0f)
//			, cornerRadius(0.0f)
//			, customShader(nullptr)
//		{ 
//
//		}
//
//		Vector2f     pos;
//		Vector2f     tex;
//		unsigned int col;
//
//		float saturation;
//		float cornerRadius;
//		ShaderInfo* customShader;
//
//	}; // Vertex


    struct postProcessingParams {
        float opacity;
        float saturation;
        float dimming;
        float blurStrength;
        unsigned int blurPasses;
        unsigned int shaders;

        postProcessingParams()
            : opacity {1.0f}
            , saturation {1.0f}
            , dimming {1.0f}
            , blurStrength {0.0f}
            , blurPasses {1}
            , shaders {0}
        {
        }
    };

    struct Rect {
        int x;
        int y;
        int w;
        int h;

        Rect()
            : x(0)
            , y(0)
            , w(0)
            , h(0)
        {
        }

        Rect(const int xValue, const int yValue, const int wValue, const int hValue)
            : x(xValue)
            , y(yValue)
            , w(wValue)
            , h(hValue)
        {
        }

	// $$ batocera
	inline bool contains(int px, int py) { return px >= x && px <= x + w && py >= y && py <= y + h; };
    };

    // $$ ThemeData.h:140:12: error: no type named 'ShaderInfo' in 'Renderer'
    struct ShaderInfo
    {
            std::string path;
            std::map<std::string, std::string> parameters;
    };

// $$ batocera

	class IRenderer
	{
	public:
		virtual std::string getDriverName() = 0;

		virtual std::vector<std::pair<std::string, std::string>> getDriverInformation() = 0;

		virtual unsigned int getWindowFlags() = 0;
		virtual void         setupWindow() = 0;

		virtual void         createContext() = 0;
		virtual void         destroyContext() = 0;

		virtual void         resetCache() = 0;

		virtual unsigned int createTexture(const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data) = 0;
		virtual void         destroyTexture(const unsigned int _texture) = 0;
		virtual void         updateTexture(const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data) = 0;
		virtual void         bindTexture(const unsigned int _texture) = 0;

		virtual void         drawLines(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA) = 0;
		virtual void         drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA, bool verticesChanged = true) = 0;
		virtual void		 drawTriangleFan(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA) = 0;

		virtual void		 drawSolidRectangle(const float _x, const float _y, const float _w, const float _h, const unsigned int _fillColor, const unsigned int _borderColor, float borderWidth = 1, float cornerRadius = 0) = 0;

		virtual void         setProjection(const Transform4x4f& _projection) = 0;
		virtual void         setMatrix(const Transform4x4f& _matrix) = 0;
		virtual void         setViewport(const Rect& _viewport) = 0;
		virtual void         setScissor(const Rect& _scissor) = 0;

		virtual void         setStencil(const Vertex* _vertices, const unsigned int _numVertices) = 0;
		virtual void		 disableStencil() = 0;

		virtual void         setSwapInterval() = 0;
		virtual void         swapBuffers() = 0;
		
		virtual void		 postProcessShader(const std::string& path, const float _x, const float _y, const float _w, const float _h, const std::map<std::string, std::string>& parameters, unsigned int* data = nullptr) { };

		virtual size_t		 getTotalMemUsage() { return (size_t) -1; };

		virtual bool		 supportShaders() { return false; }
		virtual bool		 shaderSupportsCornerSize(const std::string& shader) { return false; };
	};

	// $$ used
	class ScreenSettings
	{
	public:
		static bool  isSmallScreen();
		static bool  fullScreenMenus();
//		static float fontScale();
//		static float menuFontScale();
	};

	std::vector<std::string> getRendererNames();

	void        pushClipRect    (const Vector2i& _pos, const Vector2i& _size);
	void		pushClipRect	(int x, int y, int w, int h);
	void		pushClipRect	(Rect rect);
// batocera
// $$$ error: too few arguments to function call, expected at least 6, have 5
//        Renderer::getInstance()->drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), (mBackgroundColor & 0xFFFFFF00) | alpha);
	void        drawRect        (const float _x, const float _y, const float _w, const float _h, const unsigned int _color, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);
	void        drawRect        (const float _x, const float _y, const float _w, const float _h, const unsigned int _color, const unsigned int _colorEnd, bool horizontalGradient = false, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);

	int         getWindowWidth  ();
	int         getWindowHeight ();
	int         getScreenOffsetX();
	int         getScreenOffsetY();
	int         getScreenRotate ();
	float		getScreenProportion();
	std::string getAspectRatio();		// $$ used
	bool		isVerticalScreen();	// $$ used

	// $$ used
	Vector2i    physicalScreenToRotatedScreen(int x, int y);

	// API specific
	inline static unsigned int convertColor (const unsigned int _color) { return ((_color & 0xFF000000) >> 24) | ((_color & 0x00FF0000) >> 8) | ((_color & 0x0000FF00) << 8) | ((_color & 0x000000FF) << 24); } 
	// convertColor

	unsigned int getWindowFlags    ();
	void         setupWindow       ();
//	void         createContext     ();
//	void         destroyContext    ();
	void         resetCache        ();
//	unsigned int createTexture     (const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data);
//	void         destroyTexture    (const unsigned int _texture);
//	void         updateTexture     (const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data);
//	void         bindTexture       (const unsigned int _texture);
	void         drawLines         (const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);
	// $$ batocera version used
	void         drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA, bool verticesChanged = true);
	// $$ used
	void		 drawSolidRectangle(const float _x, const float _y, const float _w, const float _h, const unsigned int _fillColor, const unsigned int _borderColor, float borderWidth = 1, float cornerRadius = 0);
	void         setProjection     (const Transform4x4f& _projection);
	// $$ batocera Transform4x4f used
	void         setMatrix         (const Transform4x4f& _matrix);
//	void         setViewport       (const Rect& _viewport);
	Rect&        getViewport	   ();
//	void         setScissor        (const Rect& _scissor);
//	void         setSwapInterval   ();
//	void         swapBuffers       ();

	void		 blurBehind		   (const float _x, const float _y, const float _w, const float _h, const float blurSize = 4.0f);
	// $$ used
	void		 postProcessShader (const std::string& path, const float _x, const float _y, const float _w, const float _h, const std::map<std::string, std::string>& parameters, unsigned int* data = nullptr);

	size_t		 getTotalMemUsage  ();

	bool		 supportShaders();
	bool		 shaderSupportsCornerSize(const std::string& shader);

	std::string  getDriverName();
	std::vector<std::pair<std::string, std::string>> getDriverInformation();

	bool         isClippingEnabled  ();

	// $$ used
	bool         isVisibleOnScreen  (float x, float y, float w, float h);
	inline bool  isVisibleOnScreen  (const Rect& rect) { return isVisibleOnScreen(rect.x, rect.y, rect.w, rect.h); }
	bool         isSmallScreen      ();
	// ThemeAnimation.h:74:20: error: no member named 'mixColors'
	unsigned int mixColors(unsigned int first, unsigned int second, float percent);

	void		drawRoundRect(float x, float y, float w, float h, float radius, unsigned int color, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);

	// $$ used
	void		enableRoundCornerStencil(float x, float y, float size_x, float size_y, float radius);
	
	void		drawTriangleFan(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);

	// $$ used
	void		setStencil(const Vertex* _vertices, const unsigned int _numVertices);
	void		disableStencil();

	std::vector<Vertex> createRoundRect(float x, float y, float width, float height, float radius, unsigned int color = 0xFFFFFFFF);

	Rect		getScreenRect(const Transform4x4f& transform, const Vector3f& size, bool viewPort = false);
	// GuiComponent.cpp:108:24: error: no member named 'getScreenRect'
	Rect		getScreenRect(const Transform4x4f& transform, const Vector2f& size, bool viewPort = false);

	// $$ used	
	void		activateWindow();

	// $$ used
	Vector2i	setScreenMargin(int marginX, int marginY);

	int         getCurrentFrame();


// $$ ES-DE
    static Renderer* getInstance();

    void setIcon();
    bool createWindow();
    void destroyWindow();
    bool init();
    void deinit();

    virtual bool loadShaders() = 0;

    void pushClipRect(const glm::ivec2& pos, const glm::ivec2& size);
    void popClipRect();

    void drawRect(const float x,
                  const float y,
                  const float w,
                  const float h,
                  const unsigned int color,
                  const unsigned int colorEnd,
                  const bool horizontalGradient = false,
                  const float opacity = 1.0f,
                  const float dimming = 1.0f,
                  const BlendFactor srcBlendFactor = BlendFactor::SRC_ALPHA,
                  const BlendFactor dstBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA,
                  const float cornerRadius = 0.0f);

    const glm::mat4& getProjectionMatrix() { return mProjectionMatrix; }
    const glm::mat4& getProjectionMatrixNormal() { return mProjectionMatrixNormal; }
    SDL_Window* getSDLWindow() { return mSDLWindow; }
    const int getScreenRotation() { return mScreenRotation; }
    static const bool getIsVerticalOrientation() { return sIsVerticalOrientation; }
    static const float getScreenWidth() { return static_cast<float>(sScreenWidth); }
    static const float getScreenHeight() { return static_cast<float>(sScreenHeight); }
    static const float getScreenWidthModifier() { return sScreenWidthModifier; }
    static const float getScreenHeightModifier() { return sScreenHeightModifier; }
    static const float getScreenAspectRatio() { return sScreenAspectRatio; }
    static const float getScreenResolutionModifier() { return sScreenResolutionModifier; }

    static constexpr glm::mat4 getIdentity() { return glm::mat4 {1.0f}; }
    glm::mat4 mTrans {getIdentity()};

    virtual void setup() = 0;
    virtual bool createContext() = 0;
    virtual void destroyContext() = 0;
    virtual unsigned int createTexture(const unsigned int texUnit,
                                       const TextureType type,
                                       const bool linearMinify,
                                       const bool linearMagnify,
                                       const bool mipmapping,
                                       const bool repeat,
                                       const unsigned int width,
                                       const unsigned int height,
                                       void* data) = 0;
    virtual void destroyTexture(const unsigned int texture) = 0;
    virtual void updateTexture(const unsigned int texture,
                               const unsigned int texUnit,
                               const TextureType type,
                               const unsigned int x,
                               const unsigned int y,
                               const unsigned int width,
                               const unsigned int height,
                               void* data) = 0;
    virtual void bindTexture(const unsigned int texture, const unsigned int texUnit) = 0;
    virtual void drawTriangleStrips(
        const Vertex* vertices,
        const unsigned int numVertices,
        const BlendFactor srcBlendFactor = BlendFactor::ONE,
        const BlendFactor dstBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA) = 0;
    virtual void shaderPostprocessing(
        const unsigned int shaders,
        const Renderer::postProcessingParams& parameters = postProcessingParams(),
        unsigned char* textureRGBA = nullptr) = 0;
    virtual void setMatrix(const glm::mat4& matrix) = 0;
    virtual void setViewport(const Rect& viewport) = 0;
    virtual void setScissor(const Rect& scissor) = 0;
    virtual void setSwapInterval() = 0;
    virtual void swapBuffers() = 0;

protected:
    Rect mViewport;
    int mWindowWidth {0};
    int mWindowHeight {0};
    int mPaddingWidth {0};
    int mPaddingHeight {0};
    int mScreenOffsetX {0};
    int mScreenOffsetY {0};

private:
    std::stack<Rect> mClipStack;
    SDL_Window* mSDLWindow {nullptr};
    glm::mat4 mProjectionMatrix {};
    glm::mat4 mProjectionMatrixNormal {};

    static inline int sScreenWidth {0};
    static inline int sScreenHeight {0};
    int mScreenRotation {0};
    bool mInitialCursorState {true};
    static inline bool sIsVerticalOrientation {false};
    // Screen resolution modifiers relative to the 1920x1080 reference.
    static inline float sScreenHeightModifier {0.0f};
    static inline float sScreenWidthModifier {0.0f};
    static inline float sScreenAspectRatio {0.0f};
    static inline float sScreenResolutionModifier {0.0f};
};

#endif // ES_CORE_RENDERER_RENDERER_H
