#include <graphlab/util/cuckoo_map.hpp>
#include <graphlab/util/cuckoo_map_pow2.hpp>
#include <graphlab/util/timer.hpp>
#include <graphlab/util/random.hpp>
#include <graphlab/util/memory_info.hpp>
#include <boost/unordered_map.hpp>
#include <graphlab/logger/assertions.hpp>
#include <graphlab/macros_def.hpp>

void sanity_checks() {
  boost::unordered_map<size_t, size_t> um;
  graphlab::cuckoo_map_pow2<size_t, size_t> cm(-1);
  for (size_t i = 0;i < 10000; ++i) {
    cm[17 * i] = i;
    um[17 * i] = i;
  }

  for (size_t i = 0;i < 10000; ++i) {
    assert(cm[17 * i] == i);
    assert(um[17 * i] == i);
  }
  assert(cm.size() == 10000);
  assert(um.size() == 10000);

  for (size_t i = 0;i < 10000; i+=2) {
    cm.erase(17*i);
    um.erase(17*i);
  }
  for (size_t i = 0;i < 10000; i+=2) {
    assert(cm.count(17*i) == i % 2);
    assert(um.count(17*i) == i % 2);
    if (cm.count(17*i)) {
      assert(cm.find(17*i)->second == i);
    }
  }

  assert(cm.size() == 5000);
  assert(um.size() == 5000);

  typedef graphlab::cuckoo_map_pow2<size_t, size_t, (size_t)(-1)>::value_type vpair;
  {
    size_t cnt = 0;
    foreach(vpair &v, cm) {
      ASSERT_EQ(v.second, um[v.first]);
      ++cnt;
    }
    ASSERT_EQ(cnt, 5000);
  }
  {
    size_t cnt = 0;
    foreach(const vpair &v, cm) {
      ASSERT_EQ(v.second, um[v.first]);
      ++cnt;
    }
    ASSERT_EQ(cnt, 5000);
  }
}




void sanity_checks2() {
  boost::unordered_map<size_t, size_t> um;
  graphlab::cuckoo_map<size_t, size_t> cm(-1);
  for (size_t i = 0;i < 10000; ++i) {
    cm[17 * i] = i;
    um[17 * i] = i;
  }

  for (size_t i = 0;i < 10000; ++i) {
    assert(cm[17 * i] == i);
    assert(um[17 * i] == i);
  }
  assert(cm.size() == 10000);
  assert(um.size() == 10000);

  for (size_t i = 0;i < 10000; i+=2) {
    cm.erase(17*i);
    um.erase(17*i);
  }
  for (size_t i = 0;i < 10000; i+=2) {
    assert(cm.count(17*i) == i % 2);
    assert(um.count(17*i) == i % 2);
    if (cm.count(17*i)) {
      assert(cm.find(17*i)->second == i);
    }
  }

  assert(cm.size() == 5000);
  assert(um.size() == 5000);

  typedef graphlab::cuckoo_map<size_t, size_t, (size_t)(-1)>::value_type vpair;
  {
    size_t cnt = 0;
    foreach(vpair &v, cm) {
      ASSERT_EQ(v.second, um[v.first]);
      ++cnt;
    }
    ASSERT_EQ(cnt, 5000);
  }
  {
    size_t cnt = 0;
    foreach(const vpair &v, cm) {
      ASSERT_EQ(v.second, um[v.first]);
      ++cnt;
    }
    ASSERT_EQ(cnt, 5000);
  }
}

std::string randstring(size_t len) {
  std::string ret; ret.reserve(len);
  for (size_t i = 0;i < len; ++i) {
    ret = ret + graphlab::random::fast_uniform('A','Z');
  }
  return ret;
}

void more_interesting_data_types_check() {
  boost::unordered_map<std::string, std::string> um;
  graphlab::cuckoo_map_pow2<std::string, std::string> cm("");
  for (size_t i = 0;i < 10000; ++i) {
    std::string s = randstring(16);
    cm[s] = s;
    um[s] = s;
  }

  assert(cm.size() == 10000);
  assert(um.size() == 10000);

  
  typedef boost::unordered_map<std::string, std::string>::value_type vpair;
  foreach(vpair& v, um) {
    ASSERT_EQ(v.second, cm[v.first]);
  }


  foreach(vpair& v, cm) {
    ASSERT_EQ(v.second, um[v.first]);
  }


  foreach(const vpair& v, cm) {
    ASSERT_EQ(v.second, um[v.first]);
  }
}


void more_interesting_data_types_check2() {
  boost::unordered_map<std::string, std::string> um;
  graphlab::cuckoo_map<std::string, std::string> cm("");
  for (size_t i = 0;i < 10000; ++i) {
    std::string s = randstring(16);
    cm[s] = s;
    um[s] = s;
  }

  assert(cm.size() == 10000);
  assert(um.size() == 10000);


  typedef boost::unordered_map<std::string, std::string>::value_type vpair;
  foreach(vpair& v, um) {
    ASSERT_EQ(v.second, cm[v.first]);
  }


  foreach(vpair& v, cm) {
    ASSERT_EQ(v.second, um[v.first]);
  }


  foreach(const vpair& v, cm) {
    ASSERT_EQ(v.second, um[v.first]);
  }
}


