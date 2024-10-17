#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <queue>

#define REM [[nodiscard]]

struct RetransmissionTimer
{
  /// 重传计时器
  explicit RetransmissionTimer( uint64_t const initial_RTO_ms ) : RTO_ms_ { initial_RTO_ms } {}
  REM constexpr auto is_active() const noexcept -> bool { return is_active_; }
  REM constexpr auto is_expired() const noexcept -> bool { return is_active_ and timer_ >= RTO_ms_; }
  /// 指数回退
  constexpr void exponential_backoff() noexcept { RTO_ms_ *= 2; }
  constexpr void reload( uint64_t const initial_RTO_ms ) noexcept { RTO_ms_ = initial_RTO_ms, reset(); };
  constexpr void reset() noexcept { timer_ = 0; }
  constexpr void start() noexcept { is_active_ = true, reset(); }
  constexpr void stop() noexcept { is_active_ = false, reset(); }

  constexpr auto tick( uint64_t const ms_since_last_tick ) noexcept -> RetransmissionTimer&
  {
    timer_ += is_active_ ? ms_since_last_tick : 0;
    return *this;
  }

private:
  bool is_active_ {};
  uint64_t RTO_ms_;
  uint64_t timer_ {};
};

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 const isn, uint64_t const initial_RTO_ms )
    : input_ { std::move( input ) }, isn_ { isn }, initial_RTO_ms_ { initial_RTO_ms }, timer_ { initial_RTO_ms }
  {}

  /* Generate an empty TCPSenderMessage */
  REM TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  REM uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  REM uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  REM const Writer& writer() const { return input_.writer(); }
  // Access input stream reader, but const-only (can't read from outside)
  REM const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  RetransmissionTimer timer_;

  bool SYN_sent_ {};
  bool FIN_sent_ {};

  uint64_t next_abs_seqno_ {};
  uint64_t ack_abs_seqno_ {};
  uint16_t window_size_ { 1 };
  std::queue<TCPSenderMessage> outstanding_message_ {};

  uint64_t total_outstanding_ {};
  uint64_t total_retransmission_ {};
};

#undef REM