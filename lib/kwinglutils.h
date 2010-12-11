/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006-2007 Rivo Laks <rivolaks@hot.ee>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef KWIN_GLUTILS_H
#define KWIN_GLUTILS_H

#include <kwinconfig.h> // KWIN_HAVE_OPENGL

#ifdef KWIN_HAVE_OPENGL
#include <kwinglutils_funcs.h>

#include <QtGui/QPixmap>

#include <QtGui/QImage>
#include <QtCore/QSize>
#include <QtCore/QSharedData>
#include <QtCore/QStack>

/** @addtogroup kwineffects */
/** @{ */

class QVector2D;
class QVector3D;
class QVector4D;
class QMatrix4x4;

template< class K, class V > class QHash;


namespace KWin
{


class GLTexture;
class GLVertexBuffer;
class GLVertexBufferPrivate;


// Initializes GLX function pointers
void KWIN_EXPORT initGLX();
// Initializes OpenGL stuff. This includes resolving function pointers as
//  well as checking for GL version and extensions
//  Note that GL context has to be created by the time this function is called
void KWIN_EXPORT initGL();
// Initializes EGL function pointers
void KWIN_EXPORT initEGL();


// Number of supported texture units
extern KWIN_EXPORT int glTextureUnitsCount;


bool KWIN_EXPORT hasGLVersion(int major, int minor, int release = 0);
bool KWIN_EXPORT hasGLXVersion(int major, int minor, int release = 0);
bool KWIN_EXPORT hasEGLVersion(int major, int minor, int release = 0);
// use for both OpenGL and GLX extensions
bool KWIN_EXPORT hasGLExtension(const QString& extension);

// detect OpenGL error (add to various places in code to pinpoint the place)
bool KWIN_EXPORT checkGLError( const char* txt );

inline bool KWIN_EXPORT isPowerOfTwo( int x ) { return (( x & ( x - 1 )) == 0 ); }
/**
 * @return power of two integer _greater or equal to_ x.
 *  E.g. nearestPowerOfTwo(513) = nearestPowerOfTwo(800) = 1024
 **/
int KWIN_EXPORT nearestPowerOfTwo( int x );

/**
 * Renders quads using given vertices.
 * If texture is not 0, each texture coordinate much have two components (st).
 * If color is not 0, each color much have four components (rgba).
 * Note that texture coordinates must match texture type (normalized/unnormalized
 * for GL_TEXTURE_2D/GL_TEXTURE_ARB).
 *
 * In OpenGL ES this method is a no-op.
 *
 * @param count number of vertices to use.
 * @param dim number of components per vertex coordinate in vertices array.
 * @param stride byte offset of consecutive elements in arrays. If 0, then
 *  arrays must be tighly packed. Stride must be a multiple of sizeof(float)!
 * @deprecated  Use GLVertexBuffer
 * @see GLVertexBuffer
 **/
KWIN_EXPORT void renderGLGeometry( const QRegion& region, int count,
    const float* vertices, const float* texture = 0, const float* color = 0,
    int dim = 2, int stride = 0 );
/**
 * Same as above, renders without specified region
 * @deprecated  Use GLVertexBuffer
 * @see GLVertexBuffer
 **/
KWIN_EXPORT void renderGLGeometry( int count,
    const float* vertices, const float* texture = 0, const float* color = 0,
    int dim = 2, int stride = 0 );

/**
 * In OpenGL ES this method is a no-op.
 * 
 * @deprecated Use GLVertexBuffer
 * @see GLVertexBuffer
 **/
KWIN_EXPORT void renderGLGeometryImmediate( int count,
    const float* vertices, const float* texture = 0, const float* color = 0,
    int dim = 2, int stride = 0 );

/**
 * @deprecated Quads are not available in OpenGL ES
 **/
KWIN_EXPORT void addQuadVertices( QVector<float>& verts, float x1, float y1, float x2, float y2 );


class KWIN_EXPORT GLTexture
    : public QSharedData
    {
    public:
        GLTexture();
        explicit GLTexture( const QImage& image, GLenum target = GL_TEXTURE_2D );
        explicit GLTexture( const QPixmap& pixmap, GLenum target = GL_TEXTURE_2D );
        GLTexture( const QString& fileName );
        GLTexture( int width, int height );
        virtual ~GLTexture();

        bool isNull() const;
        QSize size() const;
        int width() const { return mSize.width(); }   /// @since 4.5
        int height() const { return mSize.height(); } /// @since 4.5

        virtual bool load( const QImage& image, GLenum target = GL_TEXTURE_2D );
        virtual bool load( const QPixmap& pixmap, GLenum target = GL_TEXTURE_2D );
        virtual bool load( const QString& fileName );
        virtual void discard();
        virtual void bind();
        virtual void unbind();
        void render( QRegion region, const QRect& rect );
        /**
         * Same as above, but allows to specify if the geometry of the texture
         * should be passed for a core profile shader. The shader needs to be
         * bound before. The default is to perform legacy rendering.
         * @param useShader If @c true core profile compatible rendering is used.
         * If a bound shader is not core profile compatible @c false should be used.
         * @see render
         * @see GLVertexBuffer::setUseShader
         * @since 4.7
         */
        void render( QRegion region, const QRect& rect, bool useShader );
        /**
         * Set up texture transformation matrix to automatically map unnormalized
         * texture coordinates (i.e. 0 to width, 0 to height, (0,0) is top-left)
         * to OpenGL coordinates. Automatically adjusts for different texture
         * types. This can be done only for one texture at a time, repeated
         * calls for the same texture are allowed though, requiring the same
         * amount of calls to disableUnnormalizedTexCoords().
         *
         * In OpenGL ES this method is a no-op. In core profile a shader should be used
         * @deprecated
         */
        void enableUnnormalizedTexCoords();
        /**
         * Disables transformation set up using enableUnnormalizedTexCoords().
         *
         * In OpenGL ES this method is a no-op. In core profile a shader should be used
         * @deprecated
         */
        void disableUnnormalizedTexCoords();
        /**
         * Set up texture transformation matrix to automatically map normalized
         * texture coordinates (i.e. 0 to 1, 0 to 1, (0,0) is bottom-left)
         * to OpenGL coordinates. Automatically adjusts for different texture
         * types. This can be done only for one texture at a time, repeated
         * calls for the same texture are allowed though, requiring the same
         * amount of calls to disableNormalizedTexCoords().
         *
         * In OpenGL ES this method is a no-op. In core profile a shader should be used
         * @deprecated
         */
        void enableNormalizedTexCoords();
        /**
         * Disables transformation set up using enableNormalizedTexCoords().
         *
         * In OpenGL ES this method is a no-op. In core profile a shader should be used
         * @deprecated
         */
        void disableNormalizedTexCoords();

        GLuint texture() const;
        GLenum target() const;
        GLenum filter() const;
        virtual bool isDirty() const;
        void setTexture( GLuint texture );
        void setTarget( GLenum target );
        void setFilter( GLenum filter );
        void setWrapMode( GLenum mode );
        virtual void setDirty();

        static void initStatic();
        static bool NPOTTextureSupported()  { return mNPOTTextureSupported; }
        static bool framebufferObjectSupported()  { return mFramebufferObjectSupported; }
        static bool saturationSupported()  { return mSaturationSupported; }

    protected:
        void enableFilter();
        QImage convertToGLFormat( const QImage& img ) const;

        GLuint mTexture;
        GLenum mTarget;
        GLenum mFilter;
        QSize mSize;
        QSizeF mScale; // to un-normalize GL_TEXTURE_2D
        bool y_inverted; // texture has y inverted
        bool can_use_mipmaps;
        bool has_valid_mipmaps;

    private:
        void init();
        int mUnnormalizeActive; // 0 - no, otherwise refcount
        int mNormalizeActive; // 0 - no, otherwise refcount
        GLVertexBuffer* m_vbo;
        QSize m_cachedSize;

        static bool mNPOTTextureSupported;
        static bool mFramebufferObjectSupported;
        static bool mSaturationSupported;
        Q_DISABLE_COPY( GLTexture )
    };

class KWIN_EXPORT GLShader
    {
    public:
        GLShader(const QString& vertexfile, const QString& fragmentfile);
        ~GLShader();

