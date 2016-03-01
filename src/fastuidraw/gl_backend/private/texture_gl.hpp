/*!
 * \file texture_gl.hpp
 * \brief file texture_gl.hpp
 *
 * Copyright 2016 by Intel.
 *
 * Contact: kevin.rogovin@intel.com
 *
 * This Source Code Form is subject to the
 * terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with
 * this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * \author Kevin Rogovin <kevin.rogovin@intel.com>
 *
 */


#pragma once

#include <list>
#include <vector>
#include <algorithm>

#include <fastuidraw/gl_backend/gl_header.hpp>
#include <fastuidraw/gl_backend/gl_context_properties.hpp>

namespace fastuidraw { namespace gl { namespace detail {

inline
GLenum
format_from_internal_format(GLenum fmt)
{
  switch(fmt)
    {
    default:
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB32F:
    case GL_RGB16F:
      return GL_RGB;

    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA32F:
    case GL_RGBA16F:
      return GL_RGBA;

      //integer formats:
    case GL_RGBA32UI:
    case GL_RGBA32I:
    case GL_RGBA16UI:
    case GL_RGBA16I:
    case GL_RGBA8UI:
    case GL_RGBA8I:
      //GL_BRGA_INTEGER also ok
      return GL_RGBA_INTEGER;

    case GL_RGB32UI:
    case GL_RGB32I:
    case GL_RGB16UI:
    case GL_RGB16I:
    case GL_RGB8UI:
    case GL_RGB8I:
      //GL_BGR_INTEGER also ok
      return GL_RGB_INTEGER;

    case GL_RG8:
    case GL_RG16F:
    case GL_RG32F:
      return GL_RG;

    case GL_R8:
    case GL_R16F:
    case GL_R32F:
      return GL_RED;

    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
      return GL_RG_INTEGER;

    case GL_R8I:
    case GL_R16I:
    case GL_R32I:
    case GL_R8UI:
    case GL_R16UI:
    case GL_R32UI:
      return GL_RED_INTEGER;

    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
      return GL_DEPTH_STENCIL;
    }
}

template<GLenum texture_target>
class TextureTargetDimension
{};

///////////////////////////////////
// 3D
template<>
class TextureTargetDimension<GL_TEXTURE_2D_ARRAY>
{
public:
  enum { N = 3 };
};

#ifdef GL_TEXTURE_CUBE_MAP_ARRAY
template<>
class TextureTargetDimension<GL_TEXTURE_CUBE_MAP_ARRAY>
{
public:
  enum { N = 3 };
};
#endif

inline
void
tex_storage(bool use_tex_storage,
            GLenum texture_target,
            GLint internalformat,
            vecN<GLsizei, 3> size)
{
  if(use_tex_storage)
    {
      glTexStorage3D(texture_target, 1, internalformat,
                     size.x(), size.y(), size.z());
    }
  else
    {
      glTexImage3D(texture_target,
                   0,
                   internalformat,
                   size.x(), size.y(), size.z(), 0,
                   format_from_internal_format(internalformat),
                   GL_UNSIGNED_BYTE,
                   NULL);
    }
}

inline
void
tex_sub_image(GLenum texture_target, vecN<GLint, 3> offset,
              vecN<GLsizei, 3> size, GLenum format, GLenum type,
              const void *pixels)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage3D(texture_target, 0,
                  offset.x(), offset.y(), offset.z(),
                  size.x(), size.y(), size.z(),
                  format, type, pixels);
}

//////////////////////////////////////////////
// 2D

template<>
class TextureTargetDimension<GL_TEXTURE_2D>
{
public:
  enum { N = 2 };
};

#ifdef GL_TEXTURE_1D_ARRAY
template<>
class TextureTargetDimension<GL_TEXTURE_1D_ARRAY>
{
public:
  enum { N = 2 };
};
#endif

#ifdef GL_TEXTURE_RECTANGLE
template<>
class TextureTargetDimension<GL_TEXTURE_RECTANGLE>
{
public:
  enum { N = 2 };
};
#endif

inline
void
tex_storage(bool use_tex_storage,
            GLenum texture_target, GLint internalformat, vecN<GLsizei, 2> size)
{
  if(use_tex_storage)
    {
      glTexStorage2D(texture_target, 1, internalformat, size.x(), size.y());
    }
  else
    {
      glTexImage2D(texture_target,
                   0,
                   internalformat,
                   size.x(), size.y(), 0,
                   format_from_internal_format(internalformat),
                   GL_UNSIGNED_BYTE,
                   NULL);
    }
}

inline
void
tex_sub_image(GLenum texture_target,
              vecN<GLint, 2> offset,
              vecN<GLsizei, 2> size,
              GLenum format, GLenum type, const void *pixels)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(texture_target, 0,
                  offset.x(), offset.y(),
                  size.x(), size.y(),
                  format, type, pixels);
}


//////////////////////////////////////////
// 1D
#ifdef GL_TEXTURE_1D
template<>
class TextureTargetDimension<GL_TEXTURE_1D>
{
public:
  enum { N = 1 };
};
#endif

#if defined(glTexStorage1D)

inline
void
tex_storage(bool use_tex_storage,
            GLenum texture_target, GLint internalformat, vecN<GLsizei, 1> size)
{
  if(use_tex_storage)
    {
      glTexStorage1D(texture_target, 1, internalformat, size.x());
    }
  else
    {
      glTexImage1D(texture_target,
                   0,
                   internalformat,
                   size.x(), 0,
                   format_from_internal_format(internalformat),
                   GL_UNSIGNED_BYTE,
                   NULL);
    }
}

inline
void
tex_sub_image(GLenum texture_target, vecN<GLint, 1> offset,
              vecN<GLsizei, 1> size, GLenum format, GLenum type,
              const void *pixels)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage1D(texture_target, 0, offset.x(), size.x(), format, type, pixels);
}

