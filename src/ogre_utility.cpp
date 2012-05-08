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


#include "ogre_utility.h"
#include "cube/cube.h"

#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreManualObject.h>

#include <boost/assign.hpp>



ChunkRenderable::
ChunkRenderable(const Ogre::MaterialPtr& mat, const Ogre::MovableObject& movable)
  : mat(mat), movable(movable)
{

}

const Ogre::LightList& ChunkRenderable::getLights(void ) const
{
  return movable.queryLights();
}

const Ogre::MaterialPtr& ChunkRenderable::getMaterial(void ) const
{
  return mat;
}

void ChunkRenderable::getRenderOperation(Ogre::RenderOperation& op)
{
  op = renderop;
}

void ChunkRenderable::getWorldTransforms(Ogre::Matrix4* xform) const
{
  *xform = movable._getParentNodeFullTransform() * planet_relative_transform;
}

Ogre::Real ChunkRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
{
  BOOST_ASSERT(!!movable.getParentSceneNode());
  Ogre::Vector3 camVec = cam->getDerivedPosition() - movable.getParentSceneNode()->_getDerivedPosition();
  return camVec.squaredLength();
}

ChunkRenderable::~ChunkRenderable()
{

}






void draw_axis(Ogre::ManualObject* man, std::size_t index_offset)
{
  using namespace Ogre;
  
  
  Real arrow_length = 100;
  Real arrow_head_length = 7;
  Real arrow_head_radius = 4;
  Real pole_radius = 2;
  std::size_t arrow_head_tris = 30;
  boost::array< ColourValue, 3 > arrow_colours =
  {{
    ColourValue::Red + ColourValue::Green,
    ColourValue::Green + ColourValue::Blue,
    ColourValue::Blue + ColourValue::Red
  }};
  boost::array< ColourValue, 3 > axis_colours = {{ColourValue::Red, ColourValue::Green, ColourValue::Blue}};
  
  
  
  {
    
    std::vector< Vector3 > vertices;
    std::vector< Vector3 > normals;
    std::vector< ColourValue > colours;
    std::vector< std::size_t > triangle_indices;
    
    
    BOOST_FOREACH(const cube::face_t& positive_face, cube::corner_t::get(true,true,true).faces())
    {
      const cube::direction_t& axis = positive_face.direction();
      
      std::size_t axis_index =  (axis.x() != 0)
                              ? 0
                              :   (axis.y() != 0)
                                ? 1
                                : 2;
      
      const cube::direction_t& adjacent_direction = axis.adjacent()[0];
      
      Vector3 arrow_tip(arrow_length * axis.x(), arrow_length * axis.y(), arrow_length * axis.z());
      Vector3 arrow_base = arrow_tip.normalisedCopy() * (arrow_length - arrow_head_length);
      
      
      
      ColourValue arrow_colour = arrow_colours[ axis_index ];
      ColourValue pole_colour = axis_colours[ axis_index ];
      
      Vector3 base_normal = (-arrow_tip).normalisedCopy();
      
      
      
      
      
      
      {
        Vector3 pole_base_start_position = ( Vector3(adjacent_direction.x(), adjacent_direction.y(), adjacent_direction.z())
                                      * pole_radius);
        
        
        std::vector< std::size_t > base_plate_indices;
        std::vector< std::size_t > cylinder_base_indices;
        std::vector< std::size_t > cylinder_top_indices;
        for (std::size_t i = 0; i < arrow_head_tris; ++i)
        {
          
          Radian angle = Radian(  (Real(i) * Math::TWO_PI)
                                / (Real(arrow_head_tris)));
          Quaternion q(angle, arrow_tip.normalisedCopy());
          
          Vector3 pole_base_position = q * pole_base_start_position;
          Vector3 pole_top_position = pole_base_position + arrow_base;
          
          Vector3 cylinder_normal = pole_base_position.normalisedCopy();
          
          cylinder_base_indices.push_back(vertices.size());
          vertices.push_back(pole_base_position);
          normals.push_back(cylinder_normal);
          colours.push_back(pole_colour);
          
          cylinder_top_indices.push_back(vertices.size());
          vertices.push_back(pole_top_position);
          normals.push_back(cylinder_normal);
          colours.push_back(pole_colour);
          
          base_plate_indices.push_back(vertices.size());
          vertices.push_back(pole_base_position);
          normals.push_back(base_normal);
          colours.push_back(pole_colour * .8);
        }

        std::size_t base_center_index = vertices.size();
        vertices.push_back(Vector3::ZERO);
        normals.push_back(base_normal);
        colours.push_back(pole_colour * .8);

        for (std::size_t i = 0; i < arrow_head_tris; ++i)
        {
          using namespace boost::assign;
          triangle_indices += base_plate_indices[(i + 1) % arrow_head_tris],
                              base_plate_indices[i],
                              base_center_index;
          
          triangle_indices += cylinder_base_indices[i],
                              cylinder_base_indices[(i + 1) % arrow_head_tris],
                              cylinder_top_indices[(i + 1) % arrow_head_tris];
                              
          triangle_indices += cylinder_top_indices[i],
                              cylinder_base_indices[i],
                              cylinder_top_indices[(i + 1) % arrow_head_tris];
                              
        }
        
        
      }
      
      {
        Vector3 base_start_position =   ( Vector3(adjacent_direction.x(), adjacent_direction.y(), adjacent_direction.z())
                                        * arrow_head_radius)
                                      + arrow_base;
        
        
        std::size_t arrow_tip_index = vertices.size();
        vertices.push_back(arrow_tip);
        colours.push_back(arrow_colour * .8);
        normals.push_back(arrow_tip.normalisedCopy());

        std::size_t arrow_base_index = vertices.size();
        vertices.push_back(arrow_base);
        colours.push_back(arrow_colour * .8);
        normals.push_back(base_normal);
        
        std::vector<std::size_t> base_positions_indices(arrow_head_tris, -1);
        for (std::size_t i = 0; i < arrow_head_tris; ++i)
        {
          
          Radian angle = Radian(  (Real(i) * Math::TWO_PI)
                                / (Real(arrow_head_tris)));
          Quaternion q(angle, arrow_tip.normalisedCopy());
          
          
          Vector3 base_position = q * base_start_position;
          std::size_t& index = base_positions_indices[i];
          index = vertices.size();
          vertices.push_back(base_position);
          colours.push_back(arrow_colour);
          ///FIXME:
          normals.push_back((base_position - arrow_base).normalisedCopy());
        }
        
        BOOST_ASSERT(normals.size() == vertices.size());
        BOOST_ASSERT(colours.size() == vertices.size());
        
        for (std::size_t i = 0; i < arrow_head_tris; ++i)
        {
          using namespace boost::assign;
          triangle_indices += base_positions_indices[i],
                              base_positions_indices[(i + 1) % arrow_head_tris],
                              arrow_tip_index;
        }
        
        BOOST_ASSERT(triangle_indices.size() % 3 == 0);
        
        std::vector< std::size_t > base_plate_positions_indices;
        
        BOOST_FOREACH(std::size_t vi, base_positions_indices)
        {
          std::size_t index = vertices.size();
          base_plate_positions_indices.push_back(index);
          
          BOOST_ASSERT(vi < vertices.size());
          vertices.push_back(vertices[vi]);
          normals.push_back(base_normal);
          colours.push_back(arrow_colour * .8);
          
        }
        
        BOOST_ASSERT(base_plate_positions_indices.size() == arrow_head_tris);
        
        for (std::size_t i = 0; i < arrow_head_tris; ++i)
        {
          using namespace boost::assign;
          triangle_indices += base_plate_positions_indices[(i + 1) % arrow_head_tris],
                              base_plate_positions_indices[i],
                              arrow_base_index;
        }
        

      }
      

    }
    
    BOOST_FOREACH(std::size_t vi, boost::irange<std::size_t>(0, vertices.size()))
    {
      BOOST_ASSERT(vi < vertices.size());
      BOOST_ASSERT(vi < normals.size());
      BOOST_ASSERT(vi < colours.size());
      man->position(vertices[vi]);
      //BOOST_ASSERT(normals[vi].normalisedCopy() == normals[vi]);
      man->normal(normals[vi]);
      man->colour(colours[vi]);
    }
    
    BOOST_FOREACH(const std::size_t& idx, triangle_indices)
    {
      BOOST_ASSERT(idx < vertices.size());
      man->index(index_offset + idx);
    }
  }
}
















