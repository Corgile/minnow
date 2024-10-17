#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t const n, Wrap32 const isn )
{
  return isn + n;
}

uint64_t Wrap32::unwrap( Wrap32 const seqno, uint64_t const checkpoint ) const
{
  if ( raw_value_ >= checkpoint ) {
    return raw_value_ - seqno.wrapped_value();
  }
  auto const wrapped_checkpoint { wrap( checkpoint, seqno ).wrapped_value() };

  uint32_t const counter_clockwise { wrapped_checkpoint - raw_value_ };
  uint32_t const clockwise { raw_value_ - wrapped_checkpoint };

  if ( counter_clockwise < clockwise ) {
    return checkpoint - counter_clockwise;
  }
  return checkpoint + clockwise;
}
