#include "RenderAPI_OpenGLBase.h"
#include "Log.h"

RenderAPI_OpenGLBase::RenderAPI_OpenGLBase(UnityGfxRenderer apiType)
	: m_APIType(apiType)
{
#if SUPPORT_OPENGL_CORE
    glewExperimental=true;
    if (glewInit() != GLEW_OK) {
        DEBUG("unable to initialise GLEW");
    } else {
        DEBUG("GLEW init OK");
    }
#endif
    glGetError();
}


bool RenderAPI_OpenGLBase::setup(void **opaque,
                                      const libvlc_video_setup_device_cfg_t *cfg,
                                      libvlc_video_setup_device_info_t *out)
{
    RenderAPI_OpenGLBase* that = static_cast<RenderAPI_OpenGLBase*>(opaque);
    that->width = 0;
    that->height = 0;
    return true;
}

void RenderAPI_OpenGLBase::cleanup(void* opaque)
{
    DEBUG("destroy_fbo");
    RenderAPI_OpenGLBase* that = reinterpret_cast<RenderAPI_OpenGLBase*>(opaque);
    if (that->width == 0 && that->height == 0)
        return;

    glDeleteTextures(3, that->tex);
    glDeleteFramebuffers(3, that->fbo);
}


void RenderAPI_OpenGLBase::resize(void *opaque, void (*report_size_change)(void *report_opaque, unsigned width, unsigned height), void *report_opaque)
{
    DEBUG("create_fbo %p, %lu x %lu", opaque, width, height);
    RenderAPI_OpenGLBase* that = reinterpret_cast<RenderAPI_OpenGLBase*>(opaque);

    if (width != that->width || height != that->height)
        cleanup(data);

    glGenTextures(3, that->tex);
    glGenFramebuffers(3, that->fbo);

    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, that->tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        DEBUG("bind FBO");
        glBindFramebuffer(GL_FRAMEBUFFER, that->fbo[i]);

        DEBUG("FB Texture 2D");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, that->tex[i], 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        DEBUG("failed to create the FBO");
        return;
    }

    that->width = width;
    that->height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, that->fbo[that->idx_render]);
}

bool RenderAPI_OpenGLBase::update_output(void* opaque, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *output)
{

}

void RenderAPI_OpenGLBase::swap(void* opaque)
{
    RenderAPI_OpenGLBase* that = reinterpret_cast<RenderAPI_OpenGLBase*>(opaque);
    std::lock_guard<std::mutex> lock(that->text_lock);
    that->updated = true;
    std::swap(that->idx_swap, that->idx_render);
    glBindFramebuffer(GL_FRAMEBUFFER, that->fbo[that->idx_render]);
}

void RenderAPI_OpenGLBase::frameMetadata(void* opaque, libvlc_video_metadata_type_t type, const void *metadata)
{

}

bool RenderAPI_OpenGLBase::output_select_plane(void *opaque, size_t plane)
{

}

void* RenderAPI_OpenGLBase::getVideoFrame(bool* out_updated)
{
    std::lock_guard<std::mutex> lock(text_lock);
    if (out_updated)
        *out_updated = updated;
    if (updated)
    {
        std::swap(idx_swap, idx_display);
        updated = false;
    }
    //DEBUG("get Video Frame %u", tex[idx_display]);
    return (void*)(size_t)tex[idx_display];
}