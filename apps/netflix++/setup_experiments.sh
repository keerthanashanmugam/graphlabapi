home="/home/haijieg"
glhome="$home/graphlab/graphlab2.2"
graph="/data/netflix/features/splits"
mvname="/data/netflix/meta/moviename_escaped.txt"
genre="/data/netflix/features/feat_netflix_genre.csv"
topics="/data/netflix/features/feat_netflix_topic.csv"
bin="$glhome/release/apps/netflix++/netflix_main"
iter=30

outdirbase="$glhome/apps/netflix++/output"

nlatent_list=( 5 10 )
lambda_list=( 0.01 0.1 1 )
use_bias=( 0 1 )
use_feature_weights=( 0 1 )
# use_feature_factor=( 0 1 )
use_feature_factor=( 1 )

if [ -d "$outdirbase" ]
then
  mv "$outdirbase" "$outdirbase.orig"
fi

for NLATENT in "${nlatent_list[@]}"
do
  for LAMBDA in "${lambda_list[@]}"
  do
    for USEBIAS in "${use_bias[@]}"
    do
      for USEFW in "${use_feature_weights[@]}"
      do
        for USEFF in "${use_feature_factor[@]}"
        do
          outdir="$outdirbase/D=${NLATENT}_iter=${iter}_lambda=${LAMBDA}";
          if [ "$USEBIAS" -gt "0" ] 
          then
            outdir=$outdir"_w0"
          fi
          if [ "$USEFW" -gt "0" ]
          then
            outdir=$outdir"_fw"
          fi
          if [ "$USEFF" -gt "0" ]
          then
            outdir=$outdir"_ff"
          fi
          mkdir -p "$outdir"
          cmd="$bin --matrix=$graph \
            --topic_feature=$topics \
            --truncate=false \
            --D=$NLATENT \
            --lambda=$LAMBDA \
            --lambda2=0.001 \
            --movielist=$mvname \
            --use_bias=$USEBIAS \
            --use_local_bias=false \
            --use_als=true \
            --use_feature_weights=$USEFW \
            --use_feature_latent=$USEFF \
            --max_iter=$iter \
            --interactive=false \
            --saveprefix="$outdir/result"
          --testpercent=-1 2>&1 | tee $outdir/out.0"
          echo $cmd > "$outdir/run.sh"
          chmod u+x "$outdir/run.sh"
        done
      done
    done
  done
done
echo 'for dir in D=*; do $dir/run.sh; done' > "$outdirbase/runall.sh"
chmod u+x "$outdirbase/runall.sh"
