#ifndef __APOLLO_ENGINE_IMPL_H__
#define __APOLLO_ENGINE_IMPL_H__

#include "media/engine/webrtc_video_engine.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/media_stream_interface.h"
#include "pc/peer_connection_internal.h"
#include "api/scoped_refptr.h"
#include "api/media_types.h"
#include "api/rtp_transceiver_interface.h"
#include "api/transport/media/media_transport_interface.h"
#include "pc/channel_manager.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/thread.h"
#include "pc/rtp_receiver.h"
#include "pc/rtp_sender.h"
#include "pc/rtp_transceiver.h"
#include "pc/video_track.h"
#include "pc/audio_rtp_receiver.h"
#include "pc/video_rtp_receiver.h"
#include "pc/audio_track.h"
#include "pc/channel.h"
#include "api/rtc_event_log/rtc_event_log.h"
#include "apollo-rtc-sdk2.0/src/apollo_internal_types.h"
#include "apollo-rtc-sdk2.0/interface/apollo_types.h"

namespace webrtc {

class ApolloEngineImpl :  public RtpSenderBase::SetStreamsObserver,
                          public sigslot::has_slots<> {
    
public:
  ApolloEngineImpl();
  ~ApolloEngineImpl();

  void InitLog();
  int Initialize();
  
  // Interface --begin
  // Audio device
  int IGetSpeakerCount();
  int ISetSpeakerDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]);
  int IGetSpeakerDevice(unsigned int nIndex,
                                char deviceName[MAX_DEVICE_ID_LENGTH],
                                char deviceId[MAX_DEVICE_ID_LENGTH]);
  int IStartAudioPlayout(int channel_id);
  int IStoptAudioPlayout(int channel_id);
  int ISetSpeakerVolume(int volumep);
  int IGetSpeakerVolume(unsigned int& volumep);
  int ISetSpeakerMuteStatus(bool mute);
  int IGetSpeakerMuteStatus(bool& mute);
  int ISetLoudSpeakerStatus(bool enabled);
  int IGetLoudpeakerStatus(bool& enabled);
  //mic
  int IGetRecordCount();
  int ISetRecordDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]);
  int IGetRecordDevice(unsigned int nIndex,
                                char deviceName[MAX_DEVICE_ID_LENGTH],
                                char deviceId[MAX_DEVICE_ID_LENGTH]);
  int IStartAudioRecording(int channel_id);
  int IStopAudioRecording(int channel_id);
  int ISetMicVolume(int volumep);
  int IGetMicVolume(unsigned int& volumep);
  int ISetMuteStatus(bool mute);
  int IGetMuteStatus(bool& mute);

  //Video device
  int IGetCaptureCount();
  int ISetCaptureDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]);
  int IGetCaptureDevice(unsigned int nIndex,
                                char deviceName[MAX_DEVICE_ID_LENGTH],
                                char deviceId[MAX_DEVICE_ID_LENGTH]);

  //Audio process
  int ICreateAudioChannel(int channel_id);
  
  int IAudioSetCodec(int channel_id, const AudioCodecSetting& audio_codec);
  int IAudioSetReceiveCodec(int channel_id, const AudioCodecSetting& audio_codec);
  
  int IVideoSetCodec(int channel_id, const VideoCodecSetting& video_codec);
  int IVideoSetReceiveCodec(int channel_id, const VideoCodecSetting& video_codec);
  
  int IAudioSetLocalAddress(int channel_id,
                                  const int rtp_port,
                                  const int rtcp_port,
                                  bool is_IPv6);
  int IAudioSetRemoteAddress(int channel_id,
                                   const char *rtp_addr,
                                   const int rtp_port,
                                   const char *rtcp_addr,
                                   const int rtcp_port);
                                   
  int IAudioStartReceive(int channel_id);
  int IAudioStopReceive(int channel_id);
  
  int IAudioStartSend(int channel_id);
  int IAudioStopSend(int channel_id);
  
  int ISetAudioSSRC(int channel_id, unsigned int localssrc, unsigned int remotessrc);

  //Video process
  int IVideoSetLocalAddress(int channel_id,
                                  const int rtp_port,
                                  const int rtcp_port,
                                  bool is_IPv6);
  int IVideoSetRemoteAddress(int channel_id,
                                   const char *rtp_addr,
                                   const int rtp_port,
                                   const char *rtcp_addr,
                                   const int rtcp_port);
  int IVideoStartReceive(int channel_id);
  int IVideoStopReceive(int channel_id);
  int IVideoStartSend(int channel_id);
  int IVideoStopSend(int channel_id);
  int ISetVideoPreviewView(void *handle, void *view);
  //int ISetVideoRender(int channel_id, void *view, ReturnVideoWidthHeightM videoResolutionCallback);
  int ICreateVideoChannel(int channel_id);
  int ISetVideoSSRC(int channel_id, unsigned int localssrc, unsigned int remotessrc);
  int IRequestRemoteVideoSSRC(int channel_id, unsigned int ssrc);
  // interface --end
  
  rtc::Thread* network_thread() { return network_thread_.get(); }
  rtc::Thread* worker_thread() { return worker_thread_.get(); }
  rtc::Thread* signaling_thread() { return signaling_thread_.get(); }
  void OnSetStreams() override {}
  
  // test demo -------start
  int CreateSocketClient();
  int TestSocketClient();
  int testIOSProcess(rtc::scoped_refptr<AudioSourceInterface> audio_src_,
                  rtc::scoped_refptr<VideoTrackSourceInterface> video_source_,
                  rtc::VideoSinkInterface<VideoFrame>* remote_sink_);
  // test demo -------end
  
    rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
                  AddTransceiver( rtc::scoped_refptr<MediaStreamTrackInterface>& track,
                                  const std::vector<std::string>& stream_ids);
