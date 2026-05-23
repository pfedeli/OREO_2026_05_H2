#!/bin/bash

# Define directories
SOURCE_DIR="/home/pfedeli/BeamTests/OREO_2026_03_H8/mntdaq"
ASCII_DIR="/eos/project/c/crystal-ferrara/public/OREO/TB-DATA/TB_H8_2026_03_OREO/asciidir"
ROOT_DIR="/eos/project/c/crystal-ferrara/public/OREO/TB-DATA/TB_H8_2026_03_OREO/rootdir"
MERGED_DIR="/eos/project/c/crystal-ferrara/public/OREO/TB-DATA/TB_H8_2026_03_OREO/recorootdir"

# Time to wait between each check (in seconds)
SLEEP_TIME=10

# Ensure output directories exist
#mkdir -p "$ASCII_DIR" "$ROOT_DIR" "$MERGED_DIR"

# ---------------------------------------------------------
# Recovery Mode (triggered if arguments are passed)
# ---------------------------------------------------------
if [ "$#" -gt 0 ]; then
    echo "Recovery mode activated. Processing specific runs: $@"
    
    for run_number in "$@"; do
        echo "--- Recovering run $run_number ---"
        
        # Loop through all spills for this specific run
        for file in "$SOURCE_DIR"/run${run_number}_*.dat; do
            # Skip if file does not exist
            [ -e "$file" ] || continue
            
            # Extract names
            filename=$(basename "$file")
            name_noext="${filename%.dat}"
            run_spill="${name_noext#run}"
            
            echo "Running ROOT macro for spill: $run_spill"
            root -l -b -q "Recoascii.C(\"$run_spill\")"
        done
        
        # Merge the recovered files
        echo "Merging root files for run $run_number..."
        hadd -f "$MERGED_DIR/run${run_number}.root" "$ROOT_DIR"/run${run_number}_*.root
        
        echo "Recovery for run $run_number completed."
    done
    
    echo "All requested recoveries finished! Exiting."
    exit 0
fi

# ---------------------------------------------------------
# Normal continuous monitoring loop
# ---------------------------------------------------------
echo "Starting standard DAQ monitoring..."
echo "Press [CTRL+C] to stop."

while true; do
    # 1. Sync the .dat files from the source directory
    # (Comment this out if you are just testing locally in ascii_temp)
    rsync -avz --include="run*_*.dat" --exclude="*" "$SOURCE_DIR/" "$ASCII_DIR/"

    updated_runs=""

    # 2. Process all .dat files found in the ascii directory
    for file in "$ASCII_DIR"/run*_*.dat; do
        [ -e "$file" ] || continue

        filename=$(basename "$file")
        name_noext="${filename%.dat}"
        run_spill="${name_noext#run}"
        run_number="${run_spill%_*}"

        root_file="$ROOT_DIR/run${run_spill}.root"

        if [ ! -f "$root_file" ]; then
            echo "Processing new spill: $run_spill"
            root -l -b -q "Recoascii.C(\"$run_spill\")"

            if [[ ! " $updated_runs " =~ " $run_number " ]]; then
                updated_runs="$updated_runs $run_number"
            fi
        fi
    done

    # 3. Merge the root files for any run that received new data
    for run in $updated_runs; do
        echo "Merging root files for run $run..."
        hadd -f "$MERGED_DIR/run${run}.root" "$ROOT_DIR"/run${run}_*.root
    done

    # Wait before checking again
    sleep $SLEEP_TIME
done