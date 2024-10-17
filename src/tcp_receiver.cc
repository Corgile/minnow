#include "tcp_receiver.hh"

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) { reader().set_error(); }
  if ( writer().has_error() ) { return; }

  if ( not zero_point_.has_value() ) {
    if ( not message.SYN ) { return; }
    zero_point_.emplace( message.seqno );
  }
  uint64_t const checkpoint { writer().bytes_pushed() + 1 /* SYN */ };
  uint64_t const abs_seqno { message.seqno.unwrap( zero_point_.value(), checkpoint ) };
  uint64_t const stream_index { abs_seqno + static_cast<uint64_t>( message.SYN ) - 1 /* SYN */ };
  reassembler_.insert( stream_index, std::move( message.payload ), message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  uint16_t const window_size = std::min( writer().available_capacity(), (size_t)UINT16_MAX );
  if ( zero_point_.has_value() ) {
    uint64_t const ack_for_seqno { writer().bytes_pushed() + 1 + static_cast<uint64_t>( writer().is_closed() ) };
    return { Wrap32::wrap( ack_for_seqno, zero_point_.value() ), window_size, writer().has_error() };
  }
  return { std::nullopt, window_size, writer().has_error() };
}
