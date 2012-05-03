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

#include "reference_monitor.h"


#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assert.hpp>


inline
object_monitor::object_monitor()
  : i(boost::make_shared<char>(0))
{
  BOOST_ASSERT(is_singly_owned());
}

inline
object_monitor::~object_monitor()
{
  BOOST_ASSERT(is_singly_owned());
}

inline
object_monitor::object_monitor(const object_monitor& other)
  : i(boost::make_shared<char>(0))
{
  ///We don't share with the other's shared counter because we don't share the other's address (ofc)
  
  ///Sanity
  BOOST_ASSERT(other.is_singly_owned());
  BOOST_ASSERT(is_singly_owned());
}

inline
object_monitor& object_monitor::operator=(const object_monitor& other)
{
  ///My pointer is still valid, and unchanged
  
  ///Sanity
  BOOST_ASSERT(other.is_singly_owned());
  BOOST_ASSERT(is_singly_owned());
  
  return *this;
}

inline
bool object_monitor::is_singly_owned() const
{
  return (!!indicator && indicator.unique());
}

inline
object_reporter::object_reporter(const object_reporter& other)
  : indicator(other.indicator), reference_counter(other.reference_counter)
{
  
}

inline
bool
object_reporter::
alive() const
{
  ///FIXME: what if indicator is NULL?
  return indicator.expired();
}

inline
object_reporter::
object_reporter()
  : indicator(), reference_counter()
{

}

inline
object_reporter::
object_reporter(const object_monitor& object)
  : indicator(object.indicator), reference_counter(object.reference_counter)
{

}

inline
object_reporter&
object_reporter::
operator=(const object_monitor& object)
{
  indicator = object.indicator;
  reference_counter = object.reference_counter;
  return *this;
}

inline
object_reporter&
object_reporter::
operator=(const object_reporter& other)
{
  indicator = other.indicator;
  reference_counter = other.reference_counter;
}

object_reporter::
~object_reporter()
{

}


















