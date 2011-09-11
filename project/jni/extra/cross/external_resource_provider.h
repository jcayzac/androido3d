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
#include "core/cross/external_resource.h"
#include <string>

namespace o3d {
	class Pack;

	namespace extra {

		/** This interface is responsible for fetching data from a
		  * network connection, cache, database or filesystem,
		  * given an URI.
		  */
		class IExternalResourceProvider {
		public:
			/** @brief Get an external resource.
			  *
			  * @param pack Pack the callee should use if some by-product objects are created along the way.
			  * @param uri  URI of the resource being requested.
			  * @return A valid resource, or a NULL reference if something went wrong.
			  *
			  * @sa {ExternalResource}
			  */
			virtual ExternalResource::Ref GetExternalResourceForURI(Pack& pack, const std::string& uri) = 0;
		};

	} // namespace extra
} // namespace o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
