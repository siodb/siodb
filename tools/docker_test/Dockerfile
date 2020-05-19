FROM ubuntu:18.04

RUN echo "*** PREPARING OS ***" \
    && echo "=== Switching to PL archives" \
    && sed -i 's/archive.ubuntu.com/pl.archive.ubuntu.com/g' /etc/apt/sources.list \
    && echo "=== Updating repositories" \
    && apt-get update \
    && echo "=== Upgarding system" \
    && apt-get -y dist-upgrade \
    && echo "=== Installing some programs" \
    && apt-get install -y gdb mc sudo \
    && echo "=== Installing required libraries" \
    && apt-get install -y libboost-filesystem1.65.1 libboost-log1.65.1 \
    libboost-thread1.65.1 libboost-system1.65.1 libboost-program-options1.65.1 \
    libssl1.1 libc6 libstdc++6 libgcc1 zlib1g libtinfo5 \
    && echo "=== Cleaning up APT cache" \
    && apt-get clean -y \
    \
    && echo "*** ADDING SIODB TO CONTAINER ***" \
    && mkdir -p /usr/local/lib \
    && mkdir -p /opt/siodb/bin \
    && echo Done RUN1

COPY lib/libantlr4-runtime.so.4.8 /usr/local/lib
COPY lib/libprotobuf.so.21 /usr/local/lib
COPY bin /opt/siodb/bin

RUN sudo ldconfig

WORKDIR /opt/siodb/bin
ENTRYPOINT /bin/bash
