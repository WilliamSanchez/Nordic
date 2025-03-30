#!/bin/bash

CRC_POLY=A001

## Decimal to hexadecimal
## var=$(echo "obase=16;$numero" | bc)

dec=16#C181

function checksum()
{
   local data=$1
  
   var=$(echo -n $data | xxd -p)
   echo "$data hex $var"
	
   local x=0
   local aux=0
   while [ $x -lt $2 ]
   do
      char=$(echo -n ${data:x:1} | xxd -p)
      dec1=$((16#$(echo -n ${data:x:1} | xxd -p -c 1)))
      dec=$((dec ^ dec1))
      echo "-$x = $char, $dec"
      
      local y=0
      while [ $y -lt 8 ]
      do
           aux=$dec
           echo "-$y = ${data:x:1}, $dec"
           dec=$((dec>>1))
           if [ $((aux & 1)) == 1 ]; then
               dec=$((dec ^ 16#$CRC_POLY))
           fi           
           y=$((y+1))
      done
      
      #echo "-$x = $char, $dec"
      x=$((x+1))
   done
}

text="Hello"
checksum $text ${#text}
v=$(echo "obase=16;$dec" | bc)
echo "CRC = $v"
