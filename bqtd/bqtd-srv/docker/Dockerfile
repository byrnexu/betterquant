FROM ubuntu:20.04
RUN apt update 2>/dev/null | grep packages | cut -d '.' -f 1
RUN apt install -y openssl > /dev/null 2> /dev/null
WORKDIR /usr/bin
ADD bin/ .
CMD bqtd-srv --conf=config/bqtd-srv/bqtd-srv.yaml
