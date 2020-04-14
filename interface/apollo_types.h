#ifndef __APOLLO_TYPES_H__
#define __APOLLO_TYPES_H__

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define APOLLO_CALL __cdecl
#if defined(APOLLORTC_EXPORT)
#define APOLLO_API extern "C" __declspec(dllexport)
#else
#define APOLLO_API extern "C" __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#define APOLLO_API __attribute__((visibility("default"))) extern "C"
#define APOLLO_CALL
#elif defined(__ANDROID__) || defined(__linux__)
#define APOLLO_API extern "C" __attribute__((visibility("default")))
#define APOLLO_CALL
#else
#define APOLLO_API extern "C"
#define APOLLO_CALL
#endif

namespace webrtc {
/** return code.
*/
enum APOLLO_RETURN_CODE
{
  /** 0: No error occurs.
  */
    APOLLO_DONE = 0,
    //1~1000
    /** -1: A general error occurs (no specified reason).
    */
    APOLLO_FAILED = -1,
    /** -2: An invalid parameter is used. For example, the specific channel name includes illegal characters.
    */
    INVALID_ARGUMENT_ERROR = -2,
    /** 3: The SDK module is not ready. Possible solutions:

     - Check the audio device.
     - Check the completeness of the application.
     - Re-initialize the RTC engine.
     */
    NOT_READY_ERROR = -3,
    /** 4: The SDK does not support this function.
     */
    NOT_SUPPORTED_ERROR = -4,
    /** 5: The request is rejected. This is for internal SDK use only, and it does not return to the application through any method or callback.
     */
    REFUSED_ERROR = -5,
    /** 6: The buffer size is not big enough to store the returned data.
     */
    BUFFER_TOO_SMALL_ERROR = -6,
    /** 7: The SDK is not initialized before calling this method.
     */
    NOT_INITIALIZED_ERROR = -7,
    /** 9: No permission exists. This is for internal SDK use only, and it does not return to the application through any method or callback.
     */
    NO_PERMISSION_ERROR= -9,
    /** 10: An API method timeout occurs. Some API methods require the SDK to return the execution result, and this error occurs if the request takes too long (more than 10 seconds) for the SDK to process.
     */
    TIMEDOUT_ERROR = -10,
    /** 11: The request is canceled. This is for internal SDK use only, and it does not return to the application through any method or callback.
     */
    CANCELED_ERROR = -11,
    /** 12: The method is called too often. This is for internal SDK use only, and it does not return to the application through any method or callback.
     */
    TOO_OFTEN_ERROR = -12,
    /** 13: The SDK fails to bind to the network socket. This is for internal SDK use only, and it does not return to the application through any method or callback.
     */
    BIND_SOCKET_ERROR = -13,
    /** 14: The network is unavailable. This is for internal SDK use only, and it does not return to the application through any method or callback.
     */
    NET_DOWN_ERROR = -14,
    /** 15: No network buffers are available. This is for internal SDK internal use only, and it does not return to the application through any method or callback.
     */
    NET_NOBUFS_ERROR = -15,

    /** 19: Resources are occupied and cannot be reused.
     */
    ALREADY_IN_USE_ERROR = -19,
    /** 20: The SDK gives up the request due to too many requests.
     */
    ABORTED_ERROR = -20,
    /** 21: In Windows, specific firewall settings can cause the SDK to fail to initialize and crash.
     */
    INIT_NET_ENGINE_ERROR = -21,
    /** 22: The application uses too much of the system resources and the SDK fails to allocate the resources.
     */
    RESOURCE_LIMITED_ERROR = -22,

     /** -101: Not found.
     */
    NOT_FOUND_CHANNEL_ERROR = -101,
     /** -102: Not found.
     */
    INPUT_PARAMETER_ERROR = -102,

    //-2001 ~ -2000
    /** 1001: Fails to load the media engine.
     */
    LOAD_MEDIA_ENGINE_ERROR = -1001,
    /** 1002: Fails to start the call after enabling the media engine.
     */
    START_CALL_ERROR = -1002,
    /** 1003: Fails to start the camera.
     */
    START_CAMERA_ERROR = -1003,
    /** 1004: Fails to start the video rendering module.
     */
    START_VIDEO_RENDER_ERROR = -1004,
    /** 1005: A general error occurs in the Audio Device Module (no specified reason). Check if the audio device is used by another application, or try rejoining the channel.
     */
    ADM_GENERAL_ERROR = -1005,
    /** 1006: Audio Device Module: An error occurs in using the Java resources.
     */
    ADM_JAVA_RESOURCE_ERROR = -1006,
    /** 1007: Audio Device Module: An error occurs in setting the sampling frequency.
     */
    ADM_SAMPLE_RATE_ERROR = -1007,
    /** 1008: Audio Device Module: An error occurs in initializing the playback device.
     */
    ADM_INIT_PLAYOUT_ERROR = -1008,
    /** 1009: Audio Device Module: An error occurs in starting the playback device.
     */
    ADM_START_PLAYOUT_ERROR = -1009,
    /** 1010: Audio Device Module: An error occurs in stopping the playback device.
     */
    ADM_STOP_PLAYOUT_ERROR = -1010,
    /** 1011: Audio Device Module: An error occurs in initializing the recording device.
     */
    ADM_INIT_RECORDING_ERROR = -1011,
    /** 1012: Audio Device Module: An error occurs in starting the recording device.
     */
    ADM_START_RECORDING_ERROR = -1012,
    /** 1013: Audio Device Module: An error occurs in stopping the recording device.
     */
    ADM_STOP_RECORDING_ERROR = -1013,
    /** 1015: Audio Device Module: A playback error occurs. Check your playback device and try rejoining the channel.
     */
    ADM_RUNTIME_PLAYOUT_ERROR = -1015,
    /** 1017: Audio Device Module: A recording error occurs.
     */
    ADM_RUNTIME_RECORDING_ERROR = -1017,
    /** 1018: Audio Device Module: Fails to record.
     */
    ADM_RECORD_AUDIO_FAILED = -1018,
    /** 1022: Audio Device Module: An error occurs in initializing the loopback device.
     */
    ADM_INIT_LOOPBACK_ERROR = -1022,
    /** 1023: Audio Device Module: An error occurs in starting the loopback device.
     */
    ADM_START_LOOPBACK_ERROR = -1023,
    /** 1027: Audio Device Module: No recording permission exists. Check if the recording permission is granted.
     */
    ADM_NO_PERMISSION_ERROR = -1027,
    ADM_RECORD_AUDIO_IS_ACTIVE_ERROR = -1033,
    ADM_ANDROID_JNI_JAVA_RESOURCE_ERROR = -1101,
    ADM_ANDROID_JNI_NO_RECORD_FREQUENCY_ERROR = -1108,
    ADM_ANDROID_JNI_NO_PLAYBACK_FREQUENCY_ERROR = -1109,
    ADM_ANDROID_JNI_JAVA_START_RECORD_ERROR = -1111,
    ADM_ANDROID_JNI_JAVA_START_PLAYBACK_ERROR = -1112,
    ADM_ANDROID_JNI_JAVA_RECORD_ERROR_ERROR = -1115,
    ADM_ANDROID_OPENSL_CREATE_ENGINE_ERROR = -1151,
    ADM_ANDROID_OPENSL_CREATE_AUDIO_RECORDER_ERROR = -1153,
    ADM_ANDROID_OPENSL_START_RECORDER_THREAD_ERROR = -1156,
    ADM_ANDROID_OPENSL_CREATE_AUDIO_PLAYER_ERROR = -1157,
    ADM_ANDROID_OPENSL_START_PLAYER_THREAD_ERROR = 1-160,
    ADM_IOS_INPUT_NOT_AVAILABLE_ERROR = -1201,
    ADM_IOS_ACTIVATE_SESSION_FAIL_ERROR = -1206,
    ADM_IOS_VPIO_INIT_FAIL = -1210,
    ADM_IOS_VPIO_REINIT_FAIL = -1213,
    ADM_IOS_VPIO_RESTART_FAIL = -1214,
    ADM_IOS_SET_RENDER_CALLBACK_FAIL = -1219,
    ADM_IOS_SESSION_SAMPLERATR_ZERO_ERROR = -1221,
    ADM_WIN_CORE_INIT_ERROR = -1301,
    ADM_WIN_CORE_INIT_RECORDING_ERROR = -1303,
    ADM_WIN_CORE_INIT_PLAYOUT_ERROR = -1306,
    ADM_WIN_CORE_INIT_PLAYOUT_NULL_ERROR = -1307,
    ADM_WIN_CORE_START_RECORDING_ERROR = -1309,
    ADM_WIN_CORE_CREATE_REC_THREAD_ERROR = -1311,
    ADM_WIN_CORE_CAPTURE_NOT_STARTUP_ERROR = -1314,
    ADM_WIN_CORE_CREATE_RENDER_THREAD_ERROR = -1319,
    ADM_WIN_CORE_RENDER_NOT_STARTUP_ERROR = -1320,
    ADM_WIN_CORE_NO_RECORDING_DEVICE_ERROR = -1322,
    ADM_WIN_CORE_NO_PLAYOUT_DEVICE_ERROR = -1323,
    ADM_WIN_WAVE_INIT_ERROR = -1351,
    ADM_WIN_WAVE_INIT_RECORDING_ERROR = -1353,
    ADM_WIN_WAVE_INIT_MICROPHONE_ERROR = -1354,
    ADM_WIN_WAVE_INIT_PLAYOUT_ERROR = -1355,
    ADM_WIN_WAVE_INIT_SPEAKER_ERROR = -1356,
    ADM_WIN_WAVE_START_RECORDING_ERROR = -1357,
    ADM_WIN_WAVE_START_PLAYOUT_ERROR = -1358,
    /** 1359: Audio Device Module: No recording device exists.
     */
    ADM_NO_RECORDING_DEVICE_ERROR = -1359,
    /** 1360: Audio Device Module: No playback device exists.
     */
    ADM_NO_PLAYOUT_DEVICE_ERROR = -1360,

    // VDM error code starts from 1500
    /** 1501: Video Device Module: The camera is unauthorized.
     */
    VDM_CAMERA_NOT_AUTHORIZED_ERROR = -1501,

    // VDM error code starts from 1500
    /** 1502: Video Device Module: The camera in use.
     */
    VDM_WIN_DEVICE_IN_USE_ERROR = -1502,

    // VCM error code starts from 1600
    /** 1600: Video Device Module: An unknown error occurs.
     */
    VCM_UNKNOWN_ERROR = -1600,
    /** 1601: Video Device Module: An error occurs in initializing the video encoder.
    */
    VCM_ENCODER_INIT_ERROR = -1601,
    /** 1602: Video Device Module: An error occurs in encoding.
     */
    VCM_ENCODER_ENCODE_ERROR = -1602,
    /** 1603: Video Device Module: An error occurs in setting the video encoder.
     */
    VCM_ENCODER_SET_ERROR = -1603,
	
	   /** 701: An error occurs in opening the audio mixing file.
    */
    AUDIO_MIXING_OPEN_ERROR = -1701,
	
    /** 1014: Audio Device Module: a warning occurs in the playback device.
    */
    ADM_RUNTIME_PLAYOUT_WARNING = 1014,
    /** 1016: Audio Device Module: a warning occurs in the recording device.
    */
    ADM_RUNTIME_RECORDING_WARNING = 1016,
    /** 1019: Audio Device Module: no valid audio data is collected. This warning does not affect the ongoing call.
    */
    ADM_RECORD_AUDIO_SILENCE_WARNING = 1019,
    /** 1020: Audio Device Module: the playback device fails.
    */
    ADM_PLAYOUT_MALFUNCTION_WARNING = 1020,
    /** 1021: Audio Device Module: the recording device fails.
    */
    ADM_RECORD_MALFUNCTION_WARNING = 1021,
    /**
    */
    ADM_IOS_CATEGORY_NOT_PLAYANDRECORD_WARNING = 1029,
    /**
    */
    ADM_IOS_SAMPLERATE_CHANGE_WARNING = 1030,
    /** 1031: Audio Device Module: the recorded audio voice is too low.
    */
    ADM_RECORD_AUDIO_LOWLEVEL_WARNING = 1031,
    /** 1032: Audio Device Module: the playback audio voice is too low.
    */
    ADM_PLAYOUT_AUDIO_LOWLEVEL_WARNING = 1032,
    /**
    */
    ADM_WINDOWS_NO_DATA_READY_EVENT_WARNING = 1040,
    /** 1051: Audio Device Module: howling is detected.
    */
    APM_HOWLING_WARNING = 1051,
    /** 1052: Audio Device Module: the device is in the glitch state.
    */
    ADM_GLITCH_STATE_WARNING = 1052,
    /** 1053: Audio Device Module: the underlying audio settings have changed.
    */
    ADM_IMPROPER_SETTINGS_WARNING = 1053,
    /**
        */
    ADM_WIN_CORE_NO_RECORDING_DEVICE_WARNING = 1322,
    /**
    */
    ADM_WIN_CORE_NO_PLAYOUT_DEVICE_WARNING = 1323,
    /**
    */
    ADM_WIN_CORE_IMPROPER_CAPTURE_RELEASE_WARNING = 1324,
    /** 1610: Super-resolution warning: the original video dimensions of the remote user exceed 640 &times; 480.
    */
    SUPER_RESOLUTION_STREAM_OVER_LIMITATION_WARNING = 1610,
    /** 1611: Super-resolution warning: another user is using super resolution.
    */
    SUPER_RESOLUTION_USER_COUNT_OVER_LIMITATION_WARNING = 1611,
    /** 1612: The device is not supported.
    */
    SUPER_RESOLUTION_DEVICE_NOT_SUPPORTED_WARNING = 1612,
};

/** Output log filter level. */
enum LOG_FILTER_TYPE
{
/** 0: Do not output any log information. */
    LOG_FILTER_OFF = 0,
     /** 0x080f: Output all log information. */
    LOG_FILTER_DEBUG = 0x080f,
     /** 0x000f: Output CRITICAL, ERROR, WARNING, and INFO level log information. */
    LOG_FILTER_INFO = 0x000f,
     /** 0x000e: Outputs CRITICAL, ERROR, and WARNING level log information. */
    LOG_FILTER_WARN = 0x000e,
     /** 0x000c: Outputs CRITICAL and ERROR level log information. */
    LOG_FILTER_ERROR = 0x000c,
     /** 0x0008: Outputs CRITICAL level log information. */
    LOG_FILTER_CRITICAL = 0x0008,
    LOG_FILTER_MASK = 0x80f,
};

typedef unsigned int uid_t;
typedef void* view_t;
///** Maximum length of the device ID.
//*/
//enum MAX_DEVICE_ID_LENGTH_TYPE
//{
//  /** The maximum length of the device ID is 512 bytes.
//  */
//    MAX_DEVICE_ID_LENGTH = 512
//};

/** Audio profiles.

Sets the sample rate, bitrate, encoding mode, and the number of channels:*/
enum AUDIO_PROFILE_TYPE // sample rate, bit rate, mono/stereo, speech/music codec
{
  /**
   0: Default audio profile. In the Communication profile, the default value is 1; in the Live-broadcast profile, the default value is 2.
   */
    AUDIO_PROFILE_DEFAULT = 0, // use default settings
    /**
     1: Sample rate of 32 kHz, audio encoding, mono, and a bitrate of up to 18 Kbps.
     */
    AUDIO_PROFILE_SPEECH_STANDARD = 1, // 32Khz, 18Kbps, mono, speech
    /**
     2: Sample rate of 48 kHz, music encoding, mono, and a bitrate of up to 48 Kbps.
     */
    AUDIO_PROFILE_MUSIC_STANDARD = 2, // 48Khz, 48Kbps, mono, music
    /**
     3: Sample rate of 48 kHz, music encoding, stereo, and a bitrate of up to 56 Kbps.
     */
    AUDIO_PROFILE_MUSIC_STANDARD_STEREO = 3, // 48Khz, 56Kbps, stereo, music
    /**
     4: Sample rate of 48 kHz, music encoding, mono, and a bitrate of up to 128 Kbps.
     */
    AUDIO_PROFILE_MUSIC_HIGH_QUALITY = 4, // 48Khz, 128Kbps, mono, music
    /**
     5: Sample rate of 48 kHz, music encoding, stereo, and a bitrate of up to 192 Kbps.
     */
    AUDIO_PROFILE_MUSIC_HIGH_QUALITY_STEREO = 5, // 48Khz, 192Kbps, stereo, music
    /**
     6: Sample rate of 16 kHz, audio encoding, mono, and Acoustic Echo Cancellation (AES) enabled.
     */
    AUDIO_PROFILE_IOT                       = 6,
    AUDIO_PROFILE_NUM = 7,
};


/** Audio-sample rates. */
enum AUDIO_SAMPLE_RATE_TYPE
{
    /** 32000: 32 kHz */
    AUDIO_SAMPLE_RATE_32000 = 32000,
    /** 44100: 44.1 kHz */
    AUDIO_SAMPLE_RATE_44100 = 44100,
      /** 48000: 48 kHz */
    AUDIO_SAMPLE_RATE_48000 = 48000,
};

#define MAX_AUDIO_CODEC_NAME_LENGTH 32
//#define MAX_SUPPORT_AUDIO_CODECS_NUM 32
//#define MAX_SUPPORT_VIDEO_CODECS_NUM 8

struct AudioCodecSetting {
    int pltype;
    char plname[MAX_AUDIO_CODEC_NAME_LENGTH];
    int plfreq;
    int paceSize;
    int channels;
    int rate;
    bool fecEnabled;
    double packetLossRate;
    
