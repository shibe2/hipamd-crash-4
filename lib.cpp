#include <iostream>
#include <numeric>
#include <vector>

#include <hip/hip_runtime.h>

__global__ void saxpy_kernel(const float a, const float *d_x, float *d_y,
                             const unsigned size) {
  const unsigned int global_idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (global_idx < size)
    d_y[global_idx] = a * d_x[global_idx] + d_y[global_idx];
}

#define tolerance_scale 2
template <typename t> static bool approx_equal(t v1, t v2) {
  if (std::isnan(v1))
    return std::isnan(v2);
  if (std::isnan(v2))
    return false;
  if (std::isinf(v1) || std::isinf(v2))
    return v1 == v2;
  return std::abs(v1 - v2) <=
         std::max(std::numeric_limits<t>::min(),
                  std::numeric_limits<t>::epsilon() *
                      std::max(std::abs(v1), std::abs(v2)) *
                      (t)tolerance_scale);
}

#define HIP_CHECK(condition)                                                   \
  do {                                                                         \
    hipError_t error = condition;                                              \
    if (error != hipSuccess)                                                   \
      return hipGetErrorName(error);                                           \
  } while (false)

// returns null if the test is passed or a message if the test is failed
extern "C" const char *hiptest(unsigned size) try {
  size_t size_bytes = size * sizeof(float);
  constexpr unsigned block_size = 256;
  unsigned grid_size = (size + block_size - 1U) / block_size;
  constexpr float a = -1.1;
  std::vector<float> x(size);
  std::iota(x.begin(), x.end(), 1.2f);
  std::vector<float> y1(size);
  std::fill(y1.begin(), y1.end(), 1.3f);
  float *d_x = nullptr, *d_y = nullptr;
  HIP_CHECK(hipMalloc(&d_x, size_bytes));
  HIP_CHECK(hipMalloc(&d_y, size_bytes));
  HIP_CHECK(hipMemcpy(d_x, x.data(), size_bytes, hipMemcpyHostToDevice));
  HIP_CHECK(hipMemcpy(d_y, y1.data(), size_bytes, hipMemcpyHostToDevice));
  hipLaunchKernelGGL(saxpy_kernel, dim3(grid_size), dim3(block_size), 0,
                     hipStreamDefault, a, d_x, d_y, size);
  HIP_CHECK(hipGetLastError());
  std::vector<float> y2(size);
  HIP_CHECK(hipMemcpy(y2.data(), d_y, size_bytes, hipMemcpyDeviceToHost));
  HIP_CHECK(hipFree(d_x));
  HIP_CHECK(hipFree(d_y));
  for (unsigned i = 0; i < size; ++i) {
    float got = y2[i];
    float want = (double)a * (double)x[i] + (double)y1[i];
    if (!approx_equal(got, want))
      return "wrong result";
  }
  return nullptr;
} catch (std::exception &e) {
  static std::string msg;
  msg.assign(e.what());
  return msg.c_str();
}
