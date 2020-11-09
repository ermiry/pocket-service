ARG CERVER_VERSION=2.0b-12

FROM gcc as builder

WORKDIR /opt

RUN apt-get update && apt-get install -y cmake libssl-dev

# cerver
ARG CERVER_VERSION
RUN mkdir /opt/cerver && cd /opt/cerver \
    && wget -q https://github.com/ermiry/cerver/archive/${CERVER_VERSION}.zip \
    && unzip ${CERVER_VERSION}.zip \
    && cd cerver-${CERVER_VERSION} \
    && make -j4

# mongo c driver
ARG MONGOC_VERSION=1.15.1

RUN mkdir /opt/mongoc && cd /opt/mongoc \
    && wget -q https://github.com/mongodb/mongo-c-driver/releases/download/${MONGOC_VERSION}/mongo-c-driver-${MONGOC_VERSION}.tar.gz \
    && tar xzf mongo-c-driver-${MONGOC_VERSION}.tar.gz \
    && cd mongo-c-driver-${MONGOC_VERSION} \
    && mkdir cmake-build && cd cmake-build \
    && cmake -D ENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -D CMAKE_BUILD_TYPE=Release -D ENABLE_SSL=OPENSSL -D ENABLE_SRV=ON .. \
    && make -j4 && make install

# pocket
WORKDIR /opt/tiny-pocket-api
COPY . .
RUN make -j4

############
FROM ubuntu:bionic

ARG RUNTIME_DEPS='wget libssl-dev'

RUN apt-get update && apt-get install -y ${RUNTIME_DEPS} && apt-get clean

# cerver files
ARG CERVER_VERSION
COPY --from=builder /opt/cerver-${CERVER_VERSION}/bin/libcerver.so /usr/local/lib/
COPY --from=builder /opt/cerver-${CERVER_VERSION}/include/cerver /usr/local/include/cerver

# mongoc files
COPY --from=builder /usr/lib/x86_64-linux-gnu/libicudata.so.63 /usr/lib/x86_64-linux-gnu/libicudata.so.63
COPY --from=builder /usr/lib/x86_64-linux-gnu/libicuuc.so.63 /usr/lib/x86_64-linux-gnu/libicuuc.so.63
COPY --from=builder /usr/local/include/ /usr/local/include/
COPY --from=builder /usr/local/lib/libmongoc-1.0.so /usr/local/lib/libmongoc-1.0.so
COPY --from=builder /usr/local/lib/libbson-1.0.so /usr/local/lib/libbson-1.0.so

RUN ldconfig

# pocket
WORKDIR /home/pocket
COPY ./start.sh .
RUN chmod 777 start.sh
COPY --from=builder /opt/tiny-pocket-api/bin ./bin

CMD ["./start.sh"]