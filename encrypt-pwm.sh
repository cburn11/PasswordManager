#!/bin/bash

# pwm file format:

# 32 byte sha256 hash of plaintext file
# 16 byte iv used in encryption
# 49+ byte aes256 cyphertext (plaintext is an xml file if it came from PasswordManager.exe)

file_in=$1
file_out=$2
pass_phrase=$3

iv_file=iv
hash_file=hash
cypher_file=cypher

openssl rand 16 > $iv_file
iv=$(xxd -p $iv_file)

openssl dgst -sha256 -binary $file_in > $hash_file
hash=$(xxd -p -c 32 $hash_file)

key=$(echo -n "$pass_phrase " | sed 's! !\x00!' | iconv -f utf-8 -t utf-16le | openssl dgst -sha256 | cut -d ' ' -f 2)

openssl enc -e -aes256 -iv $iv -K $key -in $file_in -out $cypher_file

cat $hash_file $iv_file $cypher_file > $file_out

rm $iv_file $hash_file $cypher_file
