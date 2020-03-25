#include "PlatformBase.h"
#include "RenderAPI.h"
#include "Log.h"

#include <map>
#include <windows.h>
#include <fstream>
#include <string>

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <vlc/vlc.h>
#include <string.h>
}

static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

libvlc_instance_t * inst;

static IUnityGraphics* s_Graphics = NULL;
static std::map<libvlc_media_player_t*,RenderAPI*> contexts = {};
static IUnityInterfaces* s_UnityInterfaces = NULL;

/** LibVLC's API function exported to Unity
 *
 * Every following functions will be exported to. Unity We have to
 * redeclare the LibVLC's function for the keyword
 * UNITY_INTERFACE_EXPORT and UNITY_INTERFACE_API
 */

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetPluginPath(char* path)
{
    DEBUG("SetPluginPath \n");
    auto e = _putenv_s("VLC_PLUGIN_PATH", path);
    if(e != 0)
        DEBUG("_putenv_s failed");
    else DEBUG("_putenv_s succeeded");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API Print(char* toPrint)
{
    DEBUG("%s", toPrint);
}

static const std::string base64_chars = 
             "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";

static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

std::string b64decode(const void* data, const size_t len)
{
    unsigned char* p = (unsigned char*)data;
    int pad = len > 0 && (len % 4 || p[len - 1] == '=');
    const size_t L = ((len + 3) / 4 - pad) * 4;
    std::string str(L / 4 * 3 + pad, '\0');

    for (size_t i = 0, j = 0; i < L; i += 4)
    {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad)
    {
        int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
        str[str.size() - 1] = n >> 16;

        if (len > L + 2 && p[L + 2] != '=')
        {
            n |= B64index[p[L + 2]] << 6;
            str.push_back(n >> 8 & 0xFF);
        }
    }
    return str;
}

extern "C" libvlc_media_player_t* UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
CreateAndInitMediaPlayer(libvlc_instance_t* libvlc)
{
    if(libvlc == NULL)
    {
        DEBUG("libvlc is NULL, aborting...");
        return NULL;
    }

    inst = libvlc;

    DEBUG("LAUNCH");

    if (inst == NULL) {
        DEBUG("LibVLC is not instanciated");
        return NULL;
    }

    libvlc_media_player_t * mp;

    mp = libvlc_media_player_new(inst);

    std::ofstream outfile;
    outfile.open("logo.png", std::ofstream::binary);
    std::string decoded = b64decode(base64_chars.c_str(), base64_chars.length());
    DEBUG("Base64 decoded: %s", decoded.c_str());
    outfile.write(decoded.c_str(), decoded.size());
    outfile.close();

    libvlc_video_set_logo_string(mp, libvlc_logo_file, "logo.png");
    libvlc_video_set_logo_int(mp, libvlc_logo_enable, 1);
    libvlc_video_set_logo_int(mp, libvlc_logo_position, 10);

    RenderAPI* s_CurrentAPI;

    if (mp == NULL) {
        DEBUG("Error initializing media player");
        goto err;
    }

    DEBUG("Calling... Initialize Render API \n");

    s_DeviceType = s_Graphics->GetRenderer();

    DEBUG("Calling... CreateRenderAPI \n");

    s_CurrentAPI = CreateRenderAPI(s_DeviceType);
    
    if(s_CurrentAPI == NULL)
    {
        DEBUG("s_CurrentAPI is NULL \n");    
    }    
    
    DEBUG("Calling... ProcessDeviceEvent \n");
    
    s_CurrentAPI->ProcessDeviceEvent(kUnityGfxDeviceEventInitialize, s_UnityInterfaces);

    DEBUG("Calling... setVlcContext s_CurrentAPI=%p mp=%p", s_CurrentAPI, mp);
    s_CurrentAPI->setVlcContext(mp);

    contexts[mp] = s_CurrentAPI;

    return mp;
err:
    if ( mp ) {
        // Stop playing
        libvlc_media_player_stop_async (mp);

        // Free the media_player
        libvlc_media_player_release (mp);
        mp = NULL;
    }    
    return NULL;
}

extern "C" void* UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
getTexture(libvlc_media_player_t* mp, bool * updated)
{
    if(mp == NULL)
        return NULL;

    RenderAPI* s_CurrentAPI = contexts.find(mp)->second;

    if (!s_CurrentAPI) {
        DEBUG("Error, no Render API");
        if (updated)
            *updated = false;
        return nullptr;
    }

    return s_CurrentAPI->getVideoFrame(updated);
}

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    DEBUG("UnityPluginLoad");
    s_UnityInterfaces = unityInterfaces;
    s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
    s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

    // Run OnGraphicsDeviceEvent(initialize) manually on plugin load
    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
  s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}


static RenderAPI* EarlyRenderAPI = NULL;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    // Create graphics API implementation upon initialization
       if (eventType == kUnityGfxDeviceEventInitialize) {
            DEBUG("Initialise Render API");
            if (EarlyRenderAPI != NULL) {
                DEBUG("*** EarlyRenderAPI != NULL while initialising ***");
                return;
            }

        DEBUG("s_Graphics->GetRenderer() \n");

        s_DeviceType = s_Graphics->GetRenderer();

        DEBUG("CreateRenderAPI(s_DeviceType) \n");

        EarlyRenderAPI = CreateRenderAPI(s_DeviceType);
        return;
    }

    if(EarlyRenderAPI){
        EarlyRenderAPI->ProcessDeviceEvent(eventType, s_UnityInterfaces);
    } else {
        DEBUG("Unable to process event, no Render API");
    }

    // Let the implementation process the device related events
    std::map<libvlc_media_player_t*, RenderAPI*>::iterator it;

    for(it = contexts.begin(); it != contexts.end(); it++)
    {
        RenderAPI* currentAPI = it->second;
        if(currentAPI) {
            DEBUG(" currentAPI->ProcessDeviceEvent(eventType, s_UnityInterfaces); \n");
            currentAPI->ProcessDeviceEvent(eventType, s_UnityInterfaces);
        }
    }
}

static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
}

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
    return OnRenderEvent;
}