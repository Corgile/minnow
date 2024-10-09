#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t const capacity ) : capacity_( capacity )
{
  buffers_.resize( capacity_ );
}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string const& data )
{
  std::string_view sv { data };
  if ( is_closed() or available_capacity() == 0 or sv.empty() ) {
    return;
  }
  if ( sv.size() > available_capacity() ) {
    sv.remove_suffix( sv.size() - available_capacity() );
  }
  size_t i = 0, idx = static_cast<long>(write_index_ % capacity_);
  auto n { ( sv.length() + 7 ) >> 3 };
  switch ( sv.length() & 0x7 ) { // NOLINT
    case 0: do { buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 7:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 6:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 5:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 4:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 3:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 2:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_; [[fallthrough]];
    case 1:      buffers_[idx++] = sv[i++], idx -= (idx == capacity_) * capacity_;
            } while ( --n );
  }
  write_index_ += sv.length();
}

void Writer::close()
{
  closed_ = true;
}
bool Writer::is_full() const
{
  return available_capacity() == 0UL;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - ( write_index_ - read_index_ );
}

uint64_t Writer::bytes_pushed() const
{
  return write_index_;
}

bool Reader::is_finished() const
{
  return closed_ and bytes_buffered() == 0;
}

uint64_t Reader::bytes_popped() const
{
  return read_index_;
}

string_view Reader::peek() const
{
  auto const real_r { read_index_ % capacity_ };
  auto const real_w { write_index_ % capacity_ };
  long const r_idx { static_cast<long>(read_index_ % capacity_) };
  size_t len {};
  if (real_r < real_w) {
    len = real_w - real_r;
  } else {
    len = (bytes_buffered() != 0) * ( capacity_ - real_r );
  }
  return { &*(buffers_.begin() + r_idx),  len};
}

void Reader::pop( uint64_t len )
{
  len = std::min( len, bytes_buffered() );
  read_index_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  return write_index_ - read_index_;
}
