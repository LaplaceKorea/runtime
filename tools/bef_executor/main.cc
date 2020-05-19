// Copyright 2020 The TensorFlow Runtime Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//===- Host Executor Test Driver ------------------------------------------===//
//
// This file parses command-line options and runs a given mlir file using test
// driver library.
//
//===----------------------------------------------------------------------===//

#include <string>

#include "llvm/Support/CommandLine.h"
#include "tfrt/bef_executor_driver/bef_executor_driver.h"
#include "tfrt/host_context/host_allocator.h"
#include "tfrt/tracing/tracing.h"

static llvm::cl::opt<std::string> cl_input_filename(  // NOLINT
    llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::init("-"));

static llvm::cl::list<std::string> cl_shared_libs(  // NOLINT
    "shared_libs", llvm::cl::desc("Specify dynamic library with ops"),
    llvm::cl::ZeroOrMore, llvm::cl::MiscFlags::CommaSeparated);

static llvm::cl::list<std::string> cl_functions(  // NOLINT
    "functions", llvm::cl::desc("Specify MLIR functions to run"),
    llvm::cl::ZeroOrMore, llvm::cl::MiscFlags::CommaSeparated);

static llvm::cl::list<std::string> cl_devices(  // NOLINT
    "devices", llvm::cl::desc("Specify devices to register"),
    llvm::cl::ZeroOrMore, llvm::cl::MiscFlags::CommaSeparated);

// Enable ConcurrentWorkQueue types to be specified on the command line.
static llvm::cl::opt<std::string> cl_work_queue_type(  // NOLINT
    "work_queue_type",
    llvm::cl::desc("Specify concurrent work queue type (s, mstd, ...):"),
    llvm::cl::init("s"));

// Enable HostAllocator types to be specified on the command line.
static llvm::cl::opt<tfrt::HostAllocatorType> cl_host_allocator_type(  // NOLINT
    "host_allocator_type", llvm::cl::desc("Specify host allocator type:"),
    llvm::cl::values(
        clEnumValN(tfrt::HostAllocatorType::kMalloc, "malloc", "Malloc."),
        clEnumValN(tfrt::HostAllocatorType::kTestFixedSizeMalloc,
                   "test_fixed_size_1k",
                   "Fixed size (1 kB) Malloc for testing."),
        clEnumValN(tfrt::HostAllocatorType::kProfiledMalloc,
                   "profiled_allocator", "Malloc with metric profiling."),
        clEnumValN(tfrt::HostAllocatorType::kLeakCheckMalloc,
                   "leak_check_allocator", "Malloc with memory leak check.")),
    llvm::cl::init(tfrt::HostAllocatorType::kLeakCheckMalloc));

// Enable aggregate op handler types to be specified on the command line.
static llvm::cl::opt<bool> cl_enable_tracing(  // NOLINT
    "enable_tracing", llvm::cl::desc("Enable Performance Tracing"),
    llvm::cl::Optional, llvm::cl::ValueDisallowed);

//===----------------------------------------------------------------------===//
// Driver main
//===----------------------------------------------------------------------===//

int main(int argc, char** argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv, "HostExecutor test driver\n");

  tfrt::RunBefConfig run_config;
  run_config.program_name = argv[0];
  run_config.input_filename = cl_input_filename;
  run_config.shared_libs = cl_shared_libs;
  run_config.functions = cl_functions;
  run_config.devices = cl_devices;
  run_config.work_queue_type = cl_work_queue_type;
  run_config.host_allocator_type = cl_host_allocator_type;

  llvm::Optional<tfrt::tracing::TracingRequester> tracing;
  if (cl_enable_tracing) tracing.emplace();

  return RunBefExecutor(run_config);
}
