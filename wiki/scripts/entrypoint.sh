#!/usr/bin/env sh
set -eu

runtime_directory="/wiki-runtime"
seed_directory="/wiki-seed"

if [ ! -d "${runtime_directory}/.git" ]; then
  git init "${runtime_directory}"
  cp -R "${seed_directory}/." "${runtime_directory}/"
  cd "${runtime_directory}"
  git config user.name "Tatnez Wiki Seed"
  git config user.email "wiki@example.invalid"
  git add .
  git commit -m "Initial wiki seed"
fi

exec gollum "${runtime_directory}" --host 0.0.0.0 --port 4567 --allow-uploads dir