void benchmark() {
  graphlab::timer ti;

  size_t NUM_ELS = 10000000;
  
  std::vector<uint32_t> v;
  uint32_t u = 0;
  for (size_t i = 0;i < NUM_ELS; ++i) {
    v.push_back(u);
    u += 1 + rand() % 8;
  }
  std::random_shuffle(v.begin(), v.end());
  graphlab::memory_info::print_usage();

  {
    boost::unordered_map<uint32_t, uint32_t> um;
    ti.start();
    for (size_t i = 0;i < NUM_ELS; ++i) {
      um[v[i]] = i;
    }
    std::cout <<  NUM_ELS / 1000000 << "M unordered map inserts in " << ti.current_time() << " (Load factor = " << um.load_factor() << ")" << std::endl;

    graphlab::memory_info::print_usage();
    
    ti.start();
    for (size_t i = 0;i < 10000000; ++i) {
      size_t t = um[v[i]];
      assert(t == i);
    }
    std::cout << "10M unordered map successful probes in " << ti.current_time() << std::endl;
    um.clear();
  }

  {
    graphlab::cuckoo_map<uint32_t, uint32_t, 3, uint32_t> cm(-1, 128);

    //cm.reserve(102400);
    ti.start();
    for (size_t i = 0;i < NUM_ELS; ++i) {
      cm[v[i]] = i;
      if (i % 1000000 == 0) std::cout << cm.load_factor() << std::endl;

    }
    std::cout <<  NUM_ELS / 1000000 << "M cuckoo map inserts in " << ti.current_time() << " (Load factor = " << cm.load_factor() << ")" << std::endl;

    graphlab::memory_info::print_usage();

    ti.start();
    for (size_t i = 0;i < 10000000; ++i) {
      size_t t = cm[v[i]];
      assert(t == i);
    }
    std::cout << "10M cuckoo map successful probes in " << ti.current_time() << std::endl;

  }
  
  {
    graphlab::cuckoo_map_pow2<uint32_t, uint32_t, 3, uint32_t> cm(-1, 128);
    
    //cm.reserve(102400);
    ti.start();
    for (size_t i = 0;i < NUM_ELS; ++i) {
      cm[v[i]] = i;
      if (i % 1000000 == 0) std::cout << cm.load_factor() << std::endl;

    }
    std::cout << NUM_ELS / 1000000 << "M cuckoo map pow2 inserts in " << ti.current_time() << " (Load factor = " << cm.load_factor() << ")" << std::endl;

    graphlab::memory_info::print_usage();

    ti.start();
    for (size_t i = 0;i < 10000000; ++i) {
      size_t t = cm[v[i]];
      assert(t == i);
    }
    std::cout << "10M cuckoo map pow2 successful probes in " << ti.current_time() << std::endl;

  }
}




void benchmark_strings() {
  graphlab::timer ti;

  size_t NUM_ELS = 1000000;

  std::vector<std::string> v;
  for (size_t i = 0;i < NUM_ELS; ++i) {
    v.push_back(randstring(16));
  }
  graphlab::memory_info::print_usage();

  {
    boost::unordered_map<std::string, std::string> um;
    ti.start();
    for (size_t i = 0;i < NUM_ELS; ++i) {
      um[v[i]] = v[i];
    }
    std::cout <<  NUM_ELS / 1000000 << "M unordered map inserts in " << ti.current_time() << " (Load factor = " << um.load_factor() << ")" << std::endl;

    graphlab::memory_info::print_usage();

    ti.start();
    for (size_t i = 0;i < 1000000; ++i) {
      std::string t = um[v[i]];
      assert(t == v[i]);
    }
    std::cout << "1M unordered map successful probes in " << ti.current_time() << std::endl;
    um.clear();
  }

  {
    graphlab::cuckoo_map<std::string, std::string, 3, uint32_t> cm("", 128);

    //cm.reserve(102400);
    ti.start();
    for (size_t i = 0;i < NUM_ELS; ++i) {
      cm[v[i]] = v[i];
      if (i % 1000000 == 0) std::cout << cm.load_factor() << std::endl;

    }
    std::cout <<  NUM_ELS / 1000000 << "M cuckoo map inserts in " << ti.current_time() << " (Load factor = " << cm.load_factor() << ")" << std::endl;

    graphlab::memory_info::print_usage();

    ti.start();
    for (size_t i = 0;i < 1000000; ++i) {
      std::string t = cm[v[i]];
      assert(t == v[i]);
    }
    std::cout << "1M cuckoo map successful probes in " << ti.current_time() << std::endl;

  }

  {
    graphlab::cuckoo_map_pow2<std::string, std::string, 3, uint32_t> cm("", 128);

    //cm.reserve(102400);
    ti.start();
    for (size_t i = 0;i < NUM_ELS; ++i) {
      cm[v[i]] = v[i];
      if (i % 1000000 == 0) std::cout << cm.load_factor() << std::endl;

    }
    std::cout << NUM_ELS / 1000000 << "M cuckoo map pow2 inserts in " << ti.current_time() << " (Load factor = " << cm.load_factor() << ")" << std::endl;

    graphlab::memory_info::print_usage();

    ti.start();
    for (size_t i = 0;i < 1000000; ++i) {
      std::string t = cm[v[i]];
      assert(t == v[i]);
    }
    std::cout << "1M cuckoo map pow2 successful probes in " << ti.current_time() << std::endl;

  }
}
int main(int argc, char** argv) {
  std::cout << "Basic Sanity Checks... ";
  std::cout.flush();
  sanity_checks();
  sanity_checks2();
  more_interesting_data_types_check();
  more_interesting_data_types_check2();

  std::cout << "Done" << std::endl;


  std::cout << "\n\n\nRunning Benchmarks. uint32-->uint32" << std::endl;
  benchmark();


  std::cout << "\n\n\nRunning Benchmarks. string-->string" << std::endl;
  benchmark_strings();

}