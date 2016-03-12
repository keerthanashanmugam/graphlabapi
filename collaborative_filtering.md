Danny Bickson





# Introduction #
This document described the popular Graphlab collaborative filtering library. The library has been download thousands of times for far, and is widely used both in the industry and academia.

Solutions based on this library won the 5th place in KDD CUP 2011 track1 (out of more than 1000 participants) as well as small part of the 1st place in the same contest.

I am often asked, which is the best algorithm for collaborative filtering? The answer is simple: there is no silver bullet. Different algorithms work differently depends on the underlying data. Overall, accurate predictions are based on a collection of algorithms blended together. This blending (a.k.a ensemble methods) can improve the quality of prediction comparing to any single algorithm.

The following bar plot shows performance on different algorithms on KDD CUP 2011 problem. The error measure is RMSE (explained later), and as lower the better.

A better results can be obtained by blending the solutions together.

Since performance of algorithms is highly depended on the data, we find it very useful to have a collection of algorithms that you can quickly experiment with and decide which works best. For that the GraphLab collaborative filtering library is very useful.

The GraphLab collaborative filtering library is designed to work well on problems up to size of one billion ratings, with tens of millions of users. We are now in the process of scaling up using distributed GraphLab to tens of  billions of ratings with hundreds of millions of users.


## Requirements ##
  * it++ or Eigen should be installed. it++ is a c++ wrapper for the efficient BLAS/LaPaCK linear algebra packages. Detailed itpp installation procedure.
  * Alternatively, we now also support Eigen linear algebra package. For configuring GraphLab to be used with Eigen, simply use the --eigen command line flag when using ./configure.
  * GraphLab should be installed. Follow installation instructions here.
  * Useful tip: it is advised to join our GraphLab users Google group. We post updates, tips and installation instructions there.
  * Memory requirement: each 8GB of memory is used for around 150,000,000 non-zero ratings. So a machine with 64GB memory can easily handle 1,000,000,000 non-zero ratings.

## Program input ##
GraphLab collaborative filtering library has three inputs files: training, validation and test. Only the training input is mandatory, the validation and test inputs are optional. The training is used for training the model on observed user-item ratings, validation file is used to assess the quality of the trained model, and test data is used to predict unobserved ratings. The convention used: assume foobar is the training, then validation should be foobare and test should be foobart.
The input is a sparse tensor (or matrix) prepared in on of the following ways:
  * Using text file in Matrix Market sparse matrix format. This is the recommended input option.
  * Using Octave/matlab to prepare the inputs
  * Mahout SVD sequence files (SequentialAccessSparseVector format)

## Matrix Market input files ##
GraphLab supports sparse Matrix Market input files. Don't forget to use the flag --matrixmarket=true when running GraphLab, when you use this format.
Preparing input via Matlab/Octave input
Use the script mmwrite.m to save files in sparse matrix market format. For example, assume we have a 2x2 ratings matrix (2 users and 2 items):
```
>> A=[1 2 ; 3 0]

A =

     1     2
     3     0
```
Namely, user 1 has rated item 1 with the rating 1, item 2 with the rating 2, and user 2 has rated item 1 with the rating of 3.
We save the rating matrix into a sparse matrix market format:
` >> mmwrite('Afile’, sparse(A)); `

And the result is a file named Afile with the following content:

```
%%MatrixMarket matrix coordinate real general
% Generated 26-May-2012
2 2 3
1 1  1
2 1  3
1 2  2
```

Converting from Mahout’s SequentialAccessSparseVector input files
Use the following instructions for converting Mahout's SVD input files to GraphLab's binary input. Note: this input is deprecated - contact me if you are interested in onverting Mahout’s sequence file into matrix market format.
Program output
The output of the program are three matrices U,V and T of size dim1 X D, dim2 X D and dim3 X D.
The output is generated to a file named [inputfile](inputfile.md).out
There are several supported formats for the program output.
  * Matrix Market sparse matrix output (recommended!)
  * Matlab output
  * Python (binary) output
Matrix market output
When using the flag --matrixmarket=true, all output files will be saved using the sparse matrix market format.  You can load the files using Matlab/Octave using the script mmread.m.
Reading output using Matlab/Octave
Any GraphLab output file can be read in Matlab/Octave using the mmread.m script:
>> A=mmread('wals-20-11.out.V')

A =

> Columns 1 through 15

> 0.0335    0.0231    0.0239    0.0358    0.0050    0.0269    0.0254    0.0221    0.0314    0.0239    0.0348    0.0380    0.0213    0.0173    0.0213
> 0.0630    0.0434    0.0450    0.0674    0.0095    0.0506    0.0477    0.0415    0.0590    0.0449    0.0655    0.0714    0.0401    0.0326    0.0401
> 0.0209    0.0026    0.0087    0.0112    0.0002    0.0064    0.0083    0.0027    0.0193    0.0073    0.0063    0.0053    0.0006    0.0146    0.0159
> 0.0685    0.0472    0.0489    0.0734    0.0103    0.0550    0.0519    0.0451    0.0642    0.0489    0.0713    0.0777    0.0436    0.0355    0.0437

> Columns 16 through 20

> 0.0227    0.0287    0.0067    0.0187    0.0313
> 0.0427    0.0539    0.0126    0.0352    0.0590
> 0.0139    0.0028    0.0101    0.0206    0.0027
> 0.0465    0.0587    0.0138    0.0383    0.0642

The output files are all matrices or vector. For each algorithm an example on how to use those file for generating recommendation is given. For example, a complete example of reading WALS algorithm data is found here.

Python (binary) output
TBD

Computing predictions
There are three possible ways to compute predictions of user/item pairs:
  * Using test input file you can get prediction for a predefined list of user/item pairs.
  * Using glcluster program for finding the top K predictions.
  * Using Matlab/Octave.
Here are some more details:
Using test input file
Assume your training input file is mydataset, your test input file should be called mydatasett (namely a "t" was appended at the end of the filename). The test file includes user/item pairs to compute prediction on. It has the exact same input of the training and validation files. (The value given in the test file for the prediction is simply ignored).
At the end of the run you will get an output file with the scalar prediction computed for each user/item pair.
Using glcluster program
For computing the top K predictions you can use the glcluster program as follows.
Note: currently glcluster program supports computation of predictions for the following algorithms: ALS, NMF, sparse-ALS, weighted-ALS.

Assume you run pmf on a file called "chapters.mm". The result will are two files chapters.mm.U and chapters.mm.V. To compute the recommended ratings, run:
ln -s chapters.U chapters ln -s chapters.V chapterse ./glcluster chapters 8 3 0 --matrixmarket=true --training\_ref=chapters.mm --ncpus=8
The output of this command is:
1) chapters.scalar-ratings.mtx
2) chapters.recommended-items.mtx

The file recommend-items include ids of items (starting from 0 and not from zero!) for each user. Each user is in a new row. The file scalar-ratings lists the computed scalar rating for each of the recommended items.

Using Matlab
Example of reading the output of ALS using matlab and computing recommendation is found here.

Understanding the error measures
RMSE
The commonly used error measure is RMSE: square root of mean square error. Lower values of RMSE are better. RMSE is computed by default in GraphLab.

MAE
MAE is mean average error. It is now computed in GraphLab on default for the validation data.

AP@3
Additional error message supported is AP@3. See KDD CUP 2012 explanation. Ap@3 is computing when the command line argument --calc\_ap=true is defined.

Unit testing
After installation, a good idea would be to try first the unit testing.
cd release/tests   ./runtests.sh 1
You will see a report of all unit tests and their results. In case of any failure, please email the resulting output file stdout.log to danny.bickson@gmail.com .

Running PMF
Command line options
After preparing the input, you should run:
./PMF [file](input.md) [mode](run.md)  --scheduler="round\_robin(max\_iterations=XXX,block\_size=1)"      // where XXX is the number of desired iterations
The following are the supported algorithms:

0
ALS
Matrix factorization using alternating least squares
1
PMF
Matrix factorization using MCMC procedure
2
BPTF
Tensor factorization using MCMC procedure, single edge  exists between user and movies
4
ALS-TENSOR
Tensor factorization using alternating least squares
5
SVD++
Koren's SVD++ algorithm
6
SGD
Stochastic gradient descent
7
SVD
Lanczos algorithm. Deprecated. Use GraphLab v2.
8
NMF
Non-negative matrix factorization algo of Lee and Seung
9
WALS
Weighted alternating least squares
10
Sparse-ALS
Alternating least squares with sparse user factor matrix
11
Sparse-ALS
Alternating least squares with sparse user and movie factor matrices
12
Sparse-ALS
Alternating least squares with sparse movie factor matrix
13
Double-Lanczos
Singular value decomposition (via double Lanczos method). Deprecated. Use GraphLab v2.
14
time-SVD++
Koren's time-svd++ method
15
bias-SGD
Stochastic gradient descent with user and item bias
16
RBM
Restricted Bolzman Machines
17
LIBFM
Factorization Machines

The following are general optional command line parameters. You can view the full list using the command "./pmf --help".


IO switches
--matrixmarket=true
Input is in matrix market format (recommended!)

--exportlinearmodel=false
Don’t save the factorized matrices

--exporttest
Write test predictions into file
Basic configuration
--npucs=XX
run with XX cpus

--maxval=XX, --minval=XX
> it is recommended to set the min allowed ratings values and max allowed rating values to improve predictions.

--zero=true
Allow zero ratings (default: false). Typically, zero value is not listed as rating and regarded as empty. In case --zero=true, we know that an zero rating was recorded.
Debugging
--stats=true
Print statistics about the input problem and exit

--debug=true
Display debugging information

--showversion=true
Show graphlab version and exit
Advanced configuration
--scaling=XX
scale time factors by (namely divide the time factors to bin them into bins)

--truncating=XX
shift time factor by (namely substruct XX from the time factors, minimal allowed bin number is zero)

--scalerating=XX
scale the rating by factor of XX (namely divide each rating by XX)

--shiftrating=XX
shift the rating by factor of XX (namely substract XX from each rating)

--halt\_on\_rmse\_increase=true
Stop execution when divergence is detected

--loadfactors=true
Load initial feature vectors  from file
Ensemble options
--aggregatevalidation=true
Use the validation data for training the model (add it to the training data)

--outputvalidation=true
Output prediction on validation data where the training is based on the training data



Algorithms

Here is a table summarizing the properties of the different algorithms in the collaborative filtering library:

ALGORITHM
Tensor? (Supports time of rating input)
Sparse output?
Multiple ratings between the same user/item pair?
Monte Carlo sampling method?
ALS




Sparse-ALS

V


SGD




bias-SGD




SVD




NMF




RBM



V
SVD++




LIBFM
V



PMF



V
time-SVD++
V



BPTF
V


V
BPTF\_MULT
V

V
V
ALS\_MULT


V

WALS
V




Note: for tensor algorithms, you need to verify you have both the rating and its time. It is recommended to parse unix timestamp into bins, such that there will be a few tens of bins. Having too fine granularity over the time bins slows down computation and does not improve prediction.
Using matrix market format, you need to specify each rating using 4 fields:
[user](user.md) [item](item.md) [rating](rating.md) [bin](time.md).
Don’t forget to run using the command line flag --matrix\_market\_tokens\_per\_row=4 (namely the row has 4 fields and not 3).

ALS (Alternating least squares)
Pros: Simple to use, not many command line arguments
Cons: intermediate accuracy, higher computational overhead

ALS is a simple yet powerful algorithm. In this model the prediction is computed as:
> r\_ui = p\_u **q\_i
Where r\_ui is a scalar rating of user u to item i, and p\_u is the user feature vector of size D, q\_i is the item feature vector of size D and the product is a vector product.
The output of ALS is two matrices: filename.U and filename.V. The matrix U holds the user feature vectors in each row. (Each vector has exactly D columns). The matrix V holds the feature vectors for each time (Each vector has again exactly D columns). In linear algebra notation the rating matrix R ~ UV**

Note: ALS can significantly benefit from efficient linear algebra packages like Intel MKL.

Below are ALS related command line options:

Basic configuration
--D=XX
Set D the feature vector width. High width results in higher accuracy but slower execution time. Typical values are 20 -  100.

--lambda=XX
Set regularization. Regularization helps to prevent overfitting.
Advanced configuration
--regnormal=true
Regularize each node by the same amount without relation to the number of ratings it has (default: false)

Example of running ALS:
1) Download movielens\_mm and movielense\_mme sample files. The movielens dataset includes around 1 millions ratings.

2) For running alternating least squares on movielens data, run the following command
./pmf movielens\_mm 0 --scheduler="round\_robin(max\_iterations=10,block\_size=1)" --matrixmarket=true --lambda=0.065 --ncpus=2
INFO:     pmf.cpp(do\_main:442): (c) PMF/BPTF/ALS/SVD++/time-SVD++/SGD/Lanczos/SVD/bias-SGD/RBM Code written By Danny Bickson, CMU
Send bug reports and comments to danny.bickson@gmail.com
WARNING:  pmf.cpp(do\_main:449): Program compiled with it++ Support
Setting run mode ALS\_MATRIX (Alternating least squares)
WARNING:  pmf.h(verify\_setup:439): It is recommended to set min and max allowed matrix values to improve prediction quality, using the flags --minval=XX, --maxval=XX
INFO:     pmf.cpp(start:300): ALS\_MATRIX (Alternating least squares) starting

loading data file movielens\_mm
Loading Matrix Market file movielens\_mm TRAINING
Loading movielens\_mm TRAINING
Matrix size is: USERS 6040 MOVIES 3952 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:145): Loaded total edges: 900000
loading data file movielens\_mme
Loading Matrix Market file movielens\_mme VALIDATION
Loading movielens\_mme VALIDATION
Matrix size is: USERS 6040 MOVIES 3952 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:145): Loaded total edges: 100209
setting regularization weight to 0.065
ALS\_MATRIX (Alternating least squares) for matrix (6040, 3952, 1):900000.  D=20
pU=0.065, pV=0.065, pT=1, D=20
complete. Objective=6.32441e+06, TRAIN RMSE=3.7489 VALIDATION RMSE=3.7533.
INFO:     pmf.cpp(run\_graphlab:243): starting with scheduler: round\_robin
max iterations = 10
step = 1
max\_iterations = 10
INFO:     asynchronous\_engine.hpp(run:111): Worker 0 started.
INFO:     asynchronous\_engine.hpp(run:111): Worker 1 started.
1.62752) Iter ALS\_MATRIX (Alternating least squares) 1  Obj=5.90344e+06, TRAIN RMSE=3.6215 VALIDATION RMSE=0.9372.
3.20309) Iter ALS\_MATRIX (Alternating least squares) 2  Obj=374881, TRAIN RMSE=0.9107 VALIDATION RMSE=0.9079.
4.78017) Iter ALS\_MATRIX (Alternating least squares) 3  Obj=328108, TRAIN RMSE=0.8516 VALIDATION RMSE=0.8686.
6.3558) Iter ALS\_MATRIX (Alternating least squares) 4  Obj=287654, TRAIN RMSE=0.7970 VALIDATION RMSE=0.8562.
7.92776) Iter ALS\_MATRIX (Alternating least squares) 5  Obj=269589, TRAIN RMSE=0.7714 VALIDATION RMSE=0.8528.
9.49469) Iter ALS\_MATRIX (Alternating least squares) 6  Obj=262066, TRAIN RMSE=0.7605 VALIDATION RMSE=0.8515.
11.0642) Iter ALS\_MATRIX (Alternating least squares) 7  Obj=258323, TRAIN RMSE=0.7550 VALIDATION RMSE=0.8507.
12.6322) Iter ALS\_MATRIX (Alternating least squares) 8  Obj=256056, TRAIN RMSE=0.7517 VALIDATION RMSE=0.8502.
14.1951) Iter ALS\_MATRIX (Alternating least squares) 9  Obj=254491, TRAIN RMSE=0.7494 VALIDATION RMSE=0.8499.
15.7617) Iter ALS\_MATRIX (Alternating least squares) 10  Obj=253327, TRAIN RMSE=0.7477 VALIDATION RMSE=0.8497.
INFO:     asynchronous\_engine.hpp(run:119): Worker 1 finished.
INFO:     asynchronous\_engine.hpp(run:119): Worker 0 finished.
Final result. Obj=253327, TRAIN RMSE= 0.7477 VALIDATION RMSE= 0.8497.
Finished in 15.794923 seconds
loading data file movielens\_mmt
Loading Matrix Market file movielens\_mmt TEST
Loading movielens\_mmt TEST
skipping file
Performance counters are: 0) EDGE\_TRAVERSAL, 8.81501
Performance counters are: 2) CALC\_RMSE\_Q, 0.001507
Performance counters are: 3) ALS\_LEAST\_SQUARES, 14.4332
Performance counters are: 6) CALC\_OBJ, 0.034359
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:262): Saved output matrix to file: movielens\_mm-20-11.out.V
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:263): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:262): Saved output matrix to file: movielens\_mm-20-11.out.U
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:263): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m

> ### REPORT FOR core() ###
[Numeric](Numeric.md)
ncpus:		2
[Other](Other.md)
affinities:	false
compile\_flags:
engine:	async
scheduler:	round\_robin
schedyield:	true
scope:	edge

> ### REPORT FOR engine() ###
[Numeric](Numeric.md)
num\_edges:		900000
num\_syncs:		0
num\_vertices:		9992
updatecount:		99920
[Timings](Timings.md)
runtime:		15.7 s
[Other](Other.md)
termination\_reason:	task depletion (natural)
[Numeric](Numeric.md)
updatecount\_vector:		99920	(count: 2, min: 49887, max: 50033, avg: 49960)
updatecount\_vector.values:		49887,50033,


Reading and processing the output for generating recommendations
Now let’s look at the output files. There are two output files: movielens\_mm-20-11.out.V
and movielens\_mm-20-11.out.U

The file which ends with .U holds the user feature vectors and the file ends with .V ends the movie feature vectors. Let’s take a look at the file format:
<15|0>bickson@bigbro6:~/newgraphlab/graphlabapi/debug/demoapps/pmf$ head movielens\_mm-20-11.out.U
%%MatrixMarket matrix array real general
%%%GraphLab Collaborative filtering library. This file holds the matrix U. Row i holds the feature vector for user i. You can compute prediction in matlab for user i movie j using U(i,:)**V(j,:)'
6040 20
0.6624230828651 0.5733561823352 0.1718419153045 0.6443482090046 0.8321259875845 0.8573343946367 0.4412109899183 0.8088226688963 0.1213220885531 0.2810372862208 0.4768147879781 -0.1120487468904 0.505579210735 0.9032413526564 0.3956331035238**

Each row contains the feature vector for a user.

Now let’s load this file in Matlab:
>> A=mmread('movielens\_mm-20-11.out.U');

Note: you can download the mmread.m script here.
We can view the feature vector of the first user:
>> A(1,:)

ans =

> Columns 1 through 7

> 0.6624    0.5734    0.1718    0.6443    0.8321    0.8573    0.4412

> Columns 8 through 14

> 0.8088    0.1213    0.2810    0.4768   -0.1120    0.5056    0.9032

> Columns 15 through 20

> 0.3956    0.4764    0.7598    0.4814    0.6231    0.3857

Now assume we want to compute the rating of the first user vs. the 10th item. We compute it in Matlab by first loading the items feature matrix
>> B=mmread('movielens\_mm-20-11.out.V');
Next we compute the product:
>> A(1,:)**B(10,:)'**

ans =

> 3.1550

The predicted rating is 3.155

Now we can repeat this product for any pair of user/items requested.

Alternating least squares with sparse factors
Pros: excellent for spectral clustering
Cons: less accurate linear model because of the sparsification step

This algorithm is based on ALS, but an additional sparsifying step is performed on either the user feature vectors, the item feature vectors or both. This algorithm is useful for spectral clustering: first the rating matrix is factorized into a product of one or two sparse matrices, and then clustering can be computed on the feature matrices to detect similar users or items.

