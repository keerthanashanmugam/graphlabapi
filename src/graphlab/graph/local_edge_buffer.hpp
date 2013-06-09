#ifndef GRAPHLAB_LOCAL_EDGE_BUFFER
#define GRAPHLAB_LOCAL_EDGE_BUFFER

#include <vector>
#include <graphlab/graph/graph_basic_types.hpp>
#include <graphlab/logger/assertions.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace graphlab {    
  template<typename EdgeData, bool merge_duplicate=false>
      // Edge class for temporary storage. Will be finalized into the CSR+CSC form.
      class local_edge_buffer {
       public:
         local_edge_buffer() { }

         void reserve_edge_space(size_t n) {
           data.reserve(n);
           source_arr.reserve(n);
           target_arr.reserve(n);
         }

         // \brief Add an edge to the temporary storage.
         void add_edge(lvid_type source, lvid_type target, EdgeData _data) {
           data.push_back(_data);
           source_arr.push_back(source);
           target_arr.push_back(target);
         }

         // \brief Add edges in block to the temporary storage.
         void add_block_edges(const std::vector<lvid_type>& src_arr, 
                              const std::vector<lvid_type>& dst_arr, 
                              const std::vector<EdgeData>& edata_arr) {
           data.insert(data.end(), edata_arr.begin(), edata_arr.end());
           source_arr.insert(source_arr.end(), src_arr.begin(), src_arr.end());
           target_arr.insert(target_arr.end(), dst_arr.begin(), dst_arr.end());
         }

         // Merge the duplicate table with the edgedata array.
         void merge_duplicate_values() { };  

         // \brief Remove all contents in the storage. 
         void clear() {
           std::vector<EdgeData>().swap(data);
           std::vector<lvid_type>().swap(source_arr);
           std::vector<lvid_type>().swap(target_arr);
         }

         // \brief Return the size of the storage.
         size_t size() const {
           return source_arr.size();
         }

         // \brief Return the estimated memory footprint used.
         size_t estimate_sizeof() const {
           return data.capacity()*sizeof(EdgeData) + 
               source_arr.capacity()*sizeof(lvid_type)*2 + 
               sizeof(data) + sizeof(source_arr)*2 + 
               sizeof(local_edge_buffer);
         }

         std::vector<EdgeData> data;
         std::vector<lvid_type> source_arr;
         std::vector<lvid_type> target_arr;
      }; // end of class local_edge_buffer.

  template<typename EdgeData> 
      class local_edge_buffer<EdgeData, true> { 
       public:
         local_edge_buffer() { }

         void reserve_edge_space(size_t n) {
           data.reserve(n);
           source_arr.reserve(n);
           target_arr.reserve(n);
         }

         // \brief Remove all contents in the storage. 
         void clear() {
           std::vector<EdgeData>().swap(data);
           std::vector<lvid_type>().swap(source_arr);
           std::vector<lvid_type>().swap(target_arr);
         }

         void add_edge(lvid_type source, lvid_type target, EdgeData _data) { 
           std::pair<lvid_type, lvid_type> key(source,target);
           if (dup_check_set.find(key) == dup_check_set.end()) {
             data.push_back(_data);
             source_arr.push_back(source);
             target_arr.push_back(target);
             dup_check_set.insert(key);
           } else {
             if (dup_table.find(key) == dup_table.end()) {
               dup_table[key] = _data;
             } else {
               dup_table[key] += _data;
             }
           }
         }

         // \brief Add edges in block to the temporary storage.
         void add_block_edges(const std::vector<lvid_type>& src_arr, 
                              const std::vector<lvid_type>& dst_arr, 
                              const std::vector<EdgeData>& edata_arr) {
           for (size_t i = 0; i < src_arr.size(); ++i) {
             add_edge(src_arr[i], dst_arr[i], edata_arr[i]);
           }
         }

         void merge_duplicate_values() {
#ifdef _OPENMP
#pragma omp parallel for
#endif
           for (ssize_t i = 0; i < (ssize_t)source_arr.size(); ++i) {
             std::pair<lvid_type, lvid_type> key(source_arr[i], target_arr[i]);
             if (dup_table.find(key) != dup_table.end()) {
               data[i] += dup_table[key]; 
             }
           }
           dup_table.clear();
         }

         // \brief Return the size of the storage.
         size_t size() const {
           return source_arr.size();
         }

         std::vector<EdgeData> data;
         std::vector<lvid_type> source_arr;
         std::vector<lvid_type> target_arr;

       private:
         // hash table/set used for checking / merge duplicates
         boost::unordered_set<std::pair<lvid_type, lvid_type> > dup_check_set;
         boost::unordered_map<std::pair<lvid_type, lvid_type>, EdgeData > dup_table;
      };
} // end of namespace
#endif


