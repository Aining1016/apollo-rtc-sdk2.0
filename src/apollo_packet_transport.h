/*
 *  Copyright 2017 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef APOLLO_PACKET_TRANSPORT_H_
#define APOLLO_PACKET_TRANSPORT_H_

#include <map>
#include <string>
#include <vector>

#include "p2p/base/packet_transport_internal.h"
#include "p2p/base/basic_packet_socket_factory.h"
#include "rtc_base/async_invoker.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/async_udp_socket.h"
#include "rtc_base/constructor_magic.h"
#include "rtc_base/critical_section.h"
#include "rtc_base/fake_clock.h"

namespace rtc {

// Used to simulate a packet-based transport.
class ApolloPacketTransport : public PacketTransportInternal {
 public:
  explicit ApolloPacketTransport(const std::string& transport_name);
  ~ApolloPacketTransport() override;

  // If async, will send packets by "Post"-ing to message queue instead of
  // synchronously "Send"-ing.
  void SetAsync(bool async) { async_ = async; }
  void SetAsyncDelay(int delay_ms) { async_delay_ms_ = delay_ms; }
  
  // SetWritable, SetReceiving and SetDestination are the main methods that can
  // be used for testing, to simulate connectivity or lack thereof.
  void SetWritable(bool writable) { set_writable(writable); }
  void SetReceiving(bool receiving) { set_receiving(receiving); }
  
  // Apollo PacketTransportInternal implementation.
  const std::string& transport_name() const override {
    return transport_name_;
  }
  bool writable() const override { return writable_; }
  bool receiving() const override { return receiving_; }
  
  int SendPacket(const char* data,
                 size_t len,
                 const PacketOptions& options,
                 int flags) override;

  int SetOption(Socket::Option opt, int value) override;
  bool GetOption(Socket::Option opt, int* value) override;

  int GetError() override;
  void SetError(int error);

  const CopyOnWriteBuffer* last_sent_packet();

  absl::optional<NetworkRoute> network_route() const override;
  void SetNetworkRoute(absl::optional<NetworkRoute> network_route);

 private:
  void set_writable(bool writable);
  void set_receiving(bool receiving);

  CopyOnWriteBuffer last_sent_packet_;
  AsyncInvoker invoker_;
  std::string transport_name_;
  bool async_ = false;
  int async_delay_ms_ = 0;
  bool writable_ = false;
  bool receiving_ = false;

  std::map<Socket::Option, int> options_;
  int error_ = 0;

  absl::optional<NetworkRoute> network_route_;
  
  //----------------------------------------------------------------
  //Socket client implement. Contain TCP and UDP from test_client.cc
 public:
  // Records the contents of a packet that was received.
  struct Packet {
    Packet(const SocketAddress& a,
           const char* b,
           size_t s,
           int64_t packet_time_us);
    Packet(const Packet& p);
    virtual ~Packet();
    
    SocketAddress addr;
    char* buf;
    size_t size;
    int64_t packet_time_us;
  };
  
  // Default timeout for NextPacket reads.
  static const int kTimeoutMs = 5000;
  
  // Create a Apollo client that will use a fake clock. NextPacket needs to wait
  // for a packet to be received, and thus it needs to advance the fake clock
  // if the Apollo is using one, rather than just sleeping.
 bool CreateClient(const SocketAddress& local_addr);
 void SetRemoteAddr(const SocketAddress& remote_addr) {
  remote_addr_ = remote_addr;
 }
  
  SocketAddress address() const { return socket_->GetLocalAddress(); }
  SocketAddress remote_address() const { return socket_->GetRemoteAddress(); }
  
  // Checks that the socket moves to the specified connect state.
  bool CheckConnState(AsyncPacketSocket::State state);
  
  // Checks that the socket is connected to the remote side.
  bool CheckConnected() {
    return CheckConnState(AsyncPacketSocket::STATE_CONNECTED);
  }
  
  // Sends using the clients socket.
  int Send(const char* buf, size_t size, const rtc::PacketOptions& options);
  
  // Sends using the clients socket to the given destination.
  int SendTo(const char* buf,
             size_t size,
             const SocketAddress& dest,
             const rtc::PacketOptions& options);
  
  // Returns the next packet received by the client or null if none is received
  // within the specified timeout.
  std::unique_ptr<Packet> NextPacket(int timeout_ms);
  
  // Checks that the next packet has the given contents. Returns the remote
  // address that the packet was sent from.
  bool CheckNextPacket(const char* buf, size_t len, SocketAddress* addr);
  
  // Checks that no packets have arrived or will arrive in the next second.
  bool CheckNoPacket();
    
  bool ready_to_send() const { return ready_to_send_count() > 0; }
  
  // How many times SignalReadyToSend has been fired.
  int ready_to_send_count() const { return ready_to_send_count_; }
 
private:
  // Timeout for reads when no packet is expected.
  static const int kNoPacketTimeoutMs = 1000;
  // Workaround for the fact that AsyncPacketSocket::GetConnState doesn't exist.
  Socket::ConnState GetState();
  // Slot for packets read on the socket.
  void OnReadPacket(AsyncPacketSocket* socket,
                const char* buf,
                size_t len,
                const SocketAddress& remote_addr,
                const int64_t& packet_time_us);
  void OnReadyToSend(AsyncPacketSocket* socket);
  bool CheckTimestamp(int64_t packet_timestamp);
  void AdvanceTime(int ms);
  
 private:
 
  CriticalSection crit_;
  std::unique_ptr<AsyncPacketSocket> socket_ = nullptr;
  ThreadProcessingFakeClock* fake_clock_ = nullptr;
  std::vector<std::unique_ptr<Packet>> packets_;
  BasicPacketSocketFactory socket_factory_;
  int ready_to_send_count_ = 0;
  int64_t prev_packet_timestamp_ = -1;
  SocketAddress local_addr_;
  SocketAddress remote_addr_;
};

}  // namespace rtc

#endif  // APOLLO_PACKET_TRANSPORT_H_
