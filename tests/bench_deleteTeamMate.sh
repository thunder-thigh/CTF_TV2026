#!/bin/bash
# Use this script to benchmark DB team member deleting handling.
start_time=$SECONDS
echo "Trying updating team members 0-3 for teams 000-999"

for i in $(seq -w 0 999); do
    for member in 0 1 2 3; do
        curl -s -X DELETE "http://localhost:8000/team/member/$member" \
            -H "Content-Type: application/json" \
            -d "{\"team_name\":\"$i\",\"password\":\"$i\"}"

        echo "Tried updating member $member for team: $i"
        sleep 0
    done
done

elapsed_time=$(( SECONDS - start_time ))
echo "Elapsed Time: $elapsed_time seconds"
