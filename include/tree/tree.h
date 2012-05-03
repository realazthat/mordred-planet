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


#ifndef TREE_TREE_H
#define TREE_TREE_H

#include <boost/iterator/indirect_iterator.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/range/iterator_range.hpp>

//#include "tree/face_traverser.h"


namespace tree{

template<typename T, std::size_t CHILDREN, typename corner_t>
struct root_t;

template<typename T, std::size_t CHILDREN, typename corner_t>
struct branch_t;



namespace detail{


#if 0

template<typename value_type, typename traverser_type>
struct  dual_pointer_traversal_iterator
  : public boost::iterator_facade<dual_pointer_traversal_iterator<value_type, traverser_type>,
                                  value_type,
                                  boost::forward_traversal_tag>
{
  BOOST_CONCEPT_ASSERT((Traverser<traverser_type, value_type>));
private:
  struct enabler {};  // a private type avoids misuse
public:
  dual_pointer_traversal_iterator()
    : root(), current(), last()
  {
    BOOST_ASSERT(!valid());
  }
  
  dual_pointer_traversal_iterator(const traverser_type& root, bool end)
    : root(root), current(root), last(root)
  {
    if (end)
      current.reset();
    
    BOOST_ASSERT(valid());
  }
  

  template <class OtherValue, class OtherTraverser>
  dual_pointer_traversal_iterator(
      dual_pointer_traversal_iterator<OtherValue, OtherTraverser> const& other
      
                        ///This parameter is ignored; its just here to make sure that @c other is convertable to this
                      , typename boost::enable_if<
                              boost::is_convertible<OtherValue*,value_type*>
                          , enabler
                        >::type = enabler()
    )
    : root(other.root), current(other.current), last(other.last)
  {
    BOOST_ASSERT(other.valid());
    BOOST_ASSERT(valid());
    
  }

  template<typename T>
  dual_pointer_traversal_iterator& operator=(const T& other)
  {
    assign(other);
    return *this;
  }

private:
  
  template <class, class>
  friend class dual_pointer_traversal_iterator;
  
  friend class boost::iterator_core_access;
  
  void increment() {
    BOOST_ASSERT(valid());
    BOOST_ASSERT(dereferencable());
    
    traverser_type old_current = current;
    {
      ///TODO think through what happens when root is actually the root of the tree and it has no children
      
      // up/right traversal loop
      while(true){
        if (on_my_way_down())
        {
          BOOST_ASSERT(current.is_child_of(last));
          BOOST_ASSERT(current != root);
          
          if (current.has_children())
          {
            last = current;
            current = current.first_child();
            break;
          } else if (!current.is_last_sibling()){
            BOOST_ASSERT(!current.has_children());
            
            last = current;
            current = current.next_sibling();
            break;
          } else {
            BOOST_ASSERT(!current.has_children());
            BOOST_ASSERT(current.is_last_sibling());
            last = current;
            current = current.parent();
            
            BOOST_ASSERT(on_my_way_up());
            continue;
          }
          
        } else if (on_my_way_up()) {
          if (current == root)
          {
            last = current;
            current.reset();
            
            break;
          } else if(!current.is_last_sibling() ) {
            BOOST_ASSERT(current != root);
            BOOST_ASSERT(!current.is_last_sibling());
            last = current;
            current = current.next_sibling();
            break;
          } else {
            last = current;
            current = current.parent();
            BOOST_ASSERT(on_my_way_up());
            continue;
          }
        } // on_my_way_up()
        else
        { // !on_my_way_up(), !on_my_way_down
          BOOST_ASSERT(!on_my_way_down() && !on_my_way_down());
        
          if (current == root)
          {
            BOOST_ASSERT(last == current);
            
            if ( current.has_children() )
            {
              last = current;
              current = current.first_child();
              break;
            } else {
              last = current;
              current.reset();
              break;
            }
          }
          else 
          {
            
            BOOST_ASSERT(last.parent() == current.parent());
            BOOST_ASSERT(!last.is_last_sibling());
            BOOST_ASSERT(last.next_sibling() == current);
            
            if (current.has_children())
            {
              last = current;
              current = current.first_child();
              BOOST_ASSERT(on_my_way_down());
              break;
            } else if (!current.is_last_sibling()){
              last = current;
              current = current.next_sibling();
              
              BOOST_ASSERT(!on_my_way_down() && !on_my_way_up());
              break;
            } else {
              last = current;
              current = current.parent();
              BOOST_ASSERT(on_my_way_up());
              continue;
            }
          }
        }
        BOOST_ASSERT(false && "Something is wrong");
      } // up/right traversal loop
    }
    
    BOOST_ASSERT(current != old_current);
    BOOST_ASSERT(valid());
    BOOST_ASSERT(!on_my_way_up());
    BOOST_ASSERT(on_my_way_down() || !on_my_way_up());
  }
  
  value_type& dereference() const {
    BOOST_ASSERT(valid());
    BOOST_ASSERT(dereferencable());
    return *current;
  }
  
  template<typename T>
  bool equal(const T& other) const
  {
    return equal_internal(other);
  }
  
private:
  bool valid() const{
    ///TODO: triple check this
    return root.valid() && last.valid();
  }
  
  bool dereferencable() const{
    ///TODO: triple check this
    
    return valid() && current.valid();
  }
  
  template <class OtherValue, class OtherTraverser>
  void
  assign(dual_pointer_traversal_iterator<OtherValue, OtherTraverser> const& other,
                ///This parameter is ignored; its just here to make sure that @c other is convertable to this
                typename boost::enable_if<
                      boost::is_convertible<OtherValue*,value_type*>
                  , enabler
                >::type = enabler() )
  {
    BOOST_ASSERT(other.valid());
    bool other_is_derefable = other.dereferencable();
    
    ///TODO: double check this is all members before giving green light
    this->root = other.root;
    this->current = other.current;
    this->last = other.last;
    
    BOOST_ASSERT(valid());
    BOOST_ASSERT(liff(other_is_derefable, dereferencable()));
  }
  
  template <class OtherValue, class OtherTraverser>
  bool equal_internal(dual_pointer_traversal_iterator<OtherValue, OtherTraverser> const& other,
                ///This parameter is ignored; its just here to make sure that @c other is convertable to this
                typename boost::enable_if<
                      boost::is_convertible<OtherValue*,value_type*>
                  , enabler
                >::type = enabler()
    ) const
  {
    return (this->root == other.root) && (this->current == other.current) && (this->last == other.last);
  }
  
  
  
  bool on_my_way_down() const{
    BOOST_ASSERT(valid());
    return current.is_child_of(last);
  }
  
  bool on_my_way_up() const{
    BOOST_ASSERT(valid());
    return last.is_child_of(current);
  }
  
  traverser_type root;
  traverser_type current;
  traverser_type last;
};

#endif

} // namespace detail




/**
 * 
 *                      stack based traversal
 *                              (best/average/worst):   O(n)/O(n)/O(n) traversal,
 *                                                      O(1)/O(1)/O(1) increment time,
 *                                                      O(depth of tree) memory
 *                      
 *                      dual pointer based
 *                              (best/average/worst/[amortized constant]):
 *                                                      O(n)/O(n)/O(n) traversal,
 *                                                      O(1)/O(1)/O(depth of tree)/O(1) increment time
 *                                                      O(1)/O(1)/O(1) memory
 *                      left-most-leaf-link:
 *                                                      O(n)/O(n)/O(n) traversal,
 *                                                      O(1)/O(1)/O(1) increment time
 *                                                      O(1)/O(1)/O(1) memory
 * Features TODO:
 * 
 * node-counter
 * ordered-face traverser
 *      clockwise vs counter clockwise
 * leaf-to-root traverser
 *      leaf-first traverser so one can modify the nodes
 * leaf-to-root face-traverser
 * leaf-traverser
 * level-traverser
 *      Traversal of all nodes on a level (or closest possible to that level)
 * octree-web
 *      A web of linked non-overlapping nodes on different levels that define some depth of an octree.
 *      An example use would be all visible nodes in a view dependent octree.
 * back-to-front traversal
 * back-to-front traversal of a web
 * 
 * destruct iteratively leaf-to-root otherwise stack might not like the destructor recursion
 * 
 * scaffolding to search the tree
 * some sort of bounded-box info:
 *      ~ allows for easy is_descendant checking
 *      ~ can be used to search the tree
 * 
 * other functionality:
 *      ~ is_descendant_of(other)
 *      ~ is_ancestor_of(other)
 *      ~ is_adjacent_to(other)/is_on_face_of(other)
 * 
 * helper classes
 *      ~ heirarchical occluder tracking
 *      ~ occlusion checking
 *      ~ occluder detector
 * 
 * mixouts
 *      make these separate optional mixins:
 *              ~ adjacency policy
 *                      nodes should keep track of their adjacent facing out-of-parent neighbors
 *              ~ parent policy
 *                      nodes should know their parent
 *              ~ sibling policy
 *                      how nodes find their sibling
 *                      methods:
 *                      ~ using the parent, keeping track of node's child id, finding the other children
 *                      ~ keeping neihbor links
 *                      ~ implicitly in a constant depth tree
 *              ~ root policy
 *                      how nodes can find the root
 *                      methods:
 *                              ~ keep a pointer to the root, obtained upon creation
 *                              ~ climbing the tree until the root, worst case O(depth of tree)
 *                              ~ implicitly in a constant depth tree
 *              ~ node counting policy
 *                      how the root should be able to count the nodes in the tree
 *                      methods:
 *                              ~ nodes keep the root informed of their creation and destruction, worst case O(1) to count
 *                              ~ the root traverses the tree counting the nodes, an O(n) operation to count
 *                      how nodes should be able to count the number of children they have
 *                      methods:
 *                              ~ the node traverses the tree counting the nodes, an O(n) worst case operation to count
 *                              ~ each node keeps a count of their desendents, insertion and deletion would now cost >= O(depth of tree) worst case
 *              ~ level policy
 *                      each node keeps track of its level
 *                      methods:
 *                              ~ each node keeps its own level, consequently requires storing an extra integer, obtained upon creation,
 *                                       O(1) to find the level
 *                              ~ recursively climb to the root, consequently an O(depth of tree) worst case operation
 *              ~ constant depth policy
 *                      All nodes can be preallocated and disallow growth of the tree
 *                      consquences:
 *                              ~ adjacency is implicit and O(1)
 *                              ~ locating parent is implicitly trivial
 *                              ~ locating siblings is implicitly trivial
 *                              ~ locating the root is trival O(1)
 *                              ~ finding the level of a node is trival
 * 
 *                              ~ memory requirement of n nodes
 *              ~ left-most-leaf-node-tracker policy
 *                      all nodes should keep track of their left most leaf nodes
 *                      consequences:
 *                              ~ insertion possibly requires informing O(depth of three) nodes of their new left-most-leaf
 *                              ~ deletion possibly requires informing O(depth of three) nodes of their new left-most-leaf
 *                              ~ now dual-pointer leaf-to-root traversal can be done in O(1)/O(1)/O(1)
 *              ~ corner policy
 *                      make use of cube::corner_t a policy that decides how many children each node has (make it no longer an octree but an N?-tree)
 */

template<typename T, std::size_t CHILDREN, typename corner_t>
struct branch_t
  : private boost::noncopyable
{
public:
  typedef root_t<T, CHILDREN, corner_t> root_type;
  
  typedef branch_t<T, CHILDREN, corner_t> self_type;
  typedef self_type child_type;
  typedef self_type adjacent_type;
  typedef std::pair<adjacent_type*, adjacent_type*> adjacent_link;
  
  typedef boost::array<boost::scoped_ptr<child_type>, CHILDREN> children_type;
  typedef boost::indirect_iterator< typename children_type::iterator > child_iterator;
  typedef boost::indirect_iterator< typename children_type::const_iterator > const_child_iterator;
  
  //typedef detail::dual_pointer_traversal_iterator<branch_t, face_traverser<branch_t, corner_t, face_t> > face_iterator;
  //typedef detail::dual_pointer_traversal_iterator<const branch_t, face_traverser<const branch_t, corner_t, face_t> > const_face_iterator;
  //typedef detail::dual_pointer_traversal_iterator<branch_t, corner_traverser<branch_t> > traversal_iterator;
  //typedef detail::dual_pointer_traversal_iterator<const branch_t, corner_traverser<const branch_t> > const_traversal_iterator;
  
  typedef T value_type;
  
  
  branch_t(root_type& root, branch_t* parent, T value, std::size_t level, const corner_t& corner);
  ~branch_t();
  
  
  
  boost::iterator_range< child_iterator > children();
  boost::iterator_range< const_child_iterator > children() const;
  
  
  //boost::iterator_range< traversal_iterator > traversal();
  //boost::iterator_range< const_traversal_iterator > traversal() const;
  
  
  
  
  void split();
  void join();
  
  child_type& child(const corner_t& corner);
  const child_type& child(const corner_t& corner) const;
  
  //adjacent_type* adjacent(const cube::direction_t& direction);
  //const adjacent_type* adjacent(const cube::direction_t& direction) const;
  
  //boost::iterator_range< face_iterator > face_traversal(const square::face_t& face);
  //boost::iterator_range< const_face_iterator > face_traversal(const square::face_t& face) const;
  //boost::iterator_range< const_face_iterator > cface_traversal(const square::face_t& face) const;
  
  
  //boost::iterator_range< parent_iterator > parents();
  //boost::iterator_range< neighbor_iterator > neighbors();
  //boost::iterator_range< adjacent_iterator > adjacent();
  
  //typedef branch_t<T> adjacent_type;
  
  const corner_t& corner() const;
  
  T& value();
  const T& value() const;
  
  root_type& root();
  const root_type& root() const;
  
  bool is_root() const;
  bool has_children() const;
  bool is_child() const;
  bool is_child_of(const self_type& other) const;
  bool is_parent_of(const self_type& other) const;
  const adjacent_type* previous_brother() const;
  const adjacent_type* next_brother() const;
  const child_type* first_child() const;
  const child_type* last_child() const;
  
  
  self_type* parent();
  const self_type* parent() const;
  
  std::size_t level() const;
private:
  //No slicing
  branch_t(const root_type&);
  
  root_type& mroot;
  self_type* mparent;
  T mvalue;
  std::size_t mlevel;
  corner_t mcorner;
  
private:
#if 0
  void set_adjacent_link(const cube::direction_t& direction, adjacent_type* adjacent_node);
  adjacent_type* get_adjacent_link(const cube::direction_t& direction);
  const adjacent_type* get_adjacent_link(const cube::direction_t& direction) const;
  template<typename cv_tree_type>
  static cv_tree_type* get_adjacent_link(cv_tree_type& tree_node, const cube::direction_t& direction);
  
  ///This code was getting to complicated to duplicate for const and non-const functions
  template<typename cv_tree_type>
  static
  cv_tree_type*
  adjacent(cv_tree_type& from_node, const cube::direction_t& direction);
  
  boost::array<adjacent_type*, 6> adjacent_links;
  
  void initialize_adjacencies();
  
  void uninitialize_adjacencies();
#endif
private:
  boost::scoped_ptr< children_type > mchildren;
};





template<typename T, std::size_t CHILDREN, typename corner_t>
struct root_t
  : public branch_t<T, CHILDREN, corner_t>
{
  typedef branch_t<T, CHILDREN, corner_t> super;
  
  root_t(T value);
  root_t();
  
};





} // namespace tree


#include "tree.inl.h"

#endif // TREE_H

