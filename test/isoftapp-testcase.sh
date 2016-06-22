#!/bin/bash 

echo '!!!Please sudo systemctl stop isoftapp-daemon.service and run sudo gdb isoftapp-daemon at first!!!'

echo 'Remove /var/cache/isoftapp'
rm -rf /var/cache/isoftapp

echo 'isoftapp search'
isoftapp search qw
isoftapp search auda
isoftapp search emaces
isoftapp search wx

echo 'isoftapp remove'
isoftapp remove qwx
isoftapp remove audacity
isoftapp remove emaces

echo 'isoftapp install'
isoftapp install qwx
isoftapp install audacity
isoftapp install emaces
