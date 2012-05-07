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


#include <cube/cube.h>
#include "logic_utility.h"

#include <vector>
#include <boost/range/irange.hpp>
#include <boost/utility.hpp>

//###################################################################
//#### face_t
//###################################################################

namespace cube{

CUBE_INLINE
face_t::
face_t(const direction_t& direction)
  : mdirection(direction)
{

}


CUBE_INLINE
boost::uint8_t
face_t::index() const
{
  return mdirection.index();
}


CUBE_INLINE
const boost::array< face_t, 6 >&
face_t::all()
{
  static const boost::array<face_t, 6> faces = 
    {{
      face_t(direction_t::index(0)),
      face_t(direction_t::index(1)),
      face_t(direction_t::index(2)),
      face_t(direction_t::index(3)),
      face_t(direction_t::index(4)),
      face_t(direction_t::index(5))
    }};
  
  return faces;
}


CUBE_INLINE
const face_t&
face_t::get(const direction_t& direction)
{
  BOOST_ASSERT(direction.index() < all().size());
  return all()[direction.index()];
}

CUBE_INLINE
const cube::face_t&
face_t::get(const boost::uint8_t& idx)
{
  BOOST_ASSERT(idx < all().size());
  return all()[idx];
}


CUBE_INLINE
const direction_t& face_t::direction() const
{
  return mdirection;
}

CUBE_INLINE
const face_t& face_t::opposite() const
{
  return face_t::get(mdirection.opposite());
}

CUBE_INLINE
boost::array<corner_t, 4>
face_t::corners() const
{
  
  /*
  boost::array<boost::int8_t, 3> direction_components =
    {{
      direction.x(),
      direction.y(),
      direction.z()
    }};
  */
  //bool positive_direction = direction.positive();
  std::bitset<3> direction_bits;
  direction_bits.set(2, mdirection.x() != 0);
  direction_bits.set(1, mdirection.y() != 0);
  direction_bits.set(0, mdirection.z() != 0);
  
  corner_set_t corner_set;
  BOOST_FOREACH(const corner_t& corner, corner_t::all())
  {
    std::bitset<3> corner_bits;
    corner_bits.set(2, corner.x() != 0);
    corner_bits.set(1, corner.y() != 0);
    corner_bits.set(0, corner.z() != 0);
    corner_bits &= direction_bits;
    if (corner_bits.any())
    {
      if (mdirection.positive())
        corner_set |= corner;
      else
        corner_set |= corner.opposite();
    }
  }
  
  BOOST_ASSERT(corner_set.size() == 4);
  std::vector<corner_t> tmp;
  
  BOOST_FOREACH(const corner_t& corner, corner_t::all())
  {
    if (corner_set.contains(corner))
      tmp.push_back(corner);
  }
  
  ///FIXME: begin() end() broken
  /*
  corner_set_t::const_iterator w = corner_set.begin();
  corner_set_t::const_iterator wend = corner_set.end();
  
  for ( ; w != wend; ++w )
  {
    tmp.push_back(*w);
  }
  */
  
  BOOST_ASSERT(tmp.size() == 4);
  
  boost::array<corner_t, 4> result =
    {{
      tmp[0],
      tmp[1],
      tmp[2],
      tmp[3]
    }};
  
  return result;
}

CUBE_INLINE
bool
face_t::operator==(const cube::face_t& other) const
{
  return mdirection == other.mdirection;
}

CUBE_INLINE
bool
face_t::operator!=(const cube::face_t& other) const
{
  return mdirection != other.mdirection;
}

CUBE_INLINE
bool
face_t::operator<(const cube::face_t& other) const
{
  return mdirection < other.mdirection;
}


//###################################################################



//###################################################################
//#### direction_t
//###################################################################



CUBE_INLINE
direction_t::
direction_t()
  : bits(BOOST_BINARY(111))
{

#ifndef NDEBUG
  mx = x();
  my = y();
  mz = z();
  mindex = index();
#endif
}


CUBE_INLINE
direction_t::direction_t(const std::bitset< 3 >& bits)
  : bits(bits)
{
#ifndef NDEBUG
  mx = x();
  my = y();
  mz = z();
  mindex = index();
#endif
}




CUBE_INLINE
const direction_t&
direction_t::
get(boost::int8_t x, boost::int8_t y, boost::int8_t z)
{
  BOOST_ASSERT(lxor(lxor(x != 0, y != 0), z != 0));
  
  BOOST_ASSERT(std::abs(x) == 1 || x == 0);
  BOOST_ASSERT(std::abs(y) == 1 || y == 0);
  BOOST_ASSERT(std::abs(z) == 1 || z == 0);
  
  
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
  
  direction_t direction(std::bitset<3>(0));
  
  std::bitset<3>& bits = direction.bits;
  if ( x != 0 )
  {
    bits.set(2);
  } else if ( y != 0 ) {
    bits.set(1);
  } else if ( z != 0 ) {
    
  }
  
  if (x + y + z > 0)
  {
    bits.set(0);
  }
  
  return direction_t::get( direction );
}

CUBE_INLINE
const direction_t&
direction_t::
get(const direction_t& direction)
{
  BOOST_ASSERT(direction.bits.to_ulong() < SIZE);
  return all()[direction.bits.to_ulong()];
}

CUBE_INLINE
const boost::array< direction_t, 6 >&
direction_t::
all()
{
  static const boost::array<direction_t, 6> directions =
    {{
      direction_t(std::bitset<3>(0)),
      direction_t(std::bitset<3>(1)),
      direction_t(std::bitset<3>(2)),
      direction_t(std::bitset<3>(3)),
      direction_t(std::bitset<3>(4)),
      direction_t(std::bitset<3>(5))
    }};
  return directions;
}

CUBE_INLINE
boost::int8_t direction_t::x() const
{
  return (bits[0] ? 1 : -1)  * (bits[2] && !bits[1] ? 1 : 0);
}

CUBE_INLINE
boost::int8_t direction_t::y() const
{
  return (bits[0] ? 1 : -1)  * (!bits[2] && bits[1] ? 1 : 0);
}

CUBE_INLINE
boost::int8_t direction_t::z() const
{
  return (bits[0] ? 1 : -1)  * (!bits[2] && !bits[1] ? 1 : 0);
}


CUBE_INLINE
boost::uint8_t direction_t::index() const
{
  return bits.to_ulong();
}

CUBE_INLINE
const direction_t&
direction_t::index(boost::uint8_t idx)
{
  BOOST_ASSERT(idx < SIZE);
  return all()[idx];
}

CUBE_INLINE
bool direction_t::
operator<(const direction_t& other) const
{
  return bits.to_ulong() < other.bits.to_ulong();
}

CUBE_INLINE bool direction_t::operator==(const direction_t& other) const
{
  return bits == other.bits;
}

CUBE_INLINE bool direction_t::operator!=(const direction_t& other) const
{
  return bits != other.bits;
}




CUBE_INLINE
const face_t& direction_t::face() const
{
  return face_t::get(*this);
}

CUBE_INLINE
const direction_t& direction_t::opposite() const
{
  std::bitset<3> result_bits = bits;
  result_bits.flip(0);
  
  BOOST_ASSERT(result_bits.to_ulong() < SIZE);
  return all()[ result_bits.to_ulong() ];
}

CUBE_INLINE
bool direction_t::positive() const
{
  return bits.test(0);
}

CUBE_INLINE
boost::uint8_t direction_t::axis() const
{
  if (x() != 0)
    return 0;
  if (y() != 0)
    return 1;
  if (z() != 0)
    return 2;
  
  BOOST_ASSERT(false);
}



CUBE_INLINE
boost::array<direction_t, 4>
direction_t::adjacent() const
{
  boost::array<direction_t, 4> result = {{ direction_t(), direction_t(), direction_t(), direction_t()}};
  std::size_t ri = 0;
  BOOST_FOREACH(const direction_t& d, direction_t::all())
  {
    if (d != *this && d != opposite())
    {
      BOOST_ASSERT(ri < result.size());
      result[ri++] = d;
    }
  }
  
  BOOST_ASSERT(ri == 4);
  
  return result;
}









//###################################################################





//###################################################################
//#### Corners
//###################################################################

CUBE_INLINE
corner_t::
corner_t()
{
  
}

CUBE_INLINE
corner_t::
corner_t(const std::bitset< 3 >& bits)
  : bits(bits)
{
#ifndef NDEBUG
   mx = bits.test(0);
   my = bits.test(1);
   mz = bits.test(2);
#endif
}

CUBE_INLINE
corner_t::
corner_t(bool x, bool y, bool z)
  : bits( (x ? 1 : 0) + (y ? 2 : 0) + (z ? 4 : 0))
{
  
#ifndef NDEBUG
   mx = bits.test(0);
   my = bits.test(1);
   mz = bits.test(2);
#endif
  //bits.set(0, x);
  //bits.set(1, y);
  //bits.set(2, z);
}


CUBE_INLINE
const corner_t&
corner_t::
get(bool x, bool y, bool z)
{
  std::bitset<3> bits;
  bits.set(0,x);
  bits.set(1,y);
  bits.set(2,z);
  return all()[bits.to_ulong()];
}


CUBE_INLINE
const boost::array< corner_t, 8 >&
corner_t::
all()
{
  static const boost::array<corner_t, 8> corners =
    {{
      corner_t(std::bitset<3>(0)),
      corner_t(std::bitset<3>(1)),
      corner_t(std::bitset<3>(2)),
      corner_t(std::bitset<3>(3)),
      corner_t(std::bitset<3>(4)),
      corner_t(std::bitset<3>(5)),
      corner_t(std::bitset<3>(6)),
      corner_t(std::bitset<3>(7))
    }};
  return corners;
}

CUBE_INLINE
boost::uint8_t
corner_t::
index() const
{
  return bits.to_ulong();
}

CUBE_INLINE
const corner_t&
corner_t::
get(boost::uint8_t i)
{
  BOOST_ASSERT(i < SIZE);
  return all()[i];
}

CUBE_INLINE
boost::array< corner_t, 3 >
corner_t::
adjacent() const
{
  boost::array<corner_t,3> result =
    {{
      corner_t(!x(), y(), z() ),
      corner_t( x(),!y(), z() ),
      corner_t( x(), y(),!z() )
    }};
  
  return result;
}

CUBE_INLINE
corner_set_t
corner_t::adjacent_set() const
{
  return corner_set_t(adjacent());
}

CUBE_INLINE
bool
corner_t::
x() const
{
  return bits[0];
}
CUBE_INLINE
bool
corner_t::
y() const
{
  return bits[1];
}

CUBE_INLINE
bool
corner_t::
z() const
{
  return bits[2];
}

CUBE_INLINE
unsigned short int
corner_t::
x_i() const
{
  return bits[0] ? 1 : 0;
}

CUBE_INLINE
unsigned short int
corner_t::
y_i() const
{
  return bits[1] ? 1 : 0;
}

CUBE_INLINE
unsigned short int
corner_t::
z_i() const
{
  return bits[2] ? 1 : 0;
}

CUBE_INLINE
bool corner_t::operator<(const corner_t& other) const
{
  return bits.to_ulong() < other.bits.to_ulong();
}

CUBE_INLINE
boost::array< face_t, 3 >
corner_t::faces() const
{
  boost::array< face_t, 3 > result = 
    {{
      direction_t::get( x() ? 1 : -1, 0, 0).face(),
      direction_t::get( 0, y() ? 1 : -1, 0).face(),
      direction_t::get( 0, 0, z() ? 1 : -1).face()
    }};
  return result;
}


CUBE_INLINE
face_set_t
corner_t::face_set() const
{
  return face_set_t(faces());
}

CUBE_INLINE
const corner_t&
corner_t::get(const corner_t& corner)
{
  BOOST_ASSERT(corner.index() < SIZE);
  return all()[corner.index()];
}

CUBE_INLINE
const corner_t&
corner_t::opposite() const
{
  std::size_t idx =  std::bitset<3>(bits).flip().to_ulong();
  BOOST_ASSERT(idx < SIZE);
  return all()[idx];
}

CUBE_INLINE
bool corner_t::operator!=(const corner_t& other) const
{
  return bits != other.bits;
}

CUBE_INLINE
bool corner_t::operator==(const corner_t& other) const
{
  return bits == other.bits;
}

CUBE_INLINE
const corner_t&
corner_t::index(boost::uint8_t idx)
{
  return corner_t::get(idx);
}

CUBE_INLINE
const corner_t&
corner_t::
adjacent(const direction_t& direction) const
{
  return corner_t::get(lxor(x(), (direction.x() != 0)),
                       lxor(y(), (direction.y() != 0)),
                       lxor(z(), (direction.z() != 0)));
}



//###################################################################




//###################################################################
//#### edge_t
//###################################################################


namespace detail{
  CUBE_INLINE
  boost::array<corner_t, 2>
  make_edge_corners(const corner_t& a, const corner_t& b)
  {
    boost::array<corner_t, 2> result =
    {{
      (a.index() < b.index() ? a : b),
      (a.index() < b.index() ? b : a)
    }};
    return result;
  }
} // namespace detail

CUBE_INLINE
edge_t::
edge_t(const corner_t& a, const corner_t& b)
  : mcorners(detail::make_edge_corners(a,b))
{
  BOOST_ASSERT(mcorners.front().index() < mcorners.back().index());
  BOOST_ASSERT(mcorners.front() != mcorners.back());
  BOOST_ASSERT(mcorners.front().adjacent_set().contains(mcorners.back()));
  BOOST_ASSERT(mcorners.back().adjacent_set().contains(mcorners.front()));
}

CUBE_INLINE
const boost::array<corner_t, 2>&
edge_t::
corners() const
{
  return mcorners;
}

CUBE_INLINE
const boost::array<edge_t, 12>&
edge_t::
all()
{
  
  ///right top far
  static const corner_t& rtf = corner_t::get( true, true, true);
  
  ///right bottom near
  static const corner_t& rbn = corner_t::get( true,false,false);
  
  ///left bottom, far
  static const corner_t& lbf = corner_t::get(false,false, true);
  
  ///left top near
  static const corner_t& lbn = corner_t::get(false, true,false);
  
  
  
  
  static const boost::array<edge_t, 12> result = 
  {{
    edge_t(rtf, rtf.adjacent()[0]),
    edge_t(rtf, rtf.adjacent()[1]),
    edge_t(rtf, rtf.adjacent()[2]),
    
    edge_t(rbn, rbn.adjacent()[0]),
    edge_t(rbn, rbn.adjacent()[1]),
    edge_t(rbn, rbn.adjacent()[2]),
    
    edge_t(lbf, lbf.adjacent()[0]),
    edge_t(lbf, lbf.adjacent()[1]),
    edge_t(lbf, lbf.adjacent()[2]),
    
    edge_t(lbn, lbn.adjacent()[0]),
    edge_t(lbn, lbn.adjacent()[1]),
    edge_t(lbn, lbn.adjacent()[2])
  }};
  
  return result;
}

//###################################################################




CUBE_INLINE
box_t::
box_t()
{

}

CUBE_INLINE
const box_t&
box_t::
identity()
{
  static const box_t box;
  return box;
}


CUBE_INLINE
const boost::array< corner_t, 8 >&
box_t::corners() const
{
  return corner_t::all();
}






















































//###################################################################
//#### set_base_t
//###################################################################

template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
set_base_t<derived_t, element_t, N>::
set_base_t()
  : bits(0)
{

}


template<typename derived_t, typename element_t, std::size_t N>
template<typename Sequence>
CUBE_INLINE
set_base_t<derived_t, element_t, N>::
set_base_t(const Sequence& sequence)
  : bits(0)
{
  *this |= sequence;
}

template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
bool
set_base_t<derived_t, element_t, N>::
contains(const element_t& element) const
{
  BOOST_ASSERT(element.index() < N);
  return bits.test(element.index());
}


template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
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
CUBE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator=(const derived_t& set)
{
  bits = set.bits;
  return self();
}


  
template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
operator|=(const derived_t& set)
{
  bits |= set.bits;
  
  return self();
}

template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
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
CUBE_INLINE
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
CUBE_INLINE
derived_t&
set_base_t<derived_t, element_t, N>::
self()
{
  
  typedef boost::is_base_of< set_base_t<derived_t, element_t, N>, derived_t > derives_correctly;
  BOOST_STATIC_ASSERT( derives_correctly::value );
  
  
  return static_cast<derived_t&>(*this);
}

template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
typename set_base_t<derived_t, element_t, N>::const_iterator 
set_base_t<derived_t, element_t, N>::
begin() const
{
  BOOST_ASSERT(false && "set_base_t::begin() is broken");
  return const_iterator(*this);
}

template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
typename set_base_t<derived_t, element_t, N>::const_iterator 
set_base_t<derived_t, element_t, N>::
end() const
{
  BOOST_ASSERT(false && "set_base_t::end() is broken");
  return const_iterator(*this, N);
}


template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
std::size_t
set_base_t<derived_t, element_t, N>::
size() const
{
  return bits.count();
}

template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
void
set_base_t<derived_t, element_t, N>::
clear()
{
  bits.reset();
}


template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
bool
set_base_t<derived_t, element_t, N>::
operator==(const derived_t& other) const
{
  return bits == other.bits;
}





template<typename derived_t, typename element_t, std::size_t N>
CUBE_INLINE
bool
set_base_t<derived_t, element_t, N>::
contains(const std::size_t& idx) const
{
  BOOST_ASSERT(idx < N);
  return bits.test(idx);
}










CUBE_INLINE
direction_set_t::direction_set_t()
  : base_t()
{
  
}

template<typename T>
CUBE_INLINE
direction_set_t::direction_set_t(const T& v)
  : base_t(v)
{
  
}


CUBE_INLINE
face_set_t::face_set_t()
  : base_t()
{

}

template<typename T>
CUBE_INLINE
face_set_t::face_set_t(const T& v)
  : base_t(v)
{

}





CUBE_INLINE
corner_set_t::corner_set_t()
  : base_t()
{

}

template<typename T>
CUBE_INLINE
corner_set_t::corner_set_t(const T& v)
  : base_t(v)
{

}









//###################################################################




} // namespace cube
