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