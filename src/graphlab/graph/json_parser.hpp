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
#ifndef GRAPHLAB_GRAPH_JSON_PARSER_HPP
#define GRAPHLAB_GRAPH_JSON_PARSER_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#if defined(__clang) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic push
#endif

#pragma GCC diagnostic ignored "-Wreorder"
#include <libjson/libjson.h>

#if defined(__clang) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic pop
#endif

#include <boost/functional.hpp>
#include <graphlab/util/stl_util.hpp>
#include <graphlab/util/fs_util.hpp>
#include <graphlab/util/hdfs.hpp>
#include <graphlab/logger/logger.hpp>
#include <graphlab/serialization/serialization_includes.hpp>
#include <graphlab/graph/distributed_graph.hpp>
#include <graphlab/graph/ingress/distributed_identity_ingress.hpp>

namespace graphlab {

  /** Builtin parsers for common datatypes: empty, string, int, float, double. */
  namespace builtin_parsers {
    template <typename T>
    bool empty_parser(T& e, const std::string& line) {
      return true;
    }

    bool string_parser(string& i, const std::string& line) {
      return true;
    }

    bool int_parser(int& i, const std::string& line) {
      i = boost::lexical_cast<int>(line);
      return true;
    }

    bool float_parser(float& i, const std::string& line) {
      i = boost::lexical_cast<float>(line);
      return true;
    }

    bool double_parser(float& i, const std::string& line) {
      i = boost::lexical_cast<double>(line);
      return true;
    }
  }


template<typename VertexData, typename EdgeData>
class distributed_graph;


template <typename VertexData, typename EdgeData>
class json_parser {

  public:
    typedef distributed_graph<VertexData, EdgeData> graph_type;
    typedef EdgeData edge_data_type;
    typedef VertexData vertex_data_type;

    typedef typename graph_type::vertex_id_type vertex_id_type;
    typedef typename graph_type::lvid_type lvid_type;
    typedef typename graph_type::vid2lvid_map_type vid2lvid_map_type;

    typedef boost::function<bool(edge_data_type&, const std::string&)> edge_parser_type;
    typedef boost::function<bool(vertex_data_type&, const std::string&)> vertex_parser_type;
    typedef boost::function<bool(const std::string&)> line_parser_type;


  public:
  json_parser (graph_type& graph, const std::string& prefix, bool gzip=false,
      edge_parser_type edge_parser=builtin_parsers::empty_parser<EdgeData>,
      vertex_parser_type vertex_parser=builtin_parsers::empty_parser<VertexData>) :
    graph(graph), prefix(prefix), gzip(gzip), edge_parser(edge_parser), vertex_parser(vertex_parser) {}

  const std::string basepath() {
    return prefix;
  }

  const std::string vrecordpath() {
    return fs_util::concat_path(basepath(), "partitionedvrec/partition" + graph.procid());
  }

  const std::string edgepath() {
    return fs_util::concat_path(basepath(), "partitionededge/partition" + graph.procid());
  }


  /** Main utility function. 
   * Run the given line_parser independently on each line of the file. 
   * */
  bool parse_by_line (const std::string& path, line_parser_type line_parser) {
    logstream(LOG_INFO) << "Load " << path << std::endl;

    boost::iostreams::filtering_stream<boost::iostreams::input> fin;
    // loading from hdfs
    if (boost::starts_with(prefix, "hdfs://")) {
      graphlab::hdfs& hdfs = hdfs::get_hdfs();
      graphlab::hdfs::fstream in_file(hdfs, path);
      if (!in_file.good()) {
        logstream(LOG_FATAL) << "Fail to open file " << path << std::endl;
        return false;
      }

      if (gzip) fin.push(boost::iostreams::gzip_decompressor());
      fin.push(in_file);

      if (!fin.good()) {
        logstream(LOG_FATAL) << "Fail to read from stream " << path << std::endl;
        return false;
      }

      load_from_stream(path, fin, line_parser);

      if (gzip) fin.pop();
      fin.pop();
    } else { // loading from disk
      std::ifstream in_file(path.c_str(),
          std::ios_base::in | std::ios_base::binary);

      if (!in_file.good()) {
        logstream(LOG_FATAL) << "Fail to open file " << path << std::endl;
        return false;
      }

      if (gzip) fin.push(boost::iostreams::gzip_decompressor());
      fin.push(in_file);

      if (!fin.good()) {
        logstream(LOG_FATAL) << "Fail to read from stream " << path << std::endl;
        return false;
      }

      load_from_stream(path, fin, line_parser);

      if (gzip) fin.pop();
      fin.pop();
    }

    return true;
  }

  /** 
   * Load the graph by loading the current partition.
   * The function first calls load_vrecord_list to load the all vertices that belong to this partition. 
   * Next, it loads each subpartition of edges which include a list of source target pair and a json array of edge data. 
   * */
  bool load() {
    bool success = load_vrecords(vrecordpath());

    // Scan the directory conatining subpartitions.
    std::vector<std::string> subpart_dirs;
    list_files_with_prefix(edgepath(), "subpart", subpart_dirs);
    foreach(std::string dirname, subpart_dirs) {
      // Load each sub partition.
      success &= load_subpartition(fs_util::concat_path(edgepath(), dirname)); 
    }

    if (!success) {
      logstream(LOG_FATAL) << "Fail parsing graph json" << std::endl;
      return false;
    }

    graph.local_graph.finalize();

    ASSERT_GE(graph.local_graph.num_vertices(), graph.local_graph.gstore.num_vertices);
    ASSERT_EQ(graph.vid2lvid.size(), graph.local_graph.num_vertices());
    ASSERT_EQ(graph.lvid2record.size(), graph.local_graph.num_vertices());

    logstream(LOG_INFO) << "Finished loading graph" << graph.procid() 
      << "\n\tnverts: " << graph.num_local_own_vertices() 
      << "\n\tnreplicas: " << graph.local_graph.num_vertices() 
      << "\n\tnedges: " << graph.local_graph.num_edges() 
      << std::endl;

    // Exchange local stats and construct global info.
    if (graph.ingress_ptr == NULL) {
      graph.ingress_ptr = new distributed_identity_ingress<VertexData, EdgeData>(graph.rpc.dc(), graph);
    }
    graph.ingress_ptr->exchange_global_info();
    delete graph.ingress_ptr;

    graph.finalized = true;
    return success;
  }

  /** Load the vrecords for this partition.   
   * The vrecord file contains a sequence of files, each having
   * a list of vertex records.
   * */
  bool load_vrecords(const std::string& path) {
     bool success = true;

     // load meta files
     size_t num_vertices = 0;
     size_t num_own_vertices = 0;
     success = parse_by_line(fs_util::concat_path(path, "meta"), boost::bind(parse_vrecord_meta, num_vertices, num_own_vertices, _1));
     if (success) {
       graph.vid2lvid.reserve(num_vertices);
       graph.lvid2record.reserve(num_vertices);
       graph.lvid2record.resize(num_vertices);
     } else {
       return false;
     }

     // load vrecord files
    std::vector<std::string> vrecfiles;
    list_files_with_prefix(path, "vrecord", vrecfiles);
    line_parser_type vrecord_parser = boost::bind(parse_vrecord, graph, _1, vertex_parser);
    foreach(std::string file, vrecfiles) {
      success &= parse_by_line(file, vrecord_parser); 
    }

    ASSERT_EQ(graph.local_own_vertices, num_own_vertices);
    return success;
  }



