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


#ifndef REFERENCE_MONITOR_H
#define REFERENCE_MONITOR_H


#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

struct object_reporter;

struct object_monitor{
  object_monitor();
  object_monitor(const object_monitor& other);
  object_monitor& operator=(const object_monitor& other);
  ~object_monitor();
private:
  friend class object_reporter;
  
  boost::shared_ptr<char> indicator;
  boost::shared_ptr<std::size_t> reference_counter;
  bool is_singly_owned() const;
};

struct object_reporter{
  object_reporter();
  object_reporter(const object_monitor& object);
  object_reporter(const object_reporter& other);
  object_reporter& operator=(const object_monitor& object);
  object_reporter& operator=(const object_reporter& other);
  ~object_reporter();
  
  bool alive() const;
private:
  
  boost::weak_ptr<char> indicator;
  boost::shared_ptr<std::size_t> reference_counter;
};

template<typename T>
struct unmonitored_reference{
  template<typename DONTCARE>
  unmonitored_reference(T* object, const DONTCARE& monitor);
  unmonitored_reference(T& object, const DONTCARE& monitor);
  
  template<typename U>
  unmonitored_reference(const monitored_reference<U>& other);
  ~unmonitored_reference();
  
  
  template<typename U>
  unmonitored_reference& operator=(const monitored_reference<U>& other);
  unmonitored_reference& operator=(const monitored_reference& other);
  
  unmonitored_reference();
  void reset();
  
  T & operator*() const; // never throws
  T * operator->() const; // never throws
  T * get() const; // never throws

  void swap(const unmonitored_reference& other);
  
  operator bool() const;
};

template<class T, class U>
bool operator<(unmonitored_reference<T> const & a, unmonitored_reference<U> const & b);

template<typename T>
struct monitored_reference : public object_reporter{
  monitored_reference(T* object, const object_monitor& monitor);
  monitored_reference(T& object, const object_monitor& monitor);
  
  template<typename U>
  monitored_reference(const monitored_reference<U>& other);
  ~monitored_reference();
  
  
  template<typename U>
  monitored_reference& operator=(const monitored_reference<U>& other);
  monitored_reference& operator=(const monitored_reference& other);
  
  monitored_reference();
  
  void reset();
  
  T & operator*() const; // never throws
  T * operator->() const; // never throws
  T * get() const; // never throws

  void swap(const monitored_reference& other);
  
  operator bool() const;
private:
  T* mptr;
};

template<class T, class U>
bool operator<(monitored_reference<T> const & a, monitored_reference<U> const & b);


#include "reference_monitor.inl.h"

#endif

