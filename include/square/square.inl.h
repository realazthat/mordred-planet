/*
    Copyright (c) 2012 Azriel Fasten azriel.fasten@gmail.com

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/


#include <square/square.h>
#include "logic_utility.h"

#include <vector>
#include <boost/range/irange.hpp>
#include <boost/utility.hpp>

#include <bitset>

//###################################################################
//#### edge_t
//###################################################################

namespace square{

SQUARE_INLINE
edge_t::
edge_t(const direction_t& direction)
  : mdirection(direction)
{

}


SQUARE_INLINE
boost::uint8_t
edge_t::index() const
{
  return mdirection.index();
}


SQUARE_INLINE
const boost::array< edge_t, edge_t::SIZE >&
edge_t::all()
{
  static const boost::array<edge_t, edge_t::SIZE> faces = 
    {{
      edge_t(direction_t::index(0)),
      edge_t(direction_t::index(1)),
      edge_t(direction_t::index(2)),
      edge_t(direction_t::index(3))
    }};
  
  return faces;
}


SQUARE_INLINE
const edge_t&
edge_t::get(const direction_t& direction)
{
  BOOST_ASSERT(direction.index() < all().size());
  return all()[direction.index()];
}

SQUARE_INLINE
const edge_t&
edge_t::get(const boost::uint8_t& idx)
{
  BOOST_ASSERT(idx < all().size());
  return all()[idx];
}


SQUARE_INLINE
const direction_t& edge_t::direction() const
{
  return mdirection;
}

SQUARE_INLINE
const edge_t& edge_t::opposite() const
{
  return edge_t::get(mdirection.opposite());
}

SQUARE_INLINE
boost::array<corner_t, 2>
edge_t::corners() const
{
  BOOST_ASSERT(false && "implement this for square");
}

SQUARE_INLINE
bool
edge_t::operator==(const edge_t& other) const
{
  return mdirection == other.mdirection;
}

SQUARE_INLINE
bool
edge_t::operator!=(const edge_t& other) const
{
  return mdirection != other.mdirection;
}

SQUARE_INLINE
bool
edge_t::operator<(const edge_t& other) const
{
  return mdirection < other.mdirection;
}


//###################################################################



//###################################################################
//#### direction_t
//###################################################################





SQUARE_INLINE
direction_t::direction_t(const std::bitset< 2 >& bits)
  : bits(bits)
{
#ifndef NDEBUG
  mx = x();
  my = y();
  mindex = index();
#endif
}




SQUARE_INLINE
const direction_t&
direction_t::
get(boost::int8_t x, boost::int8_t y)
{
  BOOST_ASSERT(lxor(x != 0, y != 0));
  
  BOOST_ASSERT(std::abs(x) == 1 || x == 0);
  BOOST_ASSERT(std::abs(y) == 1 || y == 0);
  
  
  /**
   * [2 bit number][1 bit indicating "direction is positive"]
   *
   * [1 bit indicating "direction is positive"]: If the direction is positive, this bit is set to 1.
   *
   * [2 bit number]: A number calculated as follows:
   * 
   * direction is in z: 00
   * direction is in y: 01
   * direction is in x: 10
   */
  
  direction_t direction(std::bitset<2>(0));
  
  std::bitset<2>& bits = direction.bits;
  if ( x != 0 ) {
    bits.set(1);
  } else if ( y != 0 ) {
    
  }
  
  if (x + y > 0)
  {
    bits.set(0);
  }
  
  return direction_t::get( direction );
}


SQUARE_INLINE
const direction_t&
direction_t::
get(const direction_t& direction)
{
  BOOST_ASSERT(direction.bits.to_ulong() < SIZE);
  return all()[direction.bits.to_ulong()];
}


SQUARE_INLINE
const direction_t&
direction_t::get(boost::uint8_t idx)
{
  BOOST_ASSERT(idx < SIZE);
  return all()[idx];
}

SQUARE_INLINE
const boost::array< direction_t, direction_t::SIZE >&
direction_t::
all()
{
  static const boost::array<direction_t, direction_t::SIZE> directions =
    {{
      direction_t(std::bitset<2>(0)),
      direction_t(std::bitset<2>(1)),
      direction_t(std::bitset<2>(2)),
      direction_t(std::bitset<2>(3))
    }};
  return directions;
}

SQUARE_INLINE
boost::int8_t direction_t::x() const
{
  return (bits[0] ? 1 : -1)  * (bits[1] ? 1 : 0);
}

SQUARE_INLINE
boost::int8_t direction_t::y() const
{
  return (bits[0] ? 1 : -1)  * (!bits[1] ? 1 : 0);
}


