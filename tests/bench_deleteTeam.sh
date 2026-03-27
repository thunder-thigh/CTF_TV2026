#!/bin/bash
# Use this script to benchmark DB user creation handling.
start_time=$SECONDS
echo "Trying deleting teams 0-999"
for i in $(seq -w 0 999); do
	curl -s -X POST http://localhost:8000/delete_team \
		-H "Content-Type: application/json" \
		-d "{\"team_name\":\"$i\",\"password\":\"$i\"}"
		echo "Tried deleting team: $i"
	sleep 0
done
elapsed_time=$(( SECONDS - start_time ))
echo "Elapsed Time: $elapsed_time seconds"
