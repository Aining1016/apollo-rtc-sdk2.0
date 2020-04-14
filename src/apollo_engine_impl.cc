
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

ApolloEngineImpl::ApolloEngineImpl() {
  InitLog();
  InitThreads();
}

ApolloEngineImpl::~ApolloEngineImpl() {

}

int ApolloEngineImpl::Initialize() {

    RTC_LOG(INFO) << "Initialize() worker_thread_="<<static_cast< void *>(worker_thread_.get());
    return worker_thread_.get()->Invoke<int>(RTC_FROM_HERE, [this] {
             trials_ = std::make_unique<FieldTrialBasedConfig>();
             InitMedia();
             CreateEventLog();
             CreateCall();      
             return APOLLO_DONE;
           });
}

//Part interface  -----begin
// Audio device
int ApolloEngineImpl::IGetSpeakerCount() {
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetSpeakerDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetSpeakerDevice(unsigned int nIndex,
                              char deviceName[MAX_DEVICE_ID_LENGTH],
                              char deviceId[MAX_DEVICE_ID_LENGTH]) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IStartAudioPlayout(int channelid) {
  SetAudioPlayout(true);
  return APOLLO_DONE;
}

int ApolloEngineImpl::IStoptAudioPlayout(int channelid) {
  SetAudioPlayout(false);
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetSpeakerVolume(int volumep) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetSpeakerVolume(unsigned int& volumep) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetSpeakerMuteStatus(bool mute) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetSpeakerMuteStatus(bool& mute) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetLoudSpeakerStatus(bool enabled) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetLoudpeakerStatus(bool& enabled) {
  return APOLLO_DONE;
}

//mic
int ApolloEngineImpl::IGetRecordCount() {
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetRecordDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetRecordDevice(unsigned int nIndex,
                              char deviceName[MAX_DEVICE_ID_LENGTH],
                              char deviceId[MAX_DEVICE_ID_LENGTH]) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IStartAudioRecording(int channelid) {
  SetAudioRecording(true);
  return APOLLO_DONE;
}

int ApolloEngineImpl::IStopAudioRecording(int channelid) {
  SetAudioRecording(false);
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetMicVolume(int volumep) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetMicVolume(unsigned int& volumep) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetMuteStatus(bool mute) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetMuteStatus(bool& mute) {
  return APOLLO_DONE;
}

//Video device
int ApolloEngineImpl::IGetCaptureCount() {
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetCaptureDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IGetCaptureDevice(unsigned int nIndex,
                              char deviceName[MAX_DEVICE_ID_LENGTH],
                              char deviceId[MAX_DEVICE_ID_LENGTH]) {
  return APOLLO_DONE;
}

//AV process
int ApolloEngineImpl::IAudioSetLocalAddress(int channel_id,
                                const int rtp_port,
                                const int rtcp_port,
                                bool is_IPv6) {
  
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IAudioSetLocalAddress(channel_id, rtp_port, rtcp_port, is_IPv6); });
  }
  
  rtc::SocketAddress rtp_socket_addr(rtc::IPAddress(INADDR_ANY), rtp_port);
  rtc::SocketAddress rtcp_socket_addr(rtc::IPAddress(INADDR_ANY), rtcp_port);

  cricket::VoiceChannel* voice_channel= (cricket::VoiceChannel*)FindChannel(channel_id);
  if (!voice_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  RtpTransport* transport = (RtpTransport*)voice_channel->rtp_transport();
  if (nullptr == transport) {
    return APOLLO_FAILED;
  }
  CreateRtpRtcpClient(transport, rtp_socket_addr, rtcp_socket_addr);

  return APOLLO_DONE;
}

int ApolloEngineImpl::IAudioSetRemoteAddress(int channel_id,
                                 const char *rtp_addr,
                                 const int rtp_port,
                                 const char *rtcp_addr,
                                 const int rtcp_port) {
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IAudioSetRemoteAddress(channel_id, rtp_addr, rtp_port, rtcp_addr, rtcp_port); });
  }
  
  cricket::VoiceChannel* voice_channel= (cricket::VoiceChannel*)FindChannel(channel_id);
  if (!voice_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  rtc::SocketAddress remote_rtp(rtp_addr, rtp_port);
  rtc::SocketAddress remote_rtcp(rtcp_addr, rtcp_port);
  SetRemoteAddr((RtpTransport*)voice_channel->rtp_transport(), remote_rtp, remote_rtcp);
  return APOLLO_DONE;
}

int ApolloEngineImpl::IAudioStartReceive(int channel_id) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IAudioStopReceive(int channel_id) {
  return APOLLO_DONE;
}

int ApolloEngineImpl::IAudioStartSend(int channel_id) {
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IAudioStartSend(channel_id); });
  }
  
  cricket::VoiceChannel* voice_channel= (cricket::VoiceChannel*)FindChannel(channel_id);
  if (!voice_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  RtpTransport* rtp_transport = (RtpTransport*)voice_channel->rtp_transport();
  if (!rtp_transport) {
    return INIT_NET_ENGINE_ERROR;
  }

  int nRet = SetTransportStat(rtp_transport, true);
  return nRet;
}

