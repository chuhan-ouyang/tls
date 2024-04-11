#!/bin/bash

# Check init page
# curl localhost:8000/cascade

# put [k1, v5]
curl localhost:8000/cascade/put -d '[1, 5]'

# get k1
curl localhost:8000/cascade/get -d '[1]'