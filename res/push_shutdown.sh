#!/bin/bash
gpio -g mode 26 in
gpio -g mode 26 up
while :
do
  sleep .2
  if gpio -g read 26 | grep -q 0; then
    echo  "Button pushed, the system will shutdown!"
    sudo shutdown now
  fi
done