int ApolloEngineImpl::IAudioStopSend(int channel_id) {
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IAudioStopSend(channel_id); });
  }
  
  cricket::VoiceChannel* voice_channel= (cricket::VoiceChannel*)FindChannel(channel_id);
  if (!voice_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  RtpTransport* rtp_transport = (RtpTransport*)voice_channel->rtp_transport();
  if (!rtp_transport) {
    return INIT_NET_ENGINE_ERROR;
  }

  int nRet = SetTransportStat(rtp_transport, false);
  return nRet;
}
  
int ApolloEngineImpl::IVideoSetLocalAddress(int channel_id,
                                  const int rtp_port,
                                  const int rtcp_port,
                                  bool is_IPv6)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoSetLocalAddress(channel_id, rtp_port, rtcp_port, is_IPv6); });
  }
  
  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  rtc::SocketAddress rtp_socket_addr(rtc::IPAddress(INADDR_ANY), rtp_port);
  rtc::SocketAddress rtcp_socket_addr(rtc::IPAddress(INADDR_ANY), rtcp_port);
  
  RtpTransport* transport = (RtpTransport*)video_channel->rtp_transport();
  if (nullptr == transport) {
    return APOLLO_FAILED;
  }

  CreateRtpRtcpClient(transport, rtp_socket_addr, rtcp_socket_addr);
  
  return APOLLO_DONE;
}

int ApolloEngineImpl::IVideoSetRemoteAddress(int channel_id,
                                   const char *rtp_addr,
                                   const int rtp_port,
                                   const char *rtcp_addr,
                                   const int rtcp_port)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoSetRemoteAddress(channel_id, rtp_addr, rtp_port, rtcp_addr, rtcp_port); });
  }
  
  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  rtc::SocketAddress remote_rtp(rtp_addr, rtp_port);
  rtc::SocketAddress remote_rtcp(rtcp_addr, rtcp_port);
  SetRemoteAddr((RtpTransport*)video_channel->rtp_transport(), remote_rtp, remote_rtcp);
  return APOLLO_DONE;
}

int ApolloEngineImpl::IVideoStartReceive(int channel_id)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoStartReceive(channel_id); });
  }

  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  const std::vector<cricket::StreamParams>& streams = video_channel->local_streams();
   
  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
           transceiver = FindTransceiverByChannel(video_channel);
  if (!transceiver) {
     printf("not find video_transceiver_!");
     return NOT_INITIALIZED_ERROR;
  }
  transceiver->internal()->receiver_internal()->SetupMediaChannel(streams[0].first_ssrc());
  return APOLLO_DONE;
}

int ApolloEngineImpl::IVideoStopReceive(int channel_id)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoStopReceive(channel_id); });
  }
  return APOLLO_DONE;
}

int ApolloEngineImpl::IVideoStartSend(int channel_id)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoStartSend(channel_id); });
  }
  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  RtpTransport* rtp_transport = (RtpTransport*)video_channel->rtp_transport();
  if (!rtp_transport) {
    return INIT_NET_ENGINE_ERROR;
  }

  int nRet = SetTransportStat(rtp_transport, true);
  video_channel->Enable(true);
  return nRet;
}

int ApolloEngineImpl::IVideoStopSend(int channel_id)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoStopSend(channel_id); });
  }
  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  RtpTransport* rtp_transport = (RtpTransport*)video_channel->rtp_transport();
  if (!rtp_transport) {
    return INIT_NET_ENGINE_ERROR;
  }

  int nRet = SetTransportStat(rtp_transport, false);
  return nRet;
}