#endif


template<size_t N>
class EntryLocationN
{
  public:
  typedef std::pair<EntryLocationN, std::vector<uint8_t> > with_data;
  vecN<int, N> m_location;
  vecN<GLsizei, N> m_size;
};

template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
class TextureGL
{
public:
  enum { N = TextureTargetDimension<texture_target>::N };
  typedef EntryLocationN<N> EntryLocation;

  TextureGL(vecN<int, N> dims, bool delayed);
  ~TextureGL();

  void
  delete_texture(void);

  GLuint
  texture(void) const;

  void
  flush(void);

  void
  set_data_vector(const EntryLocation &loc,
                  std::vector<uint8_t> &data);

  void
  set_data_c_array(const EntryLocation &loc,
                   const_c_array<uint8_t> data);

  void
  resize(vecN<int, N> new_num_layers)
  {
    m_dims = new_num_layers;
  }

private:

  void
  create_texture(void) const;

  void
  tex_subimage(const EntryLocation &loc,
               const_c_array<uint8_t> data);

  void
  flush_size_change(void);

  bool m_delayed;
  vecN<int, N> m_dims;
  vecN<int, N> m_texture_dimension;
  mutable GLuint m_texture;
  mutable bool m_use_tex_storage;
  mutable int m_number_times_create_texture_called;

  typedef typename EntryLocation::with_data with_data;
  typedef std::list<with_data> list_type;

  list_type m_unflushed_commands;
};


///////////////////////////////////////
//TextureGL methods
template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
TextureGL(vecN<int, N> dims, bool delayed):
  m_delayed(delayed),
  m_dims(dims),
  m_texture(0),
  m_number_times_create_texture_called(0)
{
  if(!m_delayed)
    {
      create_texture();
    }
  m_texture_dimension = m_dims;
}

template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
~TextureGL()
{
  if(m_texture != 0)
    {
      delete_texture();
    }
}

