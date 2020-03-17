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

    static bool setup(void **opaque, const libvlc_video_setup_device_cfg_t *cfg, libvlc_video_setup_device_info_t *out);
    static void cleanup(void* opaque);
    static void resize(void *opaque, void (*report_size_change)(void *report_opaque, unsigned width, unsigned height), void *report_opaque);
    static bool update_output(void* opaque, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *output);
    static void swap(void* opaque);
    static void frameMetadata(void* opaque, libvlc_video_metadata_type_t type, const void *metadata);
    static bool output_select_plane(void *opaque, size_t plane);

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) override = 0;

    void* getVideoFrame(bool* out_updated) override;

private:
	UnityGfxRenderer m_APIType;

    std::mutex text_lock;
    unsigned width = 0;
    unsigned height = 0;
    void (*m_ReportSize)(void *ReportOpaque, unsigned width, unsigned height) = nullptr;
    void *m_reportOpaque = nullptr;
    GLuint tex[3];
    GLuint fbo[3];
    size_t idx_render = 0;
    size_t idx_swap = 1;
    size_t idx_display = 2;
    bool updated = false;
};


#endif /* RENDERAPI_OPENGLBASE_H */
