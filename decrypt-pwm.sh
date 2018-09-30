#!/bin/bash

# pwm file format:

# 32 byte sha256 hash of plaintext file
# 16 byte iv used in encryption
# 49+ byte aes256 cyphertext (plaintext is an xml file if it came from PasswordManager.exe)

file_in=$1
file_out=$2
pass_phrase=$3

key=$(echo -n "$pass_phrase " | sed 's! !\x00!g' | iconv -f utf-8 -t utf-16le | openssl dgst -sha256 | cut -d ' ' -f 2)

temp_file=temp
iv_file=iv
hash_file=hash
cypher_file=cypher

head -c 48 $file_in > $temp_file

head -c 32 $temp_file > $hash_file
tail -c 16 $temp_file > $iv_file

tail -c +49 $file_in > $cypher_file

iv=$(xxd -p $iv_file)
hash=$(xxd -p -c 32 $hash_file)

openssl enc -d -aes256 -iv $iv -K $key -in $cypher_file -out $file_out

key=""

hash_calc=$(openssl dgst -sha256 $file_out | cut -d ' ' -f 2)

if [ $hash = $hash_calc ]; then
    echo "Successful decrypt of $file_in"
else
    echo "Decrypt failed."
fi

rm $temp_file $iv_file $hash_file $cypher_file


