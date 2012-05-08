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
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreHardwareBufferManager.h>

#include <boost/rational.hpp>
#include <NoisePipeline.h>
#include <boost/ptr_container/ptr_list.hpp>
#include <NoiseRidgedMulti.h>


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


struct noise_stack_t
  : boost::noncopyable
{
  noise_stack_t()
    : result_element(NULL)
    , cache(NULL)
  {
    cache = pipeline.createCache();
  }
  
  noisepp::Pipeline3D pipeline;
  noisepp::PipelineElement3D* result_element;
  noisepp::Cache* cache;
  
  boost::ptr_list< noisepp::Module > modules;
};

struct quad_bounds_t{
  typedef std::size_t integer_t;
  typedef boost::rational<integer_t> rational_t;
  struct vector2_t
  {
    vector2_t(const rational_t& x, const rational_t& y)
      : x(x)
      , y(y)
    {}
    
    bool operator==(const vector2_t& other) const
    {
      return x == other.x && y == other.y;
    }
    
    vector2_t operator-(const vector2_t& other) const
    {
      return vector2_t(x - other.x, y - other.y);
    }
    
    vector2_t operator+(const vector2_t& other) const
    {
      return vector2_t(x + other.x, y + other.y);
    }
    
    vector2_t operator*(const rational_t& rational) const
    {
      return vector2_t(x * rational, y * rational);
    }
    
    rational_t x;
    rational_t y;
  };
  
  static vector2_t minimized_vector(const vector2_t& lhs, const vector2_t& rhs)
  {
    return vector2_t(std::min(lhs.x, rhs.x),
                     std::min(lhs.y, rhs.y)); 
  }
  
  static vector2_t maximized_vector(const vector2_t& lhs, const vector2_t& rhs)
  {
    return vector2_t(std::max(lhs.x, rhs.x),
                     std::max(lhs.y, rhs.y)); 
  }
  
  quad_bounds_t()
    : min_max_array(create_min_max_array(vector2_t(rational_t(0,1), rational_t(0,1)),
                                         vector2_t(rational_t(0,1), rational_t(0,1))))
  {}
  
  quad_bounds_t(const vector2_t& min, const vector2_t& max)
    : min_max_array(create_min_max_array(min, max))
  {
    BOOST_ASSERT(min == minimized_vector(min,max));
    BOOST_ASSERT(max == maximized_vector(min,max));
  }
  
  const vector2_t& min() const
  {return min_max_array[0];}
  
  vector2_t& min()
  {return min_max_array[0];}
  
  vector2_t& max()
  {return min_max_array[1];}
  
  const vector2_t& max() const
  {return min_max_array[1];}
  
  vector2_t get_corner(const square::corner_t& corner) const
  {
    return vector2_t( min_max_array[corner.x_i()].x, min_max_array[corner.y_i()].y );
  }
  
  vector2_t get_center() const
  {
    return min() + ((max() - min()) * rational_t(1,2));
  }
  
  quad_bounds_t sub_box(const square::corner_t& corner) const
  {
    vector2_t c0 = get_center();
    vector2_t c1 = get_corner(corner);
    
    return quad_bounds_t(minimized_vector(c0, c1), maximized_vector(c0, c1));
  }
private:
  static boost::array<vector2_t, 2> create_min_max_array(const vector2_t& min, const vector2_t& max)
  {
    boost::array<vector2_t, 2> result = {{min, max}};
    return result;
  }
  boost::array<vector2_t, 2> min_max_array;
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
  Ogre::MaterialPtr material;
};

struct texture_freelist_t
{
  std::list<Ogre::TexturePtr> freelist;
};

struct vbuf_freelist_t
{
  std::list<Ogre::HardwareVertexBufferSharedPtr> freelist;
};


