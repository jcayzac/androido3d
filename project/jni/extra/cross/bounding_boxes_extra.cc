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

#include "extra/cross/bounding_boxes_extra.h"
#include "core/cross/transform.h"

namespace o3d {
	namespace extra {

		void updateBoundingBoxes(Transform& root) {
			BoundingBox box;
			// Update children first, and add their bounding box to this
			// entity's.
			const TransformRefArray& children(root.GetChildrenRefs());

			for(size_t i(0); i < children.size(); ++i) {
				Transform& child(*children[i]);
				updateBoundingBoxes(child);
				BoundingBox childBox;
				child.bounding_box().Mul(child.local_matrix(), &childBox);
				childBox.Add(box, &box);
			}

			// Inflate this entity's bounding box with any geometry
			// it might contain.
			const ShapeRefArray& shapes(root.GetShapeRefs());

			for(size_t i(0); i < shapes.size(); ++i) {
				// TODO:
				// if (isBillBoardShape(shapes[i])) continue;
				Shape& shape(*shapes[i]);
				const ElementRefArray& elements(shape.GetElementRefs());

				for(size_t j(0); j < elements.size(); ++j) {
					Element& element(*elements[j]);
					element.bounding_box().Add(box, &box);
				}
			}

			root.set_bounding_box(box);
		}

	} // extra
} // o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */

