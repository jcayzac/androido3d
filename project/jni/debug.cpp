/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include "base/cross/config.h"
#include "core/cross/buffer.h"
#include "core/cross/draw_element.h"
#include "core/cross/effect.h"
#include "core/cross/element.h"
#include "core/cross/material.h"
#include "core/cross/param_array.h"
#include "core/cross/primitive.h"
#include "core/cross/render_node.h"
#include "core/cross/sampler.h"
#include "core/cross/shape.h"
#include "core/cross/skin.h"
#include "core/cross/texture.h"
#include "core/cross/transform.h"
#include "debug.h"
namespace o3d_utils {
	using namespace o3d;

	void DumpMultiLineString(const std::string& str) {
		size_t pos = 0;
		int line_num = 1;

		for(;;) {
			size_t start = pos;
			pos = str.find_first_of('\n', pos);
			std::string line = str.substr(start, pos - start);
			O3D_LOG(INFO) << line_num << ": " << line;

			if(pos == std::string::npos) {
				break;
			}

			++pos;
			++line_num;
		}
	}

	void DumpPoint3(const o3d::Point3& v, const char* label) {
		O3D_LOG(INFO) << label << ": " << v[0] << ", " << v[1] << ", " << v[2];
	}

	void DumpVector3(const o3d::Vector3& v, const char* label) {
		O3D_LOG(INFO) << label << ": " << v[0] << ", " << v[1] << ", " << v[2];
	}

	void DumpFloat3(const o3d::Float3& v, const char* label) {
		O3D_LOG(INFO) << label << ": " << v[0] << ", " << v[1] << ", " << v[2];
	}

	void DumpVector4(const o3d::Vector4& v, const char* label) {
		O3D_LOG(INFO) << label << ": " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
	}

	void DumpFloat4(const o3d::Float4& v, const char* label) {
		O3D_LOG(INFO) << label << ": " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
	}

	void DumpParams(const o3d::ParamObject* obj, const std::string& indent) {
		if(obj) {
			const o3d::NamedParamRefMap& params = obj->params();

			for(o3d::NamedParamRefMap::const_iterator it = params.begin();
			        it != params.end();
			        ++it) {
				o3d::Param* param = it->second;
				o3d::Param* input = param->input_connection();
				std::string value = "--na--";
				char buf[256];

				if(param->IsA(o3d::ParamFloat::GetApparentClass())) {
					float v = static_cast<o3d::ParamFloat*>(param)->value();
					sprintf(buf, "%.3f", v);
					value = buf;
				}
				else if(param->IsA(o3d::ParamFloat2::GetApparentClass())) {
					o3d::Float2 v = static_cast<o3d::ParamFloat2*>(param)->value();
					sprintf(buf, "%.3f, %.3f", v[0], v[1]);
					value = buf;
				}
				else if(param->IsA(o3d::ParamFloat3::GetApparentClass())) {
					o3d::Float3 v = static_cast<o3d::ParamFloat3*>(param)->value();
					sprintf(buf, "%.3f, %.3f, %.3f", v[0], v[1], v[2]);
					value = buf;
				}
				else if(param->IsA(o3d::ParamFloat4::GetApparentClass())) {
					o3d::Float4 v = static_cast<o3d::ParamFloat4*>(param)->value();
					sprintf(buf, "%.3f, %.3f, %.3f, %.3f", v[0], v[1], v[2], v[3]);
					value = buf;
				}
				else if(param->IsA(o3d::ParamMatrix4::GetApparentClass())) {
					o3d::Matrix4 v = static_cast<o3d::ParamMatrix4*>(param)->value();
					value = "";

					for(size_t ii = 0; ii < 4; ++ii) {
						sprintf(buf, "[%.3f, %.3f, %.3f, %.3f]",
						        v[ii][0], v[ii][1], v[ii][2], v[ii][3]);
						value += buf;
					}
				}
				else if(param->IsA(o3d::ParamBoundingBox::GetApparentClass())) {
					o3d::BoundingBox v =
					    static_cast<o3d::ParamBoundingBox*>(param)->value();
					sprintf(buf, "%s [%.3f, %.3f, %.3f], [%.3f, %.3f, %.3f]",
					        v.valid() ? "valid" : "**invalid**",
					        v.min_extent()[0], v.min_extent()[1],
					        v.min_extent()[2], v.max_extent()[0], v.max_extent()[1], v.max_extent()[2]);
					value = buf;
				}
				else if(param->IsA(o3d::ParamInteger::GetApparentClass())) {
					int v = static_cast<o3d::ParamInteger*>(param)->value();
					sprintf(buf, "%d", v);
					value = buf;
				}
				else if(param->IsA(o3d::ParamBoolean::GetApparentClass())) {
					bool v = static_cast<o3d::ParamBoolean*>(param)->value();
					value = v ? "true" : "false";
				}
				else if(param->IsA(o3d::ParamString::GetApparentClass())) {
					value = static_cast<o3d::ParamString*>(param)->value();
				}
				else if(param->IsA(o3d::ParamMaterial::GetApparentClass())) {
					o3d::Material* v = static_cast<o3d::ParamMaterial*>(param)->value();
					value = v ? v->name() : "NULL";
				}
				else if(param->IsA(o3d::ParamEffect::GetApparentClass())) {
					o3d::Effect* v = static_cast<o3d::ParamEffect*>(param)->value();
					value = v ? v->name() : "NULL";
				}
				else if(param->IsA(o3d::ParamTexture::GetApparentClass())) {
					o3d::Texture* v = static_cast<o3d::ParamTexture*>(param)->value();
					value = v ? v->name() : "NULL";
				}
				else if(param->IsA(o3d::ParamSampler::GetApparentClass())) {
					o3d::Sampler* v = static_cast<o3d::ParamSampler*>(param)->value();
					value = v ? v->name() : "NULL";
				}
				else if(param->IsA(o3d::ParamSkin::GetApparentClass())) {
					o3d::Skin* v = static_cast<o3d::ParamSkin*>(param)->value();
					value = v ? v->name() : "NULL";
				}
				else if(param->IsA(o3d::ParamStreamBank::GetApparentClass())) {
					o3d::StreamBank* v = static_cast<o3d::ParamStreamBank*>(param)->value();
					value = v ? v->name() : "NULL";
				}
				else if(param->IsA(o3d::ParamParamArray::GetApparentClass())) {
					o3d::ParamArray* v = static_cast<o3d::ParamParamArray*>(param)->value();
					value = v ? v->name() : "NULL";
				}

				if(input) {
					O3D_LOG(INFO) << indent << ":Param: " << param->name()
					              << " [" << param->GetClass()->name() << "] <- "
					              << input->owner()->name() << "." << input->name();
				}
				else {
					O3D_LOG(INFO) << indent << ":Param: " << param->name()
					              << " [" << param->GetClass()->name() << "] = "
					              << value;
				}
			}
		}
	}