        bool isValid() const  { return mValid; }
        void bind();
        void unbind();

        int uniformLocation(const char* name);
        bool setUniform(const char* name, float value);
        bool setUniform(const char* name, int value);
        bool setUniform(const char* name, const QVector2D& value);
        bool setUniform(const char* name, const QVector3D& value);
        bool setUniform(const char* name, const QVector4D& value);
        bool setUniform(const char* name, const QMatrix4x4& value);
        int attributeLocation(const char* name);
        bool setAttribute(const char* name, float value);
        /**
         * Binds an attribute location for this shader.
         * The rendering pipeline assumes vertices to be bound to index 0
         * and texcoords to index 1.
         * @param index the index of the location to be bound
         * @param name the name of the attribute
         * @since 4.7
         */
        void bindAttributeLocation(int index, const char* name);

        void setTextureWidth( float width );
        void setTextureHeight( float height );
        float textureWidth();
        float textureHeight();

        static void initStatic();
        static bool fragmentShaderSupported()  { return mFragmentShaderSupported; }
        static bool vertexShaderSupported()  { return mVertexShaderSupported; }


    protected:
        bool loadFromFiles(const QString& vertexfile, const QString& fragmentfile);
        bool load(const QString& vertexsource, const QString& fragmentsource);


    private:
        unsigned int mProgram;
        bool mValid;
        static bool mFragmentShaderSupported;
        static bool mVertexShaderSupported;
        float mTextureWidth;
        float mTextureHeight;
    };

/**
 * @short Manager for Shaders.
 *
 * This class provides some built-in shaders to be used by both compositing scene and effects.
 * The ShaderManager provides methods to bind a built-in or a custom shader and keeps track of
 * the shaders which have been bound. When a shader is unbound the previously bound shader
 * will be rebound.
 *
 * @author Martin Gräßlin <kde@martin-graesslin.com>
 * @since 4.7
 **/
class KWIN_EXPORT ShaderManager
{
    public:
        /**
         * Identifiers for built-in shaders available for effects and scene
         **/
        enum ShaderType {
            /**
             * An orthographic projection shader able to render textured geometries.
             * Expects a @c vec2 uniform @c offset describing the offset from top-left corner
             * and defaults to @c (0/0). Expects a @c vec2 uniform @c textureSize to calculate
             * normalized texture coordinates. Defaults to @c (1.0/1.0). And expects a @c vec3
             * uniform @c colorManiuplation, with @c x being opacity, @c y being brightness and
             * @c z being saturation. All three values default to @c 1.0.
             * The sampler uniform is @c sample and defaults to @c 0.
             * The shader uses two vertex attributes @c vertex and @c texCoord.
             **/
            SimpleShader,
            /**
             * A generic shader able to render transformed, textured geometries.
             * This shader is mostly needed by the scene and not of much interest for effects.
             * Effects can influence this shader through @link ScreenPaintData and @link WindowPaintData.
             * The shader expects four @c mat4 uniforms @c projection, @c modelview,
             * @c screenTransformation and @c windowTransformation. The fragment shader expect the
             * same uniforms as the SimpleShader and the same vertex attributes are used.
             **/
            GenericShader,
            /**
             * An orthographic shader to render simple colored geometries without texturing.
             * Expects a @c vec2 uniform @c offset describing the offset from top-left corner
             * and defaults to @c (0/0). The fragment shader expects a single @c vec4 uniform
             * @c geometryColor, which defaults to fully opaque black.
             * The Shader uses one vertex attribute @c vertex.
             **/
            ColorShader
        };

