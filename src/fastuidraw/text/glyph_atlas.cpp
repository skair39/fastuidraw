/*!
 * \file glyph_atlas.cpp
 * \brief file glyph_atlas.cpp
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


#include <fastuidraw/text/glyph_atlas.hpp>

#include "../private/interval_allocator.hpp"
#include "../private/util_private.hpp"
#include "private/rect_atlas.hpp"

namespace
{
  class rect_atlas_layer:
    public fastuidraw::reference_counted<rect_atlas_layer>::non_concurrent,
    public fastuidraw::detail::RectAtlas
  {
  public:
    explicit
    rect_atlas_layer(const fastuidraw::ivec2 &dimensions, int player):
      fastuidraw::detail::RectAtlas(dimensions),
      m_layer(player)
    {}

    int
    layer(void) const
    {
      return m_layer;
    }

  private:
    int m_layer;

  };

  class GlyphAtlasTexelBackingStoreBasePrivate
  {
  public:
    GlyphAtlasTexelBackingStoreBasePrivate(fastuidraw::ivec3 whl, bool presizable):
      m_dimensions(whl),
      m_resizeable(presizable)
    {}

    GlyphAtlasTexelBackingStoreBasePrivate(int w, int h, int l, bool presizable):
      m_dimensions(w, h, l),
      m_resizeable(presizable)
    {}

    fastuidraw::ivec3 m_dimensions;
    bool m_resizeable;
  };

  class GlyphAtlasGeometryBackingStoreBasePrivate
  {
  public:
    GlyphAtlasGeometryBackingStoreBasePrivate(unsigned int palignment, unsigned int psize,
                                              bool presizable):
      m_resizeable(presizable),
      m_alignment(palignment),
      m_size(psize)
    {
    }

    bool m_resizeable;
    unsigned int m_alignment;
    unsigned int m_size;
  };

  class GlyphAtlasPrivate
  {
  public:
    GlyphAtlasPrivate(fastuidraw::reference_counted_ptr<fastuidraw::GlyphAtlasTexelBackingStoreBase> ptexel_store,
                      fastuidraw::reference_counted_ptr<fastuidraw::GlyphAtlasGeometryBackingStoreBase> pgeometry_store):
      m_texel_store(ptexel_store),
      m_geometry_store(pgeometry_store),
      m_geometry_data_allocator(pgeometry_store->size())
    {
      assert(m_texel_store);
      assert(m_geometry_store);
      allocate_atlas_bookkeeping(m_texel_store->dimensions().z());
    };

    void
    allocate_atlas_bookkeeping(int new_size)
    {
      fastuidraw::ivec2 dims(m_texel_store->dimensions().x(), m_texel_store->dimensions().y());
      int old_size(m_private_data.size());

      assert(new_size > old_size);
      m_private_data.resize(new_size);
      for(int i = old_size; i < new_size; ++i)
        {
          m_private_data[i] = FASTUIDRAWnew rect_atlas_layer(dims, i);
        }
    }

    fastuidraw::mutex m_mutex;
    fastuidraw::reference_counted_ptr<fastuidraw::GlyphAtlasTexelBackingStoreBase> m_texel_store;
    fastuidraw::reference_counted_ptr<fastuidraw::GlyphAtlasGeometryBackingStoreBase> m_geometry_store;
    std::vector<fastuidraw::reference_counted_ptr<rect_atlas_layer> > m_private_data;
    fastuidraw::interval_allocator m_geometry_data_allocator;
  };
}

/////////////////////////////////////////////////////
// fastuidraw::GlyphAtlasTexelBackingStoreBase methods
fastuidraw::GlyphAtlasTexelBackingStoreBase::
GlyphAtlasTexelBackingStoreBase(ivec3 whl, bool presizable)
{
  m_d = FASTUIDRAWnew GlyphAtlasTexelBackingStoreBasePrivate(whl, presizable);
}

fastuidraw::GlyphAtlasTexelBackingStoreBase::
GlyphAtlasTexelBackingStoreBase(int w, int h, int l, bool presizable)
{
  m_d = FASTUIDRAWnew GlyphAtlasTexelBackingStoreBasePrivate(w, h, l, presizable);
}

fastuidraw::GlyphAtlasTexelBackingStoreBase::
~GlyphAtlasTexelBackingStoreBase()
{
  GlyphAtlasTexelBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasTexelBackingStoreBasePrivate*>(m_d);
  FASTUIDRAWdelete(d);
  m_d = NULL;
}

fastuidraw::ivec3
fastuidraw::GlyphAtlasTexelBackingStoreBase::
dimensions(void) const
{
  GlyphAtlasTexelBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasTexelBackingStoreBasePrivate*>(m_d);
  return d->m_dimensions;
}

bool
fastuidraw::GlyphAtlasTexelBackingStoreBase::
resizeable(void) const
{
  GlyphAtlasTexelBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasTexelBackingStoreBasePrivate*>(m_d);
  return d->m_resizeable;
}

void
fastuidraw::GlyphAtlasTexelBackingStoreBase::
resize(int new_num_layers)
{
  GlyphAtlasTexelBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasTexelBackingStoreBasePrivate*>(m_d);

  assert(d->m_resizeable);
  assert(new_num_layers > d->m_dimensions.z());
  resize_implement(new_num_layers);
  d->m_dimensions.z() = new_num_layers;
}


///////////////////////////////////////////
// fastuidraw::GlyphAtlasGeometryBackingStoreBase methods
fastuidraw::GlyphAtlasGeometryBackingStoreBase::
GlyphAtlasGeometryBackingStoreBase(unsigned int palignment, unsigned int psize,
                                   bool presizable)
{
  m_d = FASTUIDRAWnew GlyphAtlasGeometryBackingStoreBasePrivate(palignment, psize, presizable);
}

fastuidraw::GlyphAtlasGeometryBackingStoreBase::
~GlyphAtlasGeometryBackingStoreBase()
{
  GlyphAtlasGeometryBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasGeometryBackingStoreBasePrivate*>(m_d);
  FASTUIDRAWdelete(d);
  m_d = NULL;
}

unsigned int
fastuidraw::GlyphAtlasGeometryBackingStoreBase::
size(void)
{
  GlyphAtlasGeometryBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasGeometryBackingStoreBasePrivate*>(m_d);
  return d->m_size;
}

unsigned int
fastuidraw::GlyphAtlasGeometryBackingStoreBase::
alignment(void) const
{
  GlyphAtlasGeometryBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasGeometryBackingStoreBasePrivate*>(m_d);
  return d->m_alignment;
}

bool
fastuidraw::GlyphAtlasGeometryBackingStoreBase::
resizeable(void) const
{
  GlyphAtlasGeometryBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasGeometryBackingStoreBasePrivate*>(m_d);
  return d->m_resizeable;
}

void
fastuidraw::GlyphAtlasGeometryBackingStoreBase::
resize(unsigned int new_size)
{
  GlyphAtlasGeometryBackingStoreBasePrivate *d;
  d = reinterpret_cast<GlyphAtlasGeometryBackingStoreBasePrivate*>(m_d);
  assert(d->m_resizeable);
  assert(new_size > d->m_size);
  resize_implement(new_size);
  d->m_size = new_size;
}

///////////////////////////////////////////
// fastuidraw::GlyphLocation methods
fastuidraw::ivec2
fastuidraw::GlyphLocation::
location(void) const
{
  const detail::RectAtlas::rectangle *p;

  p = reinterpret_cast<const detail::RectAtlas::rectangle*>(m_opaque);
  return (p != NULL) ?
    p->unpadded_minX_minY() :
    ivec2(-1, -1);
}

int
fastuidraw::GlyphLocation::
layer(void) const
{
  const detail::RectAtlas::rectangle *p;
  const rect_atlas_layer *a;

  p = reinterpret_cast<const detail::RectAtlas::rectangle*>(m_opaque);
  if(p == NULL)
    {
      return -1;
    }

  assert(dynamic_cast<const rect_atlas_layer*>(p->atlas()));
  a = static_cast<const rect_atlas_layer*>(p->atlas());

  return a->layer();
}

fastuidraw::ivec2
fastuidraw::GlyphLocation::
size(void) const
{
  const detail::RectAtlas::rectangle *p;
  p = reinterpret_cast<const detail::RectAtlas::rectangle*>(m_opaque);
  return (p != NULL) ?
    p->unpadded_size() :
    ivec2(-1, -1);
}

///////////////////////////////////////////////
// fastuidraw::GlyphAtlas methods
fastuidraw::GlyphAtlas::
GlyphAtlas(reference_counted_ptr<GlyphAtlasTexelBackingStoreBase> ptexel_store,
           reference_counted_ptr<GlyphAtlasGeometryBackingStoreBase> pgeometry_store)
{
  m_d = FASTUIDRAWnew GlyphAtlasPrivate(ptexel_store, pgeometry_store);
};

fastuidraw::GlyphAtlas::
~GlyphAtlas()
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);
  FASTUIDRAWdelete(d);
  m_d = NULL;
}


fastuidraw::GlyphLocation
fastuidraw::GlyphAtlas::
allocate(fastuidraw::ivec2 size, const_c_array<uint8_t> pdata,
         const GlyphAtlas::Padding &padding)
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);

  GlyphLocation return_value;
  const detail::RectAtlas::rectangle *r(NULL);
  unsigned int layer;

  if(size.x() > d->m_texel_store->dimensions().x()
     || size.y() > d->m_texel_store->dimensions().y())
    {
      return return_value;
    }

  autolock_mutex m(d->m_mutex);

  for(unsigned int i = 0, endi = d->m_private_data.size(); i < endi && r == NULL; ++i)
    {
      r = d->m_private_data[i]->add_rectangle(size,
                                              padding.m_left, padding.m_right,
                                              padding.m_top, padding.m_bottom);
      layer = i;
    }

  if(r == NULL && d->m_texel_store->resizeable())
    {
      int old_size;

      /* TODO:
          Should we reallocate on powers of 2, or one layer
          at a time? [Right now we are doing one layer at
          a time].
       */
      old_size = d->m_texel_store->dimensions().z();
      d->m_texel_store->resize(old_size + 1);
      d->allocate_atlas_bookkeeping(d->m_texel_store->dimensions().z());

      r = d->m_private_data[old_size]->add_rectangle(size,
                                                     padding.m_left, padding.m_right,
                                                     padding.m_top, padding.m_bottom);
      layer = old_size;
      assert(r != NULL);
    }

  if(r != NULL)
    {
      return_value.m_opaque = r;
      d->m_texel_store->set_data(r->minX_minY().x(), r->minX_minY().y(), layer,
                                 size.x(), size.y(), pdata);
    }

  return return_value;
}

