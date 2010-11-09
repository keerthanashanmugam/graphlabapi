#include "jt_splash.hpp"



void jt_worker::init(size_t wid,
                     scope_factory_type& sf, 
                     const factorized_model::factor_map_t& factors,
                     const std::vector<vertex_id_t>& root_perm, 
                     size_t ncpus,         
                     size_t treesize,
                     size_t treewidth,
                     size_t factorsize,
                     size_t max_height,
                     size_t internal_threads,
                     bool priorities) {
  // Initialize parameters
  scope_factory = &sf;
  worker_id = wid;
  worker_count = ncpus;
  max_tree_size = treesize;
  max_tree_width = treewidth;
  max_tree_height = max_height;
  
  
  assigned_factors.resize(factors.size(), false);
  
  if(factorsize <= 0) {
    max_factor_size = std::numeric_limits<size_t>::max();
  } else {
    max_factor_size = factorsize;
  }
  factors_ptr = &factors;
  use_priorities = priorities;
  
  roots = &root_perm;    
  root_index = root_perm.size();
  current_root = worker_id;
  
  // Initialize local jtcore
  if(internal_threads > 1) {
    jt_core.set_scheduler_type("multiqueue_fifo");
    jt_core.set_scope_type("edge");
    jt_core.set_ncpus(internal_threads);
    jt_core.set_engine_type("async");
  } else {
    jt_core.set_scheduler_type("fifo");
    jt_core.set_scope_type("none");
    jt_core.set_ncpus(1);
    jt_core.set_engine_type("async_sim");
  }
  
  
  // Initialize the shared data in the factorized model
  jt_core.shared_data().set_constant(junction_tree::FACTOR_KEY, 
                                     factors_ptr);
  mrf::graph_type* mrf_graph_ptr = &scope_factory->get_graph();
  jt_core.shared_data().set_constant(junction_tree::MRF_KEY, 
                                     mrf_graph_ptr); 
}













// get a root
void jt_worker::run() {   
  // looup until runtime is reached
  while(graphlab::lowres_time_seconds() < finish_time_seconds) {
    /////////////////////////////////////////////////////////
    // Construct one tree (we must succeed in order to count a tree
    size_t sampled_variables = 0;
    //      move_to_next_root();
    while(sampled_variables == 0 && 
          graphlab::lowres_time_seconds() < finish_time_seconds) {
      move_to_next_root();
      sampled_variables = sample_once();
      if(sampled_variables == 0) {
        collisions++;
        //  sched_yield();
      }
    }


    //       // Get a local copy of the graph
    //       mrf::graph_type& mrf(scope_factory->get_graph());
    //       if(worker_id == 0) {
    //         std::cout << "Saving sample: " << std::endl;
    //         size_t rows = std::sqrt(mrf.num_vertices());
    //         image img(rows, rows);
    //         for(vertex_id_t vid = 0; vid < mrf.num_vertices(); ++vid) {   
    //           img.pixel(vid) = mrf.vertex_data(vid).asg.asg_at(0);
    //         }
    //         img.pixel(0) = 0;
    //         img.pixel(1) = mrf.vertex_data(0).variable.arity -1;
    //         img.save(make_filename("sample", ".pgm", tree_count).c_str());
    //       }

    tree_count++;
    total_samples += sampled_variables;

  } 
}










/**
 * Grab this vertex into the tree owned by worker id
 */
bool jt_worker::quick_try_vertex(vertex_id_t vid) {
  const mrf::graph_type& mrf = scope_factory->get_graph();
  const mrf::vertex_data& vdata = mrf.vertex_data(vid);
  // Check that this vertex is not already in a tree
  bool in_tree = vdata.tree_id != NULL_VID;
  if(in_tree) return false;
  // check that the neighbors are not in any other trees than this
  // one
  const graphlab::edge_list& in_eids = mrf.in_edge_ids(vid);
  foreach(edge_id_t in_eid, in_eids) {
    vertex_id_t neighbor_vid = mrf.source(in_eid);
    const mrf::vertex_data& vdata = mrf.vertex_data(neighbor_vid);
    bool in_tree = vdata.tree_id != NULL_VID;
    // if the neighbor is in a tree other than this one quit
    if(in_tree && worker_id != vdata.tree_id) return false;
  }
  return true;
} // end of try grab vertex















/**
 * Grab this vertex into the tree owned by worker id
 */
