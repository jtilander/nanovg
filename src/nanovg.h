//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef NANOVG_H
#define NANOVG_H

#ifdef __cplusplus
extern "C" {
#endif

#define NVG_PI 3.14159265358979323846264338327f

struct NVGcontext;

struct NVGpaint
{
	float xform[6];
	float extent[2];
	float radius;
	float feather;
	unsigned int innerColor;
	unsigned int outerColor;
	int image;
	int repeat;
};

enum NVGwinding {
	NVG_CCW = 1,			// Winding for solid shapes
	NVG_CW = 2,				// Winding for holes
};

enum NVGsolidity {
	NVG_SOLID = 1,			// CCW
	NVG_HOLE = 2,			// CW
};

enum NVGlineCap {
	NVG_BUTT,
	NVG_ROUND,
	NVG_SQUARE,
	NVG_BEVEL,
	NVG_MITER,
};

enum NVGpatternRepeat {
	NVG_REPEATX = 0x01,		// Repeat image pattern in X direction
	NVG_REPEATY = 0x02,		// Repeat image pattern in Y direction
};

enum NVGaling {
	// Horizontal align
	NVG_ALIGN_LEFT 		= 1<<0,	// Default, align text horizontally to left.
	NVG_ALIGN_CENTER 	= 1<<1,	// Align text horizontally to center.
	NVG_ALIGN_RIGHT 	= 1<<2,	// Align text horizontally to right.
	// Vertical align
	NVG_ALIGN_TOP 		= 1<<3,	// Align text vertically to top.
	NVG_ALIGN_MIDDLE	= 1<<4,	// Align text vertically to middle.
	NVG_ALIGN_BOTTOM	= 1<<5,	// Align text vertically to bottom. 
	NVG_ALIGN_BASELINE	= 1<<6, // Default, align text vertically to baseline. 
};


// Begin drawing a new frame
// Calls to nanovg drawing API should be wrapped in nvgBeginFrame() & nvgEndFrame()
// nvgBeginFrame() defines the size of the window to render to in relation currently
// set viewport (i.e. glViewport on GL backends). Device pixel ration allows to
// control the rendering on Hi-DPI devices.
// For example, GLFW returns two dimension for an opened window: window size and
// frame buffer size. In that case you would set windowWidth/Height to the window size
// devicePixelRatio to: frameBufferWidth / windowWidth.
void nvgBeginFrame(struct NVGcontext* ctx, int windowWidth, int windowHeight, float devicePixelRatio);

// Ends drawing flushing remaining render state.
void nvgEndFrame(struct NVGcontext* ctx);

//
// Color utils
//
// Colors in NanoVG are stored as unsigned ints in ABGR format.

// Returns a color value from red, green, blue values. Alpha will be set to 255.
unsigned int nvgRGB(unsigned char r, unsigned char g, unsigned char b);

// Returns a color value from red, green, blue and alpha values.
unsigned int nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// Linearly interpoaltes from color c0 to c1, and returns resulting color value.
unsigned int nvgLerpRGBA(unsigned int c0, unsigned int c1, float u);

// Sets transparency of a color value.
unsigned int nvgTransRGBA(unsigned int c0, unsigned char a);

// Returns color value specified by hue, saturation and lightness.
// HSL values are all in range [0..1], alpha will be set to 255.
unsigned int nvgHSL(float h, float s, float l);

// Returns color value specified by hue, saturation and lightness and alpha.
// HSL values are all in range [0..1], alpha in range [0..255]
unsigned int nvgHSLA(float h, float s, float l, unsigned char a);

//
// State Handling
//
// NanoVG contains state which represents how paths will be rendered.
// The state contains transform, fill and stroke styles, text and font styles,
// and scissor clipping.

// Pushes and saves the current render state into a state stack.
// A matching nvgRestore() must be used to restore the state.
void nvgSave(struct NVGcontext* ctx);

// Pops and restores current render state.
void nvgRestore(struct NVGcontext* ctx);

// Resets current render state to default values. Does not affect the render state stack.
void nvgReset(struct NVGcontext* ctx);

//
// Render styles
//
// Fill and stroke render style can be either a solid color or a paint which is a gradient or a pattern.
// Solid color is simply defined as a color value, different kinds of paints can be created
// using nvgLinearGradient(), nvgBoxGradient(), nvgRadialGradient() and nvgImagePattern().
//
// Current render style can be saved and restored using nvgSave() and nvgRestore(). 

// Sets current stroke style to a solid color.
void nvgStrokeColor(struct NVGcontext* ctx, unsigned int color);

// Sets current stroke style to a paint, which can be a one of the gradients or a pattern.
void nvgStrokePaint(struct NVGcontext* ctx, struct NVGpaint paint);

// Sets current fill cstyle to a solid color.
void nvgFillColor(struct NVGcontext* ctx, unsigned int color);

// Sets current fill style to a paint, which can be a one of the gradients or a pattern.
void nvgFillPaint(struct NVGcontext* ctx, struct NVGpaint paint);

// Sets the miter limit of the stroke style.
// Miter limit controls when a sharp corner is beveled.
void nvgMiterLimit(struct NVGcontext* ctx, float limit);

// Sets the stroke witdth of the stroke style.
void nvgStrokeWidth(struct NVGcontext* ctx, float size);

// Sets how the end of the line (cap) is drawn,
// Can be one of: NVG_BUTT (default), NVG_ROUND, NVG_SQUARE.
void nvgLineCap(struct NVGcontext* ctx, int cap);

// Sets how sharp path corners are drawn.
// Can be one of NVG_MITER (default), NVG_ROUND, NVG_BEVEL.
void nvgLineJoin(struct NVGcontext* ctx, int join);

//
// Transforms
//
// The paths, gradients, patterns and scissor region are transformed by an transformation
// matrix at the time when they are passed to the API.
// The current transformation matrix is a affine matrix:
//   [sx kx tx]
//   [ky sy ty]
//   [ 0  0  1]
// Where: sx,sy define scaling, kx,ky skewing, and tx,ty translation.
// The last row is assumed to be 0,0,1 and is not stored.
//
// Apart from nvgResetTransform(), each transformation function first creates
// specific transformation matrix and pre-multiplies the current transformation by it.
//
// Current coordinate system (transformation) can be saved and restored using nvgSave() and nvgRestore(). 

// Resets current transform to a identity matrix.
void nvgResetTransform(struct NVGcontext* ctx);

// Premultiplies current coordinate system by specified matrix.
// The parameters are interpreted as matrix as follows:
//   [a c e]
//   [b d f]
//   [0 0 1]
void nvgTransform(struct NVGcontext* ctx, float a, float b, float c, float d, float e, float f);

// Translates current coordinate system.
void nvgTranslate(struct NVGcontext* ctx, float x, float y);

// Rotates current coordinate system.
void nvgRotate(struct NVGcontext* ctx, float angle);

// Scales the current coordinat system.
void nvgScale(struct NVGcontext* ctx, float x, float y);

//
// Images
//
// NanoVG allows you to load jpg, png, psd, tga, pic and gif files to be used for rendering.
// In addition you can upload your own image. The image loading is provided by stb_image.

// Creates image by loading it from the disk from specified file name.
// Returns handle to the image.
int nvgCreateImage(struct NVGcontext* ctx, const char* filename);

// Creates image by loading it from the specified memory chunk.
// Returns handle to the image.
int nvgCreateImageMem(struct NVGcontext* ctx, unsigned char* data, int ndata, int freeData);

// Creates image from specified image data.
// Returns handle to the image.
int nvgCreateImageRGBA(struct NVGcontext* ctx, int w, int h, const unsigned char* data);

// Updates image data specified by image handle.
void nvgUpdateImage(struct NVGcontext* ctx, int image, const unsigned char* data);

// Returns the domensions of a created image.
void nvgImageSize(struct NVGcontext* ctx, int image, int* w, int* h);

// Deletes created image.
void nvgDeleteImage(struct NVGcontext* ctx, int image);

//
// Paints
//
// NanoVG supports four types of paints: linear gradient, box gradient, radial gradient and image pattern.
// These can be used as paints for strokes and fills.

// Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the start and end coordinates
// of the linear gradient, icol specifies the start color and ocol the end color.
// The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
struct NVGpaint nvgLinearGradient(struct NVGcontext* ctx, float sx, float sy, float ex, float ey,
								  unsigned int icol, unsigned int ocol);

// Creates and returns a box gradient. Box gradient is a feathered rounded rectangle, it is useful for rendering
// drop shadows or hilights for boxes. Parameters (x,y) define the top-left corner of the rectangle,
// (w,h) define the size of the rectangle, r defines the corner radius, and f feather. Feather defines how blurry
// the border of the rectangle is. Parameter icol specifies the inner color and ocol the outer color of the gradient.
// The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
struct NVGpaint nvgBoxGradient(struct NVGcontext* ctx, float x, float y, float w, float h,
							   float r, float f, unsigned int icol, unsigned int ocol);

// Creates and returns a radial gradient. Parameters (cx,cy) specify the center, inr and outr specify
// the inner and outer radius of the gradient, icol specifies the start color and ocol the end color.
// The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
struct NVGpaint nvgRadialGradient(struct NVGcontext* ctx, float cx, float cy, float inr, float outr,
								  unsigned int icol, unsigned int ocol);

// Creates and returns an image patter. Parameters (ox,oy) specify the left-top location of the image pattern,
// (ex,ey) the size of one image, angle rotation around the top-left corner, image is handle to the image to render,
// and repeat is combination of NVG_REPEATX and NVG_REPEATY which tells if the image should be repeated across x or y.
// The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
struct NVGpaint nvgImagePattern(struct NVGcontext* ctx, float ox, float oy, float ex, float ey,
								float angle, int image, int repeat);

//
// Scissoring
//
// Scissoring allows you to clip the rendering into a rectangle. This is useful for varius
// user interface cases like rendering a text edit or a timeline. 

// Sets the current 
// The scissor rectangle is transformed by the current transform.
void nvgScissor(struct NVGcontext* ctx, float x, float y, float w, float h);

// Reset and disables scissoring.
void nvgResetScissor(struct NVGcontext* ctx);

//
// Paths
//
// Drawing a new shape starts with nvgBeginPath(), it clears all the currently defined paths.
// Then you define one or more paths and sub-paths which describe the shape. The are functions
// to draw common shapes like rectangles and circles, and lower level step-by-step functions,
// which allow to define a path curve by curve.
//
// NanoVG uses even-odd fill rule to draw the shapes. Solid shapes should have counter clockwise
// winding and holes should have counter clockwise order. To specify winding of a path you can
// call nvgPathWinding(). This is useful especially for the common shapes, which are drawn CCW.
//
// Finally you can fill the path using current fill style by calling nvgFill(), and stroke it
// with current stroke style by calling nvgStroke().
//
// The curve segments and sub-paths are transformed by the current transform.

// Clears the current path and sub-paths.
void nvgBeginPath(struct NVGcontext* ctx);

// Starts new sub-path with specified point as first point.
void nvgMoveTo(struct NVGcontext* ctx, float x, float y);

// Adds line segment from the last point in the path to the specified point.
void nvgLineTo(struct NVGcontext* ctx, float x, float y);

// Adds bezier segment from last point in the path via two control points to the specified point.
void nvgBezierTo(struct NVGcontext* ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);

// Adds an arc segment at the corner defined by the last path point, and two specified points.
void nvgArcTo(struct NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius);

// Closes current sub-path with a line segment.
void nvgClosePath(struct NVGcontext* ctx);

// Sets the current sub-path winding, see NVGwinding and NVGsolidity. 
void nvgPathWinding(struct NVGcontext* ctx, int dir);

// Creates new arc shaped sub-path.
void nvgArc(struct NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);

// Creates new rectangle shaped sub-path.
void nvgRect(struct NVGcontext* ctx, float x, float y, float w, float h);

// Creates new rounded rectangle shaped sub-path.
void nvgRoundedRect(struct NVGcontext* ctx, float x, float y, float w, float h, float r);

// Creates new ellipse shaped sub-path.
void nvgEllipse(struct NVGcontext* ctx, float cx, float cy, float rx, float ry);

// Creates new circle shaped sub-path. 
void nvgCircle(struct NVGcontext* ctx, float cx, float cy, float r);

// Fills the current path with current fill style.
void nvgFill(struct NVGcontext* ctx);

// Fills the current path with current stroke style.
void nvgStroke(struct NVGcontext* ctx);

//
// Text
//
// NanoVG allows you to load .ttf files and use the font to render text.
//
// The appearance of the text can be defined by setting the current text style
// and by specifying the fill color. Common text and font settings such as
// font size, letter spacing and text align are supported. Font blur allows you
// to create simple text effects such as drop shadows.
//
// At render time the tont face can be set based on the font handles or name.
//
// Note: currently only solid color fill is supported for text.

// Creates font by loading it from the disk from specified file name.
// Returns handle to the font.
int nvgCreateFont(struct NVGcontext* ctx, const char* name, const char* filename);

// Creates image by loading it from the specified memory chunk.
// Returns handle to the font.
int nvgCreateFontMem(struct NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData);

// Finds a loaded font of specified name, and returns handle to it, or -1 if the font is not found.
int nvgFindFont(struct NVGcontext* ctx, const char* name);

// Sets the font size of current text style.
void nvgFontSize(struct NVGcontext* ctx, float size);

// Sets the letter spacing of current text style.
void nvgLetterSpacing(struct NVGcontext* ctx, float spacing);

// Sets the blur of current text style.
void nvgFontBlur(struct NVGcontext* ctx, float blur);

// Sets the text align of current text style, see NVGaling for options.
void nvgTextAlign(struct NVGcontext* ctx, int align);

// Sets the font face based on specified id of current text style.
void nvgFontFaceId(struct NVGcontext* ctx, int font);

// Sets the font face based on specified name of current text style.
void nvgFontFace(struct NVGcontext* ctx, const char* font);

// Draws text string at specified location. If end is specified only the sub-string up to the end is drawn.
float nvgText(struct NVGcontext* ctx, float x, float y, const char* string, const char* end);

// Measures the specified text string. Parameter bounds should be a pointer to float[4] if 
// the bounding box of the text should be returned. Returns the width of the measured text.
// Current transform does not affect the measured values.
float nvgTextBounds(struct NVGcontext* ctx, const char* string, const char* end, float* bounds);

// Returns the vertical metrics based on the current text style.
// Current transform does not affect the measured values.
void nvgVertMetrics(struct NVGcontext* ctx, float* ascender, float* descender, float* lineh);


//
// Internal Render API
//
enum NVGtexture {
	NVG_TEXTURE_ALPHA = 0x01,
	NVG_TEXTURE_RGBA = 0x02,
};

struct NVGscissor
{
	float xform[6];
	float extent[2];
};

struct NVGvertex {
	float x,y,u,v;
};

struct NVGpath {
	int first;
	int count;
	unsigned char closed;
	int nbevel;
	struct NVGvertex* fill;
	int nfill;
	struct NVGvertex* stroke;
	int nstroke;
	int winding;
	int convex;
};

struct NVGparams {
	void* userPtr;
	int atlasWidth, atlasHeight;
	int edgeAntiAlias;
	int (*renderCreate)(void* uptr);
	int (*renderCreateTexture)(void* uptr, int type, int w, int h, const unsigned char* data);
	int (*renderDeleteTexture)(void* uptr, int image);
	int (*renderUpdateTexture)(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data);
	int (*renderGetTextureSize)(void* uptr, int image, int* w, int* h);
	void (*renderViewport)(void* uptr, int width, int height);
	void (*renderFlush)(void* uptr);
	void (*renderFill)(void* uptr, struct NVGpaint* paint, struct NVGscissor* scissor, const float* bounds, const struct NVGpath* paths, int npaths);
	void (*renderStroke)(void* uptr, struct NVGpaint* paint, struct NVGscissor* scissor, float strokeWidth, const struct NVGpath* paths, int npaths);
	void (*renderTriangles)(void* uptr, struct NVGpaint* paint, struct NVGscissor* scissor, const struct NVGvertex* verts, int nverts);
	void (*renderDelete)(void* uptr);
};

// Contructor and destructor, called by the render back-end.
struct NVGcontext* nvgCreateInternal(struct NVGparams* params);
void nvgDeleteInternal(struct NVGcontext* ctx);

// Compiler references
// http://sourceforge.net/p/predef/wiki/Compilers/
#if _MSC_VER >= 1800
	// VS 2013 seems to be too smart for school, it will still list the variable as unused if passed into sizeof().
	#define NVG_NOTUSED(v) do { (void)(v); } while(0)
#else
	#define NVG_NOTUSED(v)  (void)sizeof(v)
#endif

#ifdef __cplusplus
}
#endif

#endif // NANOVG_H