The underlying algorithm which is used for sparsifying is CoSaMP. See reference [K1](K1.md).

Below are sparse-ALS related command line options:

Basic configuration
--user\_sparsity=XX
A number between 0.1 to 1 which defines how sparse is the resulting user feature factor matrix

--movie\_sparsity=XX
A number between 0.1 to 1 which defines how sparse is the resulting movie feature factor matrix
Advanced configuration
--lasso\_max\_iter=XX
Set the maximal number of CoSaMP iterations


Prediction in sparse-ALS is computing like in ALS.
Running example: Netflix data with sparse movie factor matrix
In this example we show how to factorize netflix data, with the requirement that 90% of the movie factor matrix will be zeros. Next, you can use the sparse matrices for performing clustering of similar user or movies together into related groups.
bickson@biggerbro:~/newgraphlab/graphlabapi/debug/demoapps/pmf $ ./pmf netflix 12  --scheduler="round\_robin(max\_iterations=10,block\_size=1)" --float=false --ncpus=8 --desired\_factor\_sparsity=0.9 --lambda=0.06  INFO:     pmf.cpp(main:565): PMF/BPTF/ALS/SVD++/SGD/SVD Code written By Danny Bickson, CMU Send bug reports and comments to danny.bickson@gmail.com WARNING:  pmf.cpp(main:567): Code compiled with GL\_NO\_MULT\_EDGES flag - this mode does not support multiple edges between user and movie in different times WARNING:  pmf.cpp(main:570): Code compiled with GL\_NO\_MCMC flag - this mode does not support MCMC methods. Setting run mode Alternating least squares with sparse movie factor matrix INFO:     pmf.cpp(start:370): Alternating least squares with sparse movie factor matrix starting  loading data file netflix Loading netflix TRAINING Matrix size is: USERS 95526 MOVIES 3561 TIME BINS 27 Creating 3298163 edges (observed ratings)... .................loading data file netflixe Loading netflixe VALIDATION Matrix size is: USERS 95526 MOVIES 3561 TIME BINS 27 Creating 545177 edges (observed ratings)... ...loading data file netflixt Loading netflixt TEST skipping file setting regularization weight to 0.06 Alternating least squares with sparse movie factor matrix for matrix (95526, 3561, 27):3298163.  D=20 pU=0.06, pV=0.06, pT=1, D=20 Current sparsity : 0 % complete. Objective=1.30614e+07, TRAIN RMSE=2.8139 VALIDATION RMSE=2.8790. max iterations = 10 step = 1 max\_iterations = 10 INFO:     asynchronous\_engine.hpp(run:111): Worker 0 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 1 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 2 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 3 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 4 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 6 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 7 started.  INFO:     asynchronous\_engine.hpp(run:111): Worker 5 started.  Entering last iter with 1 Current sparsity : 0.9 2.61367) Iter Alternating least squares with sparse movie factor matrix 1  Obj=8.39139e+06, TRAIN RMSE=2.2338 VALIDATION RMSE=2.4251. Entering last iter with 2 Current sparsity : 0.95 % 5.25192) Iter Alternating least squares with sparse movie factor matrix 2  Obj=2.52153e+06, TRAIN RMSE=1.2152 VALIDATION RMSE=1.6419. Entering last iter with 3 Current sparsity : 0.9 7.88379) Iter Alternating least squares with sparse movie factor matrix 3  Obj=2.36985e+06, TRAIN RMSE=1.1787 VALIDATION RMSE=1.3749. Entering last iter with 4 Current sparsity : 0.9 10.5112) Iter Alternating least squares with sparse movie factor matrix 4  Obj=2.57171e+06, TRAIN RMSE=1.2280 VALIDATION RMSE=1.3589. Entering last iter with 5 Current sparsity : 0.9 13.0986) Iter Alternating least squares with sparse movie factor matrix 5  Obj=2.76916e+06, TRAIN RMSE=1.2758 VALIDATION RMSE=1.3188. Entering last iter with 6 Current sparsity : 0.9 % 15.7324) Iter Alternating least squares with sparse movie factor matrix 6  Obj=2.74914e+06, TRAIN RMSE=1.2721 VALIDATION RMSE=1.2410. Entering last iter with 7 Current sparsity : 0.9 18.3847) Iter Alternating least squares with sparse movie factor matrix 7  Obj=2.53998e+06, TRAIN RMSE=1.2239 VALIDATION RMSE=1.0778. Entering last iter with 8 Current sparsity : 0.9 20.9803) Iter Alternating least squares with sparse movie factor matrix 8  Obj=1.84584e+06, TRAIN RMSE=1.0436 VALIDATION RMSE=0.9723. Entering last iter with 9 Current sparsity : 0.9 23.6121) Iter Alternating least squares with sparse movie factor matrix 9  Obj=1.48064e+06, TRAIN RMSE=0.9341 VALIDATION RMSE=0.9608. Entering last iter with 10 Current sparsity : 0.9 26.1979) Iter Alternating least squares with sparse movie factor matrix 10  Obj=1.43894e+06, TRAIN RMSE=0.9217 VALIDATION RMSE=0.9596. I... Current sparsity : 0.9  Final result. Obj=1.43894e+06, TRAIN RMSE= 0.9217 VALIDATION RMSE= 0.9596. Finished in 26.611790 seconds Performance counters are: 0) EDGE\_TRAVERSAL, 49.7254 Performance counters are: 2) CALC\_RMSE\_Q, 0.001046 Performance counters are: 3) ALS\_LEAST\_SQUARES, 81.24 Performance counters are: 6) CALC\_OBJ, 0.59852 ..


PMF/BPTF/BPTF\_MULT/ALS\_MULT
Pros: once tuned, better accuracy than ALS
Cons: sensitive to numerical errors, needs fine tuning, does not work on every dataset, higher computational cost, higher prediction computational cost.

PMF and BPTF are two Markov Chain Monte Carlo (MCMC) sampling methods. They are based on ALS, but on each step a sampling from the probability is perform for obtaining the next state.
Prediction in PMF/BPTF is like in ALS, but instead of computing one vector product of the current feature vector, the whole chain of products is computed and the average is taken.

More formally, the prediction rule of PMF is:
r\_ui = [p\_u(1) \* q\_i(1) + p\_u(2) \* q\_i(2) + ..  + p\_u(l) \* q\_i(l) ](.md) / l

Where l is the length of the chain.

Note: typically in MCMC methods, the first XX samples of the chain are thrown away, so p\_u and q\_i will start from XX and not from 1.

The prediction rule of BPTF includes a feature vector for each time bin, denote w:
r\_uik = [p\_u(1) \* q\_i(1) \* w\_k(1) + p\_u(2) \* q\_i(2) \* w\_k(2) + ..  + p\_u(l) \* q\_i(l) \* w\_k(l) ](.md) / l
Where the product is a tensor product, namely \sum\_j p\_uj **q\_ij** w\_kj

Basic configuration
--bptf\_burn\_in=XX
Throw away the first XX samples in the chain

--bptf\_additional\_output=true
Save as output all the samples in the chain (after the burn in period).
Each sample is composed of two feature vectors. Each will be saved on its own file.