int ApolloEngineImpl::ISetVideoPreviewView(void *handle, void *view)
{
  return APOLLO_DONE;
}

//int ApolloEngineImpl::ISetVideoRender(int channel_id,
//                                            void *view,
//                                            ReturnVideoWidthHeightM videoResolutionCallback);
//{
//  return APOLLO_DONE;
//}

int ApolloEngineImpl::ICreateAudioChannel(int channel_id)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return ICreateAudioChannel(channel_id); });
  }
  return APOLLO_DONE;
}

int ApolloEngineImpl::ICreateVideoChannel(int channel_id)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return ICreateVideoChannel(channel_id); });
  }
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetAudioSSRC(int channel_id, unsigned int localssrc, unsigned int remotessrc)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return ISetAudioSSRC(channel_id, localssrc, remotessrc); });
  }
  
  cricket::VoiceChannel* voice_channel= (cricket::VoiceChannel*)FindChannel(channel_id);
  if (!voice_channel) {
    return -1;
  }
  
  bool ret = false;
  ret = voice_channel->media_channel()->AddSendStream(
                                cricket::StreamParams::CreateLegacy(localssrc));
  ret = voice_channel->media_channel()->AddRecvStream(
                                cricket::StreamParams::CreateLegacy(remotessrc));
  if (!ret) {
    //erro log
  }
  return APOLLO_DONE;
}

int ApolloEngineImpl::ISetVideoSSRC(int channel_id, unsigned int localssrc, unsigned int remotessrc)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return ISetVideoSSRC(channel_id, localssrc, remotessrc);});
  }
  
  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return -1;
  }
  
  bool ret = false;
  ret = video_channel->media_channel()->AddSendStream(
                                 cricket::StreamParams::CreateLegacy(localssrc));
  ret = video_channel->media_channel()->AddRecvStream(
                                  cricket::StreamParams::CreateLegacy(remotessrc));
  return APOLLO_DONE;
}

int ApolloEngineImpl::IRequestRemoteVideoSSRC(int channel_id, unsigned int ssrc)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IRequestRemoteVideoSSRC(channel_id, ssrc); });
  }
  
  return APOLLO_DONE;
}

int ApolloEngineImpl::IAudioSetCodec(int channel_id, const AudioCodecSetting& audio_codec)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IAudioSetCodec(channel_id, audio_codec); });
  }
  
  cricket::VoiceChannel* voice_channel_= (cricket::VoiceChannel*)FindChannel(channel_id);
  if (!voice_channel_) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
  std::unique_ptr<cricket::AudioContentDescription> audio(new cricket::AudioContentDescription());
  
  cricket::AudioCodecs filtered_codecs;
  cricket::AudioCodec codec(0,
    "pcmu",
    8000,
    64000,
    1);
  filtered_codecs.push_back(codec);
  audio->AddCodecs(filtered_codecs);

  cricket::StreamParams stream_param;
  stream_param.id = "audio";
  stream_param.GenerateSsrcs(1, false, false, &ssrc_generator_);
  stream_param.cname = "audio_rtcp";

  std::vector<std::string> stream_ids;
  stream_ids.push_back("audio0");
  stream_param.set_stream_ids(stream_ids);
  audio->AddStream(stream_param);

  audio->set_rtcp_mux(true);
  //audio->set_rtp_header_extensions(rtp_extensions);

  audio->set_protocol("RTP/AVPF");//kMediaProtocolAvpf
  audio->set_direction(RtpTransceiverDirection::kSendRecv);

  std::string error;
  voice_channel_->SetLocalContent(audio.get(), SdpType::kOffer, &error);
  voice_channel_->SetRemoteContent(audio.get(), SdpType::kAnswer, &error);

  const std::vector<cricket::StreamParams>& streams = voice_channel_->local_streams();
  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
            audio_transceiver_ = FindTransceiverByChannel(voice_channel_);
  if (!audio_transceiver_) {
    printf("not find audio_transceiver_!");
    return NOT_INITIALIZED_ERROR;
  }
  
  audio_transceiver_->internal()->sender_internal()->set_stream_ids(
    streams[0].stream_ids());
  audio_transceiver_->internal()->sender_internal()->SetSsrc(
    streams[0].first_ssrc());
  voice_channel_->Enable(true);
  return APOLLO_DONE;
}

