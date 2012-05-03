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


#include "planet_volume.h"

#include <boost/foreach.hpp>
#include <list>

#include "ogre_utility.h"
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include <OGRE/OgreSceneNode.h>
#include <cube/cube.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreTexture.h>
#include <OGRE/OgreTextureManager.h>
#include <boost/dynamic_bitset.hpp>
#include <OGRE/OgreStringConverter.h>


#if 0
template<typename integer_t>
struct quad_coordinate_t{
  typedef quad_coordinate_t value_type;
  
  quad_coordinate_t(integer_t x, integer_t y)
    : x(x)
    , y(y)
  {}
  
  integer_t x;
  integer_t y;
};

template<typename quad_coordinate_t>
struct quad_bounds_t
{
  quad_bounds_t(quad_coordinate_t min, quad_coordinate_t max)
    : min(min)
    , max(max)
  {}
  
  quad_coordinate_t min;
  quad_coordinate_t max;
};
#endif



struct quad_bounds_t{
  quad_bounds_t()
    : min_max_array(create_min_max_array(Ogre::Vector2::ZERO, Ogre::Vector2::ZERO))
  {}
  
  quad_bounds_t(const Ogre::Vector2& min, const Ogre::Vector2& max)
    : min_max_array(create_min_max_array(min, max))
  {
    BOOST_ASSERT(min == minimized_vector(min,max));
    BOOST_ASSERT(max == maximized_vector(min,max));
  }
  
  const Ogre::Vector2& min() const
  {return min_max_array[0];}
  
  Ogre::Vector2& min()
  {return min_max_array[0];}
  
  Ogre::Vector2& max()
  {return min_max_array[1];}
  
  const Ogre::Vector2& max() const
  {return min_max_array[1];}
  
  Ogre::Vector2 get_corner(const square::corner_t& corner) const
  {
    return Ogre::Vector2( min_max_array[corner.x_i()].x, min_max_array[corner.y_i()].y );
  }
  
  Ogre::Vector2 get_center() const
  {
    using namespace Ogre;
    return min() + ((max() - min()) * (Real(1)/Real(2)));
  }
  
  quad_bounds_t sub_box(const square::corner_t& corner) const
  {
    using namespace Ogre;
    
    Vector2 c0 = get_center();
    Vector2 c1 = get_corner(corner);
    
    return quad_bounds_t(minimized_vector(c0, c1), maximized_vector(c0, c1));
  }
private:
  static boost::array<Ogre::Vector2, 2> create_min_max_array(const Ogre::Vector2& min, const Ogre::Vector2& max)
  {
    boost::array<Ogre::Vector2, 2> result = {{min, max}};
    return result;
  }
  boost::array<Ogre::Vector2, 2> min_max_array;
};



///This represents a quad-node
template<typename planet_renderer_t>
struct planet_node_t
{
  typedef typename planet_renderer_t::tree_type tree_type;
  
  planet_node_t(planet_renderer_t& prenderer, const cube::face_t& face)
    : prenderer(prenderer)
    , face(face)
    , tree(NULL)
  {}
  
  std::string name() const
  {
    std::string x_bitstr;
    std::string y_bitstr;
    boost::to_string( bitset[0], x_bitstr );
    boost::to_string( bitset[1], y_bitstr );
    
    return x_bitstr + y_bitstr;
  }

  planet_renderer_t& prenderer;
  cube::face_t face;
  tree_type* tree;
  
  quad_bounds_t quad_bounds;
  boost::array< boost::dynamic_bitset<> , 2 > bitset;
  
  ///This is the ogre Renderable for this node
  boost::scoped_ptr<ChunkRenderable> renderable;
  
  Ogre::TexturePtr noise;
  Ogre::TexturePtr height;
  Ogre::TexturePtr diffuse;
  Ogre::TexturePtr normals;
};

struct texture_freelist_t
{
  std::list<Ogre::TexturePtr> freelist;
};


