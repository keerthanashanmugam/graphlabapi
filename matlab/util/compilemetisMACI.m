%% Need to recompile metis with gcc-4.0 instead of gcc-4.2

mex -largeArrayDims -I../../extern -L../../release/extern/metis -v -lmetis metiscut.cpp 


% mex -I/opt/metis-4.0/include ...
%     -L/opt/metis-4.0/lib ...
%     -lmetis ...
%     -largeArrayDims ...
%     metiscut.cpp
