#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

struct Config {
  // remain 24M
  size_t mem_size = 1024 * 1024 * 1000;
  int max_url_len = 4096;
  int split_num = 100;
  int max_split_depth = 2;
} config;

const int SEED_NUM = 31;
const int seeds[SEED_NUM] = {131, 137, 139, 149, 151, 157, 163, 167,
                             173, 179, 181, 191, 193, 197, 199, 211,
                             223, 227, 229, 233, 239, 241, 251, 257,
                             263, 269, 271, 277, 281, 283, 293};

struct OutShard {
  FILE *file;
  uint size;
  std::string filename;
  void initOutShard(const char *shardPrefix, int id) {
    filename = (std::string(shardPrefix) + "_" + std::to_string(id)).c_str();
    size = 0;
    file = NULL;
  }
  void write(char *src, size_t len) {
    if (size == 0) {
      file = fopen(filename.c_str(), "w");
    }
    size_t dsize = fwrite(src, 1, len, file);
    if (dsize != len) {
      puts("fwrite err");
      exit(1);
    }
    size += dsize;
  }
  void close() {
    if (file != NULL) fclose(file);
  }
};

bool split(const char *filename, const char *shardPrefix, int depth) {
  if (depth >= config.max_split_depth) {
    return false;
  }
  FILE *out = fopen(filename, "a");
  fputc('\n', out);
  fclose(out);

  OutShard *outShards = new OutShard[config.split_num];
  for (int i = 0; i < config.split_num; i++) {
    outShards[i].initOutShard(shardPrefix, i);
  }
  uint32_t seed = seeds[depth];

  char *url = new char[config.max_url_len];
  FILE *input = fopen(filename, "r");
  url[config.max_url_len - 2] = '\n';

  while (fgets(url, config.max_url_len, input)) {
    if (url[config.max_url_len - 2] != '\n') {
      puts("url is too long");
      exit(1);
    }
    char *p = url;
    uint32_t hash = 0;
    while (*p != '\n') {
      hash = hash * seed + (*p++);
    }
    uint32_t len = p - url;
    if (!len) continue;
    outShards[hash % config.split_num].write(url, len + 1);
  }
  delete[] url;
  fclose(stdin);

  for (int i = 0; i < config.split_num; i++) {
    outShards[i].close();
    if (outShards[i].size > config.mem_size / 2) {
      bool ok = split(outShards[i].filename.c_str(),
                      outShards[i].filename.c_str(), depth + 1);
      if (ok) remove(outShards[i].filename.c_str());
    }
  }
  delete[] outShards;
  return true;
}
int main() {
  bool ok = split("url.txt", "", 0);
  if (!ok) {
    puts("split fail");
    return 0;
  }
  return 0;
}