planet_renderer_t::planet_renderer_t(Ogre::AxisAlignedBox bounds, Ogre::Real radius, std::size_t max_level)
  : bounds(bounds)
  , radius(radius)
  , max_level(max_level)
  , noise_width(514)
  , noise_height(514)
  , diffuse_width(514)
  , diffuse_height(514)
  , normals_width(514)
  , normals_height(514)
  , heightmap_width(34)
  , heightmap_height(34)
  , mcamera(NULL)
{
  noise_texture_freelist.reset(new texture_freelist_t);
  diffuse_texture_freelist.reset(new texture_freelist_t);
  normals_texture_freelist.reset(new texture_freelist_t);
  heightmap_texture_freelist.reset(new texture_freelist_t);
  
  BOOST_FOREACH(const cube::face_t& face, cube::face_t::all())
  {
    root_ptr_t& root_ptr = roots[face.index()];
    
    root_ptr.reset(new root_type);
    
    initialize_root(*root_ptr, face);
    
    root_ptr->split();
    
    BOOST_FOREACH(tree_type& child, root_ptr->children())
    {
      initialize_tree(child);
      
      visibles.push_back(&child);
    }
  }
}

planet_renderer_t::~planet_renderer_t()
{

}


void planet_renderer_t::initialize_root(planet_renderer_t::tree_type& tree, const cube::face_t& face)
{
  
  //typedef planet_node_type::quad_coordinate_type quad_coordinate_type;
  
  using boost::assign::list_of;
  using namespace Ogre;
  
  BOOST_ASSERT(!tree.value());
  
  tree.value() = boost::make_shared<planet_node_type>(boost::ref(*this), face);
  tree.value()->tree = &tree;
  tree.value()->quad_bounds = quad_bounds_t(Vector2(0,0), Vector2(1,1));
  
  initialize_root_data(tree);
}

std::size_t
planet_renderer_t::
noise_texture_identifier = 0;

std::size_t
planet_renderer_t::
normals_texture_identifier = 0;

std::size_t
planet_renderer_t::
diffuse_texture_identifier = 0;

std::size_t
planet_renderer_t::
heightmap_texture_identifier = 0;

Ogre::TexturePtr planet_renderer_t::get_available_noise_texture()
{
  if (!noise_texture_freelist->freelist.empty())
  {
    Ogre::TexturePtr result = noise_texture_freelist->freelist.back();
    noise_texture_freelist->freelist.pop_back();
    return result;
  }
  
  ///FIXME: I hate this hack. Really really hate it.
  Ogre::String texture_name = Ogre::String("mordred-noise-texture") + Ogre::StringConverter::toString(noise_texture_identifier++);
  return Ogre::TextureManager::getSingleton().createManual(texture_name,
                                                           Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                           Ogre::TEX_TYPE_2D,
                                                           noise_width, noise_height,
                                                           0,
                                                           Ogre::PF_FLOAT32_R,
                                                           Ogre::TU_STATIC_WRITE_ONLY);
}

Ogre::TexturePtr planet_renderer_t::get_available_diffuse_texture()
{
  if (!diffuse_texture_freelist->freelist.empty())
  {
    Ogre::TexturePtr result = diffuse_texture_freelist->freelist.back();
    diffuse_texture_freelist->freelist.pop_back();
    return result;
  }
  
  ///FIXME: I hate this hack. Really really hate it.
  Ogre::String texture_name = Ogre::String("mordred-diffuse-texture") + Ogre::StringConverter::toString(diffuse_texture_identifier++);
  return Ogre::TextureManager::getSingleton().createManual(texture_name,
                                                           Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                           Ogre::TEX_TYPE_2D,
                                                           diffuse_width, diffuse_height,
                                                           0,
                                                           Ogre::PF_FLOAT32_R,
                                                           Ogre::TU_STATIC_WRITE_ONLY);

}

