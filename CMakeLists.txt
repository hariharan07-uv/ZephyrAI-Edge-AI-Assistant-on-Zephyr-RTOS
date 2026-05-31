
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyrai)

set(EI_DIR ${CMAKE_SOURCE_DIR}/ei-model)

include_directories(
    ${EI_DIR}
    ${EI_DIR}/edge-impulse-sdk
    ${EI_DIR}/edge-impulse-sdk/classifier
    ${EI_DIR}/edge-impulse-sdk/dsp
    ${EI_DIR}/edge-impulse-sdk/porting
    ${EI_DIR}/edge-impulse-sdk/porting/zephyr
    ${EI_DIR}/edge-impulse-sdk/third_party/flatbuffers/include
    ${EI_DIR}/edge-impulse-sdk/third_party/gemmlowp
    ${EI_DIR}/edge-impulse-sdk/third_party/ruy
    ${EI_DIR}/edge-impulse-sdk/CMSIS/DSP/Include
    ${EI_DIR}/edge-impulse-sdk/CMSIS/Core/Include
    ${EI_DIR}/model-parameters
    ${EI_DIR}/tflite-model
    ${EI_DIR}/edge-impulse-sdk/porting/espressif/esp-dsp/modules/fft/include
    ${EI_DIR}/edge-impulse-sdk/porting/espressif/esp-dsp/modules/math/include
    ${EI_DIR}/edge-impulse-sdk/porting/espressif/esp-dsp/modules/common/include
)

target_compile_definitions(app PRIVATE
    EI_CLASSIFIER_ALLOCATION_STATIC
    EIDSP_USE_CMSIS_DSP=0
    EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN=0
    EIDSP_LOAD_CMSIS_DSP_SOURCES=0
    EI_CLASSIFIER_USE_FULL_TFLITE=0
    EI_DSP_ENGINE_ESP_DSP=0
    __ARM_FEATURE_MVE=0
    TF_LITE_STATIC_MEMORY=1
)

# DSP sources
file(GLOB_RECURSE EI_DSP_SOURCES
    ${EI_DIR}/edge-impulse-sdk/dsp/*.cpp
)
list(FILTER EI_DSP_SOURCES EXCLUDE REGEX ".*espressif.*")
list(FILTER EI_DSP_SOURCES EXCLUDE REGEX ".*esp-dsp.*")

# TFLite Micro sources
file(GLOB_RECURSE EI_TFLITE_SOURCES
    ${EI_DIR}/edge-impulse-sdk/tensorflow/lite/kernels/*.cc
    ${EI_DIR}/edge-impulse-sdk/tensorflow/lite/kernels/internal/*.cc
    ${EI_DIR}/edge-impulse-sdk/tensorflow/lite/micro/kernels/*.cc
    ${EI_DIR}/edge-impulse-sdk/tensorflow/lite/micro/*.cc
    ${EI_DIR}/edge-impulse-sdk/tensorflow/lite/core/api/*.cc
    ${EI_DIR}/edge-impulse-sdk/tensorflow/lite/*.cc
)

# Classifier + porting sources
file(GLOB_RECURSE EI_CLASSIFIER_SOURCES
    ${EI_DIR}/edge-impulse-sdk/classifier/*.cpp
    ${EI_DIR}/edge-impulse-sdk/porting/zephyr/*.cpp
    ${EI_DIR}/tflite-model/*.cpp
)

target_sources(app PRIVATE
    src/esp_dsp_stub.c
    src/main.cpp
    ${EI_DSP_SOURCES}
    ${EI_TFLITE_SOURCES}
    ${EI_CLASSIFIER_SOURCES}
)
