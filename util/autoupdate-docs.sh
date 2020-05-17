#!/bin/bash

set -e

# This script is run by the CI setup to autoupdate our documentation

mkdir -p docs
git clone -b gh-pages git://github.com/robojackets/robocup-firmware docs/
make docs
cd docs/
git add --all
git commit -m 'auto-updated documentation'
git push https://$GH_TOKEN@github.com/robojackets/robocup-firmware gh-pages
