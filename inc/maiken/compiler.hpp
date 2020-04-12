/**
Copyright (c) 2017, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _MAIKEN_COMPILER_HPP_
#define _MAIKEN_COMPILER_HPP_

#include "kul/cli.hpp"
#include "kul/except.hpp"
#include "kul/map.hpp"
#include "kul/os.hpp"
#include "kul/proc.hpp"
#include "kul/string.hpp"

#include "maiken/global.hpp"

namespace maiken {
class KUL_PUBLISH Application;

struct CompilationInfo {
  std::string lib_prefix, lib_postfix, lib_ext;

  CompilationInfo()
      : lib_prefix(AppVars::INSTANCE().envVar("MKN_LIB_PRE_DEF")),
        lib_ext(AppVars::INSTANCE().envVar("MKN_LIB_EXT_DEF")) {}
};

namespace compiler {
enum Mode { NONE = 0, STAT, SHAR };
}

struct CompileDAO {
  maiken::Application const &app;
  std::string const &compiler, &in, &out;
  std::vector<std::string> const &args, &incs;
  compiler::Mode const &mode;
  bool dryRun = false;
};
struct LinkDAO {
  maiken::Application const &app;
  std::string const &linker, &linkerEnd, &out;
  std::vector<kul::Dir> stars;
  std::vector<std::string> const &objects, &libs, &libPaths;
  compiler::Mode const &mode;
  bool dryRun = false;
};

class Compiler;
class CompilerProcessCapture : public kul::ProcessCapture {
 private:
  std::exception_ptr ep;
  std::string c, f;

 public:
  CompilerProcessCapture() : ep() {}
  CompilerProcessCapture(kul::AProcess &p) : kul::ProcessCapture(p), ep() {}
  CompilerProcessCapture(const CompilerProcessCapture &cp)
      : kul::ProcessCapture(cp), ep(cp.ep), c(cp.c), f(cp.f) {}

  void exception(const std::exception_ptr &e) { ep = e; }
  const std::exception_ptr &exception() const { return ep; }

  void cmd(std::string const &cm) { this->c = cm; }
  std::string const &cmd() const { return c; }

  void file(std::string const &f) { this->f = f; }
  std::string const &file() const { return f; }
};

class Compiler {
 protected:
  Compiler(const int &v) : version(v) {}
  const int version;
  std::unordered_map<uint8_t, std::string> m_optimise_c, m_optimise_l_bin, m_optimise_l_lib,
      m_debug_c, m_debug_l_bin, m_debug_l_lib, m_warn_c;

 public:
  virtual ~Compiler() {}
  virtual bool sourceIsBin() const = 0;

  virtual CompilerProcessCapture compileSource(CompileDAO &dao) const KTHROW(kul::Exception) = 0;

  virtual CompilerProcessCapture buildExecutable(LinkDAO &dao) const KTHROW(kul::Exception) = 0;

  virtual CompilerProcessCapture buildLibrary(LinkDAO &dao) const KTHROW(kul::Exception) = 0;

  virtual void preCompileHeader(const std::vector<std::string> &incs,
                                const std::vector<std::string> &args, std::string const &in,
                                std::string const &out, bool dryRun = false) const
      KTHROW(kul::Exception) = 0;

  std::string compilerDebug(const uint8_t &key) const {
    return m_debug_c.count(key) ? m_debug_c.at(key) : "";
  }
  std::string compilerOptimization(const uint8_t &key) const {
    return m_optimise_c.count(key) ? m_optimise_c.at(key) : "";
  }
  std::string compilerWarning(const uint8_t &key) const {
    return m_warn_c.count(key) ? m_warn_c.at(key) : "";
  }
  std::string linkerDebugBin(const uint8_t &key) const {
    return m_debug_l_bin.count(key) ? m_debug_l_bin.at(key) : "";
  }
  std::string linkerDebugLib(const uint8_t &key) const {
    return m_debug_l_lib.count(key) ? m_debug_l_lib.at(key) : "";
  }
  std::string linkerOptimizationBin(const uint8_t &key) const {
    return m_optimise_l_bin.count(key) ? m_optimise_l_bin.at(key) : "";
  }
  std::string linkerOptimizationLib(const uint8_t &key) const {
    return m_optimise_l_lib.count(key) ? m_optimise_l_lib.at(key) : "";
  }
};

// this class exists to minimise thread captures and avoid forking too much stuff
class CompilationUnit {
 private:
  const maiken::Application &app;
  const Compiler *comp;
  const std::string compiler;
  const std::vector<std::string> args;
  const std::vector<std::string> incs;
  const std::string in;
  const std::string out;
  const compiler::Mode mode;
  const bool dryRun;

 public:
  CompilationUnit(const maiken::Application &app, const Compiler *comp, std::string const &compiler,
                  const std::vector<std::string> &args, const std::vector<std::string> &incs,
                  std::string const &in, std::string const &out, const compiler::Mode &mode,
                  bool dryRun)
      : app(app),
        comp(comp),
        compiler(compiler),
        args(args),
        incs(incs),
        in(in),
        out(out),
        mode(mode),
        dryRun(dryRun) {}

  CompilerProcessCapture compile() const KTHROW(kul::Exception);
};
}  // namespace maiken
#endif /* _MAIKEN_COMPILER_HPP_ */
