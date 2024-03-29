//  Copyright (c) 2005-2007 Hartmut Kaiser 
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(JENKINS_HASH_HPP_SEP_08_2007_0102PM)
#define JENKINS_HASH_HPP_SEP_08_2007_0102PM

#include <cstdlib>
#include <boost/cstdint.hpp>
#include <boost/detail/endian.hpp>

#if defined(MPHF_HAS_SERIALIZATION_SUPPORT)
#include <boost/serialization/serialization.hpp>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace util
{
  namespace detail
  {
    // helper functions
    inline boost::uint32_t hashsize(boost::uint32_t n) 
    {
      return (boost::uint32_t)1 << n;
    }
    
    inline boost::uint32_t hashmask(boost::uint32_t n)
    {
      return hashsize(n)-1;
    }
    
    inline boost::uint32_t rot(boost::uint32_t x, boost::uint32_t k)
    {
      return (x << k) | (x >> (32 - k));
    }

    // ------------------------------------------------------------------------
    // mix -- mix 3 32-bit values reversibly.
    // 
    // This is reversible, so any information in (a,b,c) before mix() is
    // still in (a,b,c) after mix().
    // 
    // If four pairs of (a,b,c) inputs are run through mix(), or through
    // mix() in reverse, there are at least 32 bits of the output that
    // are sometimes the same for one pair and different for another pair.
    // This was tested for:
    // * pairs that differed by one bit, by two bits, in any combination
    //   of top bits of (a,b,c), or in any combination of bottom bits of
    //   (a,b,c).
    // * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
    //   the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
    //   is commonly produced by subtraction) look like a single 1-bit
    //   difference.
    // * the base values were pseudorandom, all zero but one bit set, or 
    //   all zero plus a counter that starts at zero.
    // 
    // Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
    // satisfy this are
    //     4  6  8 16 19  4
    //     9 15  3 18 27 15
    //    14  9  3  7 17  3
    // Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
    // for "differ" defined as + with a one-bit base and a two-bit delta.  I
    // used http://burtleburtle.net/bob/hash/avalanche.html to choose 
    // the operations, constants, and arrangements of the variables.
    // 
    // This does not achieve avalanche.  There are input bits of (a,b,c)
    // that fail to affect some output bits of (a,b,c), especially of a.  The
    // most thoroughly mixed value is c, but it doesn't really even achieve
    // avalanche in c.
    // 
    // This allows some parallelism.  Read-after-writes are good at doubling
    // the number of bits affected, so the goal of mixing pulls in the opposite
    // direction as the goal of parallelism.  I did what I could.  Rotates
    // seem to cost as much as shifts on every machine I could lay my hands
    // on, and rotates are much kinder to the top and bottom bits, so I used
    // rotates.
    // ------------------------------------------------------------------------
    inline 
    void mix(boost::uint32_t& a, boost::uint32_t& b, boost::uint32_t& c) 
    {
      a -= c; a ^= rot(c, 4); c += b; 
      b -= a; b ^= rot(a, 6); a += c; 
      c -= b; c ^= rot(b, 8); b += a; 
      a -= c; a ^= rot(c, 16); c += b; 
      b -= a; b ^= rot(a, 19); a += c; 
      c -= b; c ^= rot(b, 4); b += a; 
    }

    // ------------------------------------------------------------------------
    // final -- final mixing of 3 32-bit values (a,b,c) into c
    // 
    // Pairs of (a,b,c) values differing in only a few bits will usually
    // produce values of c that look totally different.  This was tested for
    // * pairs that differed by one bit, by two bits, in any combination
    //   of top bits of (a,b,c), or in any combination of bottom bits of
    //   (a,b,c).
    // * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
    //   the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
    //   is commonly produced by subtraction) look like a single 1-bit
    //   difference.
    // * the base values were pseudorandom, all zero but one bit set, or 
    //   all zero plus a counter that starts at zero.
    // 
    // These constants passed:
    //  14 11 25 16 4 14 24
    //  12 14 25 16 4 14 24
    // and these came close:
    //   4  8 15 26 3 22 24
    //  10  8 15 26 3 22 24
    //  11  8 15 26 3 22 24
    // ------------------------------------------------------------------------
    inline 
    void final(boost::uint32_t& a, boost::uint32_t& b, boost::uint32_t& c) 
    {
      c ^= b; c -= rot(b, 14); 
      a ^= c; a -= rot(c, 11); 
      b ^= a; b -= rot(a, 25); 
      c ^= b; c -= rot(b, 16); 
      a ^= c; a -= rot(c, 4);  
      b ^= a; b -= rot(a, 14); 
      c ^= b; c -= rot(b, 24); 
    }

  }
      
  /////////////////////////////////////////////////////////////////////////////
  /// The jenkins_hash class encapsulates a hash calculation function published 
  /// by Bob Jenkins here: http://www.burtleburtle.net/bob/c/lookup3.c
  class jenkins_hash
  {
  public:
    enum seedenum { seed = 1 };
    
    jenkins_hash() : seed_(0) {}
    
    explicit jenkins_hash(boost::uint32_t size) 
      : seed_(std::rand() % size) 
    {}
    
    explicit jenkins_hash(boost::uint32_t seedval, seedenum) 
      : seed_(seedval) 
    {}
    
    ~jenkins_hash() {}

    boost::uint32_t operator[](std::string const& key)
    {
      return hash(key.data(), key.size());
    }

	boost::uint32_t compute(std::string const& key)
    {
      return hash(key.data(), key.size());
    }
    
    void reseed(boost::uint32_t size)
    {
      seed_ = rand() % size;
    }
    
    void set_seed(boost::uint32_t seedval)
    {
      seed_ = seedval;
    }

  protected:    
    // ------------------------------------------------------------------------
    // hash() -- hash a variable-length key into a 32-bit value
    //   k       : the key (the unaligned variable-length array of bytes)
    //   length  : the length of the key, counting by bytes
    //   initval : can be any 4-byte value
    // Returns a 32-bit value.  Every bit of the key affects every bit of
    // the return value.  Two keys differing by one or two bits will have
    // totally different hash values.
    // 
    // The best hash table sizes are powers of 2.  There is no need to do
    // mod a prime (mod is sooo slow!).  If you need less than 32 bits,
    // use a bitmask.  For example, if you need only 10 bits, do
    //   h = (h & hashmask(10));
    // In which case, the hash table should have hashsize(10) elements.
    // 
    // If you are hashing n strings (uint8_t **)k, do it like this:
    //   for (i=0, h=0; i<n; ++i) h = hashlittle( k[i], len[i], h);
    // 
    // By Bob Jenkins, 2006.  bob_jenkins@burtleburtle.net.  You may use this
    // code any way you wish, private, educational, or commercial.  It's free.
    // 
    // Use for hash table lookup, or anything where one collision in 2^^32 is
    // acceptable.  Do NOT use for cryptographic purposes.
    // ------------------------------------------------------------------------
    boost::uint32_t hash (const void *key, std::size_t length)
    {
      boost::uint32_t a, b, c;                         /* internal state */
      union { const void *ptr; std::size_t i; } u;     /* needed for Mac Powerbook G4 */

      /* Set up the internal state */
      a = b = c = 0xdeadbeef + ((boost::uint32_t)length) + seed_;
      u.ptr = key;
      
#if defined(BOOST_LITTLE_ENDIAN)
      if ((u.i & 0x3) == 0) {
        const boost::uint32_t *k = (const boost::uint32_t *)key;         /* read 32-bit chunks */

        /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
        while (length > 12)
        {
          a += k[0];
          b += k[1];
          c += k[2];
          detail::mix(a, b, c);
          length -= 12;
          k += 3;
        }

        /*----------------------------- handle the last (probably partial) block */
        /* 
         * "k[2]&0xffffff" actually reads beyond the end of the string, but
         * then masks off the part it's not allowed to read.  Because the
         * string is aligned, the masked-off tail is in the same word as the
         * rest of the string.  Every machine with memory protection I've seen
         * does it on word boundaries, so is OK with this.  But VALGRIND will
         * still catch it and complain.  The masking trick does make the hash
         * noticeably faster for short strings (like English words).
         */
#ifndef VALGRIND

        switch(length)
        {
        case 12: c += k[2]; b+=k[1]; a+=k[0]; break;
        case 11: c += k[2] & 0xffffff; b+=k[1]; a+=k[0]; break;
        case 10: c += k[2] & 0xffff; b+=k[1]; a+=k[0]; break;
        case 9 : c += k[2] & 0xff; b+=k[1]; a+=k[0]; break;
        case 8 : b += k[1]; a+=k[0]; break;
        case 7 : b += k[1] & 0xffffff; a+=k[0]; break;
        case 6 : b += k[1] & 0xffff; a+=k[0]; break;
        case 5 : b += k[1] & 0xff; a+=k[0]; break;
        case 4 : a += k[0]; break;
        case 3 : a += k[0] & 0xffffff; break;
        case 2 : a += k[0] & 0xffff; break;
        case 1 : a += k[0] & 0xff; break;
        case 0 : return c;              /* zero length strings require no mixing */
        }

#else /* make valgrind happy */

        const boost::uint8_t* k8 = (const uint8_t *)k;
        switch(length)
        {
        case 12: c += k[2]; b+=k[1]; a+=k[0]; break;
        case 11: c += ((boost::uint32_t)k8[10])<<16;  /* fall through */
        case 10: c += ((boost::uint32_t)k8[9])<<8;    /* fall through */
        case 9 : c += k8[8];                   /* fall through */
        case 8 : b += k[1]; a+=k[0]; break;
        case 7 : b += ((boost::uint32_t)k8[6])<<16;   /* fall through */
        case 6 : b += ((boost::uint32_t)k8[5])<<8;    /* fall through */
        case 5 : b += k8[4];                   /* fall through */
        case 4 : a += k[0]; break;
        case 3 : a += ((boost::uint32_t)k8[2])<<16;   /* fall through */
        case 2 : a += ((boost::uint32_t)k8[1])<<8;    /* fall through */
        case 1 : a += k8[0]; break;
        case 0 : return c;
        }

#endif /* !valgrind */
      } 
      else if ((u.i & 0x1) == 0) {
        const boost::uint16_t *k = (const boost::uint16_t *)key;  /* read 16-bit chunks */

        /*--------------- all but last block: aligned reads and different mixing */
        while (length > 12)
        {
          a += k[0] + (((boost::uint32_t)k[1])<<16);
          b += k[2] + (((boost::uint32_t)k[3])<<16);
          c += k[4] + (((boost::uint32_t)k[5])<<16);
          detail::mix(a, b, c);
          length -= 12;
          k += 6;
        }

        /*----------------------------- handle the last (probably partial) block */
        const boost::uint8_t* k8 = (const boost::uint8_t *)k;
        switch(length)
        {
        case 12: c += k[4]+(((boost::uint32_t)k[5])<<16);
                 b += k[2]+(((boost::uint32_t)k[3])<<16);
                 a += k[0]+(((boost::uint32_t)k[1])<<16);
                 break;
        case 11: c += ((boost::uint32_t)k8[10])<<16;     /* fall through */
        case 10: c += k[4];
                 b += k[2]+(((boost::uint32_t)k[3])<<16);
                 a += k[0]+(((boost::uint32_t)k[1])<<16);
                 break;
        case 9 : c += k8[8];                      /* fall through */
        case 8 : b += k[2]+(((boost::uint32_t)k[3])<<16);
                 a += k[0]+(((boost::uint32_t)k[1])<<16);
                 break;
        case 7 : b += ((boost::uint32_t)k8[6])<<16;      /* fall through */
        case 6 : b += k[2];
                 a += k[0]+(((boost::uint32_t)k[1])<<16);
                 break;
        case 5 : b += k8[4];                      /* fall through */
        case 4 : a += k[0]+(((boost::uint32_t)k[1])<<16);
                 break;
        case 3 : a += ((boost::uint32_t)k8[2])<<16;      /* fall through */
        case 2 : a += k[0];
                 break;
        case 1 : a += k8[0];
                 break;
        case 0 : return c;                     /* zero length requires no mixing */
        }
      } else 
#endif // BOOST_ENDIAN_LITTLE
      {                        /* need to read the key one byte at a time */
        const boost::uint8_t *k = (const boost::uint8_t *)key;

        /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
        while (length > 12)
        {
          a += k[0];
          a += ((boost::uint32_t)k[1])<<8;
          a += ((boost::uint32_t)k[2])<<16;
          a += ((boost::uint32_t)k[3])<<24;
          b += k[4];
          b += ((boost::uint32_t)k[5])<<8;
          b += ((boost::uint32_t)k[6])<<16;
          b += ((boost::uint32_t)k[7])<<24;
          c += k[8];
          c += ((boost::uint32_t)k[9])<<8;
          c += ((boost::uint32_t)k[10])<<16;
          c += ((boost::uint32_t)k[11])<<24;
          detail::mix(a, b, c);
          length -= 12;
          k += 12;
        }

        /*-------------------------------- last block: affect all 32 bits of (c) */
        switch(length)                   /* all the case statements fall through */
        {
        case 12: c += ((boost::uint32_t)k[11])<<24;
        case 11: c += ((boost::uint32_t)k[10])<<16;
        case 10: c += ((boost::uint32_t)k[9])<<8;
        case 9 : c += k[8];
        case 8 : b += ((boost::uint32_t)k[7])<<24;
        case 7 : b += ((boost::uint32_t)k[6])<<16;
        case 6 : b += ((boost::uint32_t)k[5])<<8;
        case 5 : b += k[4];
        case 4 : a += ((boost::uint32_t)k[3])<<24;
        case 3 : a += ((boost::uint32_t)k[2])<<16;
        case 2 : a += ((boost::uint32_t)k[1])<<8;
        case 1 : a += k[0];
                 break;
        case 0 : return c;
        }
      }

      detail::final(a, b, c);
      return c;
    }
    
  private:
    boost::uint32_t seed_;

#if defined(MPHF_HAS_SERIALIZATION_SUPPORT)
    // serialization support    
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) 
    {
      ar & seed_;
    }
#endif
  };
}

#endif
