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


#ifndef GPUNOISE_H
#define GPUNOISE_H
#include <string>
#include <boost/noncopyable.hpp>

namespace gpunoise
{
  
  typedef float real_t;
  
  
  
  struct PerlinModule3D : Module3D
  {
    
  };
  
  struct Const3D : Module3D
  {
    Const3D(real_t value);
    void set_value(real_t value);
    real_t get_value() const;
    
    virtual std::string name() const;
    virtual std::string generate() const;
  private:
    real_t mvalue;
  };
  
  struct Add3D : Module3D
  {
    Add3D();
    Add3D(Module3D* lhs, Module3D* rhs);
    
    void set_lhs(Module3D* lhs);
    void set_rhs(Module3D* rhs);
    Module3D* get_lhs() const;
    Module3D* get_rhs() const;
    
    virtual std::string name() const;
    virtual std::string generate() const;
    
  private:
    Module3D* mlhs;
    Module3D* mrhs;
  };
  
} // namespace gpunoise





#endif // GPUNOISE_H
