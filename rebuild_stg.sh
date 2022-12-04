set -xue

cd lib
ls | grep stg | xargs -t -i rm -rf {}
cd -

cd bin
ls | grep stg | xargs -t -i rm -rf {}
cd -

cd bqstg/bqstgengimpl && bash build-proj.sh && cd -
cd bqstg/bqstgeng-cxx && bash build-proj.sh && cd -
cd bqstg/bqstgeng-cxx && bash build-proj.sh && cd -

cd bqstg/bqstgeng-cxx && bash build-proj.sh && cd -
cd bqstg/bqstgeng-cxx-demo && bash build-proj.sh && cd -

cd bqstg/bqstgeng-py  && bash build-proj.sh && cd -
cd bqstg/bqstgeng-py-demo  && bash build-proj.sh && cd -
