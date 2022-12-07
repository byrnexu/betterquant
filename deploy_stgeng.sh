tar -cjvf stgeng-$(git describe 2>/dev/null || echo v1.0.0-alpha.3).tar.bz2 inc/ lib/libbqstgeng-cxx.a bin/bqstgeng.so bin/stgeng.py