        /**
         * @return The currently bound shader or @c null if no shader is bound.
         **/
        GLShader *getBoundShader() const;

        /**
         * @return @c true if a shader is bound, @c false otherwise
         **/
        bool isShaderBound() const;
        /**
         * @return @c true if the built-in shaders are valid, @c false otherwise
         **/
        bool isValid() const;

        /**
         * Binds the shader of specified @p type.
         * To unbind the shader use @link popShader. A previous bound shader will be rebound.
         * @param type The built-in shader to bind
         * @param reset Whether all uniforms should be reset to their default values
         * @return The bound shader or @c NULL if shaders are not valid
         * @see popShader
         **/
        GLShader *pushShader(ShaderType type, bool reset = false);
        /**
         * Binds the @p shader.
         * To unbind the shader use @link popShader. A previous bound shader will be rebound.
         * To bind a built-in shader use the more specific method.
         * @param shader The shader to be bound
         * @see popShader
         **/
        void pushShader(GLShader *shader);

        /**
         * Unbinds the currently bound shader and rebinds a previous stored shader.
         * If there is no previous shader, no shader will be rebound.
         * It is not safe to call this method if there is no bound shader.
         * @see pushShader
         * @see getBoundShader
         **/
        void popShader();

        /**
         * @return a pointer to the ShaderManager instance
         **/
        static ShaderManager *instance();

        /**
         * @internal
         **/
        static void cleanup();

    private:
        ShaderManager();
        ~ShaderManager();

        void initShaders();
        void resetShader(ShaderType type);

