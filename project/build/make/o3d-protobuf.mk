%.pb.h %.pb.cc: %.proto
	@echo "Protobuf: $(PRIVATE_MODULE) <= $(notdir $<)"
	@$(O3D_DIR)/jni/third_party/protobuf/protoc --cpp_out=$(dir $@) --proto_path=$(dir $<) $<
