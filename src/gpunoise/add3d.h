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


#ifndef LIBGPUNOISE_ADD3D_H
#define LIBGPUNOISE_ADD3D_H

#include "module3d.h"

namespace gpunoise{
  
struct add3d : combiner3d
{
  add3d();
  add3d(module3d* l, module3d* r);
  
  module3d* lhs() const;
  module3d* rhs() const;
  void lhs(module3d* l);
  void rhs(module3d* r);
  
  virtual std::string generate() const;
  virtual std::string name() const;
  
private:
  module3d* mlhs;
  module3d* mrhs;
};

} // namespace gpunoise

#endif // LIBGPUNOISE_ADD3D_H