--D=X
Feature vector width. Common values are 20 - 150.
Special Note: BPTF/BPTF\_MULT a tensor factorization algorithm. Please don’t forget to prepare a 4 column matrix market format file, with [user](user.md) [item ](.md) [rating ](.md) [time ](.md) in each row.
When running, you should use
1) --matrix\_market\_tokens\_per\_row=4 command line argument. Time should be an integer value between 0 to max\_time.
2) Use the command line argument --K=max\_time+1 to indicate the number of time bins.
Running example: BPTF (Bayesian monte carlo matrix factorization) using Twitter social graph
This example was donated by Timmy Wilson @ smarttypes.org. It contains a twitter network of 68 followers, 11646 followies, 1 day and 15883 links. Download the input file here
<29|0>bickson@biggerbro:~/newgraphlab/graphlabapi/debug/demoapps/pmf$ ./pmf smarttypes\_pmf 1 --scheduler="round\_robin(max\_iterations=20,block\_size=1)" --float=true INFO:     pmf.cpp(main:1260): PMF/ALS/SVD++/SGD Code written By Danny Bickson, CMU Send bug reports and comments to danny.bickson@gmail.com WARNING:  pmf.cpp(main:1262): Code compiled with GL\_NO\_MULT\_EDGES flag - this mode does not support multiple edges between user and movie in different times Setting run mode BPTF\_MATRIX INFO:     pmf.cpp(main:1309): BPTF\_MATRIX starting  loading data file smarttypes\_pmf Loading smarttypes\_pmf TRAINING Matrix size is: USERS 68 MOVIES 11646 TIME BINS 1 Creating 15883 edges (observed ratings)... .loading data file smarttypes\_pmfe Loading smarttypes\_pmfe VALIDATION skipping file loading data file smarttypes\_pmft Loading smarttypes\_pmft TEST skipping file setting regularization weight to 1 BPTF\_MATRIX for matrix (68, 11646, 1):15883.  D=20 pU=1, pV=1, pT=1, muT=1, D=20 nuAlpha=1, Walpha=1, mu=0, muT=1, nu=20, beta=1, W=1, WT=1 BURN\_IN=10 complete. Obj=7576.43, TRAIN RMSE=0.9513 VALIDATION RMSE=nan. sampled alpha is 0.997129 max iterations = 20 step = 1 max\_iterations = 20 INFO:     asynchronous\_engine.hpp(run:94): Worker 0 started.  INFO:     asynchronous\_engine.hpp(run:94): Worker 1 started.  Entering last iter with 1 0.361552) Iter BPTF\_MATRIX 1  Obj=4646.35, TRAIN RMSE=0.7270 VALIDATION RMSE=nan. sampled alpha is 1.90087 Entering last iter with 2 0.728271) Iter BPTF\_MATRIX 2  Obj=1698.21, TRAIN RMSE=0.3103 VALIDATION RMSE=nan. sampled alpha is 10.4702 Entering last iter with 3 1.12834) Iter BPTF\_MATRIX 3  Obj=1368.41, TRAIN RMSE=0.1506 VALIDATION RMSE=nan. sampled alpha is 44.3981 Entering last iter with 4 1.49151) Iter BPTF\_MATRIX 4  Obj=1276.31, TRAIN RMSE=0.1245 VALIDATION RMSE=nan. sampled alpha is 63.932 Entering last iter with 5 1.89511) Iter BPTF\_MATRIX 5  Obj=1203.64, TRAIN RMSE=0.0904 VALIDATION RMSE=nan. sampled alpha is 122.476 Entering last iter with 6 2.25427) Iter BPTF\_MATRIX 6  Obj=1178.26, TRAIN RMSE=0.0744 VALIDATION RMSE=nan. sampled alpha is 180.563 Entering last iter with 7 2.65659) Iter BPTF\_MATRIX 7  Obj=1170.38, TRAIN RMSE=0.0575 VALIDATION RMSE=nan. sampled alpha is 297.039 Entering last iter with 8 3.02014) Iter BPTF\_MATRIX 8  Obj=1160.73, TRAIN RMSE=0.0477 VALIDATION RMSE=nan. sampled alpha is 419.463 Entering last iter with 9 3.42518) Iter BPTF\_MATRIX 9  Obj=1162.77, TRAIN RMSE=0.0394 VALIDATION RMSE=nan. Finished burn-in period. starting to aggregate samples sampled alpha is 610.536 Entering last iter with 10 3.79515) Iter BPTF\_MATRIX 10  Obj=1161.87, TRAIN RMSE=0.0341 VALIDATION RMSE=nan. sampled alpha is 810.82 Entering last iter with 11 4.19491) Iter BPTF\_MATRIX 11  Obj=1469.61, TRAIN RMSE=0.1970 VALIDATION RMSE=nan. sampled alpha is 25.4017 Entering last iter with 12 4.56205) Iter BPTF\_MATRIX 12  Obj=1484.45, TRAIN RMSE=0.2007 VALIDATION RMSE=nan. sampled alpha is 24.5661 Entering last iter with 13 4.96378) Iter BPTF\_MATRIX 13  Obj=1230.12, TRAIN RMSE=0.0700 VALIDATION RMSE=nan. sampled alpha is 203.111 Entering last iter with 14 5.33124) Iter BPTF\_MATRIX 14  Obj=1229.07, TRAIN RMSE=0.0718 VALIDATION RMSE=nan. sampled alpha is 193.54 Entering last iter with 15 5.72784) Iter BPTF\_MATRIX 15  Obj=1209.51, TRAIN RMSE=0.0424 VALIDATION RMSE=nan. sampled alpha is 536.412 Entering last iter with 16 6.101) Iter BPTF\_MATRIX 16  Obj=1214.21, TRAIN RMSE=0.0419 VALIDATION RMSE=nan. sampled alpha is 555.104 Entering last iter with 17 6.49673) Iter BPTF\_MATRIX 17  Obj=1212.21, TRAIN RMSE=0.0310 VALIDATION RMSE=nan. sampled alpha is 1000.04 Entering last iter with 18 6.87056) Iter BPTF\_MATRIX 18  Obj=1215.99, TRAIN RMSE=0.0307 VALIDATION RMSE=nan. sampled alpha is 987.797 Entering last iter with 19 7.2658) Iter BPTF\_MATRIX 19  Obj=1217.74, TRAIN RMSE=0.0237 VALIDATION RMSE=nan. sampled alpha is 1596.85 Entering last iter with 20 7.64149) Iter BPTF\_MATRIX 20  Obj=1224.86, TRAIN RMSE=0.0233 VALIDATION RMSE=nan. sampled alpha is 1677.19 INFO:     asynchronous\_engine.hpp(run:102): Worker 1 finished.  INFO:     asynchronous\_engine.hpp(run:102): Worker 0 finished.  Final result. Obj=1222.59, TRAIN RMSE= 0.0155 VALIDATION RMSE= nan. Finished in 7.686977 Performance counters are: 0) EDGE\_TRAVERSAL, 0.735296 Performance counters are: 1) BPTF\_SAMPLE\_STEP, 0.803732 Performance counters are: 2) CALC\_RMSE\_Q, 0.005395 Performance counters are: 6) CALC\_OBJ, 0.028909 Performance counters are: 7) BPTF\_MVN\_RNDEX, 4.1201 Performance counters are: 8) BPTF\_LEAST\_SQUARES2, 1.15168  runtime:                7.6 s

Weighted Alternating Least Squares (WALS)
Pros: allows weighting of ratings (can be thought of as confidence in rating), almost the same computational cost like in ALS.
Cons: worse modeling error relative to ALS

Weighted ALS is a simple extension for ALS where each user/item pair has an additional weight. In this sense, WALS is a tensor algorithm since besides of the rating it also maintains a weight for each rating. Algorithm is described in references [I, J].

Prediction in WALS is computed as follows:
r\_ui = w\_ui **p\_u** q\_i

The scalar value r for user u and item i is computed by multiplying the weight of the rating w\_ui by the vector product p\_u **q\_i. Both p and q are feature vectors of size D.**

Command line flags for WALS are:
Basic configuration
--D=X
Feature vector width. Common values are 20 - 150.
Special Note: This is a tensor factorization algorithm. Please don’t forget to prepare a 4 column matrix market format file, with [user](user.md) [item ](.md) [rating ](.md) [weight ](.md) in each row.
When running, you should use
1) --matrix\_market\_tokens\_per\_row=4 command line argument. Weight should be a non-zero double.
Example running WALS
Create two input files. The first, the training file is named wals:
%%MatrixMarket matrix coordinate real general
4 4 9
> 3 1 1 0.9527821496566151
> 4 1 2 0.7040621667749977
  1. 2 1 0.9538774735922314
> 2 2 3 0.5981585241721892
> 3 2 1 0.8407431981130701
> 4 2 1 0.4428188422351334
  1. 4 2 0.8368196006763403
> 2 4 2 0.5187030597249157
> 4 4 1 0.02220977857260138

The the second is the validation file walse:

%%MatrixMarket matrix coordinate real general
4 4 6
  1. 1 1 0.3758856012631503
  1. 3 2 0.8985957224020654
> 2 3 1 0.4290014882399393
> 3 3 -1 0.1995708604681839
> 4 3 -0.5 0.3030947130117874
> 4 4 1 0.5382997968405258

Explanation: the training file holds a rating matrix :
training =

> 0     1     0     2
> 0     3     0     2
  1. 1     0     0
> 2     1     0     1

The validation file holds a matrix:
validation =

  1. 0000         0    2.0000         0
> > 0         0    1.0000         0
> > 0         0   -1.0000         0
> > 0         0   -0.5000    1.0000

Besides of the 9 ratings, and 6 validation ratings, there are also weights associated with each rating. The weights appear the the 4th column of the files wals and walse.

<21|134>bickson@biggerbro:~/newgraphlab/graphlabapi/debug/demoapps/pmf$ ./pmf wals 9 --scheduler="round\_robin(max\_iterations=10,block\_size=1)" --matrixmarket=true --matrix\_market\_tokens\_per\_row=4
INFO:     pmf.cpp(do\_main:444): (c) PMF/BPTF/ALS/SVD++/time-SVD++/SGD/Lanczos/SVD/bias-SGD/RBM Code written By Danny Bickson, CMU
Send bug reports and comments to danny.bickson@gmail.com
WARNING:  pmf.cpp(do\_main:451): Program compiled with it++ Support
Setting run mode Weighted alternating least squares
WARNING:  pmf.h(verify\_setup:439): It is recommended to set min and max allowed matrix values to improve prediction quality, using the flags --minval=XX, --maxval=XX
INFO:     pmf.cpp(start:302): Weighted alternating least squares starting

loading data file wals
Loading wals TRAINING
Matrix size is: USERS 4 MOVIES 4 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:159): Loaded total edges: 9 global mean: 1.55556
loading data file walse
Loading walse VALIDATION
Matrix size is: USERS 4 MOVIES 4 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:159): Loaded total edges: 6 global mean: 0.583333
setting regularization weight to 1
Weighted alternating least squares for matrix (4, 4, 1):9.  D=20
pU=1, pV=1, pT=1, D=20
complete. Objective=8.40561, TRAIN RMSE=1.3659 VALIDATION RMSE=0.9308.
INFO:     pmf.cpp(run\_graphlab:245): starting with scheduler: round\_robin
max iterations = 10
step = 1
max\_iterations = 10
INFO:     asynchronous\_engine.hpp(run:111): Worker 0 started.
INFO:     asynchronous\_engine.hpp(run:111): Worker 1 started.
0.001143) Iter Weighted alternating least squares 1  Obj=8.39839, TRAIN RMSE=1.3650 VALIDATION RMSE=0.9306.
0.001321) Iter Weighted alternating least squares 2  Obj=8.39282, TRAIN RMSE=1.3642 VALIDATION RMSE=0.9303.
0.001548) Iter Weighted alternating least squares 3  Obj=8.38446, TRAIN RMSE=1.3631 VALIDATION RMSE=0.9299.
0.001737) Iter Weighted alternating least squares 4  Obj=8.37312, TRAIN RMSE=1.3615 VALIDATION RMSE=0.9294.
0.001894) Iter Weighted alternating least squares 5  Obj=8.35815, TRAIN RMSE=1.3594 VALIDATION RMSE=0.9289.
0.002101) Iter Weighted alternating least squares 6  Obj=8.33874, TRAIN RMSE=1.3567 VALIDATION RMSE=0.9282.
0.002254) Iter Weighted alternating least squares 7  Obj=8.31412, TRAIN RMSE=1.3533 VALIDATION RMSE=0.9274.
0.002449) Iter Weighted alternating least squares 8  Obj=8.2837, TRAIN RMSE=1.3491 VALIDATION RMSE=0.9265.
0.002642) Iter Weighted alternating least squares 9  Obj=8.24728, TRAIN RMSE=1.3440 VALIDATION RMSE=0.9255.
0.002798) Iter Weighted alternating least squares 10  Obj=8.20532, TRAIN RMSE=1.3383 VALIDATION RMSE=0.9243.
INFO:     asynchronous\_engine.hpp(run:119): Worker 0 finished.
INFO:     asynchronous\_engine.hpp(run:119): Worker 1 finished.
Final result. Obj=8.20532, TRAIN RMSE= 1.3383 VALIDATION RMSE= 0.9243.
Finished in 0.003027 seconds
loading data file walst
Loading walst TEST
skipping file
Performance counters are: 0) EDGE\_TRAVERSAL, 0.000223
Performance counters are: 2) CALC\_RMSE\_Q, 1.6e-05
Performance counters are: 3) ALS\_LEAST\_SQUARES, 0.002294
Performance counters are: 6) CALC\_OBJ, 5e-05
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:277): Saved output matrix to file: wals-20-11.out.V
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:278): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:277): Saved output matrix to file: wals-20-11.out.U
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:278): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m