planet_renderer_t::planet_renderer_t(Ogre::AxisAlignedBox bounds, Ogre::Real radius, std::size_t max_level)
  : bounds(bounds)
  , radius(radius)
  , max_level(max_level)
  , noise_res(64)
  , bordered_noise_res(1 + noise_res + 1)
  , noise_width(bordered_noise_res)
  , noise_height(bordered_noise_res)
  , diffuse_width(bordered_noise_res)
  , diffuse_height(bordered_noise_res)
  , normals_width(bordered_noise_res)
  , normals_height(bordered_noise_res)
  , vertices_width(16)
  , vertices_height(16)
  , heightmap_width(1 + vertices_width + 1)
  , heightmap_height(1 + vertices_height + 1)
  , vertex_count(vertices_width * vertices_height)
  , static_index_count((vertices_width-1)*(vertices_height-1)*6)
  , mcamera(NULL)
{
  heightmap_vbuf_freelist.reset(new vbuf_freelist_t);
  noise_texture_freelist.reset(new texture_freelist_t);
  diffuse_texture_freelist.reset(new texture_freelist_t);
  normals_texture_freelist.reset(new texture_freelist_t);
  heightmap_texture_freelist.reset(new texture_freelist_t);
  
  ///FIXME: need unique name for this
  //base_material = Ogre::MaterialManager::getSingleton().create("planet_renderer-base-material",
  //                                                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  
  base_material = Ogre::MaterialManager::getSingleton().getByName("mordredmaterial");
  
  if (static_index_count < boost::integer_traits< boost::uint_t<16>::exact >::const_max)
  {
    initialize_index_buffer<boost::uint_t<16>::exact>(ibuf);
  } else {
    BOOST_ASSERT(static_index_count < boost::integer_traits< boost::uint_t<32>::exact >::const_max);
    
    initialize_index_buffer<boost::uint_t<32>::exact>(ibuf);
  }
  
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

template<typename index_type>
void planet_renderer_t::initialize_index_buffer(Ogre::HardwareIndexBufferSharedPtr& ibuf)
{
  using namespace Ogre;
  
  BOOST_ASSERT(static_index_count < std::numeric_limits<index_type>::max() - 1);
  BOOST_STATIC_ASSERT(sizeof(index_type) == 2 || sizeof(index_type) == 4);
  
  ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
            (sizeof(index_type) == 2) ? HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
            static_index_count,
            HardwareBuffer::HBU_WRITE_ONLY, false);
  
#ifndef NDEBUG
  {
    std::size_t idx_count = 0;
    for (std::size_t y0 = 0; y0 < (vertices_height- 1); ++y0)
    {
      for (std::size_t x0 = 0; x0 < (vertices_width - 1); ++x0)
      {
        
        ///Next vertex positions
        std::size_t x1 = x0 + 1;
        std::size_t y1 = y0 + 1;
        
        ///Base vertex indices
        std::size_t x0y0i = y0 * vertices_width + x0;
        std::size_t x0y1i = y1 * vertices_width + x0;
        std::size_t x1y0i = y0 * vertices_width + x1;
        std::size_t x1y1i = y1 * vertices_width + x1;
        
        BOOST_ASSERT(x0y0i < vertex_count);
        BOOST_ASSERT(x0y1i < vertex_count);
        BOOST_ASSERT(x1y0i < vertex_count);
        BOOST_ASSERT(x1y1i < vertex_count);
        
        ++idx_count;
        ++idx_count;
        ++idx_count;
        ++idx_count;
        ++idx_count;
        ++idx_count;
      }
    }
    BOOST_ASSERT(static_index_count == idx_count);
  }
#endif
  
  {
    HardwareBufferScopedLock ibuf_lock(*ibuf, HardwareBuffer::HBL_DISCARD);

    index_type* ibuf_ptr0 = static_cast<index_type*>(ibuf_lock.data());
    index_type* ibuf_ptr = ibuf_ptr0;
    
    for (std::size_t y0 = 0; y0 < (vertices_height- 1); ++y0)
    {
      for (std::size_t x0 = 0; x0 < (vertices_width - 1); ++x0)
      {
        
        ///Next vertex positions
        std::size_t x1 = x0 + 1;
        std::size_t y1 = y0 + 1;
        
        ///Vertex indices
        index_type x0y0i = (y0 * vertices_width) + x0;
        index_type x0y1i = (y1 * vertices_width) + x0;
        index_type x1y0i = (y0 * vertices_width) + x1;
        index_type x1y1i = (y1 * vertices_width) + x1;
        
        *ibuf_ptr++ = x0y0i;
        *ibuf_ptr++ = x1y0i;
        *ibuf_ptr++ = x1y1i;
        
        *ibuf_ptr++ = x1y1i;
        *ibuf_ptr++ = x0y1i;
        *ibuf_ptr++ = x0y0i;
        
      }
    }
  
  }
}



