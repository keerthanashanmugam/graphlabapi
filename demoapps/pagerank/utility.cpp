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
 *      http://www.graphlab.ml.cmu.edu
 *
 */


#include <iomanip>
#include "pagerank.hpp"



#include <graphlab/macros_def.hpp>



std::ostream& operator<<(std::ostream& out, const edge_data& edata) {
  return out << "E(w: " << edata.weight << ")";
}

std::ostream& operator<<(std::ostream& out, const vertex_data& vdata) {
  return out << "Rank=" << vdata.value;
}



void save_pagerank(const std::string& fname,
                   const graph_type& graph) {
  std::ofstream fout;
  fout.open(fname.c_str());
  fout << std::setprecision(10);
  for(graph_type::vertex_id_type vid = 0; 
      vid < graph.num_vertices(); ++vid) {
    fout << graph.vertex_data(vid).value << "\n";
  }
  fout.close();
} // end of save_pagerank


void get_top_pages(const graph_type& graph, size_t num_pages,
                   std::vector<graph_type::vertex_id_type>& ret) {
  typedef std::pair<float, graph_type::vertex_id_type> pair_type;
  std::priority_queue<pair_type> top;
  for(graph_type::vertex_id_type vid = 0; vid < graph.num_vertices(); ++vid) {
    const graph_type::vertex_data_type& vdata = graph.vertex_data(vid);
    top.push(std::make_pair(-vdata.value, vid));
    if(top.size() > num_pages) top.pop();
  }
  if(top.empty()) return;
  ret.resize(top.size());
  for(size_t i = top.size()-1; i < top.size(); --i) {
    ret[i] = top.top().second;
    top.pop();
  }
} // end of top pages


void normalize_graph(graph_type& graph) {
  logstream(LOG_INFO)
    << "Optimizing graph layout in memory." << std::endl;
  graph.finalize();
  logstream(LOG_INFO)
    << "Renormalizing transition probabilities." << std::endl;
  typedef graph_type::vertex_id_type vertex_id_type;
  for(vertex_id_type vid = 0; vid < graph.num_vertices(); ++vid) {  
    double sum = 0;
    const graph_type::edge_list_type out_edges = graph.out_edges(vid);
    // Sum up weight on out edges
    for(size_t i = 0; i < out_edges.size(); ++i) 
      sum += graph.edge_data(out_edges[i]).weight;
    for(size_t i = 0; i < out_edges.size(); ++i) 
      graph.edge_data(out_edges[i]).weight /= sum;
  }
  logstream(LOG_INFO)
    << "Finished normalizing transition probabilities." << std::endl;
} // end of normalize_graph


// void save_graph_as_edge_list(const std::string& fname,
//                              const graph_type& graph) {
//   std::ofstream fout; 
//   fout.open(fname.c_str()); 
//   fout << std::setprecision(10);
//   for(graph_type::vertex_id_type vid = 0; 
//       vid < graph.num_vertices(); ++vid) {
//     foreach(graph_type::edge_type edge, graph.out_edges(vid)) {
//       fout << vid << '\t' << edge.target() << '\t' 
//            << graph.edge_data(edge).weight << "\n";
//     }
//   }
//   fout.close();
// } // end of save graph as edge list




// bool load_graph(const std::string& filename,
//                 const std::string& format,
//                 graph_type& graph) {
//   if(filename.empty()) { 
//     std::cout << "No graph file was provided so we will make a toy graph." 
//               << std::endl;
//     make_toy_graph(graph);
//     return true;
//   } else {
//     // load the graph from the file
//     bool success = false;
//     std::cout << "Loading: " << filename << std::endl;
//     if(format == "metis") {
//       std::cout << "\t using format: " << format << std::endl;
//       success = load_graph_from_metis_file(filename, graph);
//     } else if(format == "jure") {
//       std::cout << "\t using format: " << format << std::endl;
//       success = load_graph_from_jure_file(filename, graph);
//     } else if(format == "tsv") {
//       std::cout << "\t using format: " << format << std::endl;
//       success = load_graph_from_tsv_file(filename, graph);
//     } else {
//       logstream(LOG_WARNING) 
//         << "Unsupported format \"" << format << "\"!" << std::endl;
//     }
//     return success;
//   }
// } // end of generic load graph







// // Creates simple 5x5 graph
// void make_toy_graph(graph_type& graph) {
//   // Create 5 vertices
//   graph.add_vertex(vertex_data());
//   graph.add_vertex(vertex_data());
//   graph.add_vertex(vertex_data());
//   graph.add_vertex(vertex_data());
//   graph.add_vertex(vertex_data());

	
//   // Page 0 links to page 3 only, so weight is 1
//   graph.add_edge(0, 3, edge_data(1));
	
//   // Page 1 links to 0 and 2
//   graph.add_edge(1, 0, edge_data(0.5));
//   graph.add_edge(1, 2, edge_data(0.5));
	
//   // ... and so on
//   graph.add_edge(2, 0, edge_data(1.0/3));
//   graph.add_edge(2, 1, edge_data(1.0/3));
//   graph.add_edge(2, 3, edge_data(1.0/3));

//   graph.add_edge(3, 0, edge_data(0.25));
//   graph.add_edge(3, 1, edge_data(0.25));
//   graph.add_edge(3, 2, edge_data(0.25));
//   graph.add_edge(3, 4, edge_data(0.25));

