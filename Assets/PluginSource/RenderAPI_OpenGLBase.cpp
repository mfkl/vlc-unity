#include "RenderAPI_OpenGLBase.h"
#include "Log.h"

RenderAPI_OpenGLBase::RenderAPI_OpenGLBase(UnityGfxRenderer apiType)
	: m_APIType(apiType)
{
    DEBUG("Entering RenderAPI_OpenGLBase ctor");
#if SUPPORT_OPENGL_CORE
    glewExperimental=true;
    if (glewInit() != GLEW_OK) {
        DEBUG("unable to initialise GLEW");
    } else {
        DEBUG("GLEW init OK");
    }
#endif
    glGetError();
    DEBUG("Exiting RenderAPI_OpenGLBase ctor");
}


bool RenderAPI_OpenGLBase::setup(void **opaque,
                                      const libvlc_video_setup_device_cfg_t *cfg,
                                      libvlc_video_setup_device_info_t *out)
{
    DEBUG("output callback setup");
    RenderAPI_OpenGLBase* that = static_cast<RenderAPI_OpenGLBase*>(*opaque);
    that->width = 0;
    that->height = 0;
    return true;
}

void RenderAPI_OpenGLBase::cleanup(void* opaque)
{
    DEBUG("output callback cleanup");
    DEBUG("destroy_fbo");
    RenderAPI_OpenGLBase* that = reinterpret_cast<RenderAPI_OpenGLBase*>(opaque);
    if (that->width == 0 && that->height == 0)
        return;

    glDeleteTextures(3, that->tex);
    glDeleteFramebuffers(3, that->fbo);
}

bool RenderAPI_OpenGLBase::update_output(void* opaque, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *output)
{
    RenderAPI_OpenGLBase* that = reinterpret_cast<RenderAPI_OpenGLBase*>(opaque);
    if (cfg->width != that->width || cfg->height != that->height)
        cleanup(opaque);

    glGenTextures(3, that->tex);
    glGenFramebuffers(3, that->fbo);

    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, that->tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cfg->width, cfg->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        DEBUG("bind FBO");
        glBindFramebuffer(GL_FRAMEBUFFER, that->fbo[i]);

        DEBUG("FB Texture 2D");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, that->tex[i], 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE) {
            DEBUG("failed to create the FBO");
            return false;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);

    that->width = cfg->width;
    that->height = cfg->height;

    glBindFramebuffer(GL_FRAMEBUFFER, that->fbo[that->idx_render]);

    output->opengl_format = GL_RGBA;
    output->full_range = true;
    output->colorspace = libvlc_video_colorspace_BT709;
    output->primaries  = libvlc_video_primaries_BT709;
    output->transfer   = libvlc_video_transfer_func_SRGB;

    return true;
}

void RenderAPI_OpenGLBase::swap(void* opaque)
{
    DEBUG("output callback SWAP");
    RenderAPI_OpenGLBase* that = reinterpret_cast<RenderAPI_OpenGLBase*>(opaque);
    std::lock_guard<std::mutex> lock(that->text_lock);
    that->updated = true;
    std::swap(that->idx_swap, that->idx_render);
    glBindFramebuffer(GL_FRAMEBUFFER, that->fbo[that->idx_render]);
}

void* RenderAPI_OpenGLBase::getVideoFrame(bool* out_updated)
{
    // DEBUG("getVideoFrame");

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