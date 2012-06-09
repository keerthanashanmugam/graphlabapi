/**  
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
 *      http://graphlab.org
 *
 * Code by Danny Bickson, CMU
 *
 */


#include <cstdio>
#include <map>
#include <iostream>
#include "graphlab.hpp"
#include "../shared/io.hpp"
#include "../shared/types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
using namespace graphlab;
using namespace std;

int get_day(time_t pt);

string datafile;
bool debug = false;
int nodes = 2421057;
int num_edges = 34910937;
int split_time = 1321891200; //time to split test data
int ratingA = 19349608;
int ratingB = 15561329;

struct vertex_data {
  string filename; //file name with test data (either earlier or later)
  bool testA;
  vertex_data() { testA = true; }
  vertex_data(string _filename) : filename(_filename) { }
}; // end of vertex_data

struct edge_data {
};

typedef graphlab::graph<vertex_data, edge_data> graph_type;

struct vertex_data2 {
  double A_ii;
  double value;
  vertex_data2(): A_ii(1) { } 
  void add_self_edge(double value) { A_ii = value; }
  void set_val(double value, int field_type) { 
  }  
  double get_output(int field_type){ return -1; }
}; // end of vertex_data


struct edge_data2 {
  double weight;
  edge_data2(double weight = 0) : weight(weight) { }
  void set_field(int pos, double val){ weight = val; }
  double get_field(int pos){ return weight; }
};


typedef graphlab::graph<vertex_data2, edge_data2> graph_type2;

struct kdd_test_splitter :
   public graphlab::iupdate_functor<graph_type, kdd_test_splitter>{
   void operator()(icontext_type& context) {
   
typedef graphlab::graph<vertex_data2, edge_data2>::edge_list_type edge_list;
   int nodes = context.get_global<int>("NUM_NODES");

    vertex_data& vdata = context.vertex_data();
    FILE * pfile = open_file(vdata.testA ? "testA.sorted" : "testB.sorted", "w");
    graph_type2 _graph; 
    bipartite_graph_descriptor info;
    info.force_non_square = true;
    info.rows = info.cols = nodes;
    info.nonzeros = num_edges;
    int num_ratings = vdata.testA ? ratingA : ratingB;
    load_graph(vdata.filename, "matrixmarket", info, _graph, MATRIX_MARKET_4, 
       true, true, false, true);
   
    ASSERT_EQ(info.nonzeros, num_edges); 
    MM_typecode matcode;                        
    mm_initialize_typecode(&matcode);
    mm_set_matrix(&matcode);
    mm_set_coordinate(&matcode);
    mm_set_sparse(&matcode);
    mm_set_real(&matcode);
    mm_write_banner(pfile, matcode);
    mm_write_mtx_crd_size(pfile, nodes, nodes, num_ratings);
    char linebuf[24000];
    char saveptr[1024];
     
    gzip_in_file fin(vdata.filename,false); 
    int total_ratings = 0;
     int total_lines = 0;
    while(true){
      fin.get_sp().getline(linebuf, 24000);
      if (fin.get_sp().eof())
        break;

      char *pch = strtok_r(linebuf," \r\n\t",(char**)&saveptr);
      if (!pch){
        logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << "[" << linebuf << "]" << std::endl;
        return;
       }
      int from = atoi(pch);
      if (from <= 0){
         logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << " document ID is zero or less: " << from << std::endl;
         return;
      }
      pch = strtok_r(NULL," \r\n\t",(char**)&saveptr);
      if (!pch){
        logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << "[" << linebuf << "]" << std::endl;
        return;
       }
      int to = atoi(pch);
      if (to <= 0){
         logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << " document ID is zero or less: " << from << std::endl;
         return;
      }
      pch = strtok_r(NULL," \r\n\t",(char**)&saveptr);
      if (!pch){
        logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << "[" << linebuf << "]" << std::endl;
        return;
       }
      int rating = atoi(pch);
      if (rating != 0){
         logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << " invalid rating, not -1 or 1 " << from << std::endl;
         return;
      }
      pch = strtok_r(NULL," \r\n\t",(char**)&saveptr);
      if (!pch){
        logstream(LOG_ERROR) << "Error when parsing file: " << vdata.filename << ":" << total_lines << "[" << linebuf << "]" << std::endl;
        return;
       }
      int time = atoi(pch);
      total_lines++;
           if ((vdata.testA && time < split_time) ||
               (!vdata.testA && time >= split_time)){
            total_ratings++;
            fprintf(pfile, "%d %d %d %d\n", 
                    from, 
                    to, -1, 
                    get_day(time)-283);
            }
        }
     
     fclose(pfile);
     if (vdata.testA)
        ASSERT_EQ(total_ratings , ratingA);
     else ASSERT_EQ(total_ratings , ratingB);
    logstream(LOG_INFO)<<"Finished going over " << total_ratings << " rating " << " for 1196411 unique users " << std::endl; 
    logstream(LOG_INFO)<<"Successfully created output file: " << (vdata.testA ? "testA.sorted" : "testB.sorted") << std::endl;
    } //operator()    
}; //update_functor




int main(int argc,  char *argv[]) {
  
  global_logger().set_log_level(LOG_INFO);
  global_logger().set_log_to_console(true);

  graphlab::command_line_options clopts("GraphLab Linear Solver Library");

  int numnodes = 2421057; // 1420949264;//121408373;
  
  clopts.attach_option("data", &datafile, datafile,
                       "KDD cup test input file");
  clopts.add_positional("data");

  // Parse the command line arguments
  if(!clopts.parse(argc, argv)) {
    std::cout << "Invalid arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  logstream(LOG_WARNING)
    << "Eigen detected. (This is actually good news!)" << std::endl;
  logstream(LOG_INFO) 
    << "GraphLab Parsers library code by Danny Bickson, CMU" 
    << std::endl 
    << "Send comments and bug reports to danny.bickson@gmail.com" 
    << std::endl;
 
  // Create a core
  graphlab::core<graph_type, kdd_test_splitter> core;
  core.set_options(clopts); // Set the engine options

  vertex_data data(datafile);
  data.testA = true;
  core.graph().add_vertex(0, data);
  data.testA = false;
  core.graph().add_vertex(1, data);

  std::cout << "Schedule all vertices" << std::endl;
  core.schedule_all(kdd_test_splitter());
 
  core.add_global("NUM_NODES", numnodes);
  double runtime= core.start();
  std::cout << "Finished in " << runtime << std::endl;

  logstream(LOG_INFO)<<"Finished exporting test data!" << endl;
   return EXIT_SUCCESS;
}



