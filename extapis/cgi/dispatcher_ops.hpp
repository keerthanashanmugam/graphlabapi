#ifndef GRAPHLAB_DISPATCHER_OPS_HPP
#define GRAPHLAB_DISPATCHER_OPS_HPP

#include <iostream>
#include <fstream>
#include <string>

namespace graphlab {
  
  namespace dispatcher_ops {
  
    template<typename Graph>
    bool load_structure(std::istream& fin, Graph& graph) {
    
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      
      if(!fin.good()) return false;
      size_t self_edges = 0;
      
      // loop through file reading each line
      while(fin.good() && !fin.eof()) {
      
        switch (fin.peek()){
          case '#': { // skip comments
            std::string str; std::getline(fin, str);
            continue;
          }
          case ':': { // read vertex states
            
            fin.seekg(1, std::istream::cur);
            vertex_id_type vid = 0;
            cgi_vertex vertex;
            fin >> vid >> vertex.state;
            
            if (!fin.good()) return false;
            graph.add_vertex(vid, vertex);
            std::string str; std::getline(fin, str);
            
            continue;
            
          }
          default: { // structure and edge states
            
            vertex_id_type source = 0, target = 0;
            cgi_edge edge;
            fin >> source >> target >> edge.state;
            
            if (!fin.good()) return false;
            
            // ensure that the number of vertices is correct
            if(source != target) graph.add_edge(source, target, edge);
            else if(self_edges++ == 0) 
              logstream(LOG_WARNING) 
                << "Self edge encountered but not supported!" << std::endl
                << "\t Further warnings will be suppressed." << std::endl;
                
            std::string str; std::getline(fin, str);
            
            continue;
                
          }
        }
            
      }            
      
      logstream(LOG_INFO) 
        << "Finished loading graph with: " << std::endl
        << "\t Vertices: " << graph.num_vertices() << std::endl
        << "\t Edges: " << graph.num_edges() << std::endl;        
        
      if(self_edges > 0) 
        logstream(LOG_INFO) << "\t Dropped self edges: " << self_edges 
                            << std::endl;  
                            
      return true;
      
    }
    
    template<typename Graph>
    bool save_structure(std::ostream& fout, Graph& graph) {
      
      typedef typename Graph::vertex_id_type     vertex_id_type;
      typedef typename Graph::vertex_color_type  vertex_color_type;
      typedef typename Graph::edge_type          edge_type;
      typedef typename Graph::edge_list_type     edge_list_type;
      
      // TODO: this doesn't work with the new ID assumptions
      for(vertex_id_type vid = 0; vid < graph.num_vertices(); ++vid) {
        fout << ":" << vid << " " << graph.vertex_data(vid).state << std::endl;
      }
      
      return fout.good();
      
    }
    
    template<typename Graph>
    bool load_structure(const std::string& fname, Graph& graph){
      std::ifstream fin(fname.c_str(), std::ifstream::in);
      bool result = load_structure(fin, graph);
      fin.close();
      return result;
    }
    
    
    
    template<typename Graph>
    bool save_structure(const std::string& fname, Graph& graph){
      std::ofstream fout(fname.c_str(), std::ofstream::out);
      bool result = save_structure(fout, graph);
      fout.close();
      return result;
    }
    
  }
  
}

#endif
