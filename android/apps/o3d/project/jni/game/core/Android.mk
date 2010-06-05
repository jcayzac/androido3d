#### gamecore
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gamecore
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -I$(LOCAL_PATH)/.. \


LOCAL_SRC_FILES := \
  File.cpp \
  MemoryManager.cpp \
  MemoryPool.cpp \
  MemoryPoolTracked.cpp \
  MetaBase.cpp \
  MetaObject.cpp \
  MetaRegistry.cpp \
  Object.cpp \
  ObjectManager.cpp \
  Serializable.cpp \
  Stack.cpp \
  my_assert.cpp \


include $(BUILD_STATIC_LIBRARY)