bool jt_worker::try_grab_vertex(iscope_type& scope) {
  // Check that this vertex is not already in a tree
  bool in_tree = scope.vertex_data().tree_id != NULL_VID;
  if(in_tree) return false;

  // check that the neighbors are not in any other trees than this
  // one
  foreach(edge_id_t in_eid, scope.in_edge_ids()) {
    vertex_id_t neighbor_vid = scope.source(in_eid);
    const mrf::vertex_data& vdata = 
      scope.const_neighbor_vertex_data(neighbor_vid);
    bool in_tree = vdata.tree_id != NULL_VID;
    // if the neighbor is in a tree other than this one quit
    if(in_tree && worker_id != vdata.tree_id) return false;
  }
  // Assert that this vertex is not in a tree and that none of the
  // neighbors are in other trees
  // This vertex does not neighbor any other trees than this one
  scope.vertex_data().tree_id = worker_id;
  return true;
} // end of try grab vertex












/**
 * Release the vertex
 */
void jt_worker::release_vertex(iscope_type& scope) {
  // This vertex does not neighbor any other trees than this one
  scope.vertex_data().tree_id = NULL_VID;
} // release the vertex












double jt_worker::score_vertex(vertex_id_t vid) {

  /// ONly data set where this works: ./alchemy_image_denoise
  /// --rows=200 --rings=7 --sigma=1 --lambda=3 --smoothing=square


  mrf::graph_type& mrf = scope_factory->get_graph();
  mrf::vertex_data& vdata = mrf.vertex_data(vid);

  //    return graphlab::random::rand01();

  //    return score_vertex_log_odds(vid); 
  // return score_vertex_lik(vid);
  if (vdata.updates < 100 || vdata.priority < 0) {
    vdata.priority = score_vertex_log_odds(vid); 
  }
  return vdata.priority;

  // +
  //    return double(vdata.changes + graphlab::random::rand01()) 
  //      / sqrt(double(vdata.updates + 1));

  // if(vdata.updates < 100) {


  //    return score_vertex_lik(vid); // +
  // }
  //	graphlab::random::rand01();
  // return score_vertex_l1_diff(vid);
  //}
  //    return (1.0 + graphlab::random::rand01() + score) / (vdata.updates + 1); 
  //return graphlab::random::rand01();;
}


















double jt_worker::score_vertex_l1_diff(vertex_id_t vid) {
  // Get the scope factory
  const mrf::graph_type& mrf = scope_factory->get_graph();
  const mrf::vertex_data& vdata = mrf.vertex_data(vid);

  // Construct the domain of neighbors that are already in the tree
  domain_t vars = vdata.variable;
  foreach(edge_id_t ineid, mrf.in_edge_ids(vid)) {
    const vertex_id_t neighbor_vid = mrf.source(ineid);
    const mrf::vertex_data& neighbor = mrf.vertex_data(neighbor_vid);
    // test to see if the neighbor is in the tree by checking the
    // elimination time map
    if(elim_time_map.find(neighbor_vid) != elim_time_map.end()) {
      // otherwise add the tree variable
      vars += neighbor.variable;
      // If this vertex has too many tree neighbor than the priority
      // is set to -1;
      if(vars.num_vars() > max_tree_width) return -1;
      if(vars.size() > max_factor_size) return -1;
    } 
  }

  // Compute the clique factor
  clique_factor.set_args(vars);
  clique_factor.uniform();
  // get all the factors
  const factorized_model::factor_map_t& factors(*factors_ptr);
  // Iterate over the factors and multiply each into this factor
  foreach(size_t factor_id, vdata.factor_ids) {
    const factor_t& factor = factors[factor_id];      
    // Build up an assignment for the conditional
    domain_t conditional_args = factor.args() - vars;
    if(conditional_args.num_vars() > 0) {
      assignment_t conditional_asg;
      for(size_t i = 0; i < conditional_args.num_vars(); ++i) {
        const mrf::vertex_data& neighbor_vdata = 
          mrf.vertex_data(conditional_args.var(i).id);
        conditional_asg &= 
          assignment_t(neighbor_vdata.variable, neighbor_vdata.asg);
      }
      // set the factor arguments
      conditional_factor.set_args(factor.args() - conditional_args);
      conditional_factor.condition(factor, conditional_asg);        
      // Multiply the conditional factor in
      clique_factor *= conditional_factor;
      //       clique_factor.normalize();
    } else {
      clique_factor *= factor;
    }
  } // end of loop over factors
  clique_factor.normalize();


  // Compute the product of marginals
  product_of_marginals_factor.set_args(vars);
  product_of_marginals_factor.uniform();
  for(size_t i = 0; i < vars.num_vars(); ++i) {
    marginal_factor.set_args(vars.var(i));
    marginal_factor.marginalize(clique_factor);
    marginal_factor.normalize();
    product_of_marginals_factor *= marginal_factor;
  }
  product_of_marginals_factor.normalize();

  // Compute the residual
  double residual = clique_factor.l1_diff(product_of_marginals_factor);


  assert( residual >= 0);
  assert( !std::isnan(residual) );
  assert( std::isfinite(residual) );

  // ensure score is bounded
  //    residual = std::tanh(residual);

  return residual;

} // end of score l1 diff












