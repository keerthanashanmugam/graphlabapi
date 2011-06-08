#ifndef __SVD_HPP
#define __SVD_HPP

#include "graphlab.hpp"
#include <graphlab/macros_def.hpp>


/**
 *
 *  Implementation of the Lanczos algorithm, as given in:
 *  http://en.wikipedia.org/wiki/Lanczos_algorithm
 * */

#define MAX_ITER 3

extern string infile;
extern int iiter, L, M, N, Le;
extern bool ZERO;
extern timer gt;
extern graph_type validation_graph;
extern bool debug;
extern int iiter;
using namespace graphlab;
using namespace itpp;
int m = MAX_ITER;

void last_iter();
double predict(const vertex_data& user, const vertex_data &movie, float rating, float & prediction);



//LANCZOS VARIABLES
vec lancbeta;
vec lancalpha;
int offset, offset2, offset3;



/**
 *
 * [n,k] = size(A);
   V = zeros(k,m+1);
   V(:,2) = rand(k,1);
   V(:,2)=V(:,2)/norm(V(:,2),2);
   beta(2)=0;
 *
 * */
void init_lanczos(){
   lancbeta = zeros(m+3);
   lancalpha = zeros(m+3);
   double sum = 0;

  for (int i=M; i< M+N; i++){ 
    vertex_data * data = &g->vertex_data(i);
    data->pvec = zeros(m+1);
    data->pvec[1] = debug ? 0.5 : rand();
    sum += data->pvec[1]*data->pvec[1];
  }

  sum = sqrt(sum);
  for (int i=M; i< M+N; i++){ 
    vertex_data * data = &g->vertex_data(i);
    data->pvec[1] /= sum;
  }

}

/***
 * UPDATE FUNCTION (ROWS)
 */
void Axb(gl_types::iscope &scope, 
	 gl_types::icallback &scheduler) {
    


  /* GET current vertex data */
  vertex_data& user = scope.vertex_data();
 
  
  /* print statistics */
  if (debug&& (scope.vertex() == 0 || ((int)scope.vertex() == M-1))){
    printf("Axb: entering  node  %u \n",  (int)scope.vertex());   
    debug_print_vec("V" , user.pvec, D);
  }

  user.rmse = 0;

  if (user.num_edges == 0){
    return; //if this user/movie have no ratings do nothing
  }


  edge_list outs = scope.out_edge_ids();
  timer t;
  t.start(); 

   //compute SGD Step 
   foreach(graphlab::edge_id_t oedgeid, outs) {
      edge_data & edge = scope.edge_data(oedgeid);
      vertex_data  & movie = scope.neighbor_vertex_data(scope.target(oedgeid));
      user.rmse += edge.weight * movie.pvec[offset];
   }

  counter[EDGE_TRAVERSAL] += t.current_time();
  if (debug&& (scope.vertex() == 0 || ((int)scope.vertex() == M-1))){
    printf("Axb: computed value  %u %g \n",  (int)scope.vertex(), user.rmse);   
  }


}
/***
 * UPDATE FUNCTION (COLS)
 */