SQUARE_INLINE
boost::uint8_t direction_t::index() const
{
  BOOST_ASSERT(bits.to_ulong() < SIZE);
  return bits.to_ulong();
}

SQUARE_INLINE
const direction_t&
direction_t::index(boost::uint8_t idx)
{
  BOOST_ASSERT(idx < SIZE);
  return all()[idx];
}

SQUARE_INLINE
bool direction_t::
operator<(const direction_t& other) const
{
  ///FIXME: need a better operator<() prolly
  return bits.to_ulong() < other.bits.to_ulong();
}

SQUARE_INLINE bool direction_t::operator==(const direction_t& other) const
{
  return bits == other.bits;
}

SQUARE_INLINE bool direction_t::operator!=(const direction_t& other) const
{
  return bits != other.bits;
}




SQUARE_INLINE
const edge_t& direction_t::edge() const
{
  return edge_t::get(*this);
}

SQUARE_INLINE
const direction_t& direction_t::opposite() const
{
  std::bitset<2> result_bits = bits;
  result_bits.flip(0);
  
  BOOST_ASSERT(result_bits.to_ulong() < SIZE);
  return all()[ result_bits.to_ulong() ];
}

SQUARE_INLINE
bool direction_t::positive() const
{
  return bits.test(0);
}

SQUARE_INLINE
boost::array<direction_t, 2>
direction_t::adjacent() const
{
  boost::array<direction_t, 2> result = {{ direction_t::get(0), direction_t::get(0)}};
  std::size_t ri = 0;
  BOOST_FOREACH(const direction_t& d, direction_t::all())
  {
    if (d != *this && d != opposite())
    {
      BOOST_ASSERT(ri < result.size());
      result[ri++] = d;
    }
  }
  
  BOOST_ASSERT(ri == 2);
  
  return result;
}









//###################################################################





//###################################################################
//#### Corners
//###################################################################

SQUARE_INLINE
corner_t::
corner_t()
{
  
}

SQUARE_INLINE
corner_t::
corner_t(const std::bitset< 2 >& bits)
  : bits(bits)
{
#ifndef NDEBUG
   mx = bits.test(0);
   my = bits.test(1);
#endif
}

SQUARE_INLINE
corner_t::
corner_t(bool x, bool y)
  : bits( (x ? 1 : 0) + (y ? 2 : 0))
{
  
#ifndef NDEBUG
   mx = bits.test(0);
   my = bits.test(1);
#endif
}


SQUARE_INLINE
const corner_t&
corner_t::
get(bool x, bool y)
{
  std::bitset<2> bits;
  bits.set(0,x);
  bits.set(1,y);
  return all()[bits.to_ulong()];
}


SQUARE_INLINE
const boost::array< corner_t, 4 >&
corner_t::
all()
{
  static const boost::array<corner_t, 4> corners =
    {{
      corner_t(std::bitset<2>(0)),
      corner_t(std::bitset<2>(1)),
      corner_t(std::bitset<2>(2)),
      corner_t(std::bitset<2>(3))
    }};
#ifndef NDEBUG
  BOOST_FOREACH(const corner_t& corner, corners)
  {
    BOOST_ASSERT(corner.bits.to_ulong() < SIZE);
  }
#endif
    
  return corners;
}

SQUARE_INLINE
boost::uint8_t
corner_t::
index() const
{
  BOOST_ASSERT(bits.to_ulong() < SIZE);
  return bits.to_ulong();
}

SQUARE_INLINE
const corner_t&
corner_t::
get(boost::uint8_t i)
{
  BOOST_ASSERT(i < SIZE);
  return all()[i];
}

SQUARE_INLINE
boost::array< corner_t, 2 >
corner_t::
adjacent() const
{
  boost::array<corner_t,2> result =
    {{
      corner_t(!x(), y() ),
      corner_t( x(),!y() )
    }};
  
  return result;
}

SQUARE_INLINE
corner_set_t
corner_t::adjacent_set() const
{
  return corner_set_t(adjacent());
}

SQUARE_INLINE
bool
corner_t::
x() const
{
  return bits[0];
}
SQUARE_INLINE
bool
corner_t::
y() const
{
  return bits[1];
}


SQUARE_INLINE
unsigned short int
corner_t::
x_i() const
{
  return bits[0] ? 1 : 0;
}

SQUARE_INLINE
unsigned short int
corner_t::
y_i() const
{
  return bits[1] ? 1 : 0;
}


SQUARE_INLINE
bool corner_t::operator<(const corner_t& other) const
{
  return bits.to_ulong() < other.bits.to_ulong();
}

SQUARE_INLINE
boost::array< edge_t, 2 >
corner_t::edges() const
{
  boost::array< edge_t, 2 > result = 
    {{
      direction_t::get(x() ? 1 : -1, 0).edge(),
      direction_t::get(0, y() ? 1 : -1).edge()
    }};
  return result;
}