template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
void
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
flush_size_change(void)
{
  if(m_texture_dimension != m_dims)
    {
      /* only need to issue GL commands to resize
         the underlying GL texture IF we do not have
         a texture yet.
       */
      if(m_texture != 0)
        {
          GLuint old_texture;

          old_texture = m_texture;
          /* create a new texture for the new size,
           */
          m_texture = 0;
          create_texture();

          /* copy the contents of old_texture to m_texture
           */
          vecN<GLint, 3> blit_dims;
          for(unsigned int i = 0; i < N; ++i)
            {
              blit_dims[i] = std::min(m_dims[i], m_texture_dimension[i]);
            }
          for(unsigned int i = N; i < 3; ++i)
            {
              blit_dims[i] = 1;
            }

          #ifdef GL_TEXTURE_1D_ARRAY
            {
              /* Sighs. The GL API is utterly wonky. For GL_TEXTURE_1D_ARRAY,
                 we need to permute [2] and [1].
                 "Slices of a TEXTURE_1D_ARRAY, TEXTURE_2D_ARRAY, TEXTURE_CUBE_MAP_ARRAY
                 TEXTURE_3D and faces of TEXTURE_CUBE_MAP are all compatible provided
                 they share a compatible internal format, and multiple slices or faces
                 may be copied between these objects with a single call by specifying the
                 starting slice with <srcZ> and <dstZ>, and the number of slices to
                 be copied with <srcDepth>.
              */
              if(texture_target == GL_TEXTURE_1D_ARRAY)
                {
                  std::swap(blit_dims[1], blit_dims[2]);
                }
            }
          #endif

          /* TODO:
              glCopyImageSubData is core in GL 4.3, available in
              the extension GL_ARB_copy_image for GL version pre-4.3.
              Also, the function glCopyImageSubDataNV is available in
              the extension GL_NV_copy_image

              For GLES, it is core in GLES3.2, and otherwise requires
              an extension function, being one of glCopyImageSubDataOES
              (extension GL_OES_copy_image) or glCopyImageSubDataEXT
              (extension GL_EXT_copy_image).

              If the function or any of its extension incarnations
              are not available, then we can call glFramebufferBlit
              blit_dims[2] times (changing the what layer each time
              with glFramebufferTextureLayer()).
           */
          glCopyImageSubData(old_texture, texture_target, 0,
                             0, 0, 0, //src
                             m_texture, texture_target, 0,
                             0, 0, 0, //dst
                             blit_dims[0], blit_dims[1], blit_dims[2]);

          /* now delete old_texture
           */
          glDeleteTextures(1, &old_texture);
        }
      m_texture_dimension = m_dims;
    }
}

template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
void
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
delete_texture(void)
{
  assert(m_texture != 0);
  glDeleteTextures(1, &m_texture);
  m_texture = 0;
}


template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
GLuint
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
texture(void) const
{
  assert(m_texture != 0);
  return m_texture;
}


template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
void
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
create_texture(void) const
{
  assert(m_texture == 0);
  glGenTextures(1, &m_texture);
  assert(m_texture!=0);
  glBindTexture(texture_target, m_texture);
  if(m_number_times_create_texture_called == 0)
    {
      ContextProperties ctx;
      m_use_tex_storage = ctx.is_es() || ctx.version() >= ivec2(4, 2)
        || ctx.has_extension("GL_ARB_texture_storage");
    }
  tex_storage(m_use_tex_storage, texture_target, internal_format, m_dims);
  glTexParameteri(texture_target, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(texture_target, GL_TEXTURE_MAG_FILTER, filter);
  ++m_number_times_create_texture_called;
}


template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
void
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
flush(void)
{
  flush_size_change();
  if(m_texture == 0)
    {
      create_texture();
    }

  if(!m_unflushed_commands.empty())
    {
      glBindTexture(texture_target, m_texture);
      for(typename list_type::iterator iter = m_unflushed_commands.begin(),
            end = m_unflushed_commands.end(); iter != end; ++iter)
        {
          assert(!iter->second.empty());
          tex_sub_image(texture_target,
                        iter->first.m_location,
                        iter->first.m_size,
                        external_format, external_type,
                        &iter->second[0]);
        }
      m_unflushed_commands.clear();
    }
}


template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
void
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
set_data_vector(const EntryLocation &loc,
                std::vector<uint8_t> &data)
{
  if(data.empty())
    {
      return;
    }

  if(m_delayed)
    {
      m_unflushed_commands.push_back(typename EntryLocation::with_data());
      typename EntryLocation::with_data &R(m_unflushed_commands.back());
      R.first = loc;
      R.second.swap(data);
    }
  else
    {
      flush_size_change();
      glBindTexture(texture_target, m_texture);
      tex_sub_image(texture_target,
                    loc.m_location,
                    loc.m_size,
                    external_format, external_type,
                    &data[0]);
    }
}

template<GLenum texture_target,
         GLenum internal_format,
         GLenum external_format,
         GLenum external_type,
         GLenum filter>
void
TextureGL<texture_target,
          internal_format,
          external_format,
          external_type,
          filter>::
set_data_c_array(const EntryLocation &loc,
                 fastuidraw::const_c_array<uint8_t> data)
{
  if(data.empty())
    {
      return;
    }

  if(m_delayed)
    {
      std::vector<uint8_t> data_copy;
      data_copy.resize(data.size());
      std::copy(data.begin(), data.end(), data_copy.begin());
      set_data_vector(loc, data_copy);
    }
  else
    {
      flush_size_change();
      glBindTexture(texture_target, m_texture);
      tex_sub_image(texture_target,
                    loc.m_location,
                    loc.m_size,
                    external_format, external_type,
                    data.c_ptr());

    }
}



} //namespace detail
} //namespace gl
} //namespace fastuidraw