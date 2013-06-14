/*  
 * Copyright (c) 2009 Carnegie Mellon University. 
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an "AS
 *  IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *  express or implied.  See the License for the specific language
 *  governing permissions and limitations under the License.
 *
 * For more about this software visit:
 *
 *      http://www.graphlab.ml.cmu.edu
 *
 */


// standard C++ headers
#include <iostream>
#include <cxxtest/TestSuite.h>

// includes the entire graphlab framework
#include <graphlab/graph/local_edge_buffer.hpp>
#include <graphlab/logger/assertions.hpp>
#include <graphlab/macros_def.hpp>

class graph_test : public CxxTest::TestSuite {
public:

  struct edge_data { 
    int from; 
    int to;
    int count;
    edge_data (int f = 0, int t = 0) : from(f), to(t), count(1) {}
    
    edge_data operator+=(const edge_data& other) {
      ASSERT_EQ(from, other.from);
      ASSERT_EQ(to, other.to);
      count += other.count;
      return *this;
    }
  };

  typedef graphlab::local_edge_buffer<edge_data, true> edge_buffer;
  typedef uint32_t vertex_id_type;


  void test_sparse_graph () {
    edge_buffer buffer;
    buffer.add_edge(1,3,edge_data(1,3));
    buffer.add_edge(2,3,edge_data(2,3));
    buffer.add_edge(4,3,edge_data(4,3));
    buffer.add_edge(5,3,edge_data(5,3));
    buffer.add_edge(3,2, edge_data(3,2));
    buffer.add_edge(3,5, edge_data(3,5));

    ASSERT_EQ(buffer.size(), 6);
    for (size_t i = 0; i < buffer.size(); ++i) {
      ASSERT_EQ(buffer.data[i].count, 1);
    }

    buffer.add_edge(1,3,edge_data(1,3));
    buffer.add_edge(2,3,edge_data(2,3));
    buffer.add_edge(4,3,edge_data(4,3));
    buffer.add_edge(5,3,edge_data(5,3));
    buffer.add_edge(3,2, edge_data(3,2));
    buffer.add_edge(3,5, edge_data(3,5));

    for (size_t i = 0; i < buffer.size(); ++i) {
      ASSERT_EQ(buffer.data[i].count, 1);
    }
    buffer.merge_duplicate_values();
    for (size_t i = 0; i < buffer.size(); ++i) {
      ASSERT_EQ(buffer.data[i].count, 2);
    }
  }
};

#include <graphlab/macros_undef.hpp>