int ApolloEngineImpl::IAudioSetReceiveCodec(int channel_id, const AudioCodecSetting& audio_codec)
{
  return APOLLO_DONE;
}

int ApolloEngineImpl::IVideoSetCodec(int channel_id, const VideoCodecSetting& video_codec)
{
  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return IVideoSetCodec(channel_id, video_codec); });
  }
  cricket::VideoChannel* video_channel= (cricket::VideoChannel*)FindChannel(channel_id);
  if (!video_channel) {
    return NOT_FOUND_CHANNEL_ERROR;
  }
  
//  VideoCodecs video_codecs;
//  RtpHeaderExtensions video_rtp_extensions;
//  channel_manager->GetSupportedVideoCodecs(&video_codecs);
//  channel_manager->GetSupportedVideoRtpHeaderExtensions(&video_rtp_extensions);
  
  std::unique_ptr<cricket::VideoContentDescription> video(new cricket::VideoContentDescription());
  cricket::VideoCodec kLocalCodec(96, "VP8");
  kLocalCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamRemb, cricket::kParamValueEmpty));
  kLocalCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamTransportCc, cricket::kParamValueEmpty));
  kLocalCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamCcm, cricket::kRtcpFbCcmParamFir));
  kLocalCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamNack, cricket::kParamValueEmpty));
  kLocalCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamNack, cricket::kRtcpFbNackParamPli));
  kLocalCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamLntf, cricket::kParamValueEmpty));

  
  kLocalCodec.packetization = cricket::kPacketizationParamRaw;
  //video->set_codecs({kVp8Codec, kVp9codec});
  video->set_codecs({kLocalCodec});
  //video->set_codecs(video_codecs);
  
  cricket::StreamParams stream_param;
  stream_param.id = "video";
  stream_param.GenerateSsrcs(1, false, false, &ssrc_generator_);
  stream_param.cname = "video_rtcp";

  std::vector<std::string> stream_ids;
  stream_ids.push_back("video0");
  stream_param.set_stream_ids(stream_ids);
  video->AddStream(stream_param);

  video->set_rtcp_mux(true);
  //audio->set_rtp_header_extensions(rtp_extensions);

  video->set_protocol("RTP/AVPF");//kMediaProtocolAvpf
  video->set_direction(RtpTransceiverDirection::kSendRecv);

  std::string error;
  video_channel->SetLocalContent(video.get(), SdpType::kOffer, &error);

  //remote codec
  cricket::VideoCodec kRemoteCodec(96, "VP8");
  kRemoteCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamRemb, cricket::kParamValueEmpty));
  kRemoteCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamTransportCc, cricket::kParamValueEmpty));
  kRemoteCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamCcm, cricket::kRtcpFbCcmParamFir));
  kRemoteCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamNack, cricket::kParamValueEmpty));
  kRemoteCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamNack, cricket::kRtcpFbNackParamPli));
  kRemoteCodec.AddFeedbackParam(cricket::FeedbackParam(cricket::kRtcpFbParamLntf, cricket::kParamValueEmpty));
  
  cricket::VideoContentDescription remote_video;
  remote_video.set_codecs({kRemoteCodec});
  video_channel->SetRemoteContent(&remote_video, SdpType::kAnswer, &error);
  
  const std::vector<cricket::StreamParams>& streams = video_channel->local_streams();
  
  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
            transceiver = FindTransceiverByChannel(video_channel);
  if (!transceiver) {
    printf("not find video_transceiver_!");
    return false;
  }
  
  transceiver->internal()->sender_internal()->set_stream_ids(
    streams[0].stream_ids());
  transceiver->internal()->sender_internal()->SetSsrc(
    streams[0].first_ssrc());
    
  //const std::vector<cricket::StreamParams>& remote_streams = video_channel->remote_streams();
  video_channel->media_channel()->AddRecvStream(
          cricket::StreamParams::CreateLegacy(streams[0].first_ssrc()));
  return APOLLO_DONE;
}

int ApolloEngineImpl::IVideoSetReceiveCodec(int channel_id, const VideoCodecSetting& video_codec)
{
  return APOLLO_DONE;
}

}//namespace webrtc
