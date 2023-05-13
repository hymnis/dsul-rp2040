#!/usr/bin/env bash

docker run -d -it --name pico-sdk --mount type=bind,source=${PWD},target=/home/dev lukstep/raspberry-pi-pico-sdk:latest
docker exec pico-sdk apk add clang-extra-tools
docker exec -it pico-sdk /bin/sh
echo "Now connect to 'pico-sdk' container from VSCode"
