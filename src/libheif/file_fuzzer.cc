/*
 * HEIF codec.
 * Copyright (c) 2017 struktur AG, Joachim Bauch <bauch@struktur.de>
 *
 * This file is part of libheif.
 *
 * libheif is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libheif is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libheif.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdlib.h>

#include "heif.h"

static const enum heif_colorspace kFuzzColorSpace = heif_colorspace_YCbCr;
static const enum heif_chroma kFuzzChroma = heif_chroma_420;

static void TestDecodeImage(struct heif_context* ctx,
    const struct heif_image_handle* handle) {
  struct heif_image* image;
  struct heif_error err;

  heif_image_handle_is_primary_image(handle);
  int width = heif_image_handle_get_width(handle);
  int height = heif_image_handle_get_height(handle);
  assert(width >= 0);
  assert(height >= 0);
  err = heif_decode_image(handle, &image, kFuzzColorSpace, kFuzzChroma, nullptr);
  if (err.code != heif_error_Ok) {
    return;
  }

  assert(heif_image_get_colorspace(image) == kFuzzColorSpace);
  assert(heif_image_get_chroma_format(image) == kFuzzChroma);

  // TODO(fancycode): Should we also check the planes?

  heif_image_release(image);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  struct heif_context* ctx;
  struct heif_error err;
  struct heif_image_handle* primary_handle;
  int images_count;
  heif_item_id* image_IDs = NULL;

  ctx = heif_context_alloc();
  assert(ctx);
  err = heif_context_read_from_memory(ctx, data, size, nullptr);
  if (err.code != heif_error_Ok) {
    // Not a valid HEIF file passed (which is most likely while fuzzing).
    goto quit;
  }

  err = heif_context_get_primary_image_handle(ctx, &primary_handle);
  if (err.code == heif_error_Ok) {
    assert(heif_image_handle_is_primary_image(primary_handle));
    TestDecodeImage(ctx, primary_handle);
    heif_image_handle_release(primary_handle);
  }

  images_count = heif_context_get_number_of_top_level_images(ctx);
  if (!images_count) {
    // File doesn't contain any images.
    goto quit;
  }

  image_IDs = (heif_item_id*)malloc(images_count * sizeof(heif_item_id));
  assert(image_IDs);
  images_count = heif_context_get_list_of_top_level_image_IDs(ctx, image_IDs, images_count);
  if (!images_count) {
    // Could not get list of image ids.
    goto quit;
  }

  for (int i = 0; i < images_count; ++i) {
    struct heif_image_handle* image_handle;
    err = heif_context_get_image_handle(ctx, image_IDs[i], &image_handle);
    if (err.code != heif_error_Ok) {
      // Ignore, we are only interested in crashes here.
      continue;
    }

    TestDecodeImage(ctx, image_handle);

    int num_thumbnails = heif_image_handle_get_number_of_thumbnails(image_handle);
    for (int t = 0; t < num_thumbnails; ++t) {
      struct heif_image_handle* thumbnail_handle = nullptr;
      heif_image_handle_get_thumbnail(image_handle, t, &thumbnail_handle);
      if (thumbnail_handle) {
        TestDecodeImage(ctx, thumbnail_handle);
        heif_image_handle_release(thumbnail_handle);
      }
    }

    heif_image_handle_release(image_handle);
  }

quit:
  heif_context_free(ctx);
  free(image_IDs);
  return 0;
}