> ### REPORT FOR core() ###
[Numeric](Numeric.md)
ncpus:		2
[Other](Other.md)
affinities:	false
compile\_flags:
engine:	async
scheduler:	round\_robin
schedyield:	true
scope:	edge

> ### REPORT FOR engine() ###
[Numeric](Numeric.md)
num\_edges:		9
num\_syncs:		0
num\_vertices:		8
updatecount:		80
[Timings](Timings.md)
runtime:		0 s
[Other](Other.md)
termination\_reason:	task depletion (natural)
[Numeric](Numeric.md)
updatecount\_vector:		80	(count: 2, min: 36, max: 44, avg: 40)
updatecount\_vector.values:		44,36,




Stochastic gradient descent SGD
Pros: fast method
Cons: need to tune step size, more iterations are needed relative to ALS.

SGD is a simple gradient descent algorithm. Prediction in SGD is done as in ALS:
> r\_ui = p\_u **q\_i
Where r\_ui is a scalar rating of user u to item i, and p\_u is the user feature vector of size D, q\_i is the item feature vector of size D and the product is a vector product.
The output of ALS is two matrices: filename.U and filename.V. The matrix U holds the user feature vectors in each row. (Each vector has exactly D columns). The matrix V holds the feature vectors for each time (Each vector has again exactly D columns). In linear algebra notation the rating matrix R ~ UV**



Basic configuration
--sgd\_lambda=XX
Gradient descent step size

--sgd\_gamma=XX
Gradient descent regularization

--sgd\_step\_dec=XX
Multiplicative step decrease. Should be between 0.1 to 1.
Default is 0.9

--D=X
Feature vector width. Common values are 20 - 150.


Bias-SGD
Pros: fast method
Cons: need to tune step size

Bias-SGD is a simple gradient descent algorithm, where besides of the feature vector we also compute item and user biases (how much their average rating differs from the global average).
Prediction in bias-SGD is done as follows:

r\_ui = global\_mean\_rating + b\_u + b\_i + p\_u **q\_i**

Where global\_mean\_rating is the global mean rating, b\_u is the bias of user u, b\_i is the bias of item i and p\_u and q\_i are feature vectors as in ALS. You can read more about bias-SGD in reference [N](N.md).

The output of bias-SGD consists of two matrices: filename.U and filename.V. The matrix U holds the user feature vectors in each row. (Each vector has exactly D columns). The matrix V holds the feature vectors for each time (Each vector has again exactly D columns). Additionally, the output consists of two vectors: bias for each user, bias for each item. Last, the global mean rating is also given as output.


Basic configuration
--sgd\_lambda=XX
Gradient descent step size

--sgd\_gamma=XX
Gradient descent regularization

--sgd\_step\_dec=XX
Multiplicative step decrease. Should be between 0.1 to 1.
Default is 0.9

--D=X
Feature vector width. Common values are 20 - 150.


Koren’s SVD++
Pros: more accurate method than SGD once tuned, relatively fast method
Cons: a lot of parameters for tuning, immune to numerical errors when parameters are out of scope.

Koren SVD++ is an algorithm which is slightly more fancy than bias-SGD and give somewhat better prediction results.


Basic configuration
--svdpp\_step\_dec=XX
Multiplicative step decrement (between 0.1 to 1). Default is 0.9

--svdpp\_item\_bias\_step=XX
Item bias step size

--svdpp\_item\_bias\_reg=XX
Item bias regularization

--svdpp\_usr\_bias\_step=XX
User bias step size

--svdpp\_usr\_bias\_reg=XX
User bias regularization

--svdpp\_usr\_fctr\_step=XX
User factor step size

--svdpp\_usr\_fctr\_reg=XX
User factor regularization

--svdpp\_item\_fctr\_step=XX
Item factor step size

--svdpp\_item\_fctr\_reg=XX
Item factor regularization

--svdpp\_item\_fctr2\_step=XX
Item factor2 step size

--svdpp\_item\_fctr2\_reg=XX
Item factor2 regularization

--D=X
Feature vector width. Common values are 20 - 150.

> Prediction in Koren’s SVD++ algorithm is computed as follows:

r\_ui = global\_mean\_rating + b\_u + b\_i + q\_u **( p\_i + w\_i )
Where r\_ui is the scalar rating for user u to item i, global\_mean\_rating is the global mean rating, b\_u is a scalar bias for user u, b\_i is a scalar bias for item i, q\_u is a feature vectors of length D for user u, p\_i is a feature vector of length D for item i, and w\_i is an additional feature vector of length D (the weight). The product is a vector product.**

The output of Koren’s SVD++ is 5 output files:
Global mean ratings - include the scalar global mean rating.
user\_bias  - includes a vector with bias for each user
movie\_bias - includes a vector with bias for each movie
matrix U - includes in each row the feature vector q\_u of size D.
matrix V - includes in each row the sum of feature vectors p\_i + w\_i of size D.


Koren time-SVD++
Pros: more accurate than SVD++
Cons: many parameters to tune, prone to numerical errors.

Koren’s time-SVD++ [M](reference.md) takes into account also the temporal aspect of the rating.
Prediction in time-SVD++ algorithm is computed as follows:
> r\_uik = global\_mean\_rating + b\_u + b\_i + ptemp\_u **q\_i + x\_u** z\_k + pu\_u **pt\_i** q\_k

The scalar rating r\_uik (rating for intersection of user u, item i, and time bin k) equals the above sum. Like in Koren’s SVD++ the rating equals the sum of the global mean rating and biases for user and item. The following are feature vectors. For the user we have ptemp\_i , x\_u and pu\_u. All of length D. For the item we have additional three feature vectors: ptemp\_u, x\_u and pu\_u.
For the time bins we have z\_k and q\_k, two feature vectors of size D.


Basic configuration
--timesvdpp\_lrate=XX
Learning rate

--timesvdpp\_beta
Beta parameter

--timesvdpp\_gamma
Gamma parameter

--timesvdpp\_step\_dec
Multiplicative step decrement (0.1 to 1, default 0.9)

--D=X
Feature vector width. Common values are 20 - 150.
Special Note: This is a tensor factorization algorithm. Please don’t forget to prepare a 4 column matrix market format file, with [user](user.md) [item ](.md) [rating ](.md) [time ](.md) in each row.
When running, you should use
1) --matrix\_market\_tokens\_per\_row=4 command line argument. Time should be an integer value between 0 to max\_time.
2) Use the command line argument --K=max\_time+1 to indicate the number of time bins.


Non-negative matrix factorization (NMF)
Non-negative matrix factorization (NMF) is based on Lee and Seung [H](reference.md).
Prediction is computed like in ALS:
> r\_ui = p\_u **q\_i**

Namely the scalar prediction r of user u is composed of the vector product of the user feature vector p\_u (of size D), with the item feature vector q\_i (of size D). The only difference is that both p\_u and q\_i have all nonnegative values.
The output of NMF is two matrices: filename.U and filename.V. The matrix U holds the user feature vectors in each row. (Each vector has exactly D columns). The matrix V holds the feature vectors for each time (Each vector has again exactly D columns). In linear algebra notation the rating matrix R ~ UV, U>=0, V>=0.


Basic configuration
--D=XX
Feature vector width. Common values are 20 - 150.

--max\_iter=XX
Set the number of iterations


NMF cost function and properties
Unlike many of the other methods who is Euclidean distance, NMF cost function is:
> KL( UV’ || A)
Namely the KL divergence between the approximating product UV’  and the original matrix A.
The objective is not computed in GraphLab, but you can easily compute it in Matlab if needed.

NMF is a gradient descent type algorithm which is supposed to always converge. However it may converge to a local minima. The algorithm starts from a random solution and that is why different runs may converge to different solution. For debugging, if you are interested in verifying that multiple runs converge to the same point, use the flag --debug=true when running.
Restricted Bolzman Machines (RBM)
RBM algorithm is detailed in [O](reference.md). It is a MCMC method that works on binary data.
In other words, the ratings have to be binned into a discrete space. For example, for KDD CUP 2011 rating between 0 to 100 can be binned into 10 bins: 0-10, 10-20 etc. rbm\_scaling defines the factor to divide the rating for binning (in the example it is 10). rbm\_bins defines how many bins are there in total. In this example we have 11 bins: 0,1,..,10.

Basic configuration
--rbm\_mult\_step\_dec=XX
Multiplicative step decrement (should be 0.1 to 1, default is 0.9)

--rbm\_alpha=XX
Alpha parameter: gradient descent step size

--rbm\_beta=XX
Beta parameter: regularization

--rbm\_scaling=XX
Scale the rating by dividing it with the rbm\_scaling constant. For example for KDD cup data rating of 0..100 can be scaled to the bins 0,1,2,3,.. 10 by setting the rbm\_scaling=10

--rbm\_bins=XX
Total number of binary bins used. For example in Netflix data where we have 1,2,3,4,5 the number of bins is 5

TBD: prediction in RBM.