Ogre::TexturePtr planet_renderer_t::get_available_normals_texture()
{
  if (!normals_texture_freelist->freelist.empty())
  {
    Ogre::TexturePtr result = normals_texture_freelist->freelist.back();
    normals_texture_freelist->freelist.pop_back();
    return result;
  }
  
  ///FIXME: I hate this hack. Really really hate it.
  Ogre::String texture_name = Ogre::String("mordred-normals-texture") + Ogre::StringConverter::toString(normals_texture_identifier++);
  return Ogre::TextureManager::getSingleton().createManual(texture_name,
                                                           Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                           Ogre::TEX_TYPE_2D,
                                                           normals_width, normals_height,
                                                           0,
                                                           Ogre::PF_FLOAT32_R,
                                                           Ogre::TU_STATIC_WRITE_ONLY);
}


Ogre::TexturePtr planet_renderer_t::get_available_heightmap_texture()
{
  if (!heightmap_texture_freelist->freelist.empty())
  {
    Ogre::TexturePtr result = heightmap_texture_freelist->freelist.back();
    heightmap_texture_freelist->freelist.pop_back();
    return result;
  }
  
  ///FIXME: I hate this hack. Really really hate it.
  Ogre::String texture_name = Ogre::String("mordred-heightmap-texture") + Ogre::StringConverter::toString(heightmap_texture_identifier++);
  return Ogre::TextureManager::getSingleton().createManual(texture_name,
                                                           Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                           Ogre::TEX_TYPE_2D,
                                                           heightmap_width, heightmap_height,
                                                           0,
                                                           Ogre::PF_FLOAT32_R,
                                                           Ogre::TU_STATIC_WRITE_ONLY);
}




void planet_renderer_t::initialize_root_data(planet_renderer_t::tree_type& tree)
{
  planet_node_type& planet_node = *tree.value();
  
  planet_node.noise = get_available_noise_texture();
  planet_node.diffuse = get_available_diffuse_texture();
  planet_node.normals = get_available_normals_texture();
  planet_node.height = get_available_heightmap_texture();
  
}


void planet_renderer_t::initialize_tree(planet_renderer_t::tree_type& tree)
{
  BOOST_ASSERT(!tree.value());
  BOOST_ASSERT(!tree.is_root());
  BOOST_ASSERT(tree.is_child());
  BOOST_ASSERT(tree.parent());
  
  const tree_type& parent = *tree.parent();
  const planet_node_type& parent_node = *parent.value();
  const quad_bounds_t& parent_bounds = parent_node.quad_bounds;
  const cube::face_t& parent_face = parent_node.face;
  
  tree.value() = boost::make_shared<planet_node_type>(boost::ref(*this), parent_face);
  tree.value()->tree = &tree;
  tree.value()->quad_bounds = parent_bounds.sub_box(tree.corner());
  tree.value()->bitset[0].push_back(tree.corner().x());
  tree.value()->bitset[1].push_back(tree.corner().y());
  
  initialize_tree_data(tree);
  initialize_tree_mesh(tree);
}

void planet_renderer_t::initialize_tree_data(planet_renderer_t::tree_type& tree)
{
  BOOST_ASSERT(!tree.value());
  BOOST_ASSERT(!tree.is_root());
  BOOST_ASSERT(tree.is_child());
  BOOST_ASSERT(tree.parent());
  
  const tree_type& parent = *tree.parent();
  planet_node_type& planet_node = *tree.value();

  planet_node.noise = get_available_noise_texture();
  planet_node.diffuse = get_available_diffuse_texture();
  planet_node.normals = get_available_normals_texture();
  planet_node.height = get_available_heightmap_texture();
  
  
}

void planet_renderer_t::initialize_tree_mesh(planet_renderer_t::tree_type& tree)
{
  using namespace Ogre;
  
  planet_node_type& planet_node = *tree.value();
  
  MaterialPtr material = ;
  
  planet_node.renderable.reset(new ChunkRenderable(material, *this) );
  
  
}



const Ogre::AxisAlignedBox& planet_renderer_t::getBoundingBox() const
{
  return bounds;
}

Ogre::Real planet_renderer_t::getBoundingRadius() const
{
  return bounds.getSize().length() / 2.0;
}

const Ogre::String& planet_renderer_t::getMovableType() const
{
  static Ogre::String t = "planet_volume";
  return t;
}


