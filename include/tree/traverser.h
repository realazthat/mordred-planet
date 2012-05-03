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


#ifndef OCTREE_TRAVERSER_H
#define OCTREE_TRAVERSER_H


#include <boost/concept_check.hpp>
#include <boost/concept/usage.hpp>
#include <boost/ref.hpp>


namespace tree{


/**
 * Traverser concept
 * 
 * @param X
 *        The traverser class
 * @param
 *        The value type the traverser returns
 */
template<typename X, typename V>
struct Traverser
  : boost::Assignable<X>,
    boost::EqualityComparable<X>,
    boost::CopyConstructible<X>,
    boost::DefaultConstructible<X>
{
  
  
  BOOST_CONCEPT_USAGE(Traverser)
  {
    is_traverser(boost::cref(child_traveser).get().parent());
    is_traverser(boost::cref(child_traveser).get().first_child());
    is_traverser(boost::cref(child_traveser).get().next_sibling());
    
    is_boolean(boost::cref(child_traveser).get().is_child_of(parent_traverser));
    is_boolean(boost::cref(child_traveser).get().is_child_of(X()));
    is_boolean(boost::cref(parent_traverser).get().has_children());
    is_boolean(boost::cref(parent_traverser).get().is_last_sibling());
    is_boolean(boost::cref(parent_traverser).get().valid());
    
    is_boolean(boost::cref(parent_traverser).get().operator==(X()));
    is_boolean(boost::cref(parent_traverser).get().operator==(parent_traverser));
    is_boolean(boost::cref(parent_traverser).get().operator!=(X()));
    is_boolean(boost::cref(parent_traverser).get().operator!=(child_traveser));
    
    is_value(boost::cref(child_traveser).get().operator*());
    
    parent_traverser.reset();
  }

  void is_boolean(bool);
  void is_traverser(X);
  void is_value(V&);

  X parent_traverser;
  X child_traveser;
};









} // namespace tree

#endif
