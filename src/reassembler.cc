#include "reassembler.hh"

#include <ranges>
#include <string>

#define RAII_CLOSE                                                                                                 \
  [&]() noexcept -> void {                                                                                         \
    if ( end_index_ == writer().write_index() ) {                                                                  \
      writer().close();                                                                                            \
    }                                                                                                              \
  }

auto Reassembler::split( uint64_t pos ) noexcept
{
  auto const& it { buf_.lower_bound( pos ) };
  if ( it not_eq buf_.end() and it->first == pos ) {
    return it;
  }
  if ( it == buf_.begin() ) {
    return it;
  }
  if ( auto& [offset, frame] { *std::prev( it ) }; offset + frame.length() > pos ) {
    const auto& res { buf_.emplace_hint( it, pos, frame.substr( pos - offset ) ) };
    frame.resize( pos - offset );
    return res;
  }
  return it;
}

bool Reassembler::writable() const noexcept
{
  return not( writer().is_closed() or writer().is_full() );
}

void Reassembler::insert( uint64_t offset, std::string data, bool is_eof )
{
  std::string_view sv { data };
  if ( sv.empty() ) [[unlikely]] {
    if ( end_index_ == UINT64_MAX and is_eof ) {
      end_index_ = offset;
    }
    return RAII_CLOSE();
  }
  if ( written( sv, offset ) or not writable() ) [[unlikely]] {
    return;
  }
  if ( offset < writer().write_index() ) {
    sv.remove_prefix( writer().write_index() - offset );
    offset = writer().write_index();
  }
  if ( offset + sv.length() > writer().right_bound() ) {
    sv.remove_suffix( writer().right_bound() - offset );
    is_eof = false;
  }
  if ( end_index_ == UINT64_MAX and is_eof ) {
    end_index_ = offset + sv.length();
  }

  const auto upper { split( offset + sv.length() ) };
  const auto lower { split( offset ) };
  for ( std::string const& str : std::ranges::subrange { lower, upper } | std::views::values ) {
    total_pending_ -= str.size();
  }
  total_pending_ += sv.length();
  buf_.emplace_hint( buf_.erase( lower, upper ), offset, sv );

  while ( not buf_.empty() ) {
    auto& [idx, payload] { *buf_.begin() };
    if ( idx not_eq writer().bytes_pushed() ) {
      break;
    }
    total_pending_ -= payload.length();
    writer().push( std::move( payload ) );
    buf_.erase( buf_.begin() );
  }
  return RAII_CLOSE();
}

bool Reassembler::written( std::string_view const data, size_t const offset ) const noexcept
{
  return offset + data.length() <= writer().write_index() or offset >= writer().right_bound();
}

uint64_t Reassembler::bytes_pending() const
{
  return total_pending_;
}
