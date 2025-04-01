#! /bin/bash

if ! [[ -x msh ]]; then
    echo "msh executable does not exist"
    exit 1
fi

../tester/run-tests.sh $*


