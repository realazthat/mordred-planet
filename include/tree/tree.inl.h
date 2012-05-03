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


#include "tree/tree.h"

#include <boost/range.hpp>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>


namespace tree{
  
template<typename T, std::size_t CHILDREN, typename corner_t>
inline
root_t<T, CHILDREN, corner_t>::
root_t(T value)
  : super(*this, NULL, value, 0, corner_t::get(0))
{

}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
root_t<T, CHILDREN, corner_t>::
root_t()
  : super(*this, NULL, T(), 0, corner_t::get(0))
{

}



#if 0

template<typename T>
inline
void
branch_t<T>::
initialize_adjacencies()
{
  //TODO: make sure this adjacency thingy works for the root
  //TODO: make sure this adjacency thingy works in destruction too
  //TODO: optimize adjacency in destruction of parent with lots of children
  //TODO: make sure the destructor iteratively destroys its children, bottom up
  ///@note don't use any member functions here, some things haven't finished initializing yet
  ///@note don't depend on parent knowing its children yet; and we cannot yet know our siblings either
  /// as we are being just now created and haven't yet been inserted into our parents
  
  BOOST_FOREACH(const cube::face_t& face, corner().faces())
  {
    set_adjacent_link(face.direction(), NULL);
  }
  
  if (!mparent)
    return;
  
  
  
  ///For each adjacent node (possibly non-existent),
  BOOST_FOREACH(const cube::face_t& face, mcorner.faces())
  {
    const cube::direction_t& adjacent_direction = face.direction();
    
    
    
    /*
    ///Link me to adjacent node or closest parent of adjacent node
    branch_t* adjacent_node_parent_closest_ptr = NULL;
    if (mparent->corner().face_set().contains(face))
    {
      adjacent_node_parent_closest_ptr = mparent->adjacent(adjacent_direction);
      
      ///If parent has no adjacent, we are at the edge of the tree, and we don't either
      if (!adjacent_node_parent_closest_ptr)
        continue;
    } else if(mparent->is_child()) {
      const cube::corner_t& parent_corner = mparent->corner();
      const cube::corner_t& ungle_corner = parent_corner.adjacent(adjacent_direction);
      
      adjacent_node_parent_closest_ptr = &(mparent->mparent->child(ungle_corner));
    } else {
      ///We are close to the root, and thus the edge, no adjacency
      continue;
    }
    */
    
    branch_t* adjacent_node_parent_closest_ptr = mparent->adjacent(adjacent_direction);
    
    if (!adjacent_node_parent_closest_ptr)
    {
      ///We are close to the root, or the edge, and have no adjacency
      continue;
    }
    
    BOOST_ASSERT(!!adjacent_node_parent_closest_ptr);
    
    branch_t& adjacent_node_parent_closest = *adjacent_node_parent_closest_ptr;
    
    ///My parent's adjacent node in this direction is at least as low as my parent's level
    BOOST_ASSERT(adjacent_node_parent_closest.level() <= mparent->level());
    
    ///My parent's adjacent node in this direction is definitely lower than my level
    BOOST_ASSERT(adjacent_node_parent_closest.level() < level());
    
    
    bool adjacent_node_parent_closest_has_children = adjacent_node_parent_closest.has_children();
    bool parent_links_same = (adjacent_node_parent_closest.level() == level() - 1);
    bool parent_links_lower = (adjacent_node_parent_closest.level() < level() - 1);
    
    ///If my parent's adjacent link is not on my parent's level
    ///Then my parent's adjacent doesn't have children (otherwise my parent would point to them)
    BOOST_ASSERT( lif(!parent_links_same, adjacent_node_parent_closest_has_children) );
    ///Same as above, just sanity testing
    BOOST_ASSERT( lif(parent_links_lower, adjacent_node_parent_closest_has_children) );
    
    ///If my parent's adjacent link has children,
    ///Then my parent must be on the same level as it, otherwise my parent would point lower to its children.
    BOOST_ASSERT( lif(adjacent_node_parent_closest_has_children, parent_links_same) );
    
    if (!adjacent_node_parent_closest_has_children)
    {
      ///If the parent adjacent has no children
      
      ///Link to the parent adjacent
      set_adjacent_link(adjacent_direction, &adjacent_node_parent_closest);
      
    } // !adjacent_node_parent_closest_has_children
    else
    { // adjacent_node_parent_closest_has_children
      ///If the parent adjacent has children
      
      ///If my parent's adjacent link has children,
      ///Then my parent must be on the same level as it, otherwise my parent would point lower to its children.

      BOOST_ASSERT(parent_links_same);
      
      ///If there is an existing node on my level, that is adjacent to me
      
      
      const cube::corner_t& facing_adjacent_corner = corner().adjacent(adjacent_direction.opposite());
      
      ///TODO
      branch_t& adjacent_node = adjacent_node_parent_closest.child(facing_adjacent_corner);
      
      ///Set this node as facing @c adjacent_node
      set_adjacent_link(adjacent_direction, &adjacent_node);
      
      ///Adjacent node should be linked to my parent
      BOOST_ASSERT((adjacent_node.adjacent(adjacent_direction.opposite())) == mparent);
      
      ///Adjacent node might have children, those facing me would be linked to my parent
      ///They should be relinked to me
      
      BOOST_FOREACH(branch_t& facing_node, adjacent_node.face_traversal(adjacent_direction.opposite().face()))
      {
      ///For each facing node that is inside of the node adjacent to me (includes the adjacent node itself)
        
        ///The facing node would have been facing my parent
        ///Therefore if my parent is NULL, it should think that *it's* adjacent neighbor in *my* direction
        /// is NULL.
        ///If my parent is not NULL. it should think that *it's* adjacent neighbor in *my* direction
        /// is my parent
        BOOST_ASSERT(
            (mparent == NULL && facing_node.adjacent(adjacent_direction.opposite()) == NULL)
          || (facing_node.adjacent(adjacent_direction.opposite()) == mparent));
        
        ///Inform the facing node of it's new adjacent neighbor, *me*
        facing_node.set_adjacent_link(adjacent_direction.opposite(), this);
      }
      
    } // adjacent_node_parent_closest_has_children
  } // foreach adjacent face to this corner
  
}
template<typename T>
inline
void
branch_t<T>::
uninitialize_adjacencies()
{
  if (!mparent)
    return;
  ///TODO
  
}






template<typename T>
inline
void
branch_t<T>::
set_adjacent_link(const cube::direction_t& direction, adjacent_type* adjacent_node)
{
  BOOST_ASSERT((direction.index() < adjacent_links.size()) && "Invalid direction");
  
  BOOST_ASSERT(corner().face_set().contains(direction.face()) && "Adjacent links do not include siblings");
  
  adjacent_links[direction.index()] = adjacent_node;
}

#endif

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
branch_t<T, CHILDREN, corner_t>::
branch_t(root_type& root, branch_t* parent, T value, std::size_t level, const corner_t& corner)
  : mroot(root), mparent(parent), mvalue(value), mlevel(level), mcorner(corner), mchildren()
{
  ///FIXME: re-enable this when face iterator is working again
  ///Make sure our iterator is convertable to const_iterator
  //BOOST_STATIC_ASSERT((boost::is_convertible<face_iterator, const_face_iterator>::value));
  
  

}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
branch_t<T, CHILDREN, corner_t>::
~branch_t()
{
  join();
  
}





template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::const_child_iterator >
branch_t<T, CHILDREN, corner_t>::
children() const
{
  if (mchildren)
  {
    return boost::make_iterator_range(const_child_iterator(mchildren->begin()), const_child_iterator(mchildren->end()));
  }
  
  return boost::make_iterator_range(const_child_iterator(), const_child_iterator());
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::child_iterator >
branch_t<T, CHILDREN, corner_t>::
children()
{
  if (mchildren)
  {
    return boost::make_iterator_range(child_iterator(mchildren->begin()), child_iterator(mchildren->end()));
  }
  
  return boost::make_iterator_range(child_iterator(), child_iterator());
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
T&
branch_t<T, CHILDREN, corner_t>::
value()
{
  return mvalue;
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
const T&
branch_t<T, CHILDREN, corner_t>::
value() const
{
  return mvalue;
}

template<typename T, std::size_t CHILDREN, typename corner_t>
const typename branch_t<T, CHILDREN, corner_t>::root_type&
branch_t<T, CHILDREN, corner_t>::
root() const
{
  return mroot;
}

template<typename T, std::size_t CHILDREN, typename corner_t>
typename branch_t<T, CHILDREN, corner_t>::root_type&
branch_t<T, CHILDREN, corner_t>::
root()
{
  return mroot;
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
void
branch_t<T, CHILDREN, corner_t>::
split()
{
  if (!mchildren)
  {
    mchildren.reset( new children_type() );
    
    BOOST_FOREACH(const corner_t& c, corner_t::all())
    {
      boost::scoped_ptr<child_type>& child_ptr = (*mchildren)[c.index()];
      child_ptr.reset( new child_type(root(), this, T(), mlevel + 1, c) );
      
      BOOST_ASSERT(!child_ptr->is_root());
      BOOST_ASSERT(child_ptr->is_child());
      BOOST_ASSERT(this->is_parent_of(*child_ptr));
      BOOST_ASSERT(child_ptr->is_child_of(*this));
    }
   
  }
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
void
branch_t<T, CHILDREN, corner_t>::
join()
{
  ///FIXME: this will make a recursive destructor call all the way down the tree,
  ///we should do this with a receding-depth-first traversal to destroy the highest levels of the tree first iteratively
  mchildren.reset();
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
const corner_t&
branch_t<T, CHILDREN, corner_t>::
corner() const
{
  return mcorner;
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
typename branch_t<T, CHILDREN, corner_t>::child_type&
branch_t<T, CHILDREN, corner_t>::
child(const corner_t& corner)
{
  BOOST_ASSERT(mchildren);
  return *(*mchildren)[corner.index()];
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
const typename branch_t<T, CHILDREN, corner_t>::child_type&
branch_t<T, CHILDREN, corner_t>::
child(const corner_t& corner) const
{
  BOOST_ASSERT(mchildren);
  return *(*mchildren)[corner.index()];
}



template<typename T, std::size_t CHILDREN, typename corner_t>
inline
std::size_t
branch_t<T, CHILDREN, corner_t>::
level() const
{
  return mlevel;
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
branch_t<T, CHILDREN, corner_t>*
branch_t<T, CHILDREN, corner_t>::parent()
{
  if (mparent) {
    BOOST_ASSERT(mparent != this);
    BOOST_ASSERT(mparent->has_children());
    BOOST_ASSERT(mparent->is_parent_of(*this));
  }
  
  return mparent;
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
const branch_t<T, CHILDREN, corner_t>*
branch_t<T, CHILDREN, corner_t>::
parent() const
{
  if (mparent) {
    BOOST_ASSERT(mparent != this);
    BOOST_ASSERT(mparent->has_children());
    BOOST_ASSERT(mparent->is_parent_of(*this));
  }
  
  return mparent;
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
bool
branch_t<T, CHILDREN, corner_t>::is_root() const
{
  return &mroot == this;
}

#if 0
template<typename T>
template<typename cv_tree_type>
cv_tree_type*
branch_t<T>::
get_adjacent_link(cv_tree_type& from_node, const cube::direction_t& direction)
{
  
  BOOST_ASSERT(direction.index() < from_node.adjacent_links.size());
  
  if (from_node.mcorner.face_set().contains(direction.face()))
    return from_node.adjacent_links[direction.index()];
  
  if (from_node.mparent)
  {
    const cube::corner_t& sibling_corner = from_node.mcorner.adjacent(direction);
    return &(from_node.mparent->child(sibling_corner));
  }
  return NULL;
}

template<typename T>
typename branch_t<T>::adjacent_type*
branch_t<T>::
get_adjacent_link(const cube::direction_t& direction)
{
  return get_adjacent_link(*this, direction);
}

template<typename T>
const typename branch_t<T>::adjacent_type*
branch_t<T>::
get_adjacent_link(const cube::direction_t& direction) const
{
  return get_adjacent_link(*this, direction);
}


template<typename T>
template<typename cv_tree_type>
inline
cv_tree_type*
branch_t<T>::
adjacent(cv_tree_type& from_node, const cube::direction_t& direction)
{
  cv_tree_type* result = from_node.get_adjacent_link(direction);
  if (!result)
    return result;
  
  ///I only point to nodes that are on my level, or lower
  BOOST_ASSERT(result->level() <= from_node.level());
  
  ///If I am adjacent to a node, they must be adjacent to *a* node in the opposite direction
  /// (a sibling of it's, me, or one of my ancestors) FIXME:
  //BOOST_ASSERT(!!result->adjacent(direction.opposite()));
  
#ifndef NDEBUG
  ///Find my ancestor that is on the same level as my adjacent node
  cv_tree_type* reverse_adjacent_ancestor_or_self = &from_node;
  
  ///Sanity
  BOOST_ASSERT(reverse_adjacent_ancestor_or_self);
  {
    for (std::size_t i = 0; i < from_node.level() - result->level(); ++i)
    {
      BOOST_ASSERT(reverse_adjacent_ancestor_or_self->is_child());
      BOOST_ASSERT(reverse_adjacent_ancestor_or_self->mparent);
      reverse_adjacent_ancestor_or_self = reverse_adjacent_ancestor_or_self->mparent;
      BOOST_ASSERT(reverse_adjacent_ancestor_or_self);
    }
  }
  
  ///My ancestor, on the same level as *my* "adjacent node", should be the "adjacent node" of *my* "adjacent node"
  ///FIXME:this is recursion ...
  //BOOST_ASSERT(reverse_adjacent_ancestor_or_self == result->adjacent(direction.opposite()));
#endif
  
  return result;
}

template<typename T>
inline
typename branch_t<T>::adjacent_type*
branch_t<T>::
adjacent(const cube::direction_t& direction)
{
  return adjacent(*this, direction);
}

template<typename T>
inline
const typename branch_t<T>::adjacent_type*
branch_t<T>::
adjacent(const cube::direction_t& direction) const
{
  return adjacent(*this, direction);
}

#endif

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
bool 
branch_t<T, CHILDREN, corner_t>::
has_children() const
{
  return !!mchildren;
}

/*
template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::const_traversal_iterator >
branch_t<T, CHILDREN, corner_t>::
branch_t::traversal() const
{
  typedef corner_traverser<const branch_t> traverser_t;
  traverser_t me(this);
  return boost::make_iterator_range( const_traversal_iterator(me, false),
                              const_traversal_iterator(me, true) );
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::traversal_iterator >
branch_t<T, CHILDREN, corner_t>::
branch_t::traversal()
{
  typedef corner_traverser<branch_t> traverser_t;
  traverser_t me(this);
  return boost::make_iterator_range( traversal_iterator(me, false),
                              traversal_iterator(me, true) );
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::face_iterator >
branch_t<T, CHILDREN, corner_t>::
face_traversal(const cube::face_t& face)
{
  typedef face_traverser<branch_t> traverser_t;
  traverser_t me(this, face);
  return boost::make_iterator_range( face_iterator(me, false),
                              face_iterator(me, true) );
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::const_face_iterator >
branch_t<T, CHILDREN, corner_t>::
face_traversal(const cube::face_t& face) const
{
  typedef face_traverser<const branch_t> traverser_t;
  traverser_t me(this, face);
  return boost::make_iterator_range( const_face_iterator(me, false),
                              const_face_iterator(me, true) );
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
boost::iterator_range< typename branch_t<T, CHILDREN, corner_t>::const_face_iterator >
branch_t<T, CHILDREN, corner_t>::
cface_traversal(const cube::face_t& face) const
{
  return face_traversal(face);
}
*/

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
bool
branch_t<T, CHILDREN, corner_t>::
branch_t::is_child() const
{
  return !!mparent;
}


template<typename T, std::size_t CHILDREN, typename corner_t>
inline
bool
branch_t<T, CHILDREN, corner_t>::
branch_t::is_child_of(const branch_t<T, CHILDREN, corner_t>& other) const
{
  if (!!mparent && (mparent == &other))
  {
    BOOST_ASSERT(other.mchildren);
    BOOST_ASSERT(&*(*other.mchildren)[corner().index()] == this);
    return true;
  }
  return false;
}

template<typename T, std::size_t CHILDREN, typename corner_t>
inline
bool
branch_t<T, CHILDREN, corner_t>::
branch_t::is_parent_of(const self_type& other) const
{
  if ( !!other.mparent && (other.mparent == this) )
  {
    BOOST_ASSERT(has_children());
    BOOST_ASSERT(other.is_child());
    BOOST_ASSERT(!!mchildren);
    BOOST_ASSERT(!!(*mchildren)[other.corner().index()]);
    BOOST_ASSERT(&*(*mchildren)[other.corner().index()] == &other);
    return true;
  }
  return false;
}

} // namespace tree


