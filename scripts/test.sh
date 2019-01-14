#!/usr/bin/env bash
#B="http://localhost:8080"
siege -r 10 -c 50 -f urls.txt
#wrk -t 1 -d 1 -c 10 "${URL}"
