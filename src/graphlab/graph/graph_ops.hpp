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


/**
 * \file graph_ops.hpp
 *
 * This file supports basic graph io operations to simplify reading
 * and writing adjacency structures from files.
 *
 */

#ifndef GRAPHLAB_GRAPH_OPS_HPP
#define GRAPHLAB_GRAPH_OPS_HPP



#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>


#include <graphlab/util/hdfs.hpp>



#include <graphlab/macros_def.hpp>
namespace graphlab {
  

  namespace graph_ops {

    /** \brief This function constructs a heuristic coloring for the 
        graph and returns the number of colors */
    template<typename Graph>
    size_t color(Graph& graph) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;

      // Reset the colors
      for(vertex_id_type v = 0; v < graph.num_vertices(); ++v) graph.color(v) = 0;
      // construct a permuation of the vertices to use in the greedy
      // coloring. 
      std::vector<std::pair<vertex_id_type, vertex_id_type> > 
	permutation(graph.num_vertices());
      for(vertex_id_type v = 0; v < graph.num_vertices(); ++v) 
        permutation[v] = std::make_pair(-graph.in_edges(v).size(), v);
      //      std::random_shuffle(permutation.begin(), permutation.end());
      std::sort(permutation.begin(), permutation.end());
      // Recolor
      size_t max_color = 0;
      std::set<vertex_color_type> neighbor_colors;
      for(size_t i = 0; i < permutation.size(); ++i) {
        neighbor_colors.clear();
        const vertex_id_type& vid = permutation[i].second;
        // Get the neighbor colors
        foreach(edge_type edge, graph.in_edges(vid)){
          const vertex_id_type& neighbor_vid = edge.source();
          const vertex_color_type& neighbor_color = graph.color(neighbor_vid);
          neighbor_colors.insert(neighbor_color);
        }
        foreach(edge_type edge, graph.out_edges(vid)){
          const vertex_id_type& neighbor_vid = edge.target();
          const vertex_color_type& neighbor_color = graph.color(neighbor_vid);
          neighbor_colors.insert(neighbor_color);
        }

        vertex_color_type& vertex_color = graph.color(vid);
        vertex_color = 0;
        foreach(vertex_color_type neighbor_color, neighbor_colors) {
          if(vertex_color != neighbor_color) break;
          else vertex_color++;
          // Ensure no wrap around
          ASSERT_NE(vertex_color, 0);                
        }
        max_color = std::max(max_color, size_t(vertex_color) );
      }
      // Return the NUMBER of colors
      return max_color + 1;           
    } // end of compute coloring
    
    
    /**
     * \brief Check that the colors satisfy a valid coloring of the graph.
     * return true is coloring is valid;
     */
    template<typename Graph>
    bool valid_coloring(const Graph& graph) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      for(vertex_id_type vid = 0; vid < graph.num_vertices(); ++vid) {
        const vertex_color_type& vertex_color = color(vid);
        // Get the neighbor colors
        foreach(const edge_type& edge, in_edges(vid)){
          const vertex_id_type& neighbor_vid = edge.source();
          const vertex_color_type& neighbor_color = color(neighbor_vid);
          if(vertex_color == neighbor_color) return false;
        }
      }
      return true;
    } // end of validate coloring


    /**
     * builds a topological_sort of the graph returning it in topsort. 
     * 
     * \param[out] topsort Resultant topological sort of the graph vertices.
     *
     * function will return false if graph is not acyclic.
     */
    template<typename Graph>
    bool topological_sort(const Graph& graph, 
                          std::vector<typename Graph::vertex_id_type>& topsort) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      topsort.clear();
      topsort.reserve(graph.num_vertices());
      std::vector<size_t> indeg;
      indeg.resize(graph.num_vertices());
      std::queue<vertex_id_type> q;
      for (size_t i = 0;i < graph.num_vertices(); ++i) {
        indeg[i] = graph.get_in_edges(i).size();
        if (indeg[i] == 0) {
          q.push(i);
        }
      }
    
      while (!q.empty()) {
        vertex_id_type v = q.front();
        q.pop();
        topsort.push_back(v);
        foreach(edge_type edge, graph.get_out_edges(v)) {
          vertex_id_type destv = edge.target();
          --indeg[destv];
          if (indeg[destv] == 0) {
            q.push(destv);
          }
        }
      }
      if (q.empty() && topsort.size() != graph.num_vertices()) 
        return false;
      return true;
    } // end of topological sort


    template<typename Graph>
    size_t num_neighbors(const Graph& graph, 
                         const typename Graph::vertex_id_type& vid) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      const edge_list_type in_edges =  graph.in_edges(vid); 
      const edge_list_type out_edges = graph.out_edges(vid);
      typedef typename edge_list_type::const_iterator iterator_type;
      iterator_type i = in_edges.begin();
      iterator_type j = out_edges.begin();
      size_t count = 0;      
      for( ; i != in_edges.end() && j != out_edges.end(); ++count) 
        if(i->source() == j->target()) { ++i; ++j; }
        else if(i->source() < j->target()) { ++i; }
        else { ++j; }
      for( ; i != in_edges.end(); ++i, ++count);
      for( ; j != out_edges.end(); ++j, ++count);
      return count;
    } // end of num_neighbors



    template<typename Graph>
    void neighbors(const Graph& graph, 
                   const typename Graph::vertex_id_type& vid,   
                   std::vector<typename Graph::vertex_id_type>& neighbors ) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      const edge_list_type in_edges =  graph.in_edges(vid); 
      const edge_list_type out_edges = graph.out_edges(vid);
      typedef typename edge_list_type::const_iterator iterator_type;
      iterator_type i = in_edges.begin();
      iterator_type j = out_edges.begin();
      neighbors.resize(num_neighbors(graph, vid));
      size_t idx = 0;
      for( ; i != in_edges.end() && j != out_edges.end(); ++idx) 
        if(i->source() == j->target()) { 
          neighbors[idx] = i->source(); ++i; ++j; 
        } else if(i->source() < j->target()) {
          neighbors[idx] = i->source(); ++i; 
        } else { neighbors[idx] = j->target(); ++j; } 
      for( ; i != in_edges.end(); ++i, ++idx)
        neighbors[idx] = i->source();
      for( ; j != out_edges.end(); ++j, ++idx)
        neighbors[idx] = j->target();
      ASSERT_EQ(idx, neighbors.size());
    } // end of neighbors




    

    template<typename Graph, typename Fstream>
    bool load_snap_structure(Fstream& fin, Graph& graph) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      if(!fin.good()) return false;
      // Loop through file reading each line
      size_t self_edges = 0;
      while(fin.good() && !fin.eof()) {
        if(fin.peek() == '#') {
          std::string str; std::getline(fin, str);
          std::cout << str << std::endl;
          continue;
        }
        size_t source = 0, target = 0;
        fin >> source;
        if(!fin.good()) break;
        fin >> target; assert(fin.good());
        // ignore if there is data field
        fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(source != target) graph.add_edge(source, target);
        else if(self_edges++ == 0) 
          logstream(LOG_WARNING) 
            << "Self edge encountered but not supported!" << std::endl
            << "\t Further warnings will be surpressed." << std::endl;
      } // end of while loop       
      logstream(LOG_INFO) 
        << "Finished loading graph with: " << std::endl
        << "\t Vertices: " << graph.num_vertices() << std::endl
        << "\t Edges:  " << graph.num_edges() << std::endl;
      if(self_edges > 0) 
        logstream(LOG_INFO) << "\t Dropped self edges: " << self_edges 
                            << std::endl;
      return true;
    } // end of load SNAP







    template<typename Graph, typename Fstream>
    bool load_edge_list_structure(Fstream& fin, Graph& graph) {
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      if(!fin.good()) return false;
      size_t self_edges = 0;
      // Loop through file reading each line
      while(fin.good() && !fin.eof()) {
        vertex_id_type source = 0, target = 0;
        fin >> source >> target;
        if(!fin.good()) break;
        // Ensure that the number of vertices is correct
        if(source != target) graph.add_edge(source, target);
        else if(self_edges++ == 0) 
          logstream(LOG_WARNING) 
            << "Self edge encountered but not supported!" << std::endl
            << "\t Further warnings will be surpressed." << std::endl;
      }            
      logstream(LOG_INFO) 
        << "Finished loading graph with: " << std::endl
        << "\t Vertices: " << graph.num_vertices() << std::endl
        << "\t Edges: " << graph.num_edges() << std::endl;        
      if(self_edges > 0) 
        logstream(LOG_INFO) << "\t Dropped self edges: " << self_edges 
                            << std::endl;  
      return true;
    } // end of load edge list
    





    template<typename Fstream>
    inline void skip_newline(Fstream& fin) {
      char next_char = ' '; fin.get(next_char);
    }




    
    template<typename Graph, typename Fstream>
    bool load_metis_structure(Fstream& fin, Graph& graph) { 
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      if(!fin.good()) return false;
      size_t nverts = 0, nedges = 0;
      fin >> nverts >> nedges;
      logstream(LOG_INFO) 
        << "Loading graph with: " << std::endl
        << "\t Vertices: " << nverts << std::endl
        << "\t Edges: " << nedges << std::endl;          
      skip_newline(fin);
      graph.resize(nverts);
      size_t self_edges = 0;
      for(vertex_id_type source = 0; source < nverts; ++source) {
        while(fin.peek() != '\n') {
          ASSERT_TRUE(fin.good());
          vertex_id_type target = 0;
          fin >> target; 
          ASSERT_GT(target, 0);
          // decrement the value since starting value is 1 not zero
          target--; 
          ASSERT_LT(target, graph.num_vertices());     
          if(source != target) graph.add_edge(source, target);
          else if(self_edges++ == 0) 
            logstream(LOG_WARNING) 
              << "Self edge encountered but not supported!" << std::endl
              << "\t Further warnings will be surpressed." << std::endl;
          
        }
        skip_newline(fin);
      }
      logstream(LOG_INFO) 
        << "Finished loading graph with: " << std::endl
        << "\t Vertices: " << graph.num_vertices() << std::endl
        << "\t Edges: " << graph.num_edges() << std::endl;      
      if(self_edges > 0) 
        logstream(LOG_INFO) << "\t Dropped self edges: " << self_edges 
                            << std::endl;
      return true;
    } // end of load metis




    template<typename Graph, typename Fstream>
    bool load_adj_structure(Fstream& fin, Graph& graph) { 
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      if(!fin.good()) { return false; }
      size_t self_edges = 0;     
      size_t ctr = 0;
      // Loop over the contents
      while(fin.good()) {
        // Load a vertex
        size_t source = 0, nneighbors = 0;
        fin >> source >> nneighbors; 
        if(!fin.good()) break;
        graph.add_vertex(source);
        // add neighbors
        for(size_t i = 0; i < nneighbors; ++i) {
          size_t target = 0;
          fin >> target;
          if(!fin.good()) {
            logstream(LOG_WARNING) << "Error reading neighbor" << std::endl; 
            return false;
          }
          if(source != target) graph.add_edge(source, target);
          else if(self_edges++ == 0) 
            logstream(LOG_WARNING) 
              << "Self edge encountered but not supported!" << std::endl
              << "\t Further warnings will be surpressed." << std::endl;
        } // end of loop over neighbors
        if (++ctr % 1000000 == 0) 
          logstream(LOG_INFO) 
            << "Added edata for " << ctr << " vertices: " 
            << source << std::endl; 
      } // end of loop over file
      logstream(LOG_INFO) 
        << "Finished loading graph with: " << std::endl
        << "\t Vertices: " << graph.num_vertices() << std::endl
        << "\t Edges:  " << graph.num_edges() << std::endl;
      return true;
    } // end of load_adj_list




    template<typename Graph, typename Fstream>
    bool load_structure_from_stream(Fstream& fstream,
                                    const std::string& format,
                                    Graph& graph) {
      if (format == "metis") return load_metis_structure(fstream, graph);
      else if (format == "snap") return load_snap_structure(fstream, graph);
      else if (format == "tsv") return load_edge_list_structure(fstream, graph);
      else if (format == "adj") return load_adj_structure(fstream, graph);
      else {
        logstream(LOG_WARNING)
          << "Invalid format \"" << format << "\"!" << std::endl;
      }
      return false;
    } // end of load_structure





    template<typename Graph>
    bool load_structure(const std::string& fname,
                        const std::string& format,
                        Graph& graph) {
      const bool gzip = boost::ends_with(fname, ".gz");

      // test to see if the graph_dir is an hadoop path
      if(boost::starts_with(fname, "hdfs://")) {
        static graphlab::hdfs hdfs;
        graphlab::hdfs::fstream in_file(hdfs, fname);
        boost::iostreams::filtering_stream<boost::iostreams::input> fin;  
        fin.set_auto_close(false);
        if(gzip) fin.push(boost::iostreams::gzip_decompressor());
        fin.push(in_file);
        if(!fin.good()) {
          std::cout << "Error opening file: " << fname << std::endl;
          return false;
        }
        const bool success = load_structure_from_stream(fin, format, graph);
        if (gzip) fin.pop();
        fin.pop();
        in_file.close();
        return success;
      } else {
        std::ifstream in_file(fname.c_str(), 
                              std::ios_base::in | std::ios_base::binary);
        boost::iostreams::filtering_stream<boost::iostreams::input> fin;  
        // Using gzip filter
        if (gzip) fin.push(boost::iostreams::gzip_decompressor());
        fin.push(in_file);
        if(!fin.good()) {
          std::cout << "Error opening file: " << fname << std::endl;
          return false;
        }
        const bool success = load_structure_from_stream(fin, format, graph);
        if (gzip) fin.pop();
        fin.pop();
        in_file.close();
        return success;
      } // end of else
    } // end of load structure





    template<typename Graph>
    bool load_structure(const std::string& fname, Graph& graph) {
      const size_t pos = fname.rfind('.');
      if(pos == std::string::npos || pos + 1 >= fname.size()) {
        logstream(LOG_WARNING) 
          << "Filename \"" << fname 
          << "\" does not have a suffix." << std::endl
          << "Unable to infer file format!" << std::endl;
        return false;
      }      
      const std::string format(fname.substr(pos+1, std::string::npos));
      logstream(LOG_INFO) << "File format: " << format << std::endl;
      return load_structure(fname, format, graph);
    } // end of load



    
    template<typename Graph>
    bool save_metis_structure(const std::string& filename,
                              const Graph& graph) { 
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
    
      std::ofstream fout(filename.c_str());
      if(!fout.good()) return false;
      // Count the number of actual edges
      size_t nedges = 0;
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i)
        nedges += num_neighbors(graph, i);
      fout << graph.num_vertices() << ' ' << (nedges/2) << '\n';
      // Save the adjacency structure
      std::vector<vertex_id_type> neighbor_set;
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i) {
        neighbors(graph, i, neighbor_set);
        for(size_t j = 0; j < neighbor_set.size(); ++j) {
          fout << (neighbor_set[j] + 1);
          if(j + 1 < neighbor_set.size()) fout << ' ';
        }
        fout << '\n';
      }
      fout.close();
      return true;
    } // end of save metis





    template<typename Graph>
    bool save_edge_list_structure(const std::string& filename,
                                  const Graph& graph) { 
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      std::ofstream fout(filename.c_str());
      if(!fout.good()) return false;
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i) 
        foreach(edge_type edge, graph.out_edges(i)) 
          fout << edge.source() << '\t' << edge.target() << '\n';      
      fout.close();
      return true;
    } // end of save metis


    template<typename Graph>
    bool save_patoh_hypergraph_structure(const std::string& filename,
                                         const Graph& graph) { 
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      std::ofstream fout(filename.c_str());
      if(!fout.good()) return false;

      // ok. I need to uniquely number each edge.
      // how?
      boost::unordered_map<std::pair<vertex_id_type, 
        vertex_id_type>, size_t> edgetoid;
      size_t curid = 0;
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i) {
        foreach(const typename Graph::edge_type& edge, graph.in_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          if (edgetoid.find(e) == edgetoid.end()) {
            edgetoid[e] = curid;
            ++curid;
          }
        }
        foreach(const typename Graph::edge_type& edge, graph.out_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          if (edgetoid.find(e) == edgetoid.end()) {
            edgetoid[e] = curid;
            ++curid;
          }
        }
      } 

      size_t numedges = curid;
      // each edge is a vertex, each vertex is an edge
      // a pin is total adjacency of a hyper edge
      fout << "0 " << numedges  << " " << graph.num_vertices() 
           << " " << numedges  * 2 << "\n";
      
      // loop over the "hyperedge" and write out the edges it is adjacent to
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i) {
        boost::unordered_set<size_t> adjedges;
        foreach(const typename Graph::edge_type& edge, graph.in_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          adjedges.insert(edgetoid[e]);
        }
        foreach(const typename Graph::edge_type& edge, graph.out_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          adjedges.insert(edgetoid[e]);
        }
        // write
        foreach(size_t adje, adjedges) {
          fout << adje << " ";
        }
        fout << "\n";
      }
      fout.close();
      return true;
    } // end of save_patoh_hypergraph_structure




    template<typename Graph>
    bool save_zoltan_hypergraph_structure(const std::string& filename,
                                          const Graph& graph) { 
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      std::ofstream fout(filename.c_str());
      if(!fout.good()) return false;

      // ok. I need to uniquely number each edge.
      // how?
      boost::unordered_map<std::pair<vertex_id_type, 
        vertex_id_type>, size_t> edgetoid;
      size_t curid = 0;
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i) {
        foreach(const typename Graph::edge_type& edge, graph.in_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          if (edgetoid.find(e) == edgetoid.end()) {
            edgetoid[e] = curid;
            ++curid;
          }
        }
        foreach(const typename Graph::edge_type& edge, graph.out_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          if (edgetoid.find(e) == edgetoid.end()) {
            edgetoid[e] = curid;
            ++curid;
          }
        }
      }

      size_t numedges = curid;
      // each edge is a vertex, each vertex is an edge
      // a pin is total adjacency of a hyper edge
      fout << numedges << "\n\n";
      for (size_t i = 0;i < numedges; ++i) {
        fout << i+1 << "\n";
      }
      fout << "\n";
      fout << graph.num_vertices() << "\n\n";
      
      fout << numedges * 2 << "\n\n";
      // loop over the "hyperedge" and write out the edges it is adjacent to
      for(vertex_id_type i = 0; i < graph.num_vertices(); ++i) {
        boost::unordered_set<size_t> adjedges;
        foreach(const typename Graph::edge_type& edge, graph.in_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          adjedges.insert(edgetoid[e]);
        }
        foreach(const typename Graph::edge_type& edge, graph.out_edges(i)) {
          std::pair<vertex_id_type, vertex_id_type> e = 
            std::make_pair(edge.source(), edge.target());
          if (e.first > e.second) std::swap(e.first, e.second);
          adjedges.insert(edgetoid[e]);
        }
        // write
        std::vector<size_t> adjedgesvec;
        std::copy(adjedges.begin(), adjedges.end(), 
                  std::inserter(adjedgesvec, adjedgesvec.end()));
        fout << i+1 << " " << adjedgesvec.size() << "\t";        
        for (size_t j = 0;j < adjedgesvec.size(); ++j) {
          fout << adjedgesvec[j] + 1;
          if (j < adjedgesvec.size() - 1) fout << "\t";
        }
        fout << "\n";
      }
      fout.close();
      return true;
    }  // end of save_zoltan_hypergraph_structure




  }; // end of graph ops
}; // end of namespace graphlab
#include <graphlab/macros_undef.hpp>

#endif