TBD: Currently RBM output is not written to file. Ping me if you are interested in using RBM output.
Example RBM on Netflix data
./pmf smallnetflix\_mm 16 --matrixmarket=true --scheduler="round\_robin(max\_iterations=10,block\_size=1)" --rbm\_scaling=1 --rbm\_bins=6 --rbm\_alpha=0.06 --rbm\_beta=.1 --ncpus=8 --minval=1 --maxval=5 --rbm\_mult\_step\_dec=0.8  INFO:     pmf.cpp(do\_main:430): PMF/BPTF/ALS/SVD++/time-SVD++/SGD/Lanczos/SVD Code written By Danny Bickson, CMU Send bug reports and comments to danny.bickson@gmail.com WARNING:  pmf.cpp(do\_main:434): Program compiled with Eigen Support Setting run mode RBM (Restriced Bolzman Machines) INFO:     pmf.cpp(start:306): RBM (Restriced Bolzman Machines) starting  loading data file smallnetflix\_mm Loading Matrix Market file smallnetflix\_mm TRAINING Loading smallnetflix\_mm TRAINING Matrix size is: USERS 95526 MOVIES 3561 TIME BINS 1 INFO:     read\_matrix\_market.hpp(load\_matrix\_market:131): Loaded total edges: 3298163 loading data file smallnetflix\_mme Loading Matrix Market file smallnetflix\_mme VALIDATION Loading smallnetflix\_mme VALIDATION Matrix size is: USERS 95526 MOVIES 3561 TIME BINS 1 INFO:     read\_matrix\_market.hpp(load\_matrix\_market:131): Loaded total edges: 545177 loading data file smallnetflix\_mmt Loading Matrix Market file smallnetflix\_mmt TEST Loading smallnetflix\_mmt TEST skipping file RBM (Restriced Bolzman Machines) for matrix (95526, 3561, 1):3298163.  D=20 INFO:     rbm.hpp(rbm\_init:424): RBM initialization ok complete. Objective=8.37956e-304, TRAIN RMSE=0.0000 VALIDATION RMSE=0.0000. INFO:     pmf.cpp(run\_graphlab:251): starting with scheduler: round\_robin max iterations = 10 step = 1 Entering last iter with 1 5.99073) Iter RBM 1, TRAIN RMSE=0.9242 VALIDATION RMSE=0.9762. Entering last iter with 2 11.0763) Iter RBM 2, TRAIN RMSE=0.9109 VALIDATION RMSE=0.9673. Entering last iter with 3 16.1259) Iter RBM 3, TRAIN RMSE=0.9054 VALIDATION RMSE=0.9633. Entering last iter with 4 21.2074) Iter RBM 4, TRAIN RMSE=0.9015 VALIDATION RMSE=0.9600. Entering last iter with 5 26.3222) Iter RBM 5, TRAIN RMSE=0.8986 VALIDATION RMSE=0.9560. Entering last iter with 6 31.409) Iter RBM 6, TRAIN RMSE=0.8960 VALIDATION RMSE=0.9540. Entering last iter with 7 36.4693) Iter RBM 7, TRAIN RMSE=0.8941 VALIDATION RMSE=0.9508. ...


Factorization Machines (FM)
Factorization machines is a SGD type algorithm. It has two differences relative to bias-SGD:
1) It handles time information by adding feature vectors for each time bin
2) It adds additional feature for the last item rated for each user.
Those differences are supposed to make it more accurate than bias-SGD.
Factorization machines is detailed in reference [P](P.md). There are several variants, here the SGD variant is implemented (and not the ALS).

Prediction in LIBFM is computed as follows:

r\_ui = global\_mean\_rating + b\_u + b\_i + b\_t + b\_li + 0.5\*sum(p\_u.<sup>2 + q_i.</sup>2  + w\_t.<sup>2 + s_li.</sup>2 - (p\_u + q\_i + w\_t + s\_li ).^2)

Where global\_mean\_rating is the global mean rating, b\_u is the bias of user u, b\_i is the bias of item i , b\_t is the bias of time t, b\_li is the bias of the last item li, and p\_u is the feature vector of user u, and q\_i is the feature vector of item i, w\_t is the feature vector of time t, s\_li is the feature vector of last item li. All feature vectors have size of D as in ALS.  .^2 is the element by element power operation (as in matlab).

The output of LIBFM consists of three matrices: filename.Users, filename.Movies and filename.Times. The matrix Users holds the user feature vectors in each row. (Each vector has exactly D columns). The matrix Movies holds the feature vectors for each item (Each vector has again exactly D columns). The matrix Times holds the feature vectors for each time. Additionally, the output consists of four vectors: bias for each user, bias for each item, bias for each time bin and bias for each last item. Last, the global mean rating is also given as output.



Basic configuration
--libfm\_rate=XX
Gradient descent step size

--libfm\_regw=XX
Gradient descent regularization for biases

--libfm\_regv=XX
Gradient descent regularization for feature vectors

--libfm\_mult\_dec=XX
Multiplicative step decrease. Should be between 0.1 to 1.
Default is 0.9

--D=X
Feature vector width. Common values are 20 - 150.


Factorization machines example
./pmf wals 17 --scheduler="round\_robin(max\_iterations=10,block\_size=1)" --ncpus=1 --matrixmarket=true --K=1 --libfm\_rate=0.1
INFO:     pmf.cpp(do\_main:465): (c) PMF/BPTF/ALS/SVD++/time-SVD++/SGD/Lanczos/SVD/bias-SGD/RBM Code written By Danny Bickson, CMU
Send bug reports and comments to danny.bickson@gmail.com
WARNING:  pmf.cpp(do\_main:472): Program compiled with it++ Support
Setting run mode Libfm (factorization machines)
WARNING:  pmf.h(verify\_setup:444): It is recommended to set min and max allowed matrix values to improve prediction quality, using the flags --minval=XX, --maxval=XX
WARNING:  pmf.h(verify\_setup:462): When running tensor based factorization, with matrix market input format, number of matrix market tokens per row should be 4 (each row has [from](from.md) [to](to.md) [val](val.md) [time](time.md)\n format). Use the command line argument --matrix\_market\_tokens\_per\_row=4 to avoid this warning
INFO:     pmf.cpp(start:310): Libfm (factorization machines) starting

loading data file wals
Loading wals TRAINING
Matrix size is: USERS 4 MOVIES 4 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:159): Loaded total edges: 9 global mean: 1.55556
loading data file walse
Loading walse VALIDATION
Matrix size is: USERS 4 MOVIES 4 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:159): Loaded total edges: 6 global mean: 0.583333
Libfm (factorization machines) for tensor (4, 4, 1):9.  D=20
Libfm (factorization machines) 20 factors
INFO:     pmf.cpp(run\_graphlab:253): starting with scheduler: round\_robin
max iterations = 10
step = 1
max\_iterations = 10
INFO:     asynchronous\_engine.hpp(run:111): Worker 0 started.
0.000424) Iter Libfm (factorization machines) 1  TRAIN RMSE=0.7304 VALIDATION RMSE=1.2555 VALIDATION MAE=1.0015.
0.000514) Iter Libfm (factorization machines) 2  TRAIN RMSE=0.6699 VALIDATION RMSE=1.2167 VALIDATION MAE=0.9897.
0.000766) Iter Libfm (factorization machines) 3  TRAIN RMSE=0.6170 VALIDATION RMSE=1.1995 VALIDATION MAE=0.9925.
0.000852) Iter Libfm (factorization machines) 4  TRAIN RMSE=0.5761 VALIDATION RMSE=1.1893 VALIDATION MAE=0.9969.
0.000937) Iter Libfm (factorization machines) 5  TRAIN RMSE=0.5448 VALIDATION RMSE=1.1824 VALIDATION MAE=1.0011.
0.001019) Iter Libfm (factorization machines) 6  TRAIN RMSE=0.5203 VALIDATION RMSE=1.1773 VALIDATION MAE=1.0046.
0.001098) Iter Libfm (factorization machines) 7  TRAIN RMSE=0.5006 VALIDATION RMSE=1.1735 VALIDATION MAE=1.0076.
0.001177) Iter Libfm (factorization machines) 8  TRAIN RMSE=0.4844 VALIDATION RMSE=1.1706 VALIDATION MAE=1.0102.
0.001258) Iter Libfm (factorization machines) 9  TRAIN RMSE=0.4710 VALIDATION RMSE=1.1683 VALIDATION MAE=1.0124.
0.001337) Iter Libfm (factorization machines) 10  TRAIN RMSE=0.4597 VALIDATION RMSE=1.1664 VALIDATION MAE=1.0143.
INFO:     asynchronous\_engine.hpp(run:119): Worker 0 finished.
Finished in 0.001489 seconds
loading data file walst
Loading walst TEST
skipping file
Performance counters are: 0) EDGE\_TRAVERSAL, 0.000582
Performance counters are: 2) CALC\_RMSE\_Q, 1.6e-05
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_vector:327): Saved output vector to file: wals-20-11.out.UserBias vector size: 4
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_vector:328): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_vector:327): Saved output vector to file: wals-20-11.out.MovieBias vector size: 4
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_vector:328): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:277): Saved output matrix to file: wals-20-11.out.Users matrix size: 4 x 20
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:278): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:277): Saved output matrix to file: wals-20-11.out.Movies matrix size: 4 x 20
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:278): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:277): Saved output matrix to file: wals-20-11.out.GlobalMean matrix size: 1 x 1
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:278): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_vector:327): Saved output vector to file: wals-20-11.out.TimeBias vector size: 1
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_vector:328): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:277): Saved output matrix to file: wals-20-11.out.Times matrix size: 1 x 20
INFO:     read\_matrix\_market.hpp(save\_matrix\_market\_matrix:278): You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m

> ### REPORT FOR core() ###
[Numeric](Numeric.md)
ncpus:		1
[Other](Other.md)
affinities:	false
compile\_flags:
engine:	async
scheduler:	round\_robin
schedyield:	true
scope:	edge

> ### REPORT FOR engine() ###
[Numeric](Numeric.md)
num\_edges:		9
num\_syncs:		0
num\_vertices:		9
updatecount:		40
[Timings](Timings.md)
runtime:		0 s
[Other](Other.md)
termination\_reason:	task depletion (natural)
[Numeric](Numeric.md)
updatecount\_vector:		40
updatecount\_vector.values:		40,



