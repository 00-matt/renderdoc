/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include "3rdparty/glslang/SPIRV/spirv.hpp"
#include "3rdparty/glslang/glslang/Include/ResourceLimits.h"
#include "api/replay/renderdoc_replay.h"

enum class SPIRVShaderStage
{
  Vertex,
  TessControl,
  TessEvaluation,
  Geometry,
  Fragment,
  Compute,
  Invalid,
};

enum class SPIRVSourceLanguage
{
  Unknown,
  OpenGLGLSL,
  VulkanGLSL,
  VulkanHLSL,
};

struct SPIRVCompilationSettings
{
  SPIRVCompilationSettings(SPIRVSourceLanguage l, SPIRVShaderStage s) : stage(s), lang(l) {}
  SPIRVCompilationSettings() = default;

  SPIRVShaderStage stage = SPIRVShaderStage::Invalid;
  SPIRVSourceLanguage lang = SPIRVSourceLanguage::Unknown;
  std::string entryPoint;
};

void InitSPIRVCompiler();
void ShutdownSPIRVCompiler();

struct SPVInstruction;

enum class GraphicsAPI : uint32_t;
enum class ShaderStage : uint32_t;
enum class ShaderBuiltin : uint32_t;
struct ShaderReflection;
struct ShaderBindpointMapping;

ShaderBuiltin BuiltInToSystemAttribute(ShaderStage stage, const spv::BuiltIn el);

// extra information that goes along with a ShaderReflection that has extra information for SPIR-V
// patching
struct SPIRVPatchData
{
  struct InterfaceAccess
  {
    // ID of the base variable
    uint32_t ID;

    // ID of the struct parent of this variable
    uint32_t structID;

    // the access chain of indices
    std::vector<uint32_t> accessChain;

    // is this input/output part of a matrix
    bool isMatrix = false;

    // this is an element of an array that's been exploded after [0].
    // i.e. this is false for non-arrays, and false for element [0] in an array, then true for
    // elements [1], [2], [3], etc..
    bool isArraySubsequentElement = false;
  };

  // matches the input/output signature array, with details of where to fetch the output from in the
  // SPIR-V.
  std::vector<InterfaceAccess> inputs;
  std::vector<InterfaceAccess> outputs;

  // the output topology for tessellation and geometry shaders
  Topology outTopo = Topology::Unknown;
};

struct SPVModule
{
  SPVModule();
  ~SPVModule();

  std::vector<uint32_t> spirv;

  struct
  {
    uint8_t major, minor;
  } moduleVersion;
  uint32_t generator;

  spv::SourceLanguage sourceLang;
  uint32_t sourceVer;

  std::string cmdline;
  std::vector<rdcpair<std::string, std::string>> sourceFiles;

  std::vector<std::string> extensions;

  std::vector<spv::Capability> capabilities;

  std::vector<SPVInstruction *>
      operations;    // all operations (including those that don't generate an ID)

  std::vector<SPVInstruction *> ids;    // pointers indexed by ID

  std::vector<SPVInstruction *> sourceexts;       // source extensions
  std::vector<SPVInstruction *> entries;          // entry points
  std::vector<SPVInstruction *> globals;          // global variables
  std::vector<SPVInstruction *> specConstants;    // specialization constants
  std::vector<SPVInstruction *> funcs;            // functions
  std::vector<SPVInstruction *> structs;          // struct types

  SPVInstruction *GetByID(uint32_t id);
  std::string Disassemble(const std::string &entryPoint);

  std::vector<std::string> EntryPoints() const;
  ShaderStage StageForEntry(const std::string &entryPoint) const;

  void MakeReflection(GraphicsAPI sourceAPI, ShaderStage stage, const std::string &entryPoint,
                      ShaderReflection &reflection, ShaderBindpointMapping &mapping,
                      SPIRVPatchData &patchData) const;
};

std::string CompileSPIRV(const SPIRVCompilationSettings &settings,
                         const std::vector<std::string> &sources, std::vector<uint32_t> &spirv);
void ParseSPIRV(uint32_t *spirv, size_t spirvLength, SPVModule &module);

static const uint32_t SpecializationConstantBindSet = 1234567;

struct SpecConstant
{
  uint32_t specID;
  std::vector<byte> data;
};

void FillSpecConstantVariables(const rdcarray<ShaderConstant> &invars,
                               rdcarray<ShaderVariable> &outvars,
                               const std::vector<SpecConstant> &specInfo);

namespace glslang
{
class TShader;
class TProgram;
};

glslang::TShader *CompileShaderForReflection(SPIRVShaderStage stage,
                                             const std::vector<std::string> &sources);
glslang::TProgram *LinkProgramForReflection(const std::vector<glslang::TShader *> &shaders);

extern TBuiltInResource DefaultResources;

enum class ReflectionInterface
{
  Input,
  Output,
  Uniform,
  UniformBlock,
  ShaderStorageBlock,
  AtomicCounterBuffer,
  BufferVariable,
};

enum class ReflectionProperty
{
  ActiveResources,
  BufferBinding,
  TopLevelArrayStride,
  BlockIndex,
  ArraySize,
  IsRowMajor,
  NumActiveVariables,
  BufferDataSize,
  NameLength,
  Type,
  LocationComponent,
  ReferencedByVertexShader,
  ReferencedByTessControlShader,
  ReferencedByTessEvaluationShader,
  ReferencedByGeometryShader,
  ReferencedByFragmentShader,
  ReferencedByComputeShader,
  Internal_Binding,
  AtomicCounterBufferIndex,
  Offset,
  ArrayStride,
  MatrixStride,
  Location,
};

void glslangGetProgramInterfaceiv(glslang::TProgram *program, ReflectionInterface programInterface,
                                  ReflectionProperty pname, int32_t *params);

void glslangGetProgramResourceiv(glslang::TProgram *program, ReflectionInterface programInterface,
                                 uint32_t index, const std::vector<ReflectionProperty> &props,
                                 int32_t bufSize, int32_t *length, int32_t *params);
uint32_t glslangGetProgramResourceIndex(glslang::TProgram *program, const char *name);

const char *glslangGetProgramResourceName(glslang::TProgram *program,
                                          ReflectionInterface programInterface, uint32_t index);