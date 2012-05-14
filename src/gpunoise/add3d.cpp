/*
    Copyright (c) 2012 <copyright holder> <email>

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


#include "add3d.h"

#include <boost/assert.hpp>
#include <boost/format.hpp>

namespace gpunoise{
  
  
add3d::add3d()
  : mlhs(NULL)
  , mrhs(NULL)
{

}

add3d::add3d(gpunoise::module3d* l, gpunoise::module3d* r)
  : mlhs(l)
  , mrhs(r)
{

}

module3d* add3d::lhs() const
{
  return mlhs;
}

module3d* add3d::rhs() const
{
  return mrhs;
}

void add3d::lhs(module3d* l)
{
  mlhs = l;
}

void add3d::rhs(module3d* r)
{
  mrhs = r;
}

std::string add3d::name() const
{
  BOOST_ASSERT(!!mlhs);
  BOOST_ASSERT(!!mrhs);
  return boost::str(boost::format("_%1%_Add3D_%2%_") % mlhs->name() % mrhs->name());
}


std::string add3d::generate() const
{
  BOOST_ASSERT(!!mlhs);
  BOOST_ASSERT(!!mrhs);
  
  return boost::str(
    boost::format(
      "float %1%(float3 xyz){ return %2%(xyz) + %3%(xyz); }"
      ) % name() % mlhs->name() % mrhs->name());
}


} // namespace gpunoise