double jt_worker::score_vertex_log_odds(vertex_id_t vid) {
  // Get the scope factory
  const mrf::graph_type& mrf = scope_factory->get_graph();
  const mrf::vertex_data& vdata = mrf.vertex_data(vid);

  // Construct the domain of neighbors that are already in the tree
  domain_t vars = vdata.variable;
  foreach(edge_id_t ineid, mrf.in_edge_ids(vid)) {
    const vertex_id_t neighbor_vid = mrf.source(ineid);
    const mrf::vertex_data& neighbor = mrf.vertex_data(neighbor_vid);
    // test to see if the neighbor is in the tree by checking the
    // elimination time map
    if(elim_time_map.find(neighbor_vid) != elim_time_map.end()) {
      // otherwise add the tree variable
      vars += neighbor.variable;
      // If this vertex has too many tree neighbor than the priority
      // is set to 0;
      if(vars.num_vars() > max_tree_width) return -1;
      if(vars.size() > max_factor_size) return -1;
    } 
  }
    
  assert(vars.num_vars() == 2);


  // Compute the clique factor
  clique_factor.set_args(vars);
  clique_factor.uniform();
  // get all the factors
  const factorized_model::factor_map_t& factors(*factors_ptr);
  // Iterate over the factors and multiply each into this factor
  foreach(size_t factor_id, vdata.factor_ids) {
    const factor_t& factor = factors[factor_id];      
    // Build up an assignment for the conditional
    domain_t conditional_args = factor.args() - vars;
    if(conditional_args.num_vars() > 0) {
      assignment_t conditional_asg;
      for(size_t i = 0; i < conditional_args.num_vars(); ++i) {
        const mrf::vertex_data& neighbor_vdata = 
          mrf.vertex_data(conditional_args.var(i).id);
        conditional_asg &= 
          assignment_t(neighbor_vdata.variable, neighbor_vdata.asg);
      }
      // set the factor arguments
      conditional_factor.set_args(factor.args() - conditional_args);
      conditional_factor.condition(factor, conditional_asg);        
      // Multiply the conditional factor in
      clique_factor *= conditional_factor;
      //        clique_factor.normalize();
    } else {
      clique_factor *= factor;
    }
  } // end of loop over factors
    // Compute the conditional factor and marginal factors
  conditional_factor.set_args(vars - vdata.variable);
  conditional_factor.condition(clique_factor, 
                               assignment_t(vdata.variable, vdata.asg));  
  marginal_factor.set_args(vars - vdata.variable);
  marginal_factor.marginalize(clique_factor);
    
  // Compute metric
  conditional_factor.normalize();
  marginal_factor.normalize();
  // double residual = conditional_factor.l1_logdiff(marginal_factor);
  double residual = conditional_factor.l1_diff(marginal_factor);

  // rescale by updates
  //    residual = residual / (vdata.updates + 1);

  assert( residual >= 0);
  assert( !std::isnan(residual) );
  assert( std::isfinite(residual) );

  // ensure score is bounded
  //    residual = std::tanh(residual);

  return residual;
} // end of score vertex























