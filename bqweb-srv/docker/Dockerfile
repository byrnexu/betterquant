FROM ubuntu:20.04
RUN apt update 2>/dev/null | grep packages | cut -d '.' -f 1
RUN apt install -y openssl > /dev/null 2> /dev/null
RUN apt install -y libcurl4
RUN apt install -y libjsoncpp-dev
RUN apt install -y libc-ares-dev
RUN apt install -y uuid-dev
RUN apt install -y doxygen
WORKDIR /usr/bin
ADD bin/ /usr/bin
RUN ldconfig