  /* Load the subpartition at given path. 
   * The subpartition consists of two parts: edge data files
   * and edge list files.
   * Example of subpartition files:
   * subpart0/meta:
   *    {numEdges:2000}
   * subpart0/elist:
   *    {source:12, targets:{1,2,3,4,5}}
   *    {source:13, targets:{1,2,3,4,5}}
   *    ...
   * subpart0/edata:
   *    {data0, data2, ... data24}
   *    {data25, data26, ... data50}
   *    ...
  */
  bool load_subpart(const std::string& path) {
    bool success = true;

    size_t numEdges = 0;
    // Load meta file.
    const std::string metapath = fs_util::concat_path(path, "meta");

    // Load edge data.
    const std::string edatapath = fs_util::concat_path(path, "edata");
    success &= parse_by_line(edatapath, boost::bind(parse_edata, edata_arr, _1, edge_parser));

    if (!success)  return false;

    std::vector<lvid_type> source_arr;
    std::vector<lvid_type> target_arr;
    std::vector<lvid_type> edata_arr;
    source_arr.reserve(numEdges);
    target_arr.reserve(numEdges);
    edata_arr.reserve(numEdges);

    // Load edge list.
    const std::string edgelistpath = fs_util::concat_path(path, "elist");
    success &= parse_by_line(edgelistpath, 
        boost::bind(parse_edge_list, graph.vid2lvid, source_arr, target_arr, _1)); 

    // Atomically modify the graph.
    graph_lock.lock();
      size_t oldsize = local_graph.num_edges();
      local_graph.reserve_edge_space(oldsize + numEdges);
      local_graph.add_edges(source_arr, target_arr, edata_arr);
    graph_lock.unlock();

    return success;
  }


  /* Parse the vertex record meta file. 
   * Read in number of vertices and number of local own vertices
   * of this partition.
   * */
  static bool parse_vrecord_meta (size_t& num_vertices, 
      size_t& num_own_vertices, const std::string& str) {
    bool success = true;
    JSONNode n = libjson::parse(str);
    JSONNode::const_iterator i = n.begin();
    while (i != n.end()) {
      if (i->name() == "numVertices") {
        num_vertices = i->as_int();
      } else if (i->name() == "numOwnVertices") {
        num_own_vertices = i->as_int();
      } else {
        logstream(LOG_WARNING) 
          << "Unknown field of vrecord meta: " 
          << i->name();
        success = false;
      }
    }
    return success;
  }


  /* Parse the vertex record list from json.
   * Each parsed vertex record creates a vid2lvid map entry, 
   * and is inserted into graph.vrecordlist, graph.local_graph.vertices.
   * Assumption: require graph.lvid2record is preallocated.
   */
  static bool parse_vrecord (graph_type& graph, const std::string& str,
      vertex_parser_type vertex_parser) {

    typedef typename graph_type::local_graph_type local_graph_type;
    local_graph_type& local_graph = graph.get_local_graph();

    JSONNode n = libjson::parse(str);
    JSONNode::const_iterator i = n.begin();

    vertex_data_type vdata;
    typename graph_type::vertex_record vrecord;

    while (i != n.end()) {
      if (i->name() == "mirrors") {
        JSONNode::const_iterator j = (*i).begin();
        while (j != (*i).end()) {
          int mirror = j->as_int();
          vrecord._mirrors.set_bit((procid_t)mirror);
          ++j;
        }
      } else if (i->name() == "inEdges") {
        vrecord.num_in_edges = i->as_int();
      } else if (i->name() == "outEdges") {
        vrecord.num_out_edges = i->as_int();
      } else if (i->name() == "gvid") {
        // Check unsafe
        vrecord.gvid = boost::lexical_cast<vertex_id_type>(i->as_int());
      } else if (i->name() == "owner") {
        vrecord.owner = (procid_t)i->as_int();
      } else if (i->name() == "vdata") {
        if (!(i->type() == JSON_NULL))
          vertex_parser(vdata, i->as_string());
      } else {
        logstream(LOG_ERROR) << "Error parsing json into vrecord. Unknown json node name:" <<
          i->name() << std::endl;
      }
      ++i;
    }

    if (!graph.vid2lvid.find(vrecord.gvid) == graph.vid2lvid.end()) {
      logstream(LOG_WARNING) << "Duplicate vertex record : gvid = " << vrecord.gvid << ". Ignored" << std::endl;
    } else {
      lvid_type lvid = vid2lvid.size();
      graph.vid2lvid[vrecord.gvid] = lvid;
      graph.lvid2record[lvid] = vrecord;
      local_graph.add_vertex(lvid, vdata);
      if (vrecord.owner == graph.procid()) ++graph.local_own_nverts;
    }

    return true;
  }

