#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t const capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string data )
{
  if ( is_closed() or available_capacity() == 0 or data.empty() ) {
    return;
  }
  if ( data.size() > available_capacity() ) {
    data.resize( available_capacity() );
  }
  bytes_pushed_ += data.size();
  bytes_available_ += data.size();

  buffers_.emplace_back( std::move( data ) );
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
  return capacity_ - bytes_available_;
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

uint64_t Writer::capacity() const
{
  return capacity_;
}

bool Reader::is_finished() const
{
  return closed_ and bytes_available_ == 0;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

string_view Reader::peek() const
{
  if ( buffers_.empty() ) {
    return {};
  }
  auto const& value = buffers_.front();
  // 不用一次性全部peek出去
  return { value.data() + read_prefix_, value.length() - read_prefix_ };
}

void Reader::pop( uint64_t len )
{
  len = std::min( len, bytes_available_ );
  bytes_available_ -= len;
  bytes_popped_ += len;
  while ( len != 0LU ) {
    std::size_t const size { buffers_.front().size() - read_prefix_ };
    if ( len < size ) {
      read_prefix_ += len;
      break; // with len = 0;
    }
    buffers_.pop_front();
    read_prefix_ = 0;
    len -= size;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return bytes_available_;
}