	void DumpRenderNode(
	    const o3d::RenderNode* render_node, const std::string& indent) {
		if(render_node) {
			O3D_LOG(INFO) << indent << render_node->GetClass()->name();
			DumpParams(render_node, indent + "   ");
			const o3d::RenderNodeRefArray& children = render_node->children();

			if(!children.empty()) {
				std::string inner = indent + "    ";

				for(size_t ii = 0; ii < children.size(); ++ii) {
					DumpRenderNode(children[ii], inner);
				}
			}
		}
	}

	void DumpDrawElement(
	    const o3d::DrawElement* drawelement, const std::string& indent) {
		if(drawelement) {
			O3D_LOG(INFO) << indent << "DrawElement: " << drawelement->name();
			DumpParams(drawelement, indent + "   ");
		}
	}

	void DumpElement(const o3d::Element* element, const std::string& indent) {
		if(element) {
			const char* pre = indent.c_str();
			O3D_LOG(INFO) << indent << "Element: " << element->name();
			DumpParams(element, indent + "   ");

			if(element->IsA(o3d::Primitive::GetApparentClass())) {
				const o3d::Primitive* prim = down_cast<const o3d::Primitive*>(element);
				O3D_LOG(INFO)
				        << pre << "  num_primitives: " << prim->number_primitives() << "\n"
				        << pre << "  num_vertices: " << prim->number_vertices() << "\n"
				        << pre << "  prim_type: " << prim->primitive_type() << "\n"
				        << pre << "  start index: " << prim->start_index() << "\n"
				        << pre << "  indexbuffer: " << prim->index_buffer()->name();
				o3d::StreamBank* sb = prim->stream_bank();
				const o3d::StreamParamVector& params = sb->vertex_stream_params();

				for(size_t jj = 0; jj < params.size(); ++jj) {
					const o3d::ParamVertexBufferStream* param = params[jj];
					const o3d::Stream& stream = param->stream();
					const o3d::Field& field = stream.field();
					const o3d::Buffer* buffer = field.buffer();
					O3D_LOG(INFO) << pre << "    stream: s:" << stream.semantic()
					              << " si:" << stream.semantic_index()
					              << " start:" << stream.start_index()
					              << " numv:" << stream.GetMaxVertices()
					              << " buf:" << buffer->name()
					              << ":" << buffer->GetClass()->name();
					unsigned num = std::min(buffer->num_elements(), 5u);
					float floats[5 * 4];
					field.GetAsFloats(0, floats, field.num_components(), num);

					for(unsigned elem = 0; elem < num; ++elem) {
						float* v = &floats[elem * field.num_components()];

						switch(field.num_components()) {
						case 1:
							O3D_LOG(INFO) << pre << "     " << elem << ": " << v[0];
							break;
						case 2:
							O3D_LOG(INFO) << pre << "     " << elem << ": " << v[0] << ", " << v[1];
							break;
						case 3:
							O3D_LOG(INFO) << pre << "     " << elem << ": " << v[0] << ", " << v[1] << ", " << v[2];
							break;
						case 4:
							O3D_LOG(INFO) << pre << "     " << elem << ": " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
							break;
						}
					}

					const o3d::Param* input = param->input_connection();

					if(input) {
						const o3d::ParamObject* owner = input->owner();
						O3D_LOG(INFO) << pre << "      input: "
						              << owner->name() << ":"
						              << owner->GetClass()->name() << ":"
						              << input->name() << ":"
						              << input->GetClass()->name();

						if(owner->IsA(o3d::SkinEval::GetApparentClass())) {
							const o3d::SkinEval* se = down_cast<const o3d::SkinEval*>(owner);
							O3D_LOG(INFO) << pre << "        se skin: " << se->skin()->name();
							O3D_LOG(INFO) << pre << "        se mats: " << se->matrices()->name();
							const o3d::ParamArray* pa = se->matrices();
							O3D_LOG(INFO) << pre << "        pa size: " << pa->size();

							for(size_t pp = 0; pp < pa->size(); ++pp) {
								o3d::Param* mp = pa->GetUntypedParam(pp);
								o3d::Param* inp = mp->input_connection();
								O3D_LOG(INFO) << pre << "        " << pp << ": <- "
								              << (inp ? inp->owner()->name() : "-") << ":"
								              << (inp ? inp->name() : "-");
							}

							O3D_LOG(INFO) << pre << "      -skineval-";
							DumpParams(se, indent + "    ");
						}
					}
				}
			}

			const o3d::DrawElementRefArray& draw_elements =
			    element->GetDrawElementRefs();

			if(!draw_elements.empty()) {
				std::string inner = indent + "    ";

				for(size_t ii = 0; ii < draw_elements.size(); ++ii) {
					DumpDrawElement(draw_elements[ii], inner);
				}
			}
		}
	}

