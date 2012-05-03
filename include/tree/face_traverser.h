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


#ifndef TREE_FACE_TRAVERSER_H
#define TREE_FACE_TRAVERSER_H

#include <tree/tree.h>
#include <tree/traverser.h>
#include <boost/concept/assert.hpp>

namespace tree{

  template<typename tree_type, typename corner_t, typename face_t>
  struct face_traverser{
    face_traverser();
    face_traverser(tree_type* node, const face_t& face);
    
    template<typename>
    friend class face_traverser;
    
    template<typename other_tree_type>
    face_traverser(const other_tree_type& other);
    
    
    bool has_children() const;
    bool is_child_of(const face_traverser& other) const;
    bool is_last_sibling() const;
    
    face_traverser first_child() const;
    face_traverser next_sibling() const;
    face_traverser parent() const;
    
    void reset();
    bool valid() const;
    tree_type& operator*() const;
    
    bool operator==(const face_traverser& other) const;
    bool operator!=(const face_traverser& other) const;
  private:
    tree_type* node;
    face_t face;
    typedef boost::array<corner_t, 4> face_corners_t;
  };

  
} // namespace tree

#include "face_traverser.inl.h"

#endif
