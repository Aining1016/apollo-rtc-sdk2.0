
#include "media/engine/webrtc_video_engine.h"
#include "media/engine/webrtc_voice_engine.h"
#include "media/base/rtp_data_engine.h"
#include "media/base/media_constants.h"
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
#include "pc/media_session.h"
#include "apollo-rtc-sdk2.0/src/apollo_packet_transport.h"
#include "apollo_engine_impl.h"

namespace webrtc {

void ApolloEngineImpl::InitLog() {
    
    rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
    
    task_queue_factory_ = CreateDefaultTaskQueueFactory();
    
    event_log_factory_ = std::make_unique<RtcEventLogFactory>(task_queue_factory_.get());
}


void ApolloEngineImpl::InitThreads() {
    
    if (!network_thread_) {
      network_thread_ = rtc::Thread::CreateWithSocketServer();
      network_thread_->SetName("apollo_network_thread", nullptr);
      network_thread_->Start();
    }

    if (!worker_thread_) {
      worker_thread_ = rtc::Thread::Create();
      worker_thread_->SetName("apollo_worker_thread", nullptr);
      worker_thread_->Start();
    }
  
    if (!signaling_thread_) {
      worker_thread_ = rtc::Thread::Create();
      worker_thread_->SetName("apollo_signaling_thread", nullptr);
      worker_thread_->Start();
    }
    RTC_LOG(INFO) << "network_thread_="<<static_cast< void *>(network_thread_.get())<<" worker_thread_="<<static_cast< void *>(worker_thread_.get());
}


bool ApolloEngineImpl::CreateEventLog() {

  if (!worker_thread()->IsCurrent()) {
    return worker_thread()->Invoke<bool>( RTC_FROM_HERE,
            [=] { return CreateEventLog(); });
  }
    
  auto encoding_type = RtcEventLog::EncodingType::Legacy;
  //    if (IsTrialEnabled("WebRTC-RtcEventLogNewFormat")) {
  //      encoding_type = RtcEventLog::EncodingType::NewFormat;
  //    }
  if(event_log_factory_) {
      rtc_event_log_ = event_log_factory_->CreateRtcEventLog(encoding_type);
  } else {
      rtc_event_log_ = std::make_unique<RtcEventLogNull>();
  }
  return true;
}

void ApolloEngineImpl::CreateCall() {
    
    call_ = worker_thread_.get()->Invoke<std::unique_ptr<Call>>(
                      RTC_FROM_HERE,
                      rtc::Bind(&ApolloEngineImpl::CreateCall_w, this));
}

std::unique_ptr<Call> ApolloEngineImpl::CreateCall_w() {
    
    RTC_DCHECK_RUN_ON(worker_thread_.get());
        
    Call::Config call_config(rtc_event_log_.get());
        
    call_factory_ = CreateCallFactory();
        
    if (!channel_manager_->media_engine() || !call_factory_) {
        return nullptr;
    }
    call_config.audio_state =
            channel_manager_->media_engine()->voice().GetAudioState();

    FieldTrialParameter<DataRate> min_bandwidth("min", DataRate::kbps(30));
    FieldTrialParameter<DataRate> start_bandwidth("start", DataRate::kbps(300));
    FieldTrialParameter<DataRate> max_bandwidth("max", DataRate::kbps(2000));
    ParseFieldTrial({&min_bandwidth, &start_bandwidth, &max_bandwidth},
                        trials_->Lookup("WebRTC-PcFactoryDefaultBitrates"));

    call_config.bitrate_config.min_bitrate_bps =
            rtc::saturated_cast<int>(min_bandwidth->bps());
    call_config.bitrate_config.start_bitrate_bps =
            rtc::saturated_cast<int>(start_bandwidth->bps());
    call_config.bitrate_config.max_bitrate_bps =
            rtc::saturated_cast<int>(max_bandwidth->bps());

    call_config.fec_controller_factory = fec_controller_factory_.get();
    call_config.task_queue_factory = task_queue_factory_.get();
    call_config.network_state_predictor_factory =
            network_state_predictor_factory_.get();
    call_config.neteq_factory = neteq_factory_.get();

    //    if (IsTrialEnabled("WebRTC-Bwe-InjectedCongestionController")) {
    //      RTC_LOG(LS_INFO) << "Using injected network controller factory";
    //      call_config.network_controller_factory =
    //          injected_network_controller_factory_.get();
    //    } else {
    //      RTC_LOG(LS_INFO) << "Using default network controller factory";
    //    }

    call_config.trials = trials_.get();

    return std::unique_ptr<Call>(call_factory_->CreateCall(call_config));
}

void ApolloEngineImpl::InitMedia() {
  
  std::unique_ptr<TaskQueueFactory> task_queue_factory
    = CreateDefaultTaskQueueFactory();
  rtc::scoped_refptr<AudioEncoderFactory> audio_encoder_factory
    = CreateBuiltinAudioEncoderFactory();
  rtc::scoped_refptr<AudioDecoderFactory> audio_decoder_factory
    = CreateBuiltinAudioDecoderFactory();
  rtc::scoped_refptr<AudioDeviceModule> adm = nullptr;
  rtc::scoped_refptr<AudioMixer> audio_mixer = nullptr;
  rtc::scoped_refptr<AudioProcessing> audio_processing = AudioProcessingBuilder().Create();
  auto audio_engine = std::make_unique<cricket::WebRtcVoiceEngine>(
                        task_queue_factory.get(), std::move(adm),
                        std::move(audio_encoder_factory),
                        std::move(audio_decoder_factory),
                        std::move(audio_mixer),
                        std::move(audio_processing));
  
  std::unique_ptr<VideoEncoderFactory> video_encoder_factory
      = CreateBuiltinVideoEncoderFactory();
  std::unique_ptr<VideoDecoderFactory> video_decoder_factory
      = CreateBuiltinVideoDecoderFactory();
  auto video_engine = std::make_unique<cricket::WebRtcVideoEngine>(
                         std::move(video_encoder_factory),
                         std::move(video_decoder_factory));
    
  std::unique_ptr<cricket::MediaEngineInterface> media_engine_ = std::make_unique<cricket::CompositeMediaEngine>(std::move(audio_engine), std::move(video_engine));
  
  channel_manager_ = std::make_unique<cricket::ChannelManager>(
       std::move(media_engine_), std::make_unique<cricket::RtpDataEngine>(),
       worker_thread_.get(), network_thread_.get());
  
  channel_manager_->SetVideoRtxEnabled(true);
  if (!channel_manager_->Init()) {
    RTC_LOG(LERROR)<<"channel_manager_ init error";
  }
  video_bitrate_allocator_factory_ = CreateBuiltinVideoBitrateAllocatorFactory();

}

void ApolloEngineImpl::OnSentPacket_w(const rtc::SentPacket& sent_packet) {
  RTC_DCHECK_RUN_ON(worker_thread());
  RTC_DCHECK(call_);
  call_->OnSentPacket(sent_packet);
}

// TODO(steveanton): Perhaps this should be managed by the RtpTransceiver.
cricket::VideoChannel* ApolloEngineImpl::CreateVideoChannel(int& channel_id) {
  
  {
    rtc::CritScope cs(&channel_crit_);
    char c_mid[32] = {0};
    sprintf(c_mid, "video_channel_%d", channel_index_);
    const std::string mid(c_mid);
  }
  
  RtpTransportInternal* rtp_transport = CreateDtlsSrtpTransport();

  MediaTransportConfig media_transport_config;
  cricket::MediaConfig media_config;
  
  cricket::VideoChannel* video_channel = channel_manager_->CreateVideoChannel(
               call_.get(), media_config, rtp_transport,
               media_transport_config, worker_thread_.get(), "video_channel", false,//kDefaultSrtpRequired,
               CryptoOptions(), &ssrc_generator_, cricket::VideoOptions(),
               video_bitrate_allocator_factory_.get());
  if (!video_channel) {
    return nullptr;
  }

  video_channel->SignalSentPacket.connect(this,
                                          &ApolloEngineImpl::OnSentPacket_w);
  video_channel->SetRtpTransport(rtp_transport);
  //video_channel->Enable(true);
  
  rtc::CritScope cs(&channel_crit_);
  channel_map_[channel_index_] = (void*)video_channel;
  channel_id = channel_index_;
  channel_index_++;
  return video_channel;
}

// TODO(steveanton): Perhaps this should be managed by the RtpTransceiver.
cricket::VoiceChannel* ApolloEngineImpl::CreateVoiceChannel(int& channel_id) {
  {
    rtc::CritScope cs(&channel_crit_);
    char c_mid[32] = {0};
    sprintf(c_mid, "voice_channel_%d", channel_index_);
    const std::string mid(c_mid);
  }

  RtpTransportInternal* rtp_transport = CreateDtlsSrtpTransport();

  MediaTransportConfig media_transport_config;
  cricket::MediaConfig media_config;
  media_config.video.enable_cpu_adaptation = false;
  
  cricket::VoiceChannel* voice_channel = channel_manager_->CreateVoiceChannel(
               call_.get(), media_config, rtp_transport,
               media_transport_config,  worker_thread_.get(), "voice_channel", false,
               CryptoOptions(), &ssrc_generator_, cricket::AudioOptions());
  if (!voice_channel) {
    return nullptr;
  }
  
  voice_channel->SignalSentPacket.connect(this,
                                          &ApolloEngineImpl::OnSentPacket_w);
  voice_channel->SetRtpTransport(rtp_transport);
  //voice_channel->Enable(true);
  
  rtc::CritScope cs(&channel_crit_);
  channel_map_[channel_index_] = (void*)voice_channel;
  channel_id = channel_index_;
  channel_index_++;
  
  return voice_channel;
}

void* ApolloEngineImpl::FindChannel(int channel_id)
{
  rtc::CritScope cs(&channel_crit_);
  std::map<int, void*>::const_iterator it = channel_map_.find(channel_id);
  if (it == channel_map_.end()) {
    return NULL;
  }
  return it->second;
}

int ApolloEngineImpl::DeleteChannel(int channel_id, bool is_video) {
  // Write lock to make sure no one is using the channel.
  rtc::CritScope cs(&channel_crit_);
  std::map<int, void*>::iterator it = channel_map_.find(channel_id);
  if (it == channel_map_.end()) {
    //warning log
    return 0;
  }
  
  if (!is_video) {
    cricket::VoiceChannel* voice_channel = (cricket::VoiceChannel*)it->second;
    delete voice_channel;
  } else {
    cricket::VideoChannel* video_channel = (cricket::VideoChannel*)it->second;
    delete video_channel;
  }

  channel_map_.erase(it);
  return 0;
}

//改造
RtpTransportInternal* ApolloEngineImpl::CreateDtlsSrtpTransport() {
  
  if (!network_thread()->IsCurrent()) {
    return network_thread()->Invoke<RtpTransportInternal*>(
                                          RTC_FROM_HERE,
                                          [=] { return CreateDtlsSrtpTransport(); });
  }
  //DtlsSrtpTransport* dtls_srtp_transport = new DtlsSrtpTransport(/*rtcp_mux_required=*/true);
  RtpTransport* dtls_srtp_transport = new RtpTransport(true);
  rtc::ApolloPacketTransport *rtcp_trans = new rtc::ApolloPacketTransport("rtcp_trans");
  //rtcp_trans->SetWritable(true);
  rtc::ApolloPacketTransport *rtp_trans = new rtc::ApolloPacketTransport("rtp_trans");
  //rtp_trans->SetWritable(true);
  
  dtls_srtp_transport->SetRtpPacketTransport(rtp_trans);  // rtp ready
  dtls_srtp_transport->SetRtcpPacketTransport(rtcp_trans);  // rtcp ready
  return dtls_srtp_transport;
}

bool ApolloEngineImpl::SetRemoteAddr(RtpTransport* transport,
                   rtc::SocketAddress& remote_rtp,
                   rtc::SocketAddress& remote_rtcp) {
  
  rtc::ApolloPacketTransport* rtp =
          (rtc::ApolloPacketTransport*)transport->rtp_packet_transport();
  if (rtp) {
    rtp->SetRemoteAddr(remote_rtp);
  } else {
   //log
   return false;
  }
  rtc::ApolloPacketTransport* rtcp =
          (rtc::ApolloPacketTransport*)transport->rtcp_packet_transport();
  if (rtcp) {
     rtcp->SetRemoteAddr(remote_rtp);
  } else {
    //log
  }
  return true;
}

//Create track
rtc::scoped_refptr<VideoTrackInterface> ApolloEngineImpl::CreateVideoTrack(
    const std::string& id,
    VideoTrackSourceInterface* source) {
  rtc::scoped_refptr<VideoTrackInterface> track(
      VideoTrack::Create(id, source, worker_thread()));
  return VideoTrackProxy::Create(worker_thread(), worker_thread(), track);
}

rtc::scoped_refptr<AudioTrackInterface> ApolloEngineImpl::CreateAudioTrack(
    const std::string& id,
    AudioSourceInterface* source) {
  rtc::scoped_refptr<AudioTrackInterface> track(AudioTrack::Create(id, source));
  return AudioTrackProxy::Create(worker_thread(), track);
}

//Create source
rtc::scoped_refptr<AudioSourceInterface>
ApolloEngineImpl::CreateAudioSource(const cricket::AudioOptions& options) {
  rtc::scoped_refptr<LocalAudioSource> source(
      LocalAudioSource::Create(&options));
  return source;
}

// -----add transceiver
rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
ApolloEngineImpl::AddTransceiver( rtc::scoped_refptr<MediaStreamTrackInterface>& track,
                                  const std::vector<std::string>& stream_ids) {
  //RTC_DCHECK_RUN_ON(signaling_thread());
  if (!track) {
    //LOG_AND_RETURN_ERROR(RTCErrorType::INVALID_PARAMETER, "Track is null.");
  }
  
  cricket::MediaType media_type = cricket::MEDIA_TYPE_DATA;
  if (track->kind() == MediaStreamTrackInterface::kAudioKind) {
    media_type = cricket::MEDIA_TYPE_AUDIO;
  } else if(track->kind() == MediaStreamTrackInterface::kVideoKind) {
    media_type = cricket::MEDIA_TYPE_VIDEO;
  } else {
//    LOG_AND_RETURN_ERROR(RTCErrorType::INVALID_PARAMETER,
//                         "Track has invalid kind: " + track->kind());
  }
  
  std::string sender_id = rtc::CreateRandomUuid();
  rtc::scoped_refptr<RtpSenderProxyWithInternal<RtpSenderInternal>> sender
      = CreateSender(media_type, sender_id, track, stream_ids, {});
  rtc::scoped_refptr<RtpReceiverProxyWithInternal<RtpReceiverInternal>> receiver
      = CreateReceiver(media_type, rtc::CreateRandomUuid());
  rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>> transceiver
      = RtpTransceiverProxyWithInternal<RtpTransceiver>::Create(
                     worker_thread(),
                     new RtpTransceiver(sender, receiver, channel_manager_.get()));
  transceivers_.push_back(transceiver);
  transceiver->internal()->set_created_by_addtrack(true);
  transceiver->internal()->set_direction(RtpTransceiverDirection::kSendRecv);
  return transceiver;
}

rtc::scoped_refptr<RtpTransceiverProxyWithInternal<RtpTransceiver>>
ApolloEngineImpl::FindTransceiverByChannel(cricket::ChannelInterface* channel) {
  for (auto transceiver : transceivers_) {
    if (transceiver->internal()->channel() == channel) {
      return transceiver;
    }
  }
  return nullptr;
}

//rtc::scoped_refptr<RtpSenderProxyWithInternal<RtpSenderInternal>>
//ApolloEngineImpl::FindSenderById(const std::string& sender_id) const {
//  for (const auto& transceiver : transceivers_) {
//    for (auto sender : transceiver->internal()->senders()) {
//      if (sender->id() == sender_id) {
//        return sender;
//      }
//    }
//  }
//  return nullptr;
//}

rtc::scoped_refptr<RtpSenderProxyWithInternal<RtpSenderInternal>>
ApolloEngineImpl::CreateSender(cricket::MediaType media_type,
                           const std::string& id,
                           rtc::scoped_refptr<MediaStreamTrackInterface> track,
                           const std::vector<std::string>& stream_ids,
                           const std::vector<RtpEncodingParameters>& send_encodings) {
  
  //std::unique_ptr<StatsCollector> stats_;
  //stats_.reset(new StatsCollector(this));
  
  rtc::scoped_refptr<RtpSenderProxyWithInternal<RtpSenderInternal>> sender;
  if (media_type == cricket::MEDIA_TYPE_AUDIO) {
    RTC_DCHECK(!track ||
               (track->kind() == MediaStreamTrackInterface::kAudioKind));
    sender = RtpSenderProxyWithInternal<RtpSenderInternal>::Create(
               worker_thread(),
               AudioRtpSender::Create(worker_thread(), id, nullptr, this));
  } else {
    RTC_DCHECK_EQ(media_type, cricket::MEDIA_TYPE_VIDEO);
    RTC_DCHECK(!track ||
               (track->kind() == MediaStreamTrackInterface::kVideoKind));
    sender = RtpSenderProxyWithInternal<RtpSenderInternal>::Create(
               worker_thread(), VideoRtpSender::Create(worker_thread(), id, this));
  }
  bool set_track_succeeded = sender->SetTrack(track);
  RTC_DCHECK(set_track_succeeded);
  sender->internal()->set_stream_ids(stream_ids);
  sender->internal()->set_init_send_encodings(send_encodings);
  return sender;
}
                                                                
rtc::scoped_refptr<RtpReceiverProxyWithInternal<RtpReceiverInternal>>
ApolloEngineImpl::CreateReceiver(cricket::MediaType media_type,
                               const std::string& receiver_id) {
  
  rtc::scoped_refptr<RtpReceiverProxyWithInternal<RtpReceiverInternal>> receiver;
  if (media_type == cricket::MEDIA_TYPE_AUDIO) {
    receiver = RtpReceiverProxyWithInternal<RtpReceiverInternal>::Create(
         worker_thread(), new AudioRtpReceiver(worker_thread(), receiver_id,
                                                  std::vector<std::string>({})));
  } else {
    RTC_DCHECK_EQ(media_type, cricket::MEDIA_TYPE_VIDEO);
    receiver = RtpReceiverProxyWithInternal<RtpReceiverInternal>::Create(
         worker_thread(), new VideoRtpReceiver(worker_thread(), receiver_id,
                                                  std::vector<std::string>({})));
  }
  return receiver;
}

int ApolloEngineImpl::SetTransportStat(RtpTransport* rtp_transport, bool writable)
{
  if (!network_thread()->IsCurrent()) {
    return network_thread()->Invoke<int>(
                                  RTC_FROM_HERE,
                                  [=] { return SetTransportStat(rtp_transport, writable); });
  }
  rtc::ApolloPacketTransport* rtp_packet_trans = (rtc::ApolloPacketTransport*)rtp_transport->rtp_packet_transport();
  if (rtp_packet_trans) {
    rtp_packet_trans->SetWritable(writable);
  }
  rtc::ApolloPacketTransport* rtcp_packet_trans = (rtc::ApolloPacketTransport*)rtp_transport->rtp_packet_transport();
  if (rtcp_packet_trans) {
    rtcp_packet_trans->SetWritable(writable);
  }
  return APOLLO_DONE;
}

//device
void ApolloEngineImpl::SetAudioRecording(bool recording) {
  if (!worker_thread()->IsCurrent()) {
    worker_thread()->Invoke<void>(
        RTC_FROM_HERE,
        rtc::Bind(&ApolloEngineImpl::SetAudioRecording, this, recording));
    return;
  }
  auto audio_state = channel_manager_->media_engine()->voice().GetAudioState();
  audio_state->SetRecording(recording);
}

void ApolloEngineImpl::SetAudioPlayout(bool playout) {
  if (!worker_thread()->IsCurrent()) {
    worker_thread()->Invoke<void>(
        RTC_FROM_HERE,
        rtc::Bind(&ApolloEngineImpl::SetAudioPlayout, this, playout));
    return;
  }
  auto audio_state = channel_manager_->media_engine()->voice().GetAudioState();
  audio_state->SetPlayout(playout);
}

int ApolloEngineImpl::CreateRtpRtcpClient(RtpTransport* transport,
                                            rtc::SocketAddress rtp_addr,
                                            rtc::SocketAddress rtcp_addr) {
  
  if (!network_thread()->IsCurrent()) {
    return network_thread()->Invoke<int>( RTC_FROM_HERE,
            [=] { return CreateRtpRtcpClient(transport, rtp_addr, rtcp_addr); });
  }

  if (!transport) {
    return APOLLO_FAILED;
  }
  
  rtc::ApolloPacketTransport* rtp = (rtc::ApolloPacketTransport*)(transport->rtp_packet_transport());
  if (rtp) {
    rtp->CreateClient(rtp_addr);
  }
  
  rtc::ApolloPacketTransport* rtcp = (rtc::ApolloPacketTransport*)(transport->rtcp_packet_transport());
  if (rtcp) {
    rtcp->CreateClient(rtcp_addr);
  }
  return APOLLO_DONE;
}

}//namespace webrtc

//Part test -----end


//int ApolloRtcMediaImpl::AddTracks() {
//  //audio
//  const cricket::AudioOptions options = cricket::AudioOptions();
//  RTC_DCHECK(signaling_thread_->IsCurrent());
//  rtc::scoped_refptr<LocalAudioSource> source(
//                                              LocalAudioSource::Create(&options));
//  rtc::scoped_refptr<AudioTrackInterface> track(
//                                                AudioTrack::Create(kAudioLabel, source));
//  
//  // video
//  rtc::scoped_refptr<CapturerTrackSource> video_device =
//  CapturerTrackSource::Create();
//  if (video_device) {
//    rtc::scoped_refptr<VideoTrackInterface> track(
//                                                  VideoTrack::Create(kVideoLabel, video_device, _apiWorkThread/*unkonow?*/));
//  } else {
//    RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
//  }
//}
