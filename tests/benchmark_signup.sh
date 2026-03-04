#!/bin/bash
# Use this script to benchmark DB user creation handling.
start_time=$SECONDS
for i in $(seq -w 0 9999); do
	curl -s -X POST http://localhost:8000/register \
		-H "Content-Type: application/json" \
		-d "{\"username\":\"$i\",\"password\":\"$i\"}"
		echo "Tried user: $i"
done
elapsed_time=$(( SECONDS - start_time ))
echo "Elapsed Time: $elapsed_time seconds"
