#pragma once

#include <cstdint>

/**
 * The Wrap32 type represents a 32-bit unsigned integer that:
 *    1. starts at an arbitrary "zero point" (initial value), and
 *    2. wraps back to zero when it reaches 2^32 - 1.
 */
class Wrap32
{
public:
  /**
   * @brief 将一个 \code uint32_t\endcode 打包成Wrap32
   * @param[in] raw_value 要打包的值
   */
  explicit Wrap32( uint32_t const raw_value ) : raw_value_( raw_value ) {}

  /**
   * @brief Wrap an absolute sequence number \code n\endcode and the \code ISN\endcode .
   * @param[in] n 一个\code uint64_t\endcode 的绝对序列号
   * @param[in] isn Initial Sequence Number, 即 \code ISN\endcode
   * @return 返回 n 的 `seqno`
   * */
  static Wrap32 wrap( uint64_t n, Wrap32 isn );

  /**
   * @brief The unwrap method returns an absolute sequence number that wraps to this Wrap32.
   * @param[in] seqno 给定的 seqno/\code ISN\endcode
   * @param checkpoint 距离目标absolute seqno最近的checkpoint
   *
   * @note There are many possible absolute sequence numbers that all wrap to the same Wrap32.
   * The unwrap method should return the one that is closest to the checkpoint.
   * @return 返回目标absolute seqno
   */
  [[nodiscard]] uint64_t unwrap( Wrap32 seqno, uint64_t checkpoint ) const;

  Wrap32 operator+( uint32_t const n ) const { return Wrap32 { raw_value_ + n }; }
  bool operator==( const Wrap32& other ) const { return raw_value_ == other.raw_value_; }
  [[nodiscard]] uint32_t wrapped_value() const { return raw_value_; }

protected:
  uint32_t raw_value_ {};
};
using WrapU32 = Wrap32;