        QStack<GLShader*> m_boundShaders;
        GLShader *m_orthoShader;
        GLShader *m_genericShader;
        GLShader *m_colorShader;
        bool m_inited;
        bool m_valid;
        static ShaderManager *s_shaderManager;
};

/**
 * @short Render target object
 *
 * Render target object enables you to render onto a texture. This texture can
 *  later be used to e.g. do post-processing of the scene.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class KWIN_EXPORT GLRenderTarget
{
    public:
        /**
         * Constructs a GLRenderTarget
         * @param color texture where the scene will be rendered onto
         **/
        GLRenderTarget(GLTexture* color);
        ~GLRenderTarget();

        /**
         * Enables this render target.
         * All OpenGL commands from now on affect this render target until the
         *  @ref disable method is called
         **/
        bool enable();
        /**
         * Disables this render target, activating whichever target was active
         *  when @ref enable was called.
         **/
        bool disable();

        bool valid() const  { return mValid; }

        static void initStatic();
        static bool supported()  { return mSupported; }


    protected:
        void initFBO();


    private:
        static bool mSupported;

        GLTexture* mTexture;
        bool mValid;

        GLuint mFramebuffer;
};

/**
 * @short Vertex Buffer Object
 *
 * This is a short helper class to use vertex buffer objects (VBO). A VBO can be used to buffer
 * vertex data and to store them on graphics memory. It is the only allowed way to pass vertex
 * data to the GPU in OpenGL ES 2 and OpenGL 3 with forward compatible mode.
 *
 * If VBOs are not supported on the used OpenGL profile this class falls back to legacy
 * rendering using client arrays. Therefore this class should always be used for rendering geometries.
 *
 * @author Martin Gräßlin <kde@martin-graesslin.com>
 * @since 4.6
 */
class KWIN_EXPORT GLVertexBuffer
{
    public:
        /**
         * Enum to define how often the vertex data in the buffer object changes.
         */
        enum UsageHint
            {
            Dynamic, ///< frequent changes, but used several times for rendering
            Static, ///< No changes to data
            Stream ///< Data only used once for rendering, updated very frequently
            };

        GLVertexBuffer( UsageHint hint );
        ~GLVertexBuffer();

        /**
         * Sets the vertex data.
         * @param numberVertices The number of vertices in the arrays
         * @param dim The dimension of the vertices: 2 for x/y, 3 for x/y/z
         * @param vertices The vertices, size must equal @a numberVertices * @a dim
         * @param texcoords The texture coordinates for each vertex.
         * Size must equal 2 * @a numberVertices.
         */
        void setData( int numberVertices, int dim, const float* vertices, const float* texcoords);
        /**
         * Renders the vertex data in given @a primitiveMode.
         * Please refer to OpenGL documentation of glDrawArrays or glDrawElements for allowed
         * values for @a primitiveMode. Best is to use GL_TRIANGLES or similar to be future
         * compatible.
         */
        void render( GLenum primitiveMode );
        /**
         * Same as above restricting painting to @a region.
         */
        void render( const QRegion& region, GLenum primitiveMode );
        /**
         * Use methods from core profile to perform rendering. A core compatible shader has
         * to be bound while rendering.
         * If the shader emulates fixed functionality rendering (e.g. uses gl_Vertex) using core
         * rendering should be disabled.
         * The default rendering path does not use core profile rendering.
         * @param use enable/disable use of core profile rendering.
         * @since 4.7
         **/
        void setUseShader( bool use );
        /**
         * @returns @c true if core profile methods are used for rendering, @c false otherwise.
         * @see setUseShader
         * @since 4.7
         **/
        bool isUseShader() const;
        /**
         * Sets the color the geometry will be rendered with.
         * For legacy rendering glColor is used before rendering the geometry.
         * For core shader a uniform "geometryColor" is expected and is set.
         * @param color The color to render the geometry
         * @param enableColor Whether the geometry should be rendered with a color or not
         * @see setUseColor
         * @see isUseColor
         * @since 4.7
         **/
        void setColor(const QColor& color, bool enableColor = true);
        /**
         * @return @c true if geometry will be painted with a color, @c false otherwise
         * @see setUseColor
         * @see setColor
         * @since 4.7
         **/
        bool isUseColor() const;
        /**
         * Enables/Disables rendering the geometry with a color.
         * If no color is set an opaque, black color is used.
         * @param enable Enable/Disable rendering with color
         * @see isUseColor
         * @see setColor
         * @since 4.7
         **/
        void setUseColor(bool enable);

        /**
         * @internal
         */
        static void initStatic();
        /**
         * Returns true if VBOs are supported, it is save to use this class even if VBOs are not
         * supported.
         * @returns true if vertex buffer objects are supported
         */
        static bool isSupported();

    private:
        GLVertexBufferPrivate* const d;
};

} // namespace

#endif

/** @} */

#endif
