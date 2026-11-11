#!/bin/bash
# Use this script to benchmark DB team member updating handling.
start_time=$SECONDS
echo "Trying updating team members 0-3 for teams 000-999"

for i in $(seq -w 0 999); do
    for member in 0 1 2 3; do
        curl -s -X PATCH "http://localhost:8000/team/member/$member" \
            -H "Content-Type: application/json" \
            -d "{\"team_name\":\"$i\",\"password\":\"$i\",\"mem_name\":\"aashu\",\"mem_enrol\":\"0108CS241123\"}"

        echo "Tried updating member $member for team: $i"
        sleep 0
    done
done

elapsed_time=$(( SECONDS - start_time ))
echo "Elapsed Time: $elapsed_time seconds"