SQUARE_INLINE
edge_set_t
corner_t::edge_set() const
{
  return edge_set_t(edges());
}

SQUARE_INLINE
const corner_t&
corner_t::get(const corner_t& corner)
{
  BOOST_ASSERT(corner.index() < SIZE);
  return all()[corner.index()];
}

SQUARE_INLINE
const corner_t&
corner_t::opposite() const
{
  std::size_t idx = std::bitset<2>(bits).flip().to_ulong();
  BOOST_ASSERT(idx < SIZE);
  return all()[idx];
}

SQUARE_INLINE
bool corner_t::operator!=(const corner_t& other) const
{
  return bits != other.bits;
}

SQUARE_INLINE
bool corner_t::operator==(const corner_t& other) const
{
  return bits == other.bits;
}

SQUARE_INLINE
const corner_t&
corner_t::index(boost::uint8_t idx)
{
  return corner_t::get(idx);
}

SQUARE_INLINE
const corner_t&
corner_t::
adjacent(const direction_t& direction) const
{
  return corner_t::get(lxor(x(), (direction.x() != 0)),
                       lxor(y(), (direction.y() != 0)));
}



//###################################################################





//###################################################################
//#### set_base_t
//###################################################################

template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
set_base_t<derived_t, element_t, N>::
set_base_t()
  : bits(0)
{

}


template<typename derived_t, typename element_t, std::size_t N>
template<typename Sequence>
SQUARE_INLINE
set_base_t<derived_t, element_t, N>::
set_base_t(const Sequence& sequence)
  : bits(0)
{
  *this |= sequence;
}

template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
bool
set_base_t<derived_t, element_t, N>::
contains(const element_t& element) const
{
  BOOST_ASSERT(element.index() < N);
  return bits.test(element.index());
}


template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator=(const element_t& element)
{
  BOOST_ASSERT(element.index() < N);
  
  bits.reset();
  bits.set(element.index(), true);
  return self();
}



template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator=(const derived_t& set)
{
  bits = set.bits;
  return self();
}


  
template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator|=(const derived_t& set)
{
  bits |= set.bits;
  
  return self();
}

template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator|=(const element_t& element)
{
  BOOST_ASSERT(element.index() < N);
  bits.set(element.index(), true);
  return self();
}



template<typename derived_t, typename element_t, std::size_t N>
template<typename Sequence>
SQUARE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator|=(const Sequence& sequence)
{
  
  BOOST_FOREACH(const element_t& element, sequence)
  {
    *this |= element;
  }
  
  return self();
}


template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
self()
{
  
  typedef boost::is_base_of< set_base_t<derived_t, element_t, N>, derived_t > derives_correctly;
  BOOST_STATIC_ASSERT( derives_correctly::value );
  
  
  return static_cast<derived_t&>(*this);
}

template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
typename set_base_t<derived_t, element_t, N>::const_iterator 
set_base_t<derived_t, element_t, N>::
begin() const
{
  BOOST_ASSERT(false && "set_base_t::begin() is broken");
  return const_iterator(*this);
}

template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
typename set_base_t<derived_t, element_t, N>::const_iterator 
set_base_t<derived_t, element_t, N>::
end() const
{
  BOOST_ASSERT(false && "set_base_t::end() is broken");
  return const_iterator(*this, N);
}


template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
std::size_t
set_base_t<derived_t, element_t, N>::
size() const
{
  return bits.count();
}

template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
void
set_base_t<derived_t, element_t, N>::
clear()
{
  bits.reset();
}


template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
bool
set_base_t<derived_t, element_t, N>::
operator==(const derived_t& other) const
{
  return bits == other.bits;
}





template<typename derived_t, typename element_t, std::size_t N>
SQUARE_INLINE
bool
set_base_t<derived_t, element_t, N>::
contains(const std::size_t& idx) const
{
  BOOST_ASSERT(idx < N);
  return bits.test(idx);
}










SQUARE_INLINE
direction_set_t::direction_set_t()
  : base_t()
{
  
}

template<typename T>
SQUARE_INLINE
direction_set_t::direction_set_t(const T& v)
  : base_t(v)
{
  
}


SQUARE_INLINE
edge_set_t::edge_set_t()
  : base_t()
{

}

template<typename T>
SQUARE_INLINE
edge_set_t::edge_set_t(const T& v)
  : base_t(v)
{

}





SQUARE_INLINE
corner_set_t::corner_set_t()
  : base_t()
{

}

template<typename T>
SQUARE_INLINE
corner_set_t::corner_set_t(const T& v)
  : base_t(v)
{

}









//###################################################################




} // namespace cube
