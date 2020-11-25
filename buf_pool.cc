/**
 * Created on: 6/4/16
 *     Author: xing
 */

#include "utils_cpp/buf_pool.h"

namespace utils
{
Ret
Buffer::write(const uint8_t *src, size_t length) noexcept
{
  unless(src) return Ret::E_ARG_NULL;
  if (length > size) return Ret::E_ARG;

  memcpy(data, src, length);
  len = length;

  return Ret::OK;
}

Ret
Buffer::read(uint8_t *dst, size_t &length) const noexcept
{
  unless(dst) return Ret::E_ARG_NULL;
  unless(len) return Ret::NO;

  length = (length > len) ? len : length;

  memcpy(dst, data, length);

  return Ret::OK;
}

BufferPool::BufferPool(size_t buf_count,
                       size_t buf_size,
                       const std::string &name)
    : Module{name}, buf_count{buf_count}, buf_size{buf_size}
{
  UTILS_RAISE_IF(buf_count <= 0 || buf_size < sizeof(nodeptr))

  size_t sz = UTILS_ROUND(buf_size + sizeof(Buffer), sizeof(nodeptr));
  mem = new uint8_t[buf_count * sz];
  UTILS_RAISE_UNLESS(mem)

  free_mem = (nodeptr)mem;

  sz /= sizeof(nodeptr);
  nodeptr tmp = free_mem;
  while (--buf_count) {
    tmp->next = tmp + sz;
    tmp = tmp->next;
  }

  tmp->next = nullptr;
}

Buffer *
BufferPool::alloc() noexcept
{
  ++stat.total;

  unless(free_mem) { return nullptr; }

  ++stat.succ;

  auto b = (Buffer *)free_mem;
  free_mem = free_mem->next;
  b->init(buf_size, 0);

  return b;
};

CBufferPool::CBufferPool(size_t buf_count,
                         size_t buf_size,
                         const std::string &name)
    : Module{name}, buf_count{buf_count}, buf_size{buf_size}
{
  UTILS_RAISE_IF(buf_count <= 0 || buf_size < sizeof(nodeptr))

  size_t sz = UTILS_ROUND(buf_size + sizeof(Buffer), sizeof(nodeptr));
  mem = new uint8_t[buf_count * sz];
  UTILS_RAISE_UNLESS(mem)

  free_mem = (nodeptr)mem;

  sz /= sizeof(nodeptr);
  nodeptr tmp = free_mem;
  while (--buf_count) {
    tmp->next = tmp + sz;
    tmp = tmp->next;
  }

  tmp->next = nullptr;
}

Buffer *
CBufferPool::alloc() noexcept
{
  ++stat.total;

  unless(free_mem) { return nullptr; }

  {
    std::lock_guard<std::mutex> bar{mut};
    auto b = (Buffer *)free_mem;
    free_mem = free_mem->next;
    b->init(buf_size, 0);
    ++stat.succ;
    return b;
  }
};

}  // namespace utils