	void DumpShape(const o3d::Shape* shape, const std::string& indent) {
		if(shape) {
			O3D_LOG(INFO) << indent << "Shape: " << shape->name();
		}
	}

	void DumpTransform(const o3d::Transform* transform, const std::string& indent) {
		if(transform) {
			O3D_LOG(INFO) << indent << "Transform: " << transform->name();
			DumpParams(transform, indent + "    ");
			const o3d::TransformRefArray& children = transform->GetChildrenRefs();

			if(!children.empty()) {
				std::string inner = indent + "    ";

				for(size_t ii = 0; ii < children.size(); ++ii) {
					DumpTransform(children[ii], inner);
				}
			}

			const o3d::ShapeRefArray& shapes = transform->GetShapeRefs();

			if(!shapes.empty()) {
				std::string inner = indent + "    ";

				for(size_t ii = 0; ii < shapes.size(); ++ii) {
					DumpShape(shapes[ii], inner);
				}
			}
		}
	}

	void DumpMatrix(const o3d::Matrix4& mat) {
		for(int ii = 0; ii < 4; ++ii) {
			O3D_LOG(INFO) << "   "
			              << mat[ii][0] << ", "
			              << mat[ii][1] << ", "
			              << mat[ii][2] << ", "
			              << mat[ii][3];
		}
	}

	bool EndsWith(const std::string& str, const std::string& end) {
		return str.size() >= end.size() &&
		       str.substr(str.size() - end.size()).compare(end) == 0;
	}

} // namespace o3d_utils
