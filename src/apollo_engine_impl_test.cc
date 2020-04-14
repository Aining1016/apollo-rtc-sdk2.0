
#include "media/engine/webrtc_video_engine.h"
#include "media/engine/webrtc_voice_engine.h"
#include "media/base/rtp_data_engine.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "api/transport/field_trial_based_config.h"
#include "api/rtc_event_log/rtc_event_log_factory.h"
#include "api/video/builtin_video_bitrate_allocator_factory.h"
#include "api/media_stream_track_proxy.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "rtc_base/logging.h"
#include "rtc_base/test_client.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "pc/local_audio_source.h"
#include "pc/audio_track.h"
#include "pc/session_description.h"
#include "pc/dtls_srtp_transport.h"
#include "apollo-rtc-sdk2.0/src/apollo_packet_transport.h"
#include "apollo_engine_impl.h"

namespace webrtc {

// ---------------AV demo
int ApolloEngineImpl::testIOSProcess(rtc::scoped_refptr<AudioSourceInterface> audio_src_,
                  rtc::scoped_refptr<VideoTrackSourceInterface> video_source_,
                  rtc::VideoSinkInterface<VideoFrame>* remote_sink_)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>(
      RTC_FROM_HERE, [=] { return testIOSProcess(audio_src_, video_source_, remote_sink_); });
  }
  Initialize();
  
  //------  audio
//  cricket::AudioOptions options = cricket::AudioOptions();
//  rtc::scoped_refptr<AudioSourceInterface> audio_source_
//      = CreateAudioSource(options);
//
//  //rtc::scoped_refptr<AudioTrackInterface> local_voice_track
//  rtc::scoped_refptr<MediaStreamTrackInterface> local_voice_track
//      = CreateAudioTrack("audio", audio_source_);
//
//  std::vector<std::string> audio_ids = {};
//  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
//      audio_tranceiver = AddTransceiver(local_voice_track, audio_ids);
//
//  int voice_channel_id = 0;
//  cricket::VoiceChannel* voe_channel = CreateVoiceChannel(voice_channel_id);
//  audio_tranceiver->internal()->SetChannel(voe_channel);
//
//  IAudioSetLocalAddress(voice_channel_id, 7078, 7079, false);
//  IAudioSetRemoteAddress(voice_channel_id, "127.0.0.1", 7078, "127.0.0.1", 7079);
//  StartAudioStream(voice_channel_id);
  //IAudioStartSend(voice_channel_id);
  
  //-------- video
  //rtc::scoped_refptr<VideoTrackInterface>
  rtc::scoped_refptr<MediaStreamTrackInterface>
    local_video_track = CreateVideoTrack("video", video_source_);
  
  std::vector<std::string> video_ids = {};
  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>> video_tranceiver
            = AddTransceiver(local_video_track, video_ids);
  
  rtc::scoped_refptr<MediaStreamTrackInterface> track
            = video_tranceiver->receiver()->track();
  if (track && track->kind() == MediaStreamTrackInterface::kVideoKind) {
    static_cast<VideoTrackInterface*>(track.get())
        ->AddOrUpdateSink(remote_sink_, rtc::VideoSinkWants());
  }
  

  int video_channel_id = 0;
  cricket::VideoChannel* vie_channel = CreateVideoChannel(video_channel_id);
  video_tranceiver->internal()->SetChannel(vie_channel);
  
  IVideoSetLocalAddress(video_channel_id, 9078, 9079, false);
  IVideoSetRemoteAddress(video_channel_id, "127.0.0.1", 9078, "127.0.0.1", 9079);
  //IVideoSetRemoteAddress(video_channel_id, "47.93.29.222", 8600, "47.93.29.222", 8600);
  
  const VideoCodecSetting video_codec;
  IVideoSetCodec(video_channel_id, video_codec);
  
  IVideoStartSend(video_channel_id);
  IVideoStartReceive(video_channel_id);
  
  return 0;
}

//Part test -----begin
int ApolloEngineImpl::CreateSocketClient()
{
  return network_thread_.get()->Invoke<int>(RTC_FROM_HERE, rtc::Bind(&ApolloEngineImpl::TestSocketClient, this));
}


int ApolloEngineImpl::TestSocketClient()
{
  // socket server.
  //  rtc::BasicPacketSocketFactory socket_factory_;
  //  std::unique_ptr<rtc::AsyncPacketSocket> socket1(
//    socket_factory_.CreateUdpSocket(rtc::SocketAddress("127.0.0.1", 0), 0, 0));
#if 1
    rtc::Thread* main = rtc::Thread::Current();
    rtc::SocketAddress loopback = rtc::SocketAddress("127.0.0.1", 9000);
    rtc::AsyncSocket* socket =
      main->socketserver()->CreateAsyncSocket(loopback.family(), SOCK_DGRAM);
    int ret = socket->Bind(loopback);

    //std::unique_ptr<AsyncPacketSocket> UDPSocket = std::make_unique<rtc::AsyncUDPSocket>(socket);
    rtc::TestClient client(std::make_unique<rtc::AsyncUDPSocket>(socket));
    rtc::SocketAddress addr = client.address(), from;
    //rtc::SocketAddress addr = rtc::SocketAddress("47.93.29.222", 8600), from;
  while (1) {
    ret = client.SendTo("foo", 3, addr);
    printf("zhangntest sendto ret:%d\n", ret);
    client.CheckNextPacket("foo", 3, &from);
    client.CheckNoPacket();
    sleep(5);
  }
#endif

  return 0;
}
}//namespace webrtc
