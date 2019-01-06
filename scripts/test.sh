#!/usr/bin/env bash

URL='http://localhost:8080/'

siege -r 10 -c 50 "${URL}"
#wrk -t 1 -d 1 -c 50 "${URL}"