double jt_worker::score_vertex_lik(vertex_id_t vid) {
  // Get the scope factory
  const mrf::graph_type& mrf = scope_factory->get_graph();
  const mrf::vertex_data& vdata = mrf.vertex_data(vid);

  // Construct the domain of neighbors that are already in the tree
  domain_t vars = vdata.variable;
  foreach(edge_id_t ineid, mrf.in_edge_ids(vid)) {
    const vertex_id_t neighbor_vid = mrf.source(ineid);
    const mrf::vertex_data& neighbor = mrf.vertex_data(neighbor_vid);
    // test to see if the neighbor is in the tree by checking the
    // elimination time map
    if(elim_time_map.find(neighbor_vid) != elim_time_map.end()) {
      // otherwise add the tree variable
      vars += neighbor.variable;
      // If this vertex has too many tree neighbor than the priority
      // is set to 0;
      if(vars.num_vars() > max_tree_width) return -1;
      if(vars.size() > max_factor_size) return -1;
    } 
  }
    
  // Compute the clique factor
  clique_factor.set_args(vars);
  clique_factor.uniform();
  // get all the factors
  const factorized_model::factor_map_t& factors(*factors_ptr);
  // Iterate over the factors and multiply each into this factor
  foreach(size_t factor_id, vdata.factor_ids) {
    const factor_t& factor = factors[factor_id];      
    // Build up an assignment for the conditional
    domain_t conditional_args = factor.args() - vars;
    if(conditional_args.num_vars() > 0) {
      assignment_t conditional_asg;
      for(size_t i = 0; i < conditional_args.num_vars(); ++i) {
        const mrf::vertex_data& neighbor_vdata = 
          mrf.vertex_data(conditional_args.var(i).id);
        conditional_asg &= 
          assignment_t(neighbor_vdata.variable, neighbor_vdata.asg);
      }
      // set the factor arguments
      conditional_factor.set_args(factor.args() - conditional_args);
      conditional_factor.condition(factor, conditional_asg);        
      // Multiply the conditional factor in
      clique_factor *= conditional_factor;
      //        clique_factor.normalize();
    } else {
      clique_factor *= factor;
    }
  } // end of loop over factors

    // Compute the conditional factor and marginal factors
  marginal_factor.set_args(vdata.variable);
  marginal_factor.marginalize(clique_factor);
  marginal_factor.normalize();
  double residual =  1.0 - exp(marginal_factor.logP(vdata.asg));

  assert( residual >= 0);
  assert( !std::isnan(residual) );
  assert( std::isfinite(residual) );

  // // ensure score is bounded
  // residual = std::tanh(residual);


  return residual;
} // end of max lik


















  void grow_bfs_jtree() {
    assert(scope_factory != NULL);
    // Get the scope factory
    mrf::graph_type& mrf = scope_factory->get_graph();
    // Clear local data structures
    elim_time_map.clear();
    cliques.clear();
    bfs_queue.clear();
    visited.clear();
     
    // add the root
    bfs_queue.push_back(current_root);
    visited.insert(current_root);

    while(!bfs_queue.empty()) {
      // Take the top element
      const vertex_id_t next_vertex = bfs_queue.front();
      bfs_queue.pop_front();

      // pretest that the vertex is available before trying to get it
      bool grabbed = quick_try_vertex(next_vertex);
      if(!grabbed) continue;

      // Maybe we can get the vertex so actually try to get it
      iscope_type* scope_ptr = 
        scope_factory->get_edge_scope(worker_id, next_vertex);
      assert(scope_ptr != NULL);
      iscope_type& scope(*scope_ptr);

      // See if we can get the vertex for this tree
      grabbed = try_grab_vertex(scope);

      // If we failed to grab the scope then skip this vertex
      if(grabbed) {

        // if this is the root vertex
        vertex_id_t min_height = 0;
        if(max_tree_height != 0 && !cliques.empty()) {
          min_height = max_tree_height;
          // find the closest vertex to the root
          foreach(edge_id_t eid, mrf.out_edge_ids(next_vertex)) {
            vertex_id_t neighbor_vid = mrf.target(eid);
            // if the neighbor is already in the tree
            if(elim_time_map.find(neighbor_vid) != elim_time_map.end()) {
              min_height = 
                std::min(min_height, mrf.vertex_data(neighbor_vid).height + 1);
            } 
          }
        } 
        bool safe_extension = 
          (max_tree_height == 0) ||
          (min_height < max_tree_height);

        // test that its safe  
        safe_extension = safe_extension && 
          extend_clique_list(mrf,
                             next_vertex,
                             elim_time_map,
                             cliques,
                             max_tree_width,
                             max_factor_size);

        if(safe_extension) {   
          // set the height
          mrf.vertex_data(next_vertex).height = min_height;

          // add the neighbors to the search queue
          foreach(edge_id_t eid, mrf.out_edge_ids(next_vertex)) {
            vertex_id_t neighbor_vid = mrf.target(eid);
            if(visited.count(neighbor_vid) == 0) {
              bfs_queue.push_back(neighbor_vid);
              visited.insert(neighbor_vid);
            }
          }
        } else {
          // release the vertex since it could not be used in the tree
          release_vertex(scope);
        }
      } // end of grabbed
      // release the scope
      scope_factory->release_scope(&scope);        
      // Limit the number of variables
      if(cliques.size() > max_tree_size) break;
    } // end of while loop
  } // end grow_bfs_jtree



  void grow_prioritized_jtree() {
    assert(scope_factory != NULL);
    // Get the scope factory
    mrf::graph_type& mrf = scope_factory->get_graph();
    // Clear local data structures
    elim_time_map.clear();
    cliques.clear();
    priority_queue.clear();
    visited.clear();
     
    // add the root
    priority_queue.push(current_root, 1.0);
    visited.insert(current_root);

    while(!priority_queue.empty()) {
      // Take the top element
      const vertex_id_t next_vertex = priority_queue.pop().first;

      // pretest that the vertex is available before trying to get it
      bool grabbed = quick_try_vertex(next_vertex);
      if(!grabbed) continue;


      // Get the scope
      bool released_scope = false;
      iscope_type* scope_ptr = 
        scope_factory->get_edge_scope(worker_id, next_vertex);
      assert(scope_ptr != NULL);
      iscope_type& scope(*scope_ptr);

      // See if we can get the vertex for this tree
      grabbed = try_grab_vertex(scope);

      // If we failed to grab the scope then skip this vertex
      if(grabbed) {
        // compute the tree height of the new vertex
        vertex_id_t min_height = 0;        
        // if this is not the root and we care about tree height
        if(max_tree_height != 0 && !cliques.empty()) {
          min_height = max_tree_height;
          // find the closest vertex to the root
          foreach(edge_id_t eid, mrf.out_edge_ids(next_vertex)) {
            vertex_id_t neighbor_vid = mrf.target(eid);
            // if the neighbor is already in the tree
            if(elim_time_map.find(neighbor_vid) != elim_time_map.end()) {
              min_height = 
                std::min(min_height, 
                         mrf.vertex_data(neighbor_vid).height + 1);
            } 
          }
        } // end of tree height check for non root vertex 
          
        // test the 
        bool safe_extension =
          (max_tree_height == 0) ||
          (min_height < max_tree_height);

        safe_extension = safe_extension &&
          extend_clique_list(mrf,
                             next_vertex,
                             elim_time_map,
                             cliques,
                             max_tree_width,
                             max_factor_size);

        // If the extension was safe than the elim_time_map and
        // cliques data structure are automatically extended
        if(safe_extension) {
          // // set the height
          mrf.vertex_data(next_vertex).height = min_height;

	  // release the scope early to allow other processors to move
	  // in
	  released_scope = true;
	  scope_factory->release_scope(&scope);        

          // add the neighbors to the search queue or update their priority
          foreach(edge_id_t eid, mrf.out_edge_ids(next_vertex)) {
            vertex_id_t neighbor_vid = mrf.target(eid);          
            if(visited.count(neighbor_vid) == 0) {
              // Vertex has not yet been visited
              double score = score_vertex(neighbor_vid);
              // if the score is greater than zero then add the
              // neighbor to the priority queue.  The score is zero if
              // there is no advantage or the treewidth is already too
              // large
              if(score >= 0) priority_queue.push(neighbor_vid, score);
              visited.insert(neighbor_vid);

            } 
            // else if(priority_queue.contains(neighbor_vid)) {
            //   // vertex is still in queue we may need to recompute
            //   // score
            //   double score = score_vertex(neighbor_vid);
            //   if(score >= 0) {
            //     // update the priority queue with the new score
            //     priority_queue.update(neighbor_vid, score);
            //   } else {
            //     // The score computation revealed that the clique
            //     // would be too large so simply remove the vertex from
            //     // the priority queue
            //     priority_queue.remove(neighbor_vid);
            //   }
            // } // otherwise the vertex has been visited and processed

          }
        } else {
          // release the vertex since it could not be used in the tree
          release_vertex(scope);
        }
      }

      if(!released_scope) {
	// release the scope
	scope_factory->release_scope(&scope);        
      }
      // Limit the number of variables
      if(cliques.size() > max_tree_size) break;
    } // end of while loop

  } // end grow_prioritized_jtree