void planet_renderer_t::initialize_root(planet_renderer_t::tree_type& tree, const cube::face_t& face)
{
  
  //typedef planet_node_type::quad_coordinate_type quad_coordinate_type;
  
  using boost::assign::list_of;
  using namespace Ogre;
  
  BOOST_ASSERT(!tree.value());
  
  tree.value() = boost::make_shared<planet_node_type>(boost::ref(*this), face);
  tree.value()->tree = &tree;
  tree.value()->quad_bounds = quad_bounds_t(quad_bounds_t::vector2_t(0,0), quad_bounds_t::vector2_t(1,1));
  
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
                                                           Ogre::TU_DYNAMIC);
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


Ogre::HardwareVertexBufferSharedPtr planet_renderer_t::get_available_vertex_buffer(std::size_t vertex_size, std::size_t vertex_count)
{
  using namespace Ogre;
  
  if (!heightmap_vbuf_freelist->freelist.empty())
  {
    HardwareVertexBufferSharedPtr result = heightmap_vbuf_freelist->freelist.back();
    heightmap_vbuf_freelist->freelist.pop_back();
    
    BOOST_ASSERT(result->getVertexSize() == vertex_size);
    BOOST_ASSERT(result->getNumVertices() == vertex_count);
    
    return result;
  }
  
  
  return HardwareBufferManager::getSingleton().createVertexBuffer(
    vertex_size,
    vertex_count,
    HardwareBuffer::HBU_STATIC_WRITE_ONLY);
  
}

noise_stack_t& planet_renderer_t::get_noise_stack(std::size_t level)
{
  BOOST_ASSERT(level <= max_level);
  
  if (!(level < noise_hierarchy.size()))
  {
    noise_hierarchy.resize(level + 1);
  }
  
  if (!noise_hierarchy[level].result_element)
  {
    noise_stack_t& noise_stack = noise_hierarchy[level];
    
    std::auto_ptr< noisepp::RidgedMultiModule > caves_ptr(new noisepp::RidgedMultiModule);
    noisepp::RidgedMultiModule& caves = *caves_ptr;
    noise_stack.modules.push_back(caves_ptr);
    
    caves.setFrequency(radius / 2);
    caves.setOctaveCount(2);
    caves.setScale(2);
    caves.setGain(2);
    
    noisepp::ElementID element_id = caves.addToPipeline(&noise_stack.pipeline);
    
    noise_stack.result_element = noise_stack.pipeline.getElement(element_id);
  }
  
  BOOST_ASSERT(!!noise_hierarchy[level].result_element);
  
  return noise_hierarchy[level];
}


void planet_renderer_t::initialize_root_data_noise(planet_renderer_t::tree_type& tree)
{
  using namespace Ogre;
    
  planet_node_type& planet_node = *tree.value();
  
  planet_node.noise = get_available_noise_texture();
  Vector2 omin(boost::rational_cast<Real>(planet_node.quad_bounds.min().x),
               boost::rational_cast<Real>(planet_node.quad_bounds.min().y));
  omin = (omin * 2) - Vector2(1,1);
  Vector2 omax(boost::rational_cast<Real>(planet_node.quad_bounds.max().x),
                boost::rational_cast<Real>(planet_node.quad_bounds.max().y));
  omax = (omax * 2) - Vector2(1,1);
  
  noise_stack_t& noise_stack = get_noise_stack(tree.level());
  
  {
    HardwareBufferScopedLock noise_buf_lock(*planet_node.noise->getBuffer(), HardwareBuffer::HBL_DISCARD);
  
    float* noise_buf_ptr0 = static_cast<float*>(noise_buf_lock.data());
    
    float* noise_buf_ptr = noise_buf_ptr0;
    
    for (std::size_t v = 0; v < noise_height; ++v)
    {
      for (std::size_t u = 0; u < noise_width; ++u)
      {
        Vector2 relative_sphere_face_position2d = omin + (omax - omin) * (Vector2(u,v)/Vector2(noise_width - 1, noise_height - 1));

        
        Vector3 planet_relative_position = to_planet_relative(planet_node.face, relative_sphere_face_position2d);
        
        const Real& x = planet_relative_position.x;
        const Real& y = planet_relative_position.y;
        const Real& z = planet_relative_position.z;
        
        noise_buf_ptr0[ v * noise_width + u ] = noise_stack.result_element->getValue(x,y,z, noise_stack.cache);
      }
    }
  }

}


