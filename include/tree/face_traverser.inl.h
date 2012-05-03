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


#include <tree/face_traverser.h>


namespace tree{

  
  template<typename tree_type>
  inline
  face_traverser<tree_type>::
  face_traverser()
    : node(NULL), face(cube::face_t::get(0))
  {

  }

  template<typename tree_type>
  inline
  face_traverser<tree_type>::
  face_traverser(tree_type* node, const cube::face_t& face)
    : node(node), face(face)
  {

  }
  
  template<typename tree_type>
  template<typename other_tree_type>
  inline
  face_traverser<tree_type>::
  face_traverser(const other_tree_type& other)
    : node(other.node), face(other.face)
  {
    
  }


  template<typename tree_type>
  inline
  tree_type& 
  face_traverser<tree_type>::
  face_traverser::operator*() const
  {
    BOOST_ASSERT(valid());
    return *node;
  }


  template<typename tree_type>
  inline
  bool 
  face_traverser<tree_type>::
  operator!=(const face_traverser& other) const
  {
    return node != other.node;
  }
  
  template<typename tree_type>
  inline
  bool 
  face_traverser<tree_type>::operator==(const face_traverser& other) const
  {
    return node == other.node;
  }


  template<typename tree_type>
  inline
  face_traverser<tree_type>
  face_traverser<tree_type>::
  parent() const
  {
    if (node)
    {
      if (node->is_child())
      {
        face_traverser result(node->parent(), face);
        
        BOOST_ASSERT(result.valid());
        BOOST_ASSERT(result.node->is_parent_of(*node));
        BOOST_ASSERT(node->is_child_of(*result.node));
        
        return result;
      }
    }
    return face_traverser(NULL, face);
  }

  template<typename tree_type>
  inline
  bool
  face_traverser<tree_type>::
  has_children() const
  {
    if (node)
      if (node->has_children())
        return true;
      
    return false;
  }

  template<typename tree_type>
  inline
  face_traverser<tree_type>
  face_traverser<tree_type>::
  first_child() const
  {
    const face_corners_t& face_corners = face.corners();
    
    if (node)
    {
      if (node->has_children())
      {
         face_traverser result(&(node->child(face_corners[0])), face);
         BOOST_ASSERT(result.valid());
         BOOST_ASSERT(result.node->is_child_of(*node));
         BOOST_ASSERT(node->is_parent_of(*result.node));
         
         return result;
      }
    }
    return face_traverser(NULL, face);
  }

  template<typename tree_type>
  inline
  bool 
  face_traverser<tree_type>::
  is_last_sibling() const
  {
    if (!node)
      ///This is really an error, but we will return true, since this function is
      ///usually used to terminate an iterative loop when true
      return true;
    
    const face_corners_t& face_corners = face.corners();
    
    return node->corner() == face_corners[3];
  }

  template<typename tree_type>
  inline
  face_traverser<tree_type>
  face_traverser<tree_type>::next_sibling() const
  {
    if (!node)
      ///This is an error
      return face_traverser(NULL, face);
    
    if (!node->is_child())
      ///This is an error
      return face_traverser(NULL, face);
    
    const face_corners_t& face_corners = face.corners();
    
    
    face_corners_t::const_iterator w = std::find(face_corners.begin(), face_corners.end(), node->corner());

    if (w == face_corners.end())
      ///This is an error
      return face_traverser(NULL, face);
    
    ++w;
    
    if (w == face_corners.end())
      ///This is an error
      return face_traverser(NULL, face);
    
    BOOST_ASSERT(node->parent());
    
    return face_traverser(&(node->parent()->child(*w)), face);
  }


  template<typename tree_type>
  inline
  void
  face_traverser<tree_type>::
  reset(){
    node = NULL;
  }

  template<typename tree_type>
  inline
  bool
  face_traverser<tree_type>::
  valid() const
  {
    return node != NULL;
  }

  template<typename tree_type>
  inline
  bool
  face_traverser<tree_type>::
  is_child_of(const face_traverser& other) const
  {
    if (node && other.node)
    {
      BOOST_ASSERT(other.valid());
      return (node->is_child_of(*other.node));
    }
    
    return false;
  }
  
  
  
  

} // namespace tree