Advanced topics
Handling implicit ratings
Implicit rating handles the case where we have only positive examples (for example when a user bought a certain product) but we never have indication when a user DID NOT buy another product. The paper [L](L.md) proposes to add negative examples at random for unobserved user/item pairs. Implicit rating is implemented in the collaborative filtering library and can be used with any of the algorithms explained above.


Basic configuration
--implicitratingtype=user or --implicitratingtype=uniform
Adds implicit ratings proportional to the current user rating, or uniformly to every user.


--implicitratingpercentage
A number between 0 to 1 which determines what is the percentage of edges to add to the sparse model. 0 means none while 1 means fully dense model.


--implicitratingvalue
The value of the rating added. On default it is zero, but you can change it.


--implicitratingweight
Weight of the implicit rating (for WALS).
Time of the explicit rating (for tensor algorithms)

Note: If you use the default implicit ratings value of zero, please don’t forget to run with --zero=true to avoid error when encountering a zero rating.



Setting up initial feature vectors
All of the implemented algorithms are using a random starting state (namely random feature vectors). That is why multiple runs may result in different solutions. If you like to debug the starting state, you can use the --debug=true flag, to force the starting state to be all 0.1.

It is also possible to load a starting state from file (currently supported for the algorithms ALS, weighted-ALS, sparse-ALS, NMF).

You should prepare two files of the type 

&lt;filename&gt;

.U and 

&lt;filename&gt;

.V and use the command line flag --loadfactors=true to load those initialization.
Now it is possible to run a few rounds, save the result and later
continue from the last round result as the starting point.

Example - generating factors
#running pmf to create input factors
./pmf panel7\_mmwritten.dat 8 --matrixmarket=true --D=5 --max\_iter=10
#renaming the factor files so they can be found as input:
mv panel7\_mmwritten.dat-5-1.out.V panel7\_mmwritten.dat.V
mv panel7\_mmwritten.dat-5-1.out.U panel7\_mmwritten.dat.U

Example - loading factors from file
./pmf panel7\_mmwritten.dat 8 --matrixmarket=true --D=5 --max\_iter=10
--loadfactors=true

Some debugging tips
When starting to work on a new dataset, it is always recommended to run with --stats=true and learn the properties of the dataset. Here is an example on a small Netflix subset:
<45|134>bickson@bigbro6:~/newgraphlab/graphlabapi/debug/demoapps/pmf$ ./pmf smallnetflix\_mm 0 --stats=true --matrixmarket=true
INFO:     pmf.cpp(do\_main:452): (c) PMF/BPTF/ALS/SVD++/time-SVD++/SGD/Lanczos/SVD/bias-SGD/RBM Code written By Danny Bickson, CMU
Send bug reports and comments to danny.bickson@gmail.com
WARNING:  pmf.cpp(do\_main:459): Program compiled with it++ Support
Setting run mode ALS\_MATRIX (Alternating least squares)
WARNING:  pmf.h(verify\_setup:421): It is recommended to set min and max allowed matrix values to improve prediction quality, using the flags --minval=XX, --maxval=XX
INFO:     pmf.cpp(start:310): ALS\_MATRIX (Alternating least squares) starting

loading data file smallnetflix\_mm
Loading Matrix Market file smallnetflix\_mm TRAINING
Loading smallnetflix\_mm TRAINING
Matrix size is: USERS 95526 MOVIES 3561 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:145): Loaded total edges: 3298163
loading data file smallnetflix\_mme
Loading Matrix Market file smallnetflix\_mme VALIDATION
Loading smallnetflix\_mme VALIDATION
Matrix size is: USERS 95526 MOVIES 3561 TIME BINS 1
INFO:     read\_matrix\_market.hpp(load\_matrix\_market:145): Loaded total edges: 545177
TRAINING Avg rating: 3.5992 min rating: 1 max rating: 5
TRAINING Avg time:   0 min time:   0 max time:   0
TRAINING User without ratings: 1821 item without ratings: 0
TRAINING Min V: 2.66742e-08 Max V: 0.0223607 Min U: 6.31781e-07, Max U: 0.0223606
TRAINING Negative ratings: 0, Positive ratings: 3298163
VALIDATION Avg rating: 3.66546 min rating: 1 max rating: 5
VALIDATION Avg time:   0 min time:   0 max time:   0
VALIDATION User without ratings: 8352 item without ratings: 52
VALIDATION Min V: 1.68344e-08 Max V: 0.0223607 Min U: 1.1536e-06, Max U: 0.0223605
VALIDATION Negative ratings: 0, Positive ratings: 545177
TEST is missing, skipping data

It is important to verify to ratings range (minimum and maximum) since it can be used to improve predictions by truncating the prediction to the allowed range. This is done by --maxval=XX and --minval=XX command line arguments.
Another useful information is how many users or items are without ratings in the training data. this is called the “cold start” problem of predicting ratings to users we don’t know anything about.
Debugging using GDB
In case you have a problem like an assertion or segmentation fault it is possible to debug using gbp.
1) compile the program in the debug subfolder
2) Run the command:
# gdb pmf
You get something like:
GNU gdb Red Hat Linux (6.6-8.fc7rh)
Copyright (C) 2006 Free Software Foundation, Inc.
GDB is free software, covered by the GNU General Public License, and you are
welcome to change it and/or distribute copies of it under certain conditions.
Type "show copying" to see the conditions.
There is absolutely no warranty for GDB.  Type "show warranty" for details.
This GDB was configured as "x86\_64-redhat-linux-gnu"...
Using host libthread\_db library "/lib64/libthread\_db.so.1".
(gdb)

Now run the full command you use including command line argument. Instead of ./pmf use the command run:
(gdb) run dataset.name 0 --matrixmarket=true --ncpus=1 --scheduler=”round\_robin(max\_iterations=10,block\_size)=1

When the program asserts/ seg fault you can type the command: “where” to see which is the problematic line. Send the output of “where” to our user mailing list .
Compilation error
When you encounter a compilation error, please email our user mailing list with the following information:
●	Operating system
●	Version of graphlab
●	Output of gcc --version
●	Output of g++ --version
●	Output of cmake --version
●	Optional: if you are using it++, the output of itpp-config --libs --cflags
●	The full configure command line argument and its output
●	The full compilation output

Run time error
In case of a runtime error / assertion, please email our user mailing list :
●	Your graphlab version
●	The full command line argument you used
●	If possible, a small example or the dataset you are using so we can reproduce this error.


Performance counters
Performance counters gives timing statistics about the time spent in different parts of the program. For example, for ALS run we can get something like:
Performance counters are: 0) EDGE\_TRAVERSAL, 4.8861
Performance counters are: 2) CALC\_RMSE\_Q, 0.01031
Performance counters are: 3) ALS\_LEAST\_SQUARES, 10.0902
Performance counters are: 6) CALC\_OBJ, 0.223835

0) - counter ID
EDGE\_TRAVERSAL - counter name
4.8861 - time spent in seconds
Other examples
Further examples are found in the datasets and benchmark page.

References

A) Liang Xiong, Xi Chen, Tzu-Kuo Huang, Jeff Schneider, Jaime G. Carbonell, Temporal Collaborative Filtering with Bayesian Probabilistic Tensor Factorization. In Proceedings of SIAM Data Mining, 2010. html(source code is also available).

B) Salakhutdinov and Mnih, Bayesian Probabilistic Matrix Factorization using Markov Chain Monte Carlo. in International Conference on Machine Learning, 2008. pdf project website, since our code implements matrix factorization as a sepcial case of a tensor as well.

C) Alternating least squares: Yunhong Zhou, Dennis Wilkinson, Robert Schreiber and Rong Pan. Large-Scale Parallel Collaborative Filtering for the Netflix Prize. Proceedings of the 4th international conference on Algorithmic Aspects in Information and Management. Shanghai, China pp. 337-348, 2008. pdf

D) SVD++ algorithm: Koren, Yehuda. "Factorization meets the neighborhood: a multifaceted collaborative filtering model." In Proceeding of the 14th ACM SIGKDD international conference on Knowledge discovery and data mining, 426434. ACM, 2008. http://portal.acm.org/citation.cfm?id=1401890.1401944

E) SGD (stochastic gradient descent) algorithm: Matrix Factorization Techniques for Recommender Systems Yehuda Koren, Robert Bell, Chris Volinsky In IEEE Computer, Vol. 42, No. 8. (07 August 2009), pp. 30-37.
F) Tikk, D. (2009). Scalable Collaborative Filtering Approaches for Large Recommender Systems. Journal of Machine Learning Research, 10, 623-656.

G) For Lanczos algorithm (SVD) see: wikipedia.

H) For NMF (non-negative matrix factorization) see: Lee, D..D., and Seung, H.S., (2001), 'Algorithms for Non-negative Matrix Factorization', Adv. Neural Info. Proc. Syst. 13, 556-562.

I) For Weighted-Alternating least squares: Collaborative Filtering for Implicit Feedback Datasets Hu, Y.; Koren, Y.; Volinsky, C. IEEE International Conference on Data Mining (ICDM 2008), IEEE (2008).
J) Pan, Yunhong Zhou, Bin Cao, Nathan N. Liu, Rajan Lukose, Martin Scholz, and Qiang Yang. 2008. One-Class Collaborative Filtering. In Proceedings of the 2008 Eighth IEEE International Conference on Data Mining (ICDM '08). IEEE Computer Society, Washington, DC, USA, 502-511.

K) For sparse factor matrices see: Xi Chen, Yanjun Qi, Bing Bai, Qihang Lin and Jaime Carbonell. Sparse Latent Semantic Analysis. In SIAM International Conference on Data Mining (SDM), 2011.

K1) D. Needell, J. A. Tropp CoSaMP: Iterative signal recovery from incomplete and inaccurate samples Applied and Computational Harmonic Analysis, Vol. 26, No. 3. (17 Apr 2008), pp. 301-321.

L) For SVD see Wikipedia

M) For time-SVD++, see Yehuda Koren. 2009. Collaborative filtering with temporal dynamics. In Proceedings of the 15th ACM SIGKDD international conference on Knowledge discovery and data mining (KDD '09). ACM, New York, NY, USA, 447-456. DOI=10.1145/1557019.1557072

N) For bias-SVD
Y. Koren. Factorization Meets the Neighborhood: a Multifaceted Collaborative Filtering Model. Equation (5), pdf.