void planet_renderer_t::initialize_root_data(planet_renderer_t::tree_type& tree)
{
  using namespace Ogre;
    
  planet_node_type& planet_node = *tree.value();
  
  //planet_node.diffuse = get_available_diffuse_texture();
  //planet_node.normals = get_available_normals_texture();
  //planet_node.height = get_available_heightmap_texture();
  //planet_node.material = base_material;
  initialize_root_data_noise(tree);
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
  BOOST_ASSERT(!tree.is_root());
  BOOST_ASSERT(tree.is_child());
  BOOST_ASSERT(tree.parent());
  
  const tree_type& parent = *tree.parent();
  planet_node_type& planet_node = *tree.value();

  const planet_node_type& parent_node = *parent.value();
  
  planet_node.noise = get_available_noise_texture();
  planet_node.diffuse = get_available_diffuse_texture();
  planet_node.normals = get_available_normals_texture();
  planet_node.height = get_available_heightmap_texture();
  planet_node.material = base_material;
  
  
  
  
  using namespace Ogre;
  
  
  
  
  Vector2 omin(boost::rational_cast<Real>(planet_node.quad_bounds.min().x),
               boost::rational_cast<Real>(planet_node.quad_bounds.min().y));
  omin = (omin * 2) - Vector2(1,1);
  Vector2 omax(boost::rational_cast<Real>(planet_node.quad_bounds.max().x),
               boost::rational_cast<Real>(planet_node.quad_bounds.max().y));
  omax = (omax * 2) - Vector2(1,1);
  
  
  {
    
    std::size_t pv0 = tree.corner().y() ? noise_height / 2 : 0;
    std::size_t pu0 = tree.corner().x() ? noise_width / 2 : 0;
    
    std::size_t pv_end = pv0 + noise_height / 2;
    std::size_t pu_end = pu0 + noise_width / 2;
    
#ifndef NDEBUG
    for (std::size_t pv = pv0; pv < pv_end; ++pv)
    {
      for (std::size_t pu = pu0; pu < pu_end; ++pu)
      {
        std::size_t u0 = (pu - pu0) * 2;
        std::size_t v0 = (pv - pv0) * 2;
        
        std::size_t puvi = pv * noise_width + pu;
        
        BOOST_ASSERT(puvi < (noise_width * noise_height));
        
        for( std::size_t vd = 0; vd < 2; ++vd)
        {
          for (std::size_t ud = 0; ud < 2; ++ud)
          {
            std::size_t u  = u0 + ud;
            std::size_t v  = v0 + vd;
            
            std::size_t uvi = v * noise_width + u;

            BOOST_ASSERT(uvi < (noise_width * noise_height));
          }
        }
      }
    }
#endif
    
    noise_stack_t& noise_stack = get_noise_stack(tree.level());
    
    HardwarePixelBufferSharedPtr noise_buf = planet_node.noise->getBuffer();
    HardwareBufferScopedLock noise_buf_lock(*noise_buf, HardwareBuffer::HBL_DISCARD);
    
    HardwarePixelBufferSharedPtr parent_noise_buf = parent_node.noise->getBuffer();
    HardwareBufferScopedLock parent_noise_buf_lock(*parent_noise_buf, HardwareBuffer::HBL_READ_ONLY);
  
    float* noise_buf_ptr0 = static_cast<float*>( noise_buf_lock.data() );
    float* noise_buf_ptr = noise_buf_ptr0;
    const float* p_noise_buf_ptr0 = static_cast<const float*>( parent_noise_buf_lock.data() );
    const float* p_noise_buf_ptr = p_noise_buf_ptr0;
    
    
    
    for (std::size_t pv = pv0; pv < pv_end; ++pv)
    {
      for (std::size_t pu = pu0; pu < pu_end; ++pu)
      {
        std::size_t u0 = (pu - pu0) * 2;
        std::size_t v0 = (pv - pv0) * 2;
        
        std::size_t puvi = pv * noise_width + pu;
        
        float pvalue = p_noise_buf_ptr0[puvi];
        
        for( std::size_t vd = 0; vd < 2; ++vd)
        {
          for (std::size_t ud = 0; ud < 2; ++ud)
          {
            std::size_t u  = u0 + ud;
            std::size_t v  = v0 + vd;
            
            std::size_t uvi = v * noise_width + u;
            
            Vector2 relative_sphere_face_position2d = omin + (omax - omin)
              * (Vector2(Real(u)-Real(1),Real(v)-Real(1))/Vector2(noise_res - 1, noise_res - 1));

      
            Vector3 planet_relative_position = to_planet_relative(planet_node.face, relative_sphere_face_position2d);
      
            const Real& x = planet_relative_position.x;
            const Real& y = planet_relative_position.y;
            const Real& z = planet_relative_position.z;
            
            //volatile float garbage = pvalue + noise_stack.result_element->getValue(x, y, z, noise_stack.cache);
            Real factor = (radius / 500) / Math::Pow(2, tree.level());
            noise_buf_ptr0[ uvi ] = pvalue + noise_stack.result_element->getValue(x, y, z, noise_stack.cache) * factor;
            
          }
        }
      
      }
    }
  }
  
  
  
  
  {
    const HardwarePixelBufferSharedPtr& hm_buf = planet_node.height->getBuffer();
    HardwareBufferScopedLock hm_buf_lock(*hm_buf, HardwareBuffer::HBL_DISCARD);
    
    const HardwarePixelBufferSharedPtr& noise_buf = planet_node.noise->getBuffer();
    HardwareBufferScopedLock noise_buf_lock(*noise_buf, HardwareBuffer::HBL_READ_ONLY);
    
    float* hm_buf_ptr0 = static_cast<float*>( hm_buf_lock.data() );
    float* hm_buf_ptr = hm_buf_ptr0;
    const float* noise_buf_ptr0 = static_cast<const float*>( noise_buf_lock.data() );
    const float* noise_buf_ptr = noise_buf_ptr0;
    
    for(std::size_t hv = 0; hv < heightmap_height; ++hv)
    {
      for(std::size_t hu = 0; hu < heightmap_width; ++hu)
      {
        std::size_t huvi = hv* heightmap_width + hu;
        std::size_t nu = hu * (noise_res / vertices_width);
        std::size_t nv = hv * (noise_res / vertices_height);
        
        std::size_t nuvi = nv * noise_width + nu;
        
        hm_buf_ptr0[ huvi ] = noise_buf_ptr0[nuvi];
        //*hm_buf_ptr++ = 0;
      }
    }
  }
}