void ATxb(gl_types::iscope &scope, 
	 gl_types::icallback &scheduler) {
    


  /* GET current vertex data */
  vertex_data& user = scope.vertex_data();
 
  
  /* print statistics */
  if (debug&& ((int)scope.vertex() == M || ((int)scope.vertex() == M+N-1))){
    printf("Axb: entering  node  %u \n",  (int)scope.vertex());   
    debug_print_vec("V" , user.pvec, D);
  }

  user.rmse = 0;

  if (user.num_edges == 0){
    return; //if this user/movie have no ratings do nothing
  }


  edge_list ins = scope.in_edge_ids();
  timer t;
  t.start(); 

   //compute SGD Step 
   foreach(graphlab::edge_id_t iedgeid, ins) {
      edge_data & edge = scope.edge_data(iedgeid);
      vertex_data  & movie = scope.neighbor_vertex_data(scope.source(iedgeid));
      user.rmse += edge.weight * movie.rmse;
   }

   user.rmse -= lancbeta[offset2] - user.pvec[offset3];

   counter[EDGE_TRAVERSAL] += t.current_time();

  if (debug&& ((int)scope.vertex() == M || ((int)scope.vertex() == M+N-1))){
    printf("Axb: computed value  %u %g beta: %g v %g \n",  (int)scope.vertex(), user.rmse,lancbeta[offset2],  user.pvec[offset3]);   
  }




}

 
double wTV(int j){

  timer t; t.start();
  double lancalpha = 0;
  for (int i=M; i< M+N; i++){ 
    const vertex_data * data = &g->vertex_data(i);
    lancalpha+= data->rmse*data->pvec[j];
  }
  counter[CALC_RMSE_Q] += t.current_time();
  return lancalpha;

}
double w_lancalphaV(int j){

  timer t; t.start();
  double norm = 0;
  for (int i=M; i< M+N; i++){ 
    vertex_data * data = &g->vertex_data(i);
    data->rmse -= lancalpha[j]*data->pvec[j];
    norm += data->rmse*data->rmse;
  }
  counter[CALC_RMSE_Q] += t.current_time();
  return sqrt(norm);
}

void update_V(int j){
  timer t; t.start();
  for (int i=M; i< M+N; i++){ 
    vertex_data * data = &g->vertex_data(i);
    data->pvec[j+1] = data->rmse / lancbeta[j+1];
  }
  counter[CALC_RMSE_Q] += t.current_time();
}

mat calc_V(){
  mat V = zeros(M,m);
  for (int i=M; i< M+N; i++){ 
    const vertex_data * data = &g->vertex_data(i);
    V.set_row(i-M, data->pvec);
  }
  return V;
}


void lanczos(gl_types::core & glcore){

   std::vector<vertex_id_t> rows,cols;
   for (int i=0; i< M; i++)
      rows.push_back(i);
   for (int i=M; i< M+N; i++)
      cols.push_back(i);
 

   //for j=2:m+2
   for (int j=1; j<= m+1; j++){
        //w = A*V(:,j) 
        offset = j;
	glcore.add_tasks(rows, Axb, 1);
        glcore.start();

        //w = w - lancbeta(j)*V(:,j-1);
        offset2 = j;
        offset3 = j-1;
        glcore.add_tasks(cols, ATxb, 1);
	glcore.start();

        //lancalpha(j) = w'*V(:,j);
	lancalpha[j] = wTV(j);

        //w =  w - lancalpha(j)*V(:,j);
        //lancbeta(j+1)=norm(w,2);
        lancbeta[j+1] = w_lancalphaV(j);

        //V(:,j+1) = w/lancbeta(j+1);
        update_V(j);
   }

  /* 
 * T=sparse(m+1,m+1);
 * for i=2:m+1
 *     T(i-1,i-1)=lancalpha(i);
 *     T(i-1,i)=lancbeta(i+1);
 *     T(i,i-1)=lancbeta(i+1);
 * end 
 * T(m+1,m+1)=lancalpha(m+2);
 * V = V(:,2:end-1);
 */

 mat T=zeros(m+1,m+1);
 for (int i=1; i<=m+1; i++){
   T.set(i-1,i-1,lancalpha[i]);
   T.set(i-1,i,lancbeta[i+1]);
   T.set(i,i-1,lancbeta[i+1]);
 }
 T.set(m,m,lancalpha[m+1]);
 mat V=calc_V(); 
   
 vec eigenvalues; 
 mat eigenvectors;
 assert(eig_sym(V, eigenvalues, eigenvectors));
 if (debug){
   for (int i=0; i< eigenvectors.size(); i++)
	cout<<"eigenvalue " << i << " val: " << eigenvalues[i] << endl;
 }

}

#include "graphlab/macros_undef.hpp"
#endif //__SVD_HPP
