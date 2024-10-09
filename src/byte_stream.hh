#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class Reader;
class Writer;

class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  Reader& reader();
  Writer& writer();
  void set_error() { error_ = true; } // Signal that the stream suffered an error.

  [[nodiscard]] const Reader& reader() const;
  [[nodiscard]] const Writer& writer() const;
  [[nodiscard]] bool has_error() const { return error_; } // Has the stream had an error?
  [[nodiscard]] size_t capacity() const { return capacity_; }

protected:
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  bool error_ {};
  bool closed_ {};
  std::size_t capacity_;
  std::size_t read_index_ {};
  std::size_t write_index_ {};
  std::vector<char> buffers_ {};
};

class Writer : public ByteStream
{
public:
  void push( std::string const& data ); // Push data to stream, but only as much as available capacity allows.
  void close();                  // Signal that the stream has reached its ending. Nothing more will be written.

  [[nodiscard]] bool is_full() const;                // Has the stream been closed?
  [[nodiscard]] bool is_closed() const;              // Has the stream been closed?
  [[nodiscard]] uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?
  [[nodiscard]] uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream
  [[nodiscard]] uint64_t write_index() const { return write_index_; }
  [[nodiscard]] uint64_t right_bound() const { return write_index_ + available_capacity(); }
};

class Reader : public ByteStream
{
public:
  void pop( uint64_t len ); // Remove `len` bytes from the buffer

  [[nodiscard]] std::string_view peek() const;   // Peek at the next bytes in the buffer
  [[nodiscard]] bool is_finished() const;        // Is the stream finished (closed and fully popped)?
  [[nodiscard]] uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)
  [[nodiscard]] uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream
};

/*
 * read: A (provided) helper function that peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read( Reader& reader, uint64_t len, std::string& out );