void
fastuidraw::GlyphAtlas::
deallocate(fastuidraw::GlyphLocation G)
{
  assert(G.valid());
  const detail::RectAtlas::rectangle *r;

  r = reinterpret_cast<const detail::RectAtlas::rectangle*>(G.m_opaque);
  if(r != NULL)
    {
      detail::RectAtlas::delete_rectangle(r);
    }
}

int
fastuidraw::GlyphAtlas::
allocate_geometry_data(const_c_array<generic_data> pdata)
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);

  autolock_mutex m(d->m_mutex);
  unsigned int count, alignment;
  int block_count, return_value;

  count = pdata.size();
  alignment = d->m_geometry_store->alignment();

  assert(count > 0);
  assert(alignment > 0);
  assert(count % alignment == 0);

  block_count = count / alignment;
  return_value = d->m_geometry_data_allocator.allocate_interval(block_count);
  if(return_value == -1)
    {
      if(d->m_geometry_store->resizeable())
        {
          d->m_geometry_store->resize(block_count + 2 * d->m_geometry_store->size());
          d->m_geometry_data_allocator.resize(d->m_geometry_store->size());
          return_value = d->m_geometry_data_allocator.allocate_interval(block_count);
          assert(return_value != -1);
        }
      else
        {
          return return_value;
        }
    }

  d->m_geometry_store->set_values(return_value, pdata);
  return return_value;
}

