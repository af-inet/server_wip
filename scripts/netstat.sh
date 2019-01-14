#!/usr/bin/env bash
TIME_WAIT_COUNT=$(netstat -f inet -n | grep tcp | wc -l)
printf "TIME_WAIT_COUNT: ${TIME_WAIT_COUNT}\n"
