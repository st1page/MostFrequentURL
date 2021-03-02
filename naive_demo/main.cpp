#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct Config {
  // remain 24M
  size_t mem_size = 20;
  int max_url_len = 4096;
  int split_num = 100;
  int max_split_depth = 2;
  int top_num = 5;
  const char *result_file = "result.txt";
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

std::vector<std::string> shard_names;

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
  fclose(input);

  for (int i = 0; i < config.split_num; i++) {
    outShards[i].close();
    if (outShards[i].size == 0) continue;
    if (outShards[i].size > config.mem_size / 2) {
      bool ok = split(outShards[i].filename.c_str(),
                      outShards[i].filename.c_str(), depth + 1);
      if (ok)
        remove(outShards[i].filename.c_str());
      else
        shard_names.push_back(outShards[i].filename);
    } else {
      shard_names.push_back(outShards[i].filename);
    }
  }
  delete[] outShards;
  return true;
}

template <typename T>
struct TopK {
  struct Pair {
    uint32_t count;
    T data;
    bool operator<(const Pair &other) const { return count > other.count; }
  };

  uint32_t k;
  std::priority_queue<Pair> q;
  TopK(uint32_t kk) { k = kk; }
  void update(uint32_t count, const T &data) {
    Pair pair;
    pair.count = count;
    pair.data = data;
    q.push(pair);
    if (q.size() > k) {
      q.pop();
    }
  }
};

struct StringTable {
  struct String {
    uint32_t count;
    std::string str;
    String(uint32_t c, std::string s) : count(c), str(s) {}
  };
  void updateString(char *&p, char *endpoint) {
    char *st = p;
    while (*p == '\n' && p != endpoint) p++;
    while (p != endpoint && *p != '\n') {
      cur_hash = cur_hash * 131 + *p;
      // XXX(st1page): might optimize
      cur_string.push_back(*p);
      p++;
    }
    if (*p == '\n') {
      auto iter = map.find(cur_hash);
      if (iter == map.end()) {
        uint32_t d_mem_use = sizeof(String) + cur_string.size();
        map.insert({cur_hash, {1, cur_string}});
        mem_use += d_mem_use;
      } else {
        iter->second.count++;
      }
      cur_hash = 0;
      cur_string.clear();
      p++;
    }
    return;
  }

  void consumeBuf(char *buf, uint32_t len) {
    char *p = buf;
    char *endpoint = buf + len;
    while (p != endpoint) updateString(p, endpoint);
  }
  std::string cur_string;
  uint64_t cur_hash;
  uint32_t mem_use;
  std::unordered_map<uint64_t, String> map;
};

// XXX(st1page): url might hash collision, probility = url_num/2^64
void combian(TopK<std::string> &topk, const char *filename) {
  FILE *input = fopen(filename, "r");
  uint32_t bufsize = config.mem_size;
  char *buf = (char *)malloc(bufsize);
  StringTable urls;
  uint32_t readsize;
  while (readsize = fread(buf, 1, bufsize, input)) {
    urls.consumeBuf(buf, readsize);
  }
  fclose(input);
  free(buf);
  for (auto pair : urls.map) {
    topk.update(pair.second.count, pair.second.str);
  }
}
int main() {
  bool ok = split("url.txt", "", 0);
  if (!ok) {
    puts("split fail");
    return 0;
  }
  puts("split ok");
  TopK<std::string> topk(config.top_num);
  for (auto file_name : shard_names) {
    combian(topk, file_name.c_str());
    printf("combianed %s\n", file_name.c_str());
  }
  FILE *out = fopen(config.result_file, "w");
  while (!topk.q.empty()) {
    auto str_pair = topk.q.top();
    topk.q.pop();
    fprintf(out, "%d %s\n", str_pair.count, str_pair.data.c_str());
  }
  fclose(out);

  return 0;
}