  /* Parse the subpartition meta file. 
   * Read in number of edges of the sub partition.
   * */
  static bool parse_subpart_meta (size_t& num_edges, const std::string& str) {
    bool success = true;
    JSONNode n = libjson::parse(str);
    JSONNode::const_iterator i = n.begin();
    while (i != n.end()) {
      if (i->name() == "numEdges") {
        num_edges = i->as_int();
      } else {
        logstream(LOG_WARNING) 
          << "Unknown field of edge meta: " 
          << i->name();
        success = false;
      }
    }
    return success;
  }

  /* Parse the graph structure, in this case the edge adjacency list. 
   * The vertices of each parsed edge will be translated into local vid, 
   * and inserted into source_arr, and target_arr. 
   * Assumption: require vid2lvid map. 
   * */
  static bool parse_edge_list(const vid2lvid_map_type& map, 
      std::vector<lvid_type>& source_arr, 
      std::vector<lvid_type>& target_arr,
      const std::string& str) {
    JSONNode n = libjson::parse(str);
    JSONNode::const_iterator i = n.begin();

    lvid_type source(-1);
    lvid_type target(-1);

    while(i != n.end()) {
      if (i->name() == "source") {
        if (map.find(i->as_int()) == map.end()) {
          logstream(LOG_ERROR) 
            << "Unknown vid from adjcency list: gvid = " 
            << i->as_int() << std::endl;
          return false;
        } else {
          source = map[i->as_int()];
        }
      } else if (i->name() == "targets") {
        if (i->type() != JSON_ARRAY) {
          logstream(LOG_ERROR)
            << "Type of targets is not JSON_ARRAY" << std::endl;
          return false;
        } else {
          JSONNode::const_iterator iter = i->begin();
          while (iter != i->end()) {
            if (map.find(iter->as_int()) == map.end()) {
              logstream(LOG_ERROR) 
                << "Unknown vid from adjcency list: gvid = " 
                << iter->as_int() << std::endl;
              return false;
            } else {
              source_arr.push_back(source);
              target_arr.push_back(map[iter->as_int()]);
            }
          }
        }
      } 
      ++i;
    } // end while
   return true;
  }

  /* Parse the edata list from json */
  static bool parse_edata(std::vector<EdgeData>& edata_arr, const std::string& str, edge_parser_type edge_parser) {
    JSONNode n = libjson::parse(str);
    JSONNode::const_iterator i = n.begin();
    while(i != n.end()) {
       if (i->type() == JSON_ARRAY) {
        // parse  edatalist -> graph.local_graph.gstore.edata
        JSONNode::const_iterator j = i->begin();
        edge_data_type e;
        while (j != i->end()) {
          edge_parser(e, j->as_string());
          edata_arr.push_back(e);
          ++j;
        }
       } else {
         logstream(LOG_ERROR) 
            << "Error in parsing edata. Unknown field name : "
            << i->name() << std::endl;
         return false;
       }
      ++i;
    }
    return true;
  }


  /*  Helper function starts here  */
  private:
  /**
    \internal
    This internal function is used to load a single line from an input stream
    */
  template<typename Fstream>
    bool load_from_stream(std::string filename, Fstream& fin,
        line_parser_type& line_parser) {
      size_t linecount = 0;
      timer ti; ti.start();
      while(fin.good() && !fin.eof()) {
        std::string line;
        std::getline(fin, line);
        if(line.empty()) continue;
        if(fin.fail()) break;
        const bool success = line_parser(line);
        if (!success) {
          logstream(LOG_WARNING)
            << "Error parsing line " << linecount << " in "
            << filename << ": " << std::endl
            << "\t\"" << line << "\"" << std::endl;
          return false;
        }
        ++linecount;
        if (ti.current_time() > 5.0) {
          logstream(LOG_INFO) << linecount << " Lines read" << std::endl;
          ti.start();
        }
      }
      return true;
    } // end of load from stream


  private:
    graph_type& graph;
    std::string prefix;
    bool gzip;
    edge_parser_type edge_parser;
    vertex_parser_type vertex_parser;
}; // json_parser


} // namespace graphlab
#endif
