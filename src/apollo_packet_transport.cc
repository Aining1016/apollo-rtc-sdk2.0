/*
 *  Copyright 2017 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <string.h>

#include <memory>
#include <utility>

//#include "rtc_base/gunit.h"
#include "rtc_base/thread.h"
#include "rtc_base/time_utils.h"

#include "apollo_packet_transport.h"

namespace rtc {

  ApolloPacketTransport::ApolloPacketTransport(const std::string& transport_name)
    : transport_name_(transport_name) {
    fake_clock_ = new ThreadProcessingFakeClock();
  }
  
  ApolloPacketTransport::~ApolloPacketTransport() {
    if (fake_clock_) {
      delete fake_clock_;
    }
  }

  int ApolloPacketTransport::SendPacket(const char* data,
                                         size_t len,
                                         const PacketOptions& options,
                                         int flags) {

    last_sent_packet_ = CopyOnWriteBuffer(data, len);
    //TCP or UDP?
    size_t length = 0;
    if (async_) {
      invoker_.AsyncInvokeDelayed<void>(
          RTC_FROM_HERE, Thread::Current(),
          Bind(&ApolloPacketTransport::SendTo, this, data, len, remote_addr_, options),
          async_delay_ms_);
    } else {
      length = SendTo(data, len, remote_addr_, options);
    }
    
    SentPacket sent_packet(options.packet_id, TimeMillis());
    SignalSentPacket(this, sent_packet);
    return static_cast<int>(length);
  }

  int ApolloPacketTransport::SetOption(Socket::Option opt, int value) {
    options_[opt] = value;
    return 0;
  }

  bool ApolloPacketTransport::GetOption(Socket::Option opt, int* value) {
    auto it = options_.find(opt);
    if (it == options_.end()) {
      return false;
    }
    *value = it->second;
    return true;
  }

  int ApolloPacketTransport::GetError() { return error_; }
  void ApolloPacketTransport::SetError(int error) { error_ = error; }

  const CopyOnWriteBuffer* ApolloPacketTransport::last_sent_packet() { return &last_sent_packet_; }

  absl::optional<NetworkRoute> ApolloPacketTransport::network_route() const {
    return network_route_;
  }
  void ApolloPacketTransport::SetNetworkRoute(absl::optional<NetworkRoute> network_route) {
    network_route_ = network_route;
    SignalNetworkRouteChanged(network_route);
  }

  void ApolloPacketTransport::set_writable(bool writable) {
    if (writable_ == writable) {
      return;
    }
    writable_ = writable;
    if (writable_) {
      SignalReadyToSend(this);
    }
    SignalWritableState(this);
  }

  void ApolloPacketTransport::set_receiving(bool receiving) {
    if (receiving_ == receiving) {
      return;
    }
    receiving_ = receiving;
    SignalReceivingState(this);
  }
  
 //-----------------Client ---------------
  bool ApolloPacketTransport::CreateClient(const SocketAddress& local_addr) {

    AsyncPacketSocket* async_socket =
            socket_factory_.CreateUdpSocket(local_addr, 0, 0);
    if (nullptr == async_socket) {
      RTC_LOG(LS_ERROR) << "CreateUdpSocket failed";
      return false;
    }
    
    socket_.reset(async_socket);
    for (std::map<Socket::Option, int>::const_iterator it = options_.begin();
          it != options_.end(); ++it) {
      socket_->SetOption(it->first, it->second);
    }
    socket_->SignalReadPacket.connect(this, &ApolloPacketTransport::OnReadPacket);
    socket_->SignalReadyToSend.connect(this, &ApolloPacketTransport::OnReadyToSend);
    local_addr_ = local_addr;
    return true;
  }
  
  bool ApolloPacketTransport::CheckConnState(AsyncPacketSocket::State state) {
    // Wait for our timeout value until the socket reaches the desired state.
    int64_t end = TimeAfter(kTimeoutMs);
    while (socket_->GetState() != state && TimeUntil(end) > 0) {
      AdvanceTime(1);
    }
    return (socket_->GetState() == state);
  }
  
  int ApolloPacketTransport::Send(const char* buf, size_t size, const rtc::PacketOptions& options) {
    return socket_->Send(buf, size, options);
  }
  
  int ApolloPacketTransport::SendTo(const char* buf,
                           size_t size,
                           const SocketAddress& dest,
                           const rtc::PacketOptions& options) {
    //printf("zhangnApollo send :%s %s\n", buf, dest.ToString().c_str());
    return socket_->SendTo(buf, size, dest, options);
  }
  
  std::unique_ptr<ApolloPacketTransport::Packet> ApolloPacketTransport::NextPacket(int timeout_ms) {
    // If no packets are currently available, we go into a get/dispatch loop for
    // at most timeout_ms.  If, during the loop, a packet arrives, then we can
    // stop early and return it.
    
    // Note that the case where no packet arrives is important.  We often want to
    // Apollo that a packet does not arrive.
    
    // Note also that we only try to pump our current thread's message queue.
    // Pumping another thread's queue could lead to messages being dispatched from
    // the wrong thread to non-thread-safe objects.
    
    int64_t end = TimeAfter(timeout_ms);
    while (TimeUntil(end) > 0) {
      {
        CritScope cs(&crit_);
        if (packets_.size() != 0) {
          break;
        }
      }
      AdvanceTime(1);
    }
    
    // Return the first packet placed in the queue.
    std::unique_ptr<Packet> packet;
    CritScope cs(&crit_);
    if (packets_.size() > 0) {
      packet = std::move(packets_.front());
      packets_.erase(packets_.begin());
    }
    
    return packet;
  }
  
  bool ApolloPacketTransport::CheckNextPacket(const char* buf,
                                     size_t size,
                                     SocketAddress* addr) {
    bool res = false;
    std::unique_ptr<Packet> packet = NextPacket(kTimeoutMs);
    if (packet) {
      res = (packet->size == size && memcmp(packet->buf, buf, size) == 0 &&
             CheckTimestamp(packet->packet_time_us));
      if (addr)
        *addr = packet->addr;
    }
    return res;
  }
  
  bool ApolloPacketTransport::CheckTimestamp(int64_t packet_timestamp) {
    bool res = true;
    if (packet_timestamp == -1) {
      res = false;
    }
    if (prev_packet_timestamp_ != -1) {
      if (packet_timestamp < prev_packet_timestamp_) {
        res = false;
      }
    }
    prev_packet_timestamp_ = packet_timestamp;
    return res;
  }
  
  void ApolloPacketTransport::AdvanceTime(int ms) {
    // If the Apollo is using a fake clock, we must advance the fake clock to
    // advance time. Otherwise, ProcessMessages will work.
    if (fake_clock_) {
      //SIMULATED_WAIT(false, ms, *fake_clock_);
    } else {
      Thread::Current()->ProcessMessages(1);
    }
  }
  
  bool ApolloPacketTransport::CheckNoPacket() {
    return NextPacket(kNoPacketTimeoutMs) == nullptr;
  }
  
  void ApolloPacketTransport::OnReadPacket(AsyncPacketSocket* socket,
                              const char* buf,
                              size_t size,
                              const SocketAddress& remote_addr,
                              const int64_t& packet_time_us) {
    CritScope cs(&crit_);
//    packets_.push_back(
//                       std::make_unique<Packet>(remote_addr, buf, size, packet_time_us));
    SignalReadPacket(this, buf, size, packet_time_us, 0);
  }
  
  void ApolloPacketTransport::OnReadyToSend(AsyncPacketSocket* socket) {
    ++ready_to_send_count_;
  }
  
  ApolloPacketTransport::Packet::Packet(const SocketAddress& a,
                                         const char* b,
                                         size_t s,
                                         int64_t packet_time_us)
  : addr(a), buf(0), size(s), packet_time_us(packet_time_us) {
    buf = new char[size];
    memcpy(buf, b, size);
  }
  
  ApolloPacketTransport::Packet::Packet(const Packet& p)
  : addr(p.addr), buf(0), size(p.size), packet_time_us(p.packet_time_us) {
    buf = new char[size];
    memcpy(buf, p.buf, size);
  }
  
  ApolloPacketTransport::Packet::~Packet() {
    delete[] buf;
  }
  
}  // namespace rtc

