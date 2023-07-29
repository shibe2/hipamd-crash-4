#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#include <dlfcn.h>

static const char func_name[] = "hiptest";

int main(int argc, char **argv) {

  // allow specifying the same library multiple times,
  // but load each library once
  std::vector<std::string> rps;
  if (argc > 1)
    rps.reserve(argc - 1);
  for (int i = 1; i < argc; ++i) {
    auto rp = realpath(argv[i], nullptr);
    if (!rp) {
      auto e = errno;
      std::cerr << "realpath(" << argv[i] << ") failed";
      if (e) {
        std::cerr << ": ";
        auto msg = strerror(errno);
        if (msg)
          std::cerr << msg;
        else
          std::cerr << "error " << e;
      }
      std::cerr << '\n';
      return EXIT_FAILURE;
    }
    rps.emplace_back(rp);
    free(rp);
  }

  // load all libraries before running tests
  std::map<std::string, void *> libs;
  for (const auto &fn : rps) {
    if (fn.empty())
      continue;
    auto [lib, isnew] = libs.emplace(fn, nullptr);
    if (!isnew)
      continue;
    lib->second = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!lib->second) {
      std::cerr << "dlopen(" << fn << ") failed: " << dlerror() << '\n';
      return EXIT_FAILURE;
    }
  }

  // run tests in the specified order, including duplicates
  bool passed = true;
  for (unsigned i = 0; i < rps.size(); ++i) {
    const auto &fn = rps[i];
    if (fn.empty())
      continue;
    auto lib = libs.find(fn);
    if (lib == libs.end())
      continue;
    auto f = dlsym(lib->second, func_name);
    if (!f) {
      std::cerr << "dlsym(" << fn << ',' << func_name
                << ") failed: " << dlerror() << '\n';
      return EXIT_FAILURE;
    }
    std::cout << argv[i + 1U] << ": ";
    std::cout.flush();
    auto msg = ((const char *(*)(unsigned))f)(3);
    if (msg) {
      std::cout << msg;
      passed = false;
    } else
      std::cout << "ok";
    std::cout << '\n';
  }

  for (auto &[fn, lib] : libs)
    if (dlclose(lib)) {
      std::cerr << "dlclose(" << fn << ") failed: " << dlerror() << '\n';
      return EXIT_FAILURE;
    }

  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