//   graph.add_edge(4, 0, edge_data(0.2));
//   graph.add_edge(4, 1, edge_data(0.2));
//   graph.add_edge(4, 2, edge_data(0.2));
//   graph.add_edge(4, 3, edge_data(0.2));


//   normalize_graph(graph);
// } // end of make_toy_graph





// inline void skip_newline(std::ifstream& fin) {
//   char next_char = ' ';
//   fin.get(next_char);
//   ASSERT_EQ(next_char, '\n');  
// }


// bool load_graph_from_metis_file(const std::string& filename,
//                                 graph_type& graph) {
  
//   std::ifstream fin(filename.c_str());
//   if(!fin.good()) return false;
//   size_t nverts = 0, nedges = 0;
//   fin >> nverts >> nedges;
//   std::cout << "Loading graph with " 
//             << nverts << " vertices and "
//             << nedges << " edges." << std::endl;
//   skip_newline(fin);
//   std::string line_buffer;
//   graph.resize(nverts);
//   graph_type::vertex_id_type source = 0 ;
//   size_t edges_processed = 0;
//   for(; source < nverts; ++source) {
//     while(fin.peek() != '\n') {
//       ASSERT_TRUE(fin.good());
//       graph_type::vertex_id_type target = 0;
//       fin >> target; 
//       edges_processed++;
//       ASSERT_GT(target, 0);
//       // decrement the value since starting value is 1 not zero
//       target--; 
//       ASSERT_LT(target, graph.num_vertices());     
//       if(source != target) {
//         graph.add_edge(source, target);
//       } else {
//         // add the self edge by updating the vertex weight
//         graph.vertex_data(source).self_weight = 1;
//       }
//     }
//     skip_newline(fin);
//   }
//   fin.close();
//   if(edges_processed == nedges) {
//     std::cout << "\tDirected Graph." << std::endl;
//   } else {
//     std::cout << "\tUndirected Graph." << std::endl;
//   }



//   std::cout << "Normalizing the Graph." << std::endl;
//   normalize_graph(graph);  
//   std::cout << "Finished normalizing." << std::endl;
  
//   return true;
// } // end of load graph from metis file.


// bool load_graph_from_jure_file(const std::string& filename,
//                                 graph_type& graph) {
//   std::ifstream fin(filename.c_str());
//   if(!fin.good()) return false;
//   // Loop through file reading each line
//   while(fin.good() && !fin.eof()) {
//     if(fin.peek() == '#') {
//       std::string str;
//       std::getline(fin, str);
//       std::cout << str << std::endl;
//       continue;
//     }
//     size_t source = 0;
//     size_t target = 0;
//     fin >> source;
//     if(!fin.good()) break;
//     //  fin.ignore(1); // skip comma
//     fin >> target;
//     assert(fin.good());
//     // Ensure that the number of vertices is correct
//     if(source >= graph.num_vertices() ||
//        target >= graph.num_vertices())
//       graph.resize(std::max(source, target) + 1);
//     if(source != target) {
//       // Add the edge
//       const edge_data edata(1);
//       graph.add_edge(source, target, edata);
//     } else {
//       // add the self edge by updating the vertex weight
//       graph.vertex_data(source).self_weight = 1;
//     }       
//   }
//   std::cout 
//     << "Finished loading graph with: " << std::endl
//     << "\t Vertices: " << graph.num_vertices() << std::endl
//     << "\t Edges: " << graph.num_edges() << std::endl;

//   normalize_graph(graph);

//   std::cout << "Graph storage size: " 
//             <<  double(graph.estimate_sizeof()) / (1024*1024) << "MB" << std::endl;


//   return true;
// } // end of load graph from jure file




// /**
//  * Load a graph file specified in the format:
//  *
//  *   source_id <tab> target_id <tab> weight
//  *   source_id <tab> target_id <tab> weight
//  *   source_id <tab> target_id <tab> weight
//  *               ....
//  *
//  * The file should not contain repeated edges.
//  */
// bool load_graph_from_tsv_file(const std::string& filename,
//                               graph_type& graph) {
//   std::ifstream fin(filename.c_str());
//   if(!fin.good()) return false;
//   // Loop through file reading each line
//   while(fin.good() && !fin.eof()) {
//     size_t source = 0;
//     size_t target = 0;
//     float weight = -1;
//     fin >> source;
//     if(!fin.good()) break;
//     //  fin.ignore(1); // skip comma
//     fin >> target;
//     assert(fin.good());
//     //  fin.ignore(1); // skip comma
//     fin >> weight;
//     assert(fin.good());
//     // Ensure that the number of vertices is correct
//     if(source >= graph.num_vertices() ||
//        target >= graph.num_vertices())
//       graph.resize(std::max(source, target) + 1);
//     if(source != target) {
//       // Add the edge
//       const edge_data edata(weight);
//       graph.add_edge(source, target, edata);
//     } else {
//       // add the self edge by updating the vertex weight
//       graph.vertex_data(source).self_weight = weight;
//     }       
//   }
//   std::cout 
//     << "Finished loading graph with: " << std::endl
//     << "\t Vertices: " << graph.num_vertices() << std::endl
//     << "\t Edges: " << graph.num_edges() << std::endl;



//   normalize_graph(graph);

//   return true;
// } // end of load graph
