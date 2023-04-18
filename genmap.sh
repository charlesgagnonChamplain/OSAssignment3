#!/bin/bash

# set the default map dimensions
WIDTH=50
HEIGHT=50

# check if command line arguments were provided
if [[ $# -eq 1 ]]; then
    WIDTH=$1
elif [[ $# -eq 2 ]]; then
    WIDTH=$1
    HEIGHT=$2
fi

# define function to generate a random ASCII character
function genchar() {
    #create a character
    char=$(($RANDOM % 93 + 33))
    #check if character is equal to \ and reset if true
    while [[ $char -eq 92 ]]; do
        char=$(($RANDOM % 93 + 33))
    done
    
    #return character
    printf \\$(printf '%03o' $char)
}

# set the maximum number of map generation attempts
MAX_ATTEMPTS=10
attempts=0

# loop until a valid map is generated or the maximum attempts are reached
while [[ $attempts -lt $MAX_ATTEMPTS ]]; do
    # create an empty map
    map=''
    # fill the map with random ASCII characters
    for (( y=1; y<=$HEIGHT; y++ )); do
        # create a temporary line to count blank spaces
        line=''
        for (( x=1; x<=$WIDTH; x++ )); do
            line+="$(genchar)"
        done
        # count the number of blank spaces in the line
        num_blanks=$(echo "$line" | tr -cd '[:blank:]' | wc -c)
        # check if the line has enough non-blank characters
        if [[ $num_blanks -le $(($WIDTH/2)) ]]; then
            map+="$line\n"
        else
            # force regeneration of the entire map
            break
        fi
    done

    # check if the map has enough non-blank characters
    num_blanks=$(echo "$map" | tr -cd '[:blank:]' | wc -c)
    if [[ $num_blanks -lt $(($WIDTH * $HEIGHT / 2)) ]]; then
        # print the map to STDOUT and exit
        echo -e "$map"
        exit 0
    else
        # increment the number of attempts
        attempts=$((attempts+1))
    fi
done

# if the maximum number of attempts is reached, print an error message to STDERR and exit with error code
echo "Error: could not generate a map with at least 50% non-blank characters after $MAX_ATTEMPTS attempts" >&2
exit 1
