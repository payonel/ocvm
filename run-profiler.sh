make prof=1 && env LD_PRELOAD=../gperftools-2.5/.libs/libprofiler.so LD_LIBRARY_PATH=~/code/gperftools-2.5/.libs CPUPROFILE_FREQUENCY=10000 CPUPROFILE=ocvm.prof ./ocvm-profiled tmp --frame=basic

