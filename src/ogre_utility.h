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
#ifndef MORDRED_OGRE_UTILITY_H
#define MORDRED_OGRE_UTILITY_H


#include <cstddef>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>


#include <OGRE/OgreHardwareBuffer.h>
#include <OGRE/OgreRenderable.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreAxisAlignedBox.h>
#include <boost/array.hpp>
#include <OGRE/OgreVector2.h>




struct HardwareBufferScopedLock
  : private boost::noncopyable
{
  template<typename LockOptions>
  HardwareBufferScopedLock(Ogre::HardwareBuffer& buffer,
                           const LockOptions& options)
    : buffer(buffer)
  {
    mdata = buffer.lock(options);
  }
  
  template<typename LockOptions>
  HardwareBufferScopedLock(Ogre::HardwareBuffer& buffer,
                           std::size_t offset,
                           std::size_t size,
                           const LockOptions& options)
    : buffer(buffer)
  {
    mdata = buffer.lock(offset, size, options);
  }
  
  ~HardwareBufferScopedLock()
  {
    buffer.unlock();
  }
  
  void* data()
  {
    return mdata;
  }
private:
  Ogre::HardwareBuffer& buffer;
  void* mdata;
};



struct ChunkRenderable : public Ogre::Renderable
{
  ChunkRenderable(const Ogre::MaterialPtr& mat, const Ogre::MovableObject& movable);
  virtual ~ChunkRenderable();
  virtual const Ogre::LightList& getLights(void ) const;
  virtual const Ogre::MaterialPtr& getMaterial(void ) const;
  virtual void getRenderOperation(Ogre::RenderOperation& op);
  virtual Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
  virtual void getWorldTransforms(Ogre::Matrix4* xform) const;
  
  Ogre::MaterialPtr mat;
  const Ogre::MovableObject& movable;
  boost::scoped_ptr<Ogre::VertexData> vertex_data;
  boost::scoped_ptr<Ogre::IndexData> index_data;
  Ogre::RenderOperation renderop;
};


struct transformer{
  
  transformer(const Ogre::AxisAlignedBox& from, const Ogre::AxisAlignedBox& to)
  {
    {
      Ogre::Matrix4 unit2from;
      Ogre::Vector3 ref = from.getMinimum();
      Ogre::Vector3 size = from.getSize();
      unit2from.makeTransform(ref, size, Ogre::Quaternion());
      from2unit = unit2from.inverse();
    }
    
    
    {
      Ogre::Vector3 ref = to.getMinimum();
      Ogre::Vector3 size = to.getSize();
      unit2to.makeTransform(ref, size, Ogre::Quaternion());
    }
    
    tform = unit2to * from2unit;
  }
  
  
  Ogre::Vector3 transform(const Ogre::Vector3& pos) const
  {
    return tform * pos;
  }
  
private:
  
  Ogre::Matrix4 from2unit, unit2to, tform;

};






template<typename tree_type>
tree_type* tree_search(tree_type& tree, const Ogre::Vector3& position, std::size_t max_level)
{
  tree_type* current_tree = &tree;
  
  if (!current_tree->value()->bounds.contains(position))
    return NULL;
  
  while(current_tree->level() < max_level){
    BOOST_ASSERT(current_tree->value()->bounds.contains(position));
    
    if (!current_tree->has_children())
      return current_tree;
    
    tree_type* old_current_tree = current_tree;
    
    BOOST_FOREACH(tree_type& child, current_tree->children())
    {
      const Ogre::AxisAlignedBox& box = child.value()->bounds;
      
      if(box.contains(position))
      {
        current_tree = &child;
        break;
      }
    }
    
    BOOST_ASSERT(current_tree != old_current_tree);
  }
  
  return current_tree;
}

void draw_axis(Ogre::ManualObject* man, std::size_t index_offset = 0);







/**
 * Lock a hardware buffer, and fill it with indices from a std::vector container.
 * If the hardware buffer is using indices of the same size as the container,
 * this function is specialized to use the optimized HardwareIndexBuffer::writeData(),
 * using the data from std::vector::data(). This avoids some complexity with regard to
 * locking, and might show some performance advantage.
 */
template<typename gpu_index_type, typename indices_t>
static std::size_t lock_and_fill_indices(Ogre::HardwareIndexBuffer& ibuf,
                                         std::size_t buffer_indices_offset,
                                         const indices_t& indices,
                                         typename boost::enable_if<
                                                    boost::mpl::bool_< (sizeof(gpu_index_type) == sizeof(typename indices_t::value_type)) >
                                                    >::type* dummy = 0)
{
  ///Avoid warning
  (void)dummy;
  
  
  std::size_t bytes_offset = buffer_indices_offset * sizeof(gpu_index_type);
  std::size_t bytes_data_length = indices.size() * sizeof(gpu_index_type);
  
  ibuf.writeData(bytes_offset, bytes_data_length, indices.data());
  
  return indices.size();
}

/**
 * Lock a hardware buffer, and fill it with indices from a std::vector container.
 * If the hardware buffer is using indices of the same size as the container,
 * this function is specialized to lock the buffer and insert all of the indices.
 */
template<typename gpu_index_type, typename indices_t>
static std::size_t lock_and_fill_indices(Ogre::HardwareIndexBuffer& ibuf,
                                         std::size_t buffer_indices_offset,
                                         const indices_t& indices,
                                         typename boost::enable_if<
                                                    boost::mpl::bool_< (sizeof(gpu_index_type) != sizeof(typename indices_t::value_type)) >
                                                    >::type* dummy = 0)
{
  ///Avoid warning
  (void)dummy;
  
  using namespace Ogre;
  
  std::size_t bytes_offset = buffer_indices_offset * sizeof(gpu_index_type);
  std::size_t bytes_data_length = indices.size() * sizeof(gpu_index_type);
  
  std::size_t tries = 0;
  while (tries++ < 3)
  {
    try {
      ///FIXME: can this be HBL_DISCARD??
      HardwareBufferScopedLock lock_guard(ibuf, bytes_offset, bytes_data_length, HardwareBuffer::HBL_NORMAL);
    
      gpu_index_type* gpu_index_ptr0 = static_cast<gpu_index_type*>(lock_guard.data());
      gpu_index_type* gpu_index_ptr = gpu_index_ptr0;
      
      BOOST_FOREACH(const typename indices_t::value_type& index, indices)
      {
        *gpu_index_ptr++ = index;
      }
      
      return gpu_index_ptr - gpu_index_ptr0;
    ///FIXME: what is the exact exception for gpu lock failure?
    }  catch (Ogre::Exception& e) {
      ///Failed to lock,
      
      ///Try again
      continue;
    }
  }
  
  throw std::runtime_error("GPU lock failing on HardwareIndexBuffer");
}


inline
Ogre::Vector2
minimized_vector(const Ogre::Vector2& lhs, const Ogre::Vector2& rhs)
{
  return Ogre::Vector2(std::min(lhs.x, rhs.x),
                       std::min(lhs.y, rhs.y));
}

inline
Ogre::Vector2
maximized_vector(const Ogre::Vector2& lhs, const Ogre::Vector2& rhs)
{
  return Ogre::Vector2(std::max(lhs.x, rhs.x),
                       std::max(lhs.y, rhs.y));
}










#endif // OGRE_UTILITY_H