void
fastuidraw::GlyphAtlas::
deallocate_geometry_data(int location, int count)
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);

  if(location < 0)
    {
      assert(count == 0);
      return;
    }

  autolock_mutex m(d->m_mutex);

  assert(count > 0);
  d->m_geometry_data_allocator.free_interval(location, count);
}


void
fastuidraw::GlyphAtlas::
clear(void)
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);

  autolock_mutex m(d->m_mutex);

  d->m_geometry_data_allocator.reset(d->m_geometry_data_allocator.size());
  for(unsigned int i = 0, endi = d->m_private_data.size(); i < endi; ++i)
    {
      d->m_private_data[i]->clear();
    }
}

void
fastuidraw::GlyphAtlas::
flush(void) const
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);

  autolock_mutex m(d->m_mutex);
  d->m_texel_store->flush();
  d->m_geometry_store->flush();
}

fastuidraw::reference_counted_ptr<const fastuidraw::GlyphAtlasTexelBackingStoreBase>
fastuidraw::GlyphAtlas::
texel_store(void) const
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);
  return d->m_texel_store;
}


fastuidraw::reference_counted_ptr<const fastuidraw::GlyphAtlasGeometryBackingStoreBase>
fastuidraw::GlyphAtlas::
geometry_store(void) const
{
  GlyphAtlasPrivate *d;
  d = reinterpret_cast<GlyphAtlasPrivate*>(m_d);
  return d->m_geometry_store;
}
