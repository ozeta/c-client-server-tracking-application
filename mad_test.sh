#!/bin/bash

for i in {1..2}
do
	xfce4-terminal -e "./client 127.0.0.1 44444"
done