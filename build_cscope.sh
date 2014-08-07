#!/bin/bash

for i in h cpp c;do find ./source/ ../include/ -name "*.$i";done > cscope.files 
ctags -L cscope.files
echo -ne "\033]0;cscope    ---    $(basename $(pwd)) \007"
cscope -C -i cscope.files