void planet_renderer_t::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
{
  
  
  BOOST_FOREACH(tree_type* visible, visibles)
  {
    planet_node_type& node = *visible->value();
    
    if (node.renderable)
      visitor->visit( node.renderable.get(), 0, false );
  }
  
  (void)debugRenderables;

}

void planet_renderer_t::_updateRenderQueue(Ogre::RenderQueue* queue)
{
  using namespace Ogre;
  
  BOOST_ASSERT(!!getParentSceneNode());
  getParentSceneNode()->setScale(Ogre::Vector3::UNIT_SCALE);
  
  if (mcamera)
  {
    render_visibles(*mcamera);
    render_transitions();
    render_frame(*mcamera);
  }
  

  BOOST_FOREACH(tree_type* visible, visibles)
  {
    planet_node_type& node = *visible->value();
    
    if (node.renderable)
    {
      queue->addRenderable( node.renderable.get() );
      /*
      if ( mcamera )
      {
        if (is_visible(*mcamera, node.bounds, *getParentSceneNode()))
          queue->addRenderable( node.renderable.get() );
      } else {
          queue->addRenderable( node.renderable.get() );
      }*/
    }
    
  }

}

void planet_renderer_t::render_frame(Ogre::Camera& camera)
{

}

void planet_renderer_t::render_transitions()
{

}

void planet_renderer_t::render_visibles(Ogre::Camera& camera)
{
  typedef visibles_t::nth_index<0>::type visibles_list_t;
  typedef visibles_t::nth_index<1>::type visibles_set_t;
  
  visibles_list_t& visibles_list = visibles.get<0>();
  //visibles_set_t& visibles_set = visibles.get<1>();
  
  
  
  visibles_list_t::iterator w = visibles_list.begin();
  
  
  
  while ( w != visibles_list.end() )
  {
    tree_type* visible = *w;
    
    
    BOOST_ASSERT(visible);
    BOOST_ASSERT(!visible->is_root());
    
    tree_type* parent = visible->parent();
    BOOST_ASSERT(parent);
    
    bool parent_acceptable_error = !parent->is_root() && acceptable_pixel_error(*parent, camera);
    bool acceptable_error = acceptable_pixel_error(*visible, camera);
    
    BOOST_ASSERT(lif(parent_acceptable_error, acceptable_error));
    BOOST_ASSERT(lif(!acceptable_error, !parent_acceptable_error));
    
    if (parent_acceptable_error)
    {
      
      ///Add the parent to visibles
      visibles_list.push_back(parent);
      
      {
        visibles_list_t::iterator e = w;
        ++w;
        visibles_list.erase(e);
        continue;
      }
    } else if ( acceptable_error ) {
      ///Let things stay the same
    } else {
      if (visible->level() < max_level && !!visible->value()->renderable)
      {
        
        ///If visible doesn't have children
        if (!visible->has_children())
        {
          ///Create children for visible
          
          visible->split();
          BOOST_FOREACH(tree_type& child, visible->children())
          {
            initialize_tree(child);
          }
        }
        
        ///Foreach child of visible
        BOOST_FOREACH(tree_type& child, visible->children())
        {
          ///Add the child to the visibles
          visibles_list.push_back(&child);
        }
        
        
        ///Remove visible from visibles
        {
          visibles_list_t::iterator e = w;
          ++w;
          visibles_list.erase(e);
          continue;
        }
        
      } else {
        
      }
    }
  
    ++w;
  }
  
  
#ifndef NDEBUG
std::set<tree_type*> debug_unique_visibles;

BOOST_FOREACH(tree_type* visible, visibles)
{
if (!(debug_unique_visibles.find(visible) == debug_unique_visibles.end()))
  //std::cout << "visible->level() = " << visible->level() << "visible->value()->bounds = " << visible->value()->bounds << std::endl;

  BOOST_ASSERT(debug_unique_visibles.find(visible) == debug_unique_visibles.end());
  debug_unique_visibles.insert(visible);
}
#endif
}

bool planet_renderer_t::acceptable_pixel_error(const planet_renderer_t::tree_type& tree, const Ogre::Camera& camera) const
{
  BOOST_ASSERT(false);
}