    AudioCodecSetting()
      :pltype(-1),plfreq(-1)
      ,paceSize(-1),channels(-1), rate(-1)
      ,fecEnabled(false),packetLossRate(-1)
    {
      memset(plname,0,sizeof(plname));
    }
};

/** Video codec profile types. */
enum VIDEO_CODEC_PROFILE_TYPE
{  /** 66: Baseline video codec profile. Generally used in video calls on mobile phones. */
    VIDEO_CODEC_PROFILE_BASELINE = 66,
    /** 77: Main video codec profile. Generally used in mainstream electronics such as MP4 players, portable video players, PSP, and iPads. */
    VIDEO_CODEC_PROFILE_MAIN = 77,
      /** 100: (Default) High video codec profile. Generally used in high-resolution broadcasts or television. */
    VIDEO_CODEC_PROFILE_HIGH = 100,
};

/** Video frame rates. */
enum FRAME_RATE
{
      /** 1: 1 fps */
    FRAME_RATE_FPS_1 = 1,
        /** 7: 7 fps */
    FRAME_RATE_FPS_7 = 7,
      /** 10: 10 fps */
    FRAME_RATE_FPS_10 = 10,
    /** 15: 15 fps */
    FRAME_RATE_FPS_15 = 15,
        /** 24: 24 fps */
    FRAME_RATE_FPS_24 = 24,
    /** 30: 30 fps */
    FRAME_RATE_FPS_30 = 30,
    /** 60: 60 fps (Windows and macOS only) */
    FRAME_RATE_FPS_60 = 60,
};

/** Video degradation preferences when the bandwidth is a constraint. */
enum QUALITY_PREFERENCE {
    /** 0: (Default) Degrade the frame rate in order to maintain the video quality. */
    MAINTAIN_QUALITY = 0,
    /** 1: Degrade the video quality in order to maintain the frame rate. */
    MAINTAIN_FRAMERATE = 1,
    /** 2: (For future use) Maintain a balance between the frame rate and video quality. */
    MAINTAIN_BALANCED = 2,
};
/** Video encoder configurations.
 */
struct VideoCodecSetting {
    /** Width (pixels) of the video. */
    int width;
      /** Height (pixels) of the video. */
    int height;
    /** The frame rate of the video: #FRAME_RATE.
     
     Note that we do not recommend setting this to a value greater than 30.
    */
    FRAME_RATE frameRate;
    /** The minimum frame rate of the video. The default value is -1.
     */
    int minFrameRate;    
    /** The video encoding bitrate (Kbps).
     Choose one of the following options:
     
     The following table lists the recommended video encoder configurations, where the base bitrate applies to the Communication profile. Set your bitrate based on this table. If you set a bitrate beyond the proper range, the SDK automatically sets it to within the range.

     | Resolution             | Frame Rate (fps) | Base Bitrate (Kbps, for Communication) | Live Bitrate (Kbps, for Live Broadcast)|
     |------------------------|------------------|----------------------------------------|----------------------------------------|
     | 160 &times; 120        | 15               | 65                                     | 130                                    |
     | 120 &times; 120        | 15               | 50                                     | 100                                    |
     | 320 &times; 180        | 15               | 140                                    | 280                                    |
     | 180 &times; 180        | 15               | 100                                    | 200                                    |
     | 240 &times; 180        | 15               | 120                                    | 240                                    |
     | 320 &times; 240        | 15               | 200                                    | 400                                    |
     | 240 &times; 240        | 15               | 140                                    | 280                                    |
     | 424 &times; 240        | 15               | 220                                    | 440                                    |
     | 640 &times; 360        | 15               | 400                                    | 800                                    |
     | 360 &times; 360        | 15               | 260                                    | 520                                    |
     | 640 &times; 360        | 30               | 600                                    | 1200                                   |
     | 360 &times; 360        | 30               | 400                                    | 800                                    |
     | 480 &times; 360        | 15               | 320                                    | 640                                    |
     | 480 &times; 360        | 30               | 490                                    | 980                                    |
     | 640 &times; 480        | 15               | 500                                    | 1000                                   |
     | 480 &times; 480        | 15               | 400                                    | 800                                    |
     | 640 &times; 480        | 30               | 750                                    | 1500                                   |
     | 480 &times; 480        | 30               | 600                                    | 1200                                   |
     | 848 &times; 480        | 15               | 610                                    | 1220                                   |
     | 848 &times; 480        | 30               | 930                                    | 1860                                   |
     | 640 &times; 480        | 10               | 400                                    | 800                                    |
     | 1280 &times; 720       | 15               | 1130                                   | 2260                                   |
     | 1280 &times; 720       | 30               | 1710                                   | 3420                                   |
     | 960 &times; 720        | 15               | 910                                    | 1820                                   |
     | 960 &times; 720        | 30               | 1380                                   | 2760                                   |
     | 1920 &times; 1080      | 15               | 2080                                   | 4160                                   |
     | 1920 &times; 1080      | 30               | 3150                                   | 6300                                   |
     | 1920 &times; 1080      | 60               | 4780                                   | 6500                                   |
     | 2560 &times; 1440      | 30               | 4850                                   | 6500                                   |
     | 2560 &times; 1440      | 60               | 6500                                   | 6500                                   |
     | 3840 &times; 2160      | 30               | 6500                                   | 6500                                   |
     | 3840 &times; 2160      | 60               | 6500                                   | 6500                                   |

     */
    int bitrate;
    /** The minimum encoding bitrate (Kbps).
     */
    int minBitrate;

    /** the video encoding degradation preference under limited bandwidth: #DEGRADATION_PREFERENCE.
     */
    QUALITY_PREFERENCE qualityPreference;

    VideoCodecSetting(
        int width, int height, FRAME_RATE f,
        int b)
        : width(width), height(height), frameRate(f),
          minFrameRate(-1), bitrate(b),
          minBitrate(-1),
          qualityPreference(MAINTAIN_QUALITY)
    {}
    VideoCodecSetting()
        : width(640), height(480)
        , frameRate(FRAME_RATE_FPS_15)
        , minFrameRate(-1)
        , bitrate(0)
        , minBitrate(-1)
        , qualityPreference(MAINTAIN_QUALITY)
    {}
};

enum MAX_DEVICE_ID_LENGTH_TYPE
{
  /** The maximum length of the device ID is 512 bytes.
  */
  MAX_DEVICE_ID_LENGTH = 512
};

} //namespace webrtc
#endif