private:
  
  void InitMedia();
  void InitThreads();
  void CreateCall();
  bool CreateEventLog();
  void* FindChannel(int channel_id);
  
  std::unique_ptr<Call> CreateCall_w();
  cricket::VideoChannel* CreateVideoChannel(int& channel_id);
  cricket::VoiceChannel* CreateVoiceChannel(int& channel_id);
  int DeleteChannel(int channel_id, bool is_video);
  
  //use rtc::scoped_refptr<>??
  RtpTransportInternal* CreateDtlsSrtpTransport();
  
  
  rtc::scoped_refptr<VideoTrackInterface> CreateVideoTrack(
                                                        const std::string& id,
                                                        VideoTrackSourceInterface* source);
  rtc::scoped_refptr<AudioTrackInterface> CreateAudioTrack(
                                                        const std::string& id,
                                                        AudioSourceInterface* source);
  rtc::scoped_refptr<AudioSourceInterface>
                  CreateAudioSource(const cricket::AudioOptions& options);


  rtc::scoped_refptr<RtpSenderProxyWithInternal<RtpSenderInternal>>
                  CreateSender(cricket::MediaType media_type,
                           const std::string& id,
                           rtc::scoped_refptr<MediaStreamTrackInterface> track,
                           const std::vector<std::string>& stream_ids,
                           const std::vector<RtpEncodingParameters>& send_encodings);
  rtc::scoped_refptr<RtpReceiverProxyWithInternal<RtpReceiverInternal>>
                  CreateReceiver(cricket::MediaType media_type,
                               const std::string& receiver_id);

  void SetAudioRecording(bool recording);
  void SetAudioPlayout(bool playout);
  
  void OnSentPacket_w(const rtc::SentPacket& sent_packet);
  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
                            FindTransceiverByChannel(cricket::ChannelInterface* channel);
  bool SetRemoteAddr(RtpTransport* transport,
                   rtc::SocketAddress& remote_rtp,
                   rtc::SocketAddress& remote_rtcp);
  int CreateRtpRtcpClient(RtpTransport* transport,
                        rtc::SocketAddress rtp_addr,
                        rtc::SocketAddress rtcp_addr);
  int SetTransportStat(RtpTransport* rtp_transport, bool writable);
private:
  
  std::unique_ptr<rtc::Thread> network_thread_ = nullptr;
  std::unique_ptr<rtc::Thread> worker_thread_ = nullptr;
  std::unique_ptr<rtc::Thread> signaling_thread_ = nullptr;
  
  std::unique_ptr<cricket::ChannelManager> channel_manager_ = nullptr;
  //std::unique_ptr<cricket::MediaEngineInterface> media_engine_ = nullptr;

  std::unique_ptr<CallFactoryInterface> call_factory_ = nullptr;
  std::unique_ptr<Call> call_ = nullptr;
  std::unique_ptr<FecControllerFactoryInterface> fec_controller_factory_ = nullptr;
  std::unique_ptr<TaskQueueFactory> task_queue_factory_ = nullptr;
  std::unique_ptr<NetworkStatePredictorFactoryInterface> network_state_predictor_factory_ = nullptr;
  std::unique_ptr<NetEqFactory> neteq_factory_ = nullptr;
  std::unique_ptr<WebRtcKeyValueConfig> trials_ = nullptr;
  
  std::unique_ptr<RtcEventLogFactoryInterface> event_log_factory_ = nullptr;
  std::unique_ptr<RtcEventLog> rtc_event_log_ = nullptr;
  
  std::unique_ptr<VideoBitrateAllocatorFactory>
                              video_bitrate_allocator_factory_ = nullptr;
  rtc::UniqueRandomIdGenerator ssrc_generator_;
  
//  cricket::VoiceChannel* voice_channel_ = nullptr;
//  cricket::VideoChannel* video_channel_ = nullptr;
  std::unique_ptr<RtpTransportInternal> rtp_transport_;
  std::vector<rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>>
              transceivers_;
  // and network thread.
//  rtc::scoped_refptr<AudioRtpSender> audio_rtp_sender_;
//  rtc::scoped_refptr<VideoRtpSender> video_rtp_sender_;
//  rtc::scoped_refptr<AudioRtpReceiver> audio_rtp_receiver_;
//  rtc::scoped_refptr<VideoRtpReceiver> video_rtp_receiver_;
//  rtc::scoped_refptr<MediaStreamInterface> local_stream_;
//  rtc::scoped_refptr<VideoTrackInterface> video_track_;
//  rtc::scoped_refptr<AudioTrackInterface> audio_track_;
  
  rtc::CriticalSection channel_crit_;
  std::map<int, void*> channel_map_;
  int channel_index_ = 0;
  
  std::unique_ptr<rtc::VideoSinkInterface<VideoFrame>> remote_sink_ = nullptr;
  rtc::scoped_refptr<VideoTrackSourceInterface> video_source_ = nullptr;
};

}//namespace webrtc

#endif