O) For RBM:
G. Hinton. A Practical Guide to Training Restricted Boltzmann Machines. University of Toronto Tech report UTML TR 2010-003 pdf.

P) For LIBFM:
Steffen Rendle (2010): Factorization Machines, in Proceedings of the 10th IEEE International Conference on Data Mining (ICDM 2010), Sydney, Australia. pdf

Acknowledgements
As the project is growing, the list of people we should thank is growing..
Liang Xiong, CMU for providing the Matlab code of BPTF, numerous discussions and infinite support!! Thanks!!
Timmy Wilson, Smarttypes.org for providing twitter network snapshot example, and Python scripts for reading the output.
Sanmi Koyejo, from the University of Austin, Texas, for providing Python scripts for preparing the inputs.
Dan Brickely, from VU University Amsertdam, for helping debugging installation and prepare the input in Octave.
Nicholas Ampazis, University of the Aegean, for providing his SVD++ source ode.
Yehuda Koren, Yahoo! Research, for providing his SVD++ source code implementation.
Marinka Zitnik, University of Ljubljana, Slovenia, for helping debugging ALS and suggesting NMF algos to implement.
Joel Welling from Pittsburgh Supercomputing Center, for optimizing GraphLab on BlackLight supercomputer and simplifying installation procedure.
Sagar Soni from Gujarat Technological University and Hasmukh Goswami College of Engineering for helping testing the code.
Young Cha, UCLA for testing the code.
Mohit Singh for helping improve documentation.
Nicholas Kolegraff for testing our examples.
Theo Throuillon, Ecole Nationale Superieure d'Informatique et de Mathematiques Appliquees de Grenoble for debugging NMF.
Qiang Yan, Chinese Academy of Science for providing time-svd++, bias-SVD, RBM and LIBFM code that the Graphlab version is based on.
Ramakrishnan Kannan, Georgia Tech, for helping debugging and simplifying usage.
Charles Martin, GLG, for debugging NMF.


Appendix: Matrix Market Format
Matrix Market is a very simple format devised by NIST to store different types of matrices.

For GraphLab matrix libraries: linear solvers, matrix factorization and clustering we recommend using this format for the input file. Once this format is used for the input, the same format will be used for the output files.

TIP: When using graphlab, don't forget to use --matrixmarket=true command line flag.

Sparse matrices
Matrix market has a very simple format: 2 header lines. Here are some examples. Here is
example 3x4 matrix:

A =  0.8147         0    0.2785         0 0.9058    0.6324    0.5469    0.1576 0.1270    0.0975    0.9575         0

And here is the matching matrix market file:

%%MatrixMarket matrix coordinate real general % Generated 27-Feb-2012 3 4 9 1 1  0.8147236863931789 2 1  0.9057919370756192 3 1  0.1269868162935061 2 2  0.6323592462254095 3 2  0.09754040499940952 1 3  0.2784982188670484 2 3  0.5468815192049838 3 3  0.9575068354342976 2 4  0.1576130816775483

The first line, include the header. coordinate means this is a sparse matrix.
The third row includes the matrix size: 3 4 9 means 3 rows, 4 columns and 9 non zero entries.
The rest of the rows include the edges. For example the 4th row is:
1 1 0.8147236863931789, namely means that it is the first row, first column and its value.

TIP: Sparse matrices should NOT include zero entries!
For example, the row 1 1 0 is not allowed!
TIP: First two numbers in each non-zero entry line should be integers and not double notation. For example 1e+2 is not a valid row/col number. It should be 100 instead.
TIP: Row/column number always start from one (and not from zero as in C!)
TIP: It is not legal to include the same non zero entry twice. In GraphLab it will result in a duplicate edge error. Note that the number of edges in GraphLab starts in zero, so you have to add one to source and target values to detect the edge in the matrix market file.

Dense matrices:
Here is an example on how to save the same matrix as dense matrix:

A =  0.8147         0    0.2785         0 0.9058    0.6324    0.5469    0.1576 0.1270    0.0975    0.9575         0


%%MatrixMarket matrix array real general 3 4
0.8147
0.9058  0.1269  0  0.6323  0.0975  0.2784  0.5468  0.9575  0
0.1576
0

Symmetric sparse matrices:
Here is an example for sparse symmetric matrix:
B =      1.5652         0    1.4488          0    2.0551    2.1969     1.4488    2.1969    2.7814

And here is the matching matrix market file:
%%MatrixMarket matrix coordinate real symmetric % Generated 27-Feb-2012 3 3 5 1 1  1.5652 3 1  1.4488 2 2  2.0551 3 2  2.1969 3 3  2.7813

Note that each non-diagonal edges is written only once.

Sparse vectors:
Here is an example for sparse vector:
v =       1     0     0     1

%%MatrixMarket matrix coordinate real general % Generated 27-Feb-2012 1 4 2 1 1 1 1 4 1

Working with matlab:
download the files http://graphlab.org/mmwrite.m and http://graphlab.org/mmread.m
In Matlab you can save a dense matrix using:
>> mmwrite('filename', full(matrixname));
And save a sparse matrix using:
>> mmwrite('filename', sparse(matrixname));
For reading a sparse or dense matrix you can:
>> A = mmread('filename');

Writing a conversion function in Java
This section explains how to convert Mahout 0.4 sequence vectors into matrix market format.

Create a file named Vec2mm.java with the following content:
import java.io.BufferedWriter; import java.io.FileWriter; import java.util.Iterator;  import org.apache.mahout.math.SequentialAccessSparseVector; import org.apache.mahout.math.Vector; import org.apache.mahout.math.VectorWritable; import org.apache.hadoop.conf.Configuration; import org.apache.hadoop.fs.FileSystem; import org.apache.hadoop.fs.Path; import org.apache.hadoop.io.IntWritable; import org.apache.hadoop.io.SequenceFile;  /  **Code for converting Hadoop seq vectors to matrix market format** @author Danny Bickson, CMU  /  public class Vec2mm{      public static int Cardinality;    /    @param args[0](0.md) - input svd file   **@param args[1](1.md) - output matrix market file** @param args[2](2.md) - number of rows   **@param args[3](3.md) - number of columns** @param args[4](4.md) - number oi non zeros   **@param args[5](5.md) - transpose**/  public static void main(String[.md](.md) args){      try {      if (args.length != 6){         System.err.println(Usage: java Vec2mm <input seq vec file> < output matrix market file> <number of rows> <number of cols> <number of non zeros> <transpose output>);            System.exit(1);         }          final Configuration conf = new Configuration();         final FileSystem fs = FileSystem.get(conf);         final SequenceFile.Reader reader = new SequenceFile.Reader(fs, new Path(args[0](0.md)), conf);         BufferedWriter br = new BufferedWriter(new FileWriter(args[1](1.md)));         int rows = Integer.parseInt(args[2](2.md));         int cols = Integer.parseInt(args[3](3.md));         int nnz = Integer.parseInt(args[4](4.md));         boolean transpose = Boolean.parseBoolean(args[5](5.md));         IntWritable key = new IntWritable();         VectorWritable vec = new VectorWritable();         br.write("%%MatrixMarket matrix coordinate real general\n");             if (!transpose)           br.write(rows + " " +cols + " " + nnz + "\n");         else br.write(cols + " " + rows + " " +  nnz + "\n");         while (reader.next(key, vec)) {           //System.out.println("key " + key);           SequentialAccessSparseVector vect = (SequentialAccessSparseVector)vec.get();           //System.out.println("key " + key + " value: " + vect);           Iterator iter = vect.iterateNonZero();            while(iter.hasNext()){             Vector.Element element = iter.next();            //note: matrix market offsets starts from one and not from zero            if (!transpose)              br.write((element.index()+1) + " " + (key.get()+1)+ " " + vect.getQuick(element.index())+"\n");            else               br.write((key.get()+1) + " " + (element.index()+1) + " " + vect.getQuick(element.index())+"\n");            }        }           reader.close();        br.close();    } catch(Exception ex){      ex.printStackTrace();    }  } }
Compile this code using the command:
javac -cp /mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/lib/core-3.1.1.jar:/mnt/bigbrofs/usr7/bickson/mahout-0.4/taste-web/target/mahout-taste-webapp-0.5-SNAPSHOT/WEB-INF/lib/mahout-core-0.5-SNAPSHOT.jar:/mnt/bigbrofs/usr7/bickson/mahout-0.4/taste-web/target/mahout-taste-webapp-0.5-SNAPSHOT/WEB-INF/lib/mahout-math-0.5-SNAPSHOT.jar:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/lib/commons-cli-1.2.jar:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/hadoop-0.20.2-core.jar **.java**

Now run it using the command:
java -cp .:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/lib/core-3.1.1.jar:/mnt/bigbrofs/usr7/bickson/mahout-0.4/taste-web/target/mahout-taste-webapp-0.5-SNAPSHOT/WEB-INF/lib/mahout-core-0.5-SNAPSHOT.jar:/mnt/bigbrofs/usr7/bickson/mahout-0.4/taste-web/target/mahout-taste-webapp-0.5-SNAPSHOT/WEB-INF/lib/mahout-math-0.5-SNAPSHOT.jar:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/lib/commons-cli-1.2.jar:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/hadoop-0.20.2-core.jar:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/lib/commons-logging-1.0.4.jar:/mnt/bigbrofs/usr7/bickson/hadoop-0.20.2/lib/commons-logging-api-1.0.4.jar:/mnt/bigbrofs/usr7/bickson/mahout-0.4/taste-web/target/mahout-taste-webapp-0.5-SNAPSHOT/WEB-INF/lib/google-collections-1.0-rc2.jar Vec2mm A.seq A.mm 25 100 2500 false

Where A.seq is the input file in SequentialAccessSparseVector key/value store. A.mm
will be the resulting output file in matrix market format. 25 is the number of rows, 100 columns, and 2500 non zeros. false - not to transpose the resulting matrix.

Depends on the saved vector type, you may want to change my code from SequentialAccessSparseVector to the specific type you need to convert.