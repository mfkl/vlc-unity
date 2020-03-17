#ifndef RENDERAPI_OPENGLBASE_H
#define RENDERAPI_OPENGLBASE_H

#include "RenderAPI.h"
#include "PlatformBase.h"

#if UNITY_IPHONE
#	include <OpenGLES/ES2/gl.h>
#elif UNITY_ANDROID || UNITY_WEBGL
#	include <GLES2/gl2.h>
#else
#   define GL_GLEXT_PROTOTYPES
#	include "GLEW/glew.h"
#endif


#if UNITY_WIN
#  include <mingw.mutex.h>
#else
#  include <mutex>
#endif


class RenderAPI_OpenGLBase : public RenderAPI
{
public:
	RenderAPI_OpenGLBase(UnityGfxRenderer apiType);
	virtual ~RenderAPI_OpenGLBase() { }

    virtual void setVlcContext(libvlc_media_player_t *mp) override = 0 ;

    static bool setup(void* data) ;
    static void cleanup(void* data);
    static void resize(void* data, unsigned width, unsigned height);
    static void swap(void* data);


	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) override = 0;

    void* getVideoFrame(bool* out_updated) override;

private:
	UnityGfxRenderer m_APIType;

    std::mutex text_lock;
    unsigned width = 0;
    unsigned height = 0;
    GLuint tex[3];
    GLuint fbo[3];
    size_t idx_render = 0;
    size_t idx_swap = 1;
    size_t idx_display = 2;
    bool updated = false;
};


#endif /* RENDERAPI_OPENGLBASE_H */
