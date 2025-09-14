#!/bin/bash

make
echo "Compilation complete"

p=$(pwd);
echo $p

mkdir -p ~/.local/share/ctask_manager
echo "/home/USER/.local/share/ctask_manager directory succesfully created"

touch ~/.local/share/ctask_manager/tasks.json
chmod 763 ~/.local/share/ctask_manager/tasks.json
echo "tasks.json file succesfully created"

sudo ln -s $p/bin/task /usr/local/bin/ctask_man
echo "succesfully created symlink"

echo "Now use ctask_man usage to get started"
