#!/bin/bash

# ensure a clean build
make clean

# remove any active container
sudo docker kill $(sudo docker ps -q)

# compile docker
sudo docker build -t ermiry/tiny-pocket-api:test -f Dockerfile.test .

# compile tests
make TYPE=test -j4 test || { exit 1; }

# run
sudo docker run \
  -d \
  --name pocket --rm \
  -p 5000:5000 --net ermiry \
  -v /home/ermiry/Documents/ermiry/website/tiny-pocket-api/keys:/home/pocket/keys \
  -e RUNTIME=test \
  -e PORT=5000 \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  -e CERVER_CONNECTION_QUEUE=4 \
  -e MONGO_APP_NAME=pocket -e MONGO_DB=ermiry \
  -e MONGO_URI=mongodb://handler:handlerpassword@192.168.100.39:27017/ermiry \
  -e PRIV_KEY=/home/pocket/keys/key.key -e PUB_KEY=/home/pocket/keys/key.pub \
  -e ENABLE_USERS_ROUTES=TRUE \
  ermiry/tiny-pocket-api:test

sleep 2

sudo docker inspect pocket --format='{{.State.ExitCode}}' || { exit 1; }

# users
./test/bin/users || { exit 1; }

# categories
./test/bin/categories || { exit 1; }

# places
./test/bin/places || { exit 1; }

# transactions
./test/bin/transactions || { exit 1; }
