#!/bin/bash

# Set the port number
PORT=${1:-4020}

# Change to area directory.
cd ../area || { echo "Directory ../area not found!"; exit 1; }

# Set limits
ulimit -c unlimited
rm -f shutdown.txt

while true; do
    # Find the next available log index
    INDEX=1000
    while [ -e "../log/$INDEX.log" ]; do
        ((INDEX++))
    done
    LOGFILE="../log/$INDEX.log"

    # Record starting time
    date > "$LOGFILE"
    date > boot.txt

    # Check if port is already in use using ss (more modern than netstat)
    if ss -tuln | grep -q ":$PORT "; then
        echo "Port $PORT is already in use."
        exit 1
    fi

    # Run SMAUG
    ../src/smaug "$PORT" >> "$LOGFILE" 2>&1

    # Check for clean shutdown
    if [ -f "shutdown.txt" ]; then
        rm -f shutdown.txt
        exit 0
    fi

    # Wait before restarting
    sleep 5
done