void planet_renderer_t::initialize_tree_mesh(planet_renderer_t::tree_type& tree)
{
  using namespace Ogre;
  
  planet_node_type& planet_node = *tree.value();
  
  const cube::face_t& face = planet_node.face;
  const cube::direction_t& direction = face.direction();
  
  
  
  
  planet_node.renderable.reset(new ChunkRenderable(planet_node.material, *this));
  
  ChunkRenderable& renderable = *planet_node.renderable;
  
  
  {
    //Vector3 translation = Vector3::ZERO;
    //translation[direction.axis()] = direction.positive() ? radius : -radius;
    
    Vector3 translation = Vector3::UNIT_Z;
    
    Vector3 scale(Vector3::UNIT_SCALE);
    scale *= radius / Math::Pow(2,Real(tree.level()));
    
    
    boost::array<Quaternion, 6> face_rotations;
    
    face_rotations[cube::direction_t::get( 0, 0, 1).index()] = Quaternion::IDENTITY;
    face_rotations[cube::direction_t::get( 0, 0,-1).index()] = Quaternion(Radian(+Math::PI), Vector3::UNIT_Y);
    face_rotations[cube::direction_t::get( 0, 1, 0).index()] = Quaternion(Radian(+Math::PI), Vector3::UNIT_X);
    face_rotations[cube::direction_t::get( 0,-1, 0).index()] = Quaternion(Radian(-Math::PI), Vector3::UNIT_X);
    face_rotations[cube::direction_t::get( 1, 0, 0).index()] = Quaternion(Radian(+Math::PI / Real(2)), Vector3::UNIT_Y);
    face_rotations[cube::direction_t::get(-1, 0, 0).index()] = Quaternion(Radian(-Math::PI / Real(2)), Vector3::UNIT_Y);
    
    Quaternion orientation = face_rotations[direction.index()];
    
    translation = orientation * translation * radius;
    
    {
      quad_bounds_t::vector2_t min = planet_node.quad_bounds.min();
      Vector2 omin(boost::rational_cast<Real>(min.x),
                   boost::rational_cast<Real>(min.y));
      omin = (omin * 2) - Vector2(1,1);
      
      translation = to_planet_relative(planet_node.face, omin);
    }
    //renderable.planet_relative_transform = Matrix4::IDENTITY;
    renderable.planet_relative_transform.makeTransform(translation, scale, orientation);
    
  }
  
  
  renderable.index_data.reset(new IndexData);
  renderable.vertex_data.reset(new VertexData);
  
  VertexData& vertex_data = *renderable.vertex_data;
  IndexData& index_data = *renderable.index_data;

  renderable.renderop.indexData = &index_data;
  renderable.renderop.vertexData = &vertex_data;
  renderable.renderop.useIndexes = true;
  renderable.renderop.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
  renderable.renderop.srcRenderable = &renderable;
  
  index_data.indexCount = static_index_count;
  index_data.indexStart = 0;
  index_data.indexBuffer = ibuf;
  
  vertex_data.vertexStart = 0;
  vertex_data.vertexCount = vertex_count;

  
  
  VertexDeclaration* decl = vertex_data.vertexDeclaration;
  VertexBufferBinding* bind = vertex_data.vertexBufferBinding;

  int STATIC_BINDING = 0;

  {
    std::size_t element_offset = 0;

    element_offset += decl->addElement(STATIC_BINDING, element_offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
    element_offset += decl->addElement(STATIC_BINDING, element_offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE).getSize();
  }
  
  HardwareVertexBufferSharedPtr static_buf = get_available_vertex_buffer(decl->getVertexSize(STATIC_BINDING), vertex_count);
  
  bind->setBinding((STATIC_BINDING), static_buf);
  
#ifndef NDEBUG
  
  {
    std::size_t vertex_index_count = 0;
    for (std::size_t vy = 0; vy < vertices_height; ++vy)
    {
      for (std::size_t vx = 0; vx < vertices_width; ++vx)
      {
        std::size_t vertex_buf_index = vy * vertices_width + vx;
        std::size_t heightmap_buf_index = (vy+1) * heightmap_width + (vx+1);
        
        BOOST_ASSERT(vertex_buf_index < vertex_count);
        BOOST_ASSERT(heightmap_buf_index < (heightmap_width * heightmap_height));
        ++vertex_index_count;
      }
    }
    BOOST_ASSERT(vertex_index_count == vertex_count);
  }
#endif
  
  {
    Texture& heightmap_texture = *planet_node.height;
    HardwarePixelBuffer& heightmap_buf = *heightmap_texture.getBuffer();
    
    HardwareBufferScopedLock heightmap_buf_lock(heightmap_buf, HardwareBuffer::HBL_READ_ONLY);
    HardwareBufferScopedLock static_buf_lock(*static_buf, HardwareBuffer::HBL_DISCARD);

    void* static_buf_ptr0 = static_cast<void*>(static_buf_lock.data());
    
    void* static_buf_ptr = static_buf_ptr0;
    
    const float* heightmap_buf_ptr0 = static_cast<const float*>(heightmap_buf_lock.data());
    
    for (std::size_t vv = 0; vv < vertices_height; ++vv)
    {
      for (std::size_t vu = 0; vu < vertices_width; ++vu)
      {
        std::size_t vertex_buf_index = vv * vertices_width + vu;
        std::size_t hu = vu + 1;
        std::size_t hv = vv + 1;
        std::size_t heightmap_buf_index = hv * heightmap_width + hu;
        
        //float height = heightmap_buf_ptr0[ heightmap_buf_index ];
        
        //Vector3 position(vx, 0, vy);
        
        quad_bounds_t::vector2_t min = planet_node.quad_bounds.min();
        quad_bounds_t::vector2_t max = planet_node.quad_bounds.max();
        
        Vector2 relative_position2D = Vector2(vu,vv)/Vector2(vertices_width - 1, vertices_height - 1);
        
        relative_position2D = (relative_position2D * 2) - Vector2(1,1);
        
        
        Vector2 omin(boost::rational_cast<Real>(min.x),
                     boost::rational_cast<Real>(min.y));
        omin = (omin * 2) - Vector2(1,1);
        Vector2 omax(boost::rational_cast<Real>(max.x),
                     boost::rational_cast<Real>(max.y));
        omax = (omax * 2) - Vector2(1,1);
        
        
        
        
        Vector2 relative_sphere_face_position2d = omin + (omax - omin) * (Vector2(vu,vv)/Vector2(vertices_width - 1, vertices_height - 1));
        Vector3 surface_postion = to_planet_relative(planet_node.face, relative_sphere_face_position2d);
        
        //surface_postion.normalise();
        //surface_postion *= radius + height;
        
        surface_postion = renderable.planet_relative_transform.inverse() * surface_postion;
        
        float* vertex_buf_ptr = static_cast<float*>(static_buf_ptr);
        *vertex_buf_ptr++ = surface_postion.x;
        *vertex_buf_ptr++ = surface_postion.y;
        *vertex_buf_ptr++ = surface_postion.z;
        
        static_buf_ptr = vertex_buf_ptr;
        
        RGBA* colour_ptr = static_cast<RGBA*>(static_buf_ptr);
        
        const cube::direction_t& direction = planet_node.face.direction();
        Vector3 colour_vector(direction.x(), direction.y(), direction.z());
        colour_vector += Vector3(1,1,1);
        colour_vector /= 2;
        ColourValue colour(colour_vector.x, colour_vector.y, colour_vector.z);
        
        *colour_ptr++ = Ogre::VertexElement::convertColourValue(colour, VET_COLOUR);
        
        static_buf_ptr = colour_ptr;
        
        //static_buf_ptr0[vertex_buf_index] = position;
      }
    }
  }
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
  //getParentSceneNode()->setScale(Ogre::Vector3::UNIT_SCALE);
  
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

template<typename vector2_t>
Ogre::Vector3 planet_renderer_t::to_planet_relative(const cube::face_t& face, const vector2_t& uv) const
{
  using namespace Ogre;
  
  Vector2 o_uv(boost::rational_cast<Real>( uv.x ), boost::rational_cast<Real>( uv.y ));
  o_uv *= 2;
  o_uv -= Vector2(1,1);
  return to_planet_relative(face,o_uv);
}

Ogre::Vector3 planet_renderer_t::to_planet_relative(const cube::face_t& face, const Ogre::Vector2& uv) const
{
  const cube::direction_t& direction = face.direction();
  
  using namespace Ogre;
  
  Vector3 cube_xyz;
  
  /*
  std::size_t j = 0;
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    if (face.direction().axis() != axis)
      cube_xyz[ axis ] = uv[ j++ ];
    else
      cube_xyz[ axis ] = 1;
  }
  BOOST_ASSERT(j == 2);
  */
  
  boost::uint8_t axis = direction.axis();
  
  
  cube_xyz[ (axis + 0) % 3 ] = 1;
  cube_xyz[ (axis + 1) % 3 ] = uv[0];
  cube_xyz[ (axis + 2) % 3 ] = uv[1];
  
  if (!direction.positive())
  {
    boost::swap(cube_xyz[(axis + 1) % 3], cube_xyz[(axis + 2) % 3]);
    cube_xyz = -cube_xyz;
  }
  
  //cube_xyz[0] = uv[0];
  //cube_xyz[1] = uv[1];
  //cube_xyz[2] = 1;
  
  
  Vector3 sphere_xyz;
  
  for (std::size_t i = 0; i < 3; ++i)
  {
    Real& x_i_p = sphere_xyz[i];
    const Real& x_i = cube_xyz[ (i + 0) % 3];
    const Real& y_i = cube_xyz[ (i + 1) % 3];
    const Real& z_i = cube_xyz[ (i + 2) % 3];
    
    x_i_p = x_i 
          * Math::Sqrt(
            Real(1)
          - Math::Sqr(y_i) / Real(2)
          - Math::Sqr(z_i) / Real(2) + 
          + (Math::Sqr(y_i) * Math::Sqr(z_i)) / Real(3));
  }
  
  sphere_xyz *= radius;
  
  return sphere_xyz;
}


bool planet_renderer_t::acceptable_pixel_error(const planet_renderer_t::tree_type& tree, const Ogre::Camera& camera) const
{
  using namespace Ogre;
  
  const planet_node_type& planet_node = *tree.value();
  const root_type& root = *roots[planet_node.face.index()];
  
  const planet_node_type& root_node = *root.value();
  
  const quad_bounds_t& quad = planet_node.quad_bounds;
  
  
  Vector3 cam_pos = camera.getDerivedPosition();
  
  
  //Vector2 quad_c00 = quad.get_corner(square::corner_t::get(false,false));
  //Vector2 quad_c11 = quad.get_corner(square::corner_t::get( true, true));
  
  Vector2 min(boost::rational_cast<Real>(quad.min().x),
              boost::rational_cast<Real>(quad.min().y));
  min = (min*2) - Vector2(1,1);
  Vector2 max(boost::rational_cast<Real>(quad.max().x),
              boost::rational_cast<Real>(quad.max().y));
  max = (max*2) - Vector2(1,1);
  
  Vector3 planet_relative_min = to_planet_relative(planet_node.face, min);
  Vector3 planet_relative_max = to_planet_relative(planet_node.face, max);
  
  BOOST_ASSERT(!!getParentSceneNode());
  SceneNode& sn = *getParentSceneNode();
  
  Vector3 world_relative_min = sn.convertLocalToWorldPosition(planet_relative_min);
  Vector3 world_relative_max = sn.convertLocalToWorldPosition(planet_relative_max);
  
  
  Real node_size = (world_relative_max - world_relative_min).length();
  
  Ogre::Vector3 node_center = (world_relative_max + world_relative_min) / 2.0;

  
  
  ///World length of highest LOD node
  Ogre::Real x_0 = Real(1) / Real(2048);
  //Ogre::Real x_0 = Real(320);
  
  ///All nodes within this distance will surely be rendered
  Ogre::Real f_0 = x_0 * 1.1;
  
  ///Total nodes
  // Ogre::Real t = Ogre::Math::Pow(2 * (f_0 / x_0), 3);
  
  ///Lowest octree level
  //Ogre::Real n_max = Ogre::Math::Log2(bounds.getSize().length() / x_0);
  Ogre::Real n_max = max_level;
  
  
  ///Node distance from camera
  Ogre::Real d = std::max(f_0, node_center.distance(cam_pos));
  
  ///Minimum optimal node level
  Ogre::Real n_opt = n_max - Ogre::Math::Log2(d / f_0);
  
  
  //Ogre::Real size_opt = bounds.getSize().length() / Ogre::Math::Pow(2, n_opt);
  
  Ogre::Real size_opt = radius / Ogre::Math::Pow(2, n_opt);
  
  return size_opt > node_size;
}


