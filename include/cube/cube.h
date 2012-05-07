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
#ifndef CUBE_CUBE_H
#define CUBE_CUBE_H


#include <bitset>
#include <boost/array.hpp>
#include <boost/integer.hpp>
#include <boost/foreach.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/logical.hpp>


namespace cube{

#ifndef CUBE_INLINE
#define CUBE_INLINE inline
#endif

struct face_t;
struct face_set_t;
struct direction_t;
struct direction_set_t;
struct corner_t;
struct corner_set_t;
struct edge_t;
struct edge_set_t;
struct box_t;





template<typename value_type, typename set_type>
struct  const_element_set_iterator_t
  : public boost::iterator_facade<const_element_set_iterator_t<value_type, set_type>,
                                  value_type,
                                  boost::forward_traversal_tag>
{
private:
  struct enabler {};  // a private type avoids misuse
public:
  CUBE_INLINE
  const_element_set_iterator_t()
    : mset(NULL), mindex(value_type::SIZE)
  {
    
  }
  
  CUBE_INLINE
  const_element_set_iterator_t(set_type& set, std::size_t index)
    : mset(&set), mindex(index)
  {
    BOOST_ASSERT(valid());
  }
  
  CUBE_INLINE
  const_element_set_iterator_t(set_type& set)
    : mset(&set), mindex(0)
  {
    BOOST_ASSERT(valid());
    
    while (mindex < value_type::SIZE)
    {
      if (mset->contains(mindex))
      {
        break;
      }
      ++mindex;
    }
    
    BOOST_ASSERT(valid());
    BOOST_ASSERT(dereferencable() || mindex == value_type::SIZE);
    
  }

  template <class OtherValue, class OtherSet>
  CUBE_INLINE
  const_element_set_iterator_t(
      const_element_set_iterator_t<OtherValue, OtherSet> const& other
    , typename boost::enable_if<
          boost::mpl::and_<
            boost::is_convertible<OtherValue*,value_type*>,
            boost::is_convertible<OtherSet*,set_type*>
            >
        , enabler
      >::type = enabler()
    )
    : mset(other.mset), mindex(other.mindex)
  {
    BOOST_ASSERT(other.valid());
    BOOST_ASSERT(valid());
    
    
  }

  template<typename T>
  CUBE_INLINE
  const_element_set_iterator_t& operator=(const T& other)
  {
    assign(other);
    return *this;
  }

private:
  friend class boost::iterator_core_access;
  CUBE_INLINE
  void increment() {
    BOOST_ASSERT(valid());
      BOOST_ASSERT(valid());
      BOOST_ASSERT(mset);
      BOOST_ASSERT(mindex != value_type::SIZE);
      BOOST_ASSERT(mset->contains(mindex));
    BOOST_ASSERT(dereferencable());
    
    ++mindex;
      
    while (mindex < value_type::SIZE)
    {
      if (dereferencable())
        break;
      ++mindex;
    }
    BOOST_ASSERT(valid());
  }
  
  CUBE_INLINE
  value_type& dereference() const {
    BOOST_ASSERT(valid());
    BOOST_ASSERT(dereferencable());
    return value_type::get(mindex);
  }
  
  template <class OtherValue, class OtherSet>
  CUBE_INLINE
  bool equal(const_element_set_iterator_t<OtherValue, OtherSet> const& other) const
  {
    return
      ///The two iterators are referring to the same set
      (this->mset == other.mset
        ///Or one of them is a default constructed iterator
        || ( this->mset == NULL || other.mset == NULL ))
      ///And the indices are equal
      && this->mindex == other.mindex;
  }
private:
  CUBE_INLINE
  bool valid() const{
    return mindex <= value_type::SIZE
      ///If not an end iterator, mset should be set, and mindex should be contained in it
      && (mindex == value_type::SIZE || ((mset) && mset->contains(mindex)));
  }
  
  CUBE_INLINE
  bool dereferencable() const{
    return valid() && (mset) && mindex != value_type::SIZE && mset->contains(mindex);
  }
  
  template <class OtherValue, class OtherSet>
  CUBE_INLINE
  void
  assign(const_element_set_iterator_t<OtherValue, OtherSet> const& other,
            typename boost::enable_if<
                boost::mpl::and_<
                  boost::is_convertible<OtherValue*,value_type*>,
                  boost::is_convertible<OtherSet*,set_type*>
                  >
              , enabler
            >::type = enabler() )
  {
    BOOST_ASSERT(other.valid());
    mset = other.mset;
    mindex = other.mindex;
    BOOST_ASSERT(valid());
  }
  
  
  set_type* mset;
  std::size_t mindex;
  
};





template<typename derived_t, typename element_t, std::size_t N>
struct set_base_t
{
  typedef element_t value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  
  typedef const_element_set_iterator_t<value_type const, derived_t const> const_iterator;
  typedef const_iterator iterator;
  
  const_iterator begin() const;
  const_iterator end() const;
  
  
  template<typename Sequence>
  set_base_t(const Sequence& sequence);
  set_base_t(const element_t& element);
  set_base_t(const derived_t& set);
  set_base_t();
  
  template<typename Sequence>
  derived_t& operator=(const Sequence& sequence);
  derived_t& operator=(const element_t& element);
  derived_t& operator=(const derived_t& set);
  
  template<typename Sequence>
  derived_t& operator|=(const Sequence& sequence);
  derived_t& operator|=(const derived_t& set);
  derived_t& operator|=(const element_t& element);
  
