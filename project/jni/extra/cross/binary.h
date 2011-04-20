/*
 * Copyright (C) 2010 Tonchidot Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <iostream>
#include "extra/cross/external_resource_provider.h"

namespace o3d {
  class Pack;
  class Transform;
}

namespace o3d {
namespace extra {

/** @brief Deserialize a scenegraph from a stream.
  *
  * This consumes a scenegraph from a stream, rebuilding it in the
  * order it is received.
  *
  * @param stream Input stream to receive the scenegraph from.
  * @param pack   A valid pack the function will use to create the various entities.
  * @param erp    An object we can get resources' data from.
  * @return       <code>NULL</code> if something went wrong, or the root of a valid scenegraph.
  *
  * @note If deserialization fails, stream's <i>fail</i> bit is set.
  *
  * @note This function will leave the stream open so it can be used to further
  * read data in another part of the application (so it is possible, for
  * instance, to stream image files and models using the same iostream).
  *
  * @sa {o3d_utils::Scene::LoadBinaryScene}
  */
Transform* LoadFromBinaryStream(std::istream& stream, Pack& pack, IExternalResourceProvider& erp);

enum TCompressionAlgorithm {
	COMPRESSION_NONE,
	COMPRESSION_GZIP,
	COMPRESSION_LZMA,
};

/** @brief Serialize a scenegraph to a stream.
  *
  * This sends a scenegraph down a stream, traversing it recursively.
  *
  * @param stream      Output stream to send the scenegraph to.
  * @param root        Root fo the scenegraph to send.
  * @param compression Compression algorithm. Defaults to {COMPRESSION_GZIP}.
  * @return            true on success, false otherwise.
  *
  * @note If serialization fails, stream's <i>fail</i> bit is set.
  *
  * @note This function will leave the stream open so it can be used to further
  * write data in another part of the application (so it is possible, for
  * instance, to stream image files and models using the same iostream).
  *
  * @note Write support can be disabled by defining the
  * <code>O3D_NO_BINARY_EXPORT</code> preprocessor macro.
  */
bool SaveToBinaryStream(std::ostream& stream, Transform& root, TCompressionAlgorithm compression = COMPRESSION_LZMA);

} // namespace extra
} // namespace o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
