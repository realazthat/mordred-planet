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


#ifndef PLANET_VOLUME_H
#define PLANET_VOLUME_H

#include <OGRE/OgreMovableObject.h>
#include <tree/tree.h>
#include <square/square.h>

#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index_container.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

namespace cube {
class direction_t;class face_t;
}

template<typename planet_renderer_t>
struct planet_node_t;
struct texture_freelist_t;
struct vbuf_freelist_t;

namespace Ogre {
class Camera;
}


struct planet_renderer_t
  : Ogre::MovableObject
{
  typedef planet_renderer_t self_t;
  
  typedef planet_node_t<self_t> planet_node_type;
  
  ///Store it as a shared_ptr so it can be copied around cheaply in the tree,
  /// or copied at all (tree requires values to be copy-constructable and Assignable).
  typedef boost::shared_ptr< planet_node_type > planet_node_ptr_t;
  
  typedef tree::root_t<planet_node_ptr_t, 4, square::corner_t> root_type;
  typedef tree::branch_t<planet_node_ptr_t, 4, square::corner_t> tree_type;
  
  

  
  
  planet_renderer_t(Ogre::AxisAlignedBox bounds, Ogre::Real radius, std::size_t max_level);
  virtual ~planet_renderer_t();
  
  ///Regenerate the @c visibles container
  void render_visibles(Ogre::Camera& camera);
  
  ///Regenerate patches between nodes
  void render_transitions();
  
  ///Regenerate the debug frame manual object
  void render_frame(Ogre::Camera& camera);
  
public:
  const Ogre::AxisAlignedBox bounds;
  const Ogre::Real radius;
  const std::size_t max_level;
  
  const std::size_t noise_res;
  
  //bordered noise resolution
  const std::size_t bordered_noise_res;
  
  const std::size_t noise_width;
  const std::size_t noise_height;
  
  const std::size_t diffuse_width;
  const std::size_t diffuse_height;
  
  const std::size_t normals_width;
  const std::size_t normals_height;
  
  const std::size_t vertices_width;
  const std::size_t vertices_height;
  
  const std::size_t heightmap_width;
  const std::size_t heightmap_height;
  
  const std::size_t static_index_count;
  const std::size_t vertex_count;
  
  
  Ogre::Camera* mcamera;
protected:
  //MovableObject overides
  
  virtual const Ogre::AxisAlignedBox& getBoundingBox() const;
  virtual Ogre::Real getBoundingRadius() const;
  virtual const Ogre::String& getMovableType() const;
  virtual void _updateRenderQueue(Ogre::RenderQueue* queue);
  virtual void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);
private:
  typedef boost::multi_index_container<
    tree_type*,
    boost::multi_index::indexed_by<
      boost::multi_index::sequenced<>, // list-like index
      boost::multi_index::ordered_unique< boost::multi_index::identity<tree_type*> > // set like
    >
  > visibles_t;
  
  typedef boost::scoped_ptr<root_type> root_ptr_t;
  boost::array< root_ptr_t, 6> roots;
  visibles_t visibles;
  
  Ogre::MaterialPtr base_material;
  Ogre::HardwareIndexBufferSharedPtr ibuf;
private:
  //tree init functions
  
  void initialize_root(tree_type& tree, const cube::face_t& face);
  void initialize_root_data(tree_type& tree);
  
  void initialize_tree(tree_type& tree);
  
  void initialize_tree_mesh(tree_type& tree);
  void initialize_tree_data(tree_type& tree);
private:
  //init functions
  
  template<typename index_type>
  void initialize_index_buffer(Ogre::HardwareIndexBufferSharedPtr& ibuf);
private:
  //utility functions
  
  Ogre::Vector3 to_planet_relative(const cube::face_t& face, Ogre::Vector2 angles) const;
private:
  
  bool acceptable_pixel_error(const tree_type& tree, const Ogre::Camera& camera) const;
  
  Ogre::TexturePtr get_available_noise_texture();
  Ogre::TexturePtr get_available_diffuse_texture();
  Ogre::TexturePtr get_available_normals_texture();
  Ogre::TexturePtr get_available_heightmap_texture();
  Ogre::HardwareVertexBufferSharedPtr get_available_vertex_buffer(std::size_t vertex_size, std::size_t vertex_count);
  
  
  typedef boost::scoped_ptr< texture_freelist_t > texture_freelist_ptr_t;
  typedef boost::scoped_ptr< vbuf_freelist_t > vbuf_freelist_ptr_t;
  
  texture_freelist_ptr_t noise_texture_freelist;
  static std::size_t noise_texture_identifier;
  
  texture_freelist_ptr_t diffuse_texture_freelist;
  static std::size_t diffuse_texture_identifier;
  
  texture_freelist_ptr_t normals_texture_freelist;
  static std::size_t normals_texture_identifier;
  
  texture_freelist_ptr_t heightmap_texture_freelist;
  static std::size_t heightmap_texture_identifier;
  
  vbuf_freelist_ptr_t heightmap_vbuf_freelist;
  
};


#endif // PLANET_VOLUME_H