  template<typename Sequence>
  derived_t operator|(const Sequence& sequence);
  derived_t operator|(const derived_t& set);
  derived_t operator|(const element_t& element);
  
  bool contains(const element_t& element) const;
  bool contains(const std::size_t& idx) const;
  
  std::size_t size() const;
  
  void clear();
  
  bool operator==(const derived_t& other) const;
private:
  
  derived_t& self();
  const derived_t& self() const;
  
  std::bitset<N> bits;
};


struct corner_set_t : public set_base_t<corner_set_t, corner_t, 8>
{
  typedef corner_set_t self_t;
  typedef set_base_t<corner_set_t, corner_t, 8> base_t;
  
  template<typename T>
  corner_set_t(const T& v);
  corner_set_t();
  
};

struct face_set_t : public set_base_t<face_set_t, face_t, 6>
{
  typedef face_set_t self_t;
  typedef set_base_t<face_set_t, face_t, 6> base_t;
  
  template<typename T>
  face_set_t(const T& v);
  face_set_t();
  
};


struct direction_set_t : public set_base_t<direction_set_t, direction_t, 6>
{
  typedef direction_set_t self_t;
  typedef set_base_t<direction_set_t, direction_t, 6> base_t;
  
  template<typename T>
  direction_set_t(const T& v);
  direction_set_t();
  
};













struct direction_t
{
  
  const face_t& face() const;
  const direction_t& opposite() const;
  boost::array<direction_t, 4> adjacent() const;
  
  static const direction_t& get(boost::int8_t x, boost::int8_t y, boost::int8_t z);
  static const direction_t& get(const direction_t& direction);
  static const boost::array<direction_t, 6>& all();
  
  boost::int8_t x() const;
  boost::int8_t y() const;
  boost::int8_t z() const;
  boost::uint8_t axis() const;
  bool positive() const;
  
  boost::uint8_t index() const;
  static const direction_t& index(boost::uint8_t idx);
  bool operator<(const direction_t& other) const;
  bool operator==(const direction_t& other) const;
  bool operator!=(const direction_t& other) const;
  
  bool valid() const;
  static const std::size_t SIZE = 6;
protected:
#ifndef NDEBUG
  int mx, my, mz;
  boost::uint8_t mindex;
#endif
private:
  std::bitset<3> bits;
  
  ///Default constructs an invalid direction with index >= @c SIZE
  direction_t();
  
  direction_t(const std::bitset<3>& bits);
};

struct face_t{
  
  const direction_t& direction() const;
  const face_t& opposite() const;
  boost::array<face_t, 4> adjacent() const;
  boost::array<corner_t, 4> corners() const;
  corner_set_t corner_set() const;
  boost::array<edge_t, 4> edges() const;
  
  static const boost::array<face_t, 6>& all();
  static const face_t& get(const direction_t& direction);
  static const face_t& get(const boost::uint8_t& idx);
  
  
  boost::uint8_t index() const;
  static const direction_t& index(boost::uint8_t idx);
  bool operator<(const face_t& other) const;
  bool operator==(const face_t& other) const;
  bool operator!=(const face_t& other) const;
  static const std::size_t SIZE = 6;
  
protected:
  direction_t mdirection;
private:
  face_t();
  face_t(const direction_t& direction);
  
};

struct corner_t{
  typedef direction_t direction_type;
  typedef edge_t edge_type;
  typedef face_t face_type;
  
  
  const corner_t& adjacent(const direction_t& direction) const;
  boost::array<corner_t, 3> adjacent() const;
  corner_set_t adjacent_set() const;
  
  boost::array<face_t, 3> faces() const;
  face_set_t face_set() const;
  
  boost::array<edge_t, 3> edges() const;
  edge_t edge(const direction_t& direction);
  
  
  const corner_t& opposite() const;
  
  static const corner_t& get(boost::uint8_t i);
  static const corner_t& get(bool x, bool y, bool z);
  static const corner_t& get(const corner_t& corner);
  static const corner_t& index(boost::uint8_t idx);
  
  boost::uint8_t index() const;
  bool operator<(const corner_t& other) const;
  bool operator==(const corner_t& other) const;
  bool operator!=(const corner_t& other) const;
  
  
  static const boost::array<corner_t, 8>& all();
  
  
  bool x() const;
  bool y() const;
  bool z() const;
  
  unsigned short int x_i() const;
  unsigned short int y_i() const;
  unsigned short int z_i() const;
  
  ///Number of corners
  static const std::size_t SIZE = 8;
  corner_t();
  
protected:
#ifndef NDEBUG
  bool mx, my, mz;
#endif
private:
  corner_t(bool x, bool y, bool z);
  corner_t(const std::bitset<3>& bits);
  
  std::bitset<3> bits;
};


struct edge_t{
  const boost::array<corner_t, 2>& corners() const;
  boost::array<edge_t, 4> adjacent() const;
  boost::array<face_t, 2> faces() const;
  
  corner_set_t corner_set() const;
  
  static const boost::array<edge_t, 12>& all();
  
  bool operator<(const edge_t& other) const;
  
private:
  edge_t(const corner_t& a, const corner_t& b);
  edge_t();
  
  boost::array<corner_t, 2> mcorners;
};

struct box_t{
  boost::array<face_t, 6> faces() const;
  const boost::array<corner_t, 8>& corners() const;
  boost::array<edge_t, 12> edges() const;
  boost::array<direction_t, 6> directions() const;
  
  static const box_t& identity();
private:
  box_t();
};


} // namespace cube

#include <cube/cube.inl.h>


#endif // CUBE_